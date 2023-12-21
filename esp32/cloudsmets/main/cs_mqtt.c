/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * MQTT communication with the Azure IotHub.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>

/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_crt_bundle.h"
#include "mqtt_client.h"
#include "mbedtls/md.h"
#include "mbedtls/base64.h"

#include "cs_cfg.h"
#include "cs_time.h"
#include "cs_mqtt.h"

/* Extension definitions to make coding simpler. */
#include "mbedtls_err.h"
#include "esp_mqtt_err.h"

/* Some helpful macros. */
#define MAYBE_FREE(X)       if ((X) != NULL) { free(X); (X) = NULL; }

#define MALLOC_ERROR_CHECK(DST, LEN)                                        \
    (DST) = malloc(LEN);                                                    \
    if ((DST) == NULL)                                                      \
    {                                                                       \
        ESP_LOGE(TAG, "malloc failed for length: %d", (LEN));               \
        ESP_ERROR_CHECK(ESP_FAIL);                                          \
    }

/**
 * If we think we can reconnect immediately using a different access key,
 * we try.  Otherwise we wait for a minute.
*/
#define RECONNECT_WAIT      ((uint64_t)1 * 60 * 1000 * 1000)
#define RECONNECT_NOW       ((uint64_t)1)

#define MQTT_TIMER          "MQTT"

/* SAS token strings. */
#define SAS_TOKEN_START         "SharedAccessSignature sr="
#define SAS_TOKEN_START_LEN     (sizeof(SAS_TOKEN_START) - 1)
#define SAS_TOKEN_AND_SIG       "&sig="
#define SAS_TOKEN_AND_SIG_LEN   (sizeof(SAS_TOKEN_AND_SIG) - 1)
#define SAS_TOKEN_AND_SE        "&se="
#define SAS_TOKEN_AND_SE_LEN    (sizeof(SAS_TOKEN_AND_SIG) - 1)

/**
 * A lot of the SAS token operations require a string-with-length so this simple
 * structure tracks this.
*/
typedef struct
{
    unsigned char *value;
    size_t length;
} cs_string;

#define TAG cs_mqtt_task_name

/* Escape a string such that it can be used in a URL query. */
static const unsigned char url_quote_plus_esc_chars[] = " <>#%+{}|\\^~[]`;/?:@=&$";
static const unsigned char url_quote_plus_hex_map[] = "0123456789ABCDEF";

static esp_mqtt_client_handle_t esp_mqtt_client_handle = NULL;

static esp_timer_handle_t reconnect_timer_handle = NULL;

static cs_string access_keys[2] = {{NULL, 0}, {NULL, 0}};
static cs_string iothub_url = {NULL, 0};
static cs_string quoted_iothub_url = {NULL, 0};
static cs_string sign_key = {NULL, 0};
static cs_string device = {NULL, 0};
static cs_string sas_token = {NULL, 0};
static int current_access_key = 0;
static int stale_access_keys = 0;

/* We cannot authenticate until we know the correct time or WiFi connected. */
bool time_is_set = false;
bool wifi_ap_connected = false;

static void HMAC_digest(const cs_string *key, const cs_string *signed_key, cs_string *hmac)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    /**
     * Perform the HMAC calculation by leveraging code intended for TLS usage.
    */
    mbedtls_md_init(&ctx);
    MBEDTLS_ERROR_CHECK(mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1));
    MBEDTLS_ERROR_CHECK(mbedtls_md_hmac_starts(&ctx, key->value, key->length));
    MBEDTLS_ERROR_CHECK(mbedtls_md_hmac_update(&ctx, signed_key->value, signed_key->length));
    MBEDTLS_ERROR_CHECK(mbedtls_md_hmac_finish(&ctx, hmac->value));
    mbedtls_md_free(&ctx);
}

static void url_quote_plus(cs_string *in_url, cs_string *out_url)
{
    unsigned char *sptr = in_url->value;
    unsigned char *tptr;
    size_t in_len = in_url->length;
    size_t out_len = in_url->length;
    size_t ii;

    for (ii = 0; ii < in_len; ii++)
    {
        if (NULL != memchr(url_quote_plus_esc_chars, (int)(*sptr++), sizeof(url_quote_plus_esc_chars)))
        {
            // We will have to escape this character.
            out_len += 2;
        }
    }

    out_url->length = out_len;
    out_url->value = (unsigned char *)malloc(out_len);
    if (out_url->value == NULL)
    {
        ESP_LOGE(TAG, "Cannot malloc out_url: %d", out_len);
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    sptr = in_url->value;
    tptr = out_url->value;
    for (ii = 0; ii < in_len; ii++)
    {
        if (NULL != memchr(url_quote_plus_esc_chars, (int)(*sptr), sizeof(url_quote_plus_esc_chars)))
        {
            // We will have to escape this character.
            *tptr++ = '%';
            *tptr++ = url_quote_plus_hex_map[*sptr >> 4];
            *tptr++ = url_quote_plus_hex_map[*sptr & 0x0F];
            sptr++;
        }
        else
        {
            // Not escaped.
            *tptr++ = *sptr++;
        }
    }
    return;
}

/**
 * Note that at present all the inputs to this are global variables or fixed.
*/
static void generate_sas_token()
{
    cs_string sign_key = { NULL, 0 };
    size_t b64_length;
    unsigned char hmac_sig_value[32];
    cs_string hmac_sig = {hmac_sig_value, 32};
    time_t ttl = time(NULL) + 3600;
    unsigned char *sptr;

    /* Access keys are Base64 decoded when read from configuration. */

    /* The IotHub URL is escaped when read from configuration. */

    /**
     * The sign-key is the escaped URL concatenated with the a newline and the
     * TTL.  We prealllocate this, with the escaped URL newline in place
     * and just copy the TTL in here.
     */
    sign_key.length = quoted_iothub_url.length + 1;
    utoa(ttl, (char *)&sign_key.value[sign_key.length], 10);
    sign_key.length += strlen((char *)&sign_key.value[sign_key.length]);

    /* The SHA256 digest of an HMAC is a 32 byte string. */
    HMAC_digest(&access_keys[current_access_key], &sign_key, &hmac_sig);

    /**
     * The base64 encoding of a 32 byte string is 44 bytes and we can write
     * this straight into the SAS token.
     */
    sptr = sas_token.value;
    sptr += 34 + iothub_url.length + 5;
    MBEDTLS_ERROR_CHECK(mbedtls_base64_encode(sptr, 44, &b64_length, hmac_sig.value, 32));
    sptr += 44;
    memcpy(sptr, SAS_TOKEN_AND_SE, SAS_TOKEN_AND_SE_LEN);
    sptr += 4;

    /* Finally add the TTL, which will also add the NULL for us! */
    utoa(ttl, (char *)sptr, 10);
}

// TODO: Anyway to avoid global variables?
static void read_config()
{
    uint8_t enabled;
    unsigned char *sptr;

    /* Free existing values and read new ones. */
    MAYBE_FREE(iothub_url.value);
    MAYBE_FREE(quoted_iothub_url.value);
    MAYBE_FREE(sign_key.value);
    MAYBE_FREE(device.value);
    MAYBE_FREE(access_keys[0].value);
    MAYBE_FREE(access_keys[1].value);
    MAYBE_FREE(sas_token.value);

    /**
     * Note that the length of sread strings includes the NULL terminator so
     * we reduce the length to "remove" this.
     */
    cs_cfg_read_uint8(CS_CFG_NMSP_MQTT, CS_CFG_KEY_AZURE_ENA, &enabled);
    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_AZURE_IOTHUB, (char **)&iothub_url.value, &iothub_url.length);
    iothub_url.length--;
    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_AZURE_DEVICE, (char **)&device.value, &device.length);
    device.length--;

    /**
     * The access keys are Base64 encoded and the first thing we always do is
     * decode them so do that here.
     * Base64 encoding is always 4 bytes for every 3 so the decoded string must
     * fit in the string we start with.
     */
    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_AZURE_KEY1, (char **)&access_keys[0].value, &access_keys[0].length);
    access_keys[0].length--;
    MBEDTLS_ERROR_CHECK(mbedtls_base64_decode(access_keys[0].value, access_keys[0].length, &access_keys[0].length, access_keys[0].value, access_keys[0].length));
    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_AZURE_KEY2, (char **)&access_keys[1].value, &access_keys[1].length);
    access_keys[0].length--;
    MBEDTLS_ERROR_CHECK(mbedtls_base64_decode(access_keys[1].value, access_keys[1].length, &access_keys[1].length, access_keys[1].value, access_keys[1].length));

    /* At various times we need the URL in escaped form so do this now. */
    url_quote_plus(&iothub_url, &quoted_iothub_url);

    /**
     * The final shared access signature (SAS) token use for authentication looks
     * like this:
     *
     * "SharedAccessSignature sr=<uri>&sig=<signature>&se=<ttl>"
     *
     * where:
     * - <uri> is the IotHub URI and the lenght is known here
     * - <signature> is a Base64 encoding of a 32 byte string which must have
     *   length 44
     * - <ttl> is a string encoding of a time_t which has maximum value 0xFFFFFFFF
     *   or base10 value 4,294,967,295 so must have length <= 10.
     *
     * This means we know how long to allocate the SAS token for and we can
     * preload some of the information.  Allow an extra byte for the NULL
     * terminator that we need to add before we pass it to the MQTT library.
    */
    sas_token.length = 34 + iothub_url.length + 44 + 10 + 1;
    MALLOC_ERROR_CHECK(sas_token.value, sas_token.length);

    /* Now we preload as much as we can into the token. */
    sptr = sas_token.value;

    memcpy(sptr, SAS_TOKEN_START, SAS_TOKEN_START_LEN);
    sptr += 25;
    memcpy(sptr, iothub_url.value, iothub_url.length);
    sptr += iothub_url.length;
    memcpy(sptr, SAS_TOKEN_AND_SIG, SAS_TOKEN_AND_SIG_LEN);

    /**
     * We will require a signing key, which is the quoted URL plus a newline
     * plus a stringified time.
    */
    sign_key.length = quoted_iothub_url.length + 1 + 11;
    MALLOC_ERROR_CHECK(sign_key.value, sign_key.length);
}

/**
 * Determine whether we can now start MQTT.
*/
static void maybe_start_mqtt()
{
    ESP_LOGV(TAG, "MQTT capable?: %d, %d", (int)time_is_set, (int)wifi_ap_connected);
    if ((time_is_set) && (wifi_ap_connected))
    {
        /* We can try and connect MQTT. */
        ESP_LOGI(TAG, "Start MQTT");
        ESP_ERROR_CHECK(esp_mqtt_client_start(esp_mqtt_client_handle));
    }
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_CONNECTED:
                /**
                 * We are connected to the AP (router) so we can try and
                 * connect to the Azure IoTHub using MQTT.
                */
                ESP_LOGI(TAG, "Connect to IoTHub");
                wifi_ap_connected = true;
                maybe_start_mqtt();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                /**
                 * AP disconnected to MQTT cannot work.
                 * Ignore errors if we were never connected.
                */
                ESP_LOGI(TAG, "WiFi AP lost - disconnect");
                wifi_ap_connected = false;
                ESP_MQTT_ERROR_CHECK(esp_mqtt_client_stop(esp_mqtt_client_handle));
                break;

            default:
                ESP_LOGI(TAG, "Other WiFi event: %d", event_id);
                break;
        }
    }
    else if (event_base == CS_CONFIG_EVENT)
    {
        // Config change so relearn all configuration.
        read_config();
        maybe_start_mqtt();
    }
    else if (event_base == CS_TIME_EVENT)
    {
        ESP_LOGI(TAG, "Time is now set");
        time_is_set = true;
        maybe_start_mqtt();
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event_base: %s", event_base);
    }
}

static void reconnect_timer_cb(void *args)
{
    /* Attempt to reconnect. */
    ESP_ERROR_CHECK(esp_mqtt_client_reconnect(esp_mqtt_client_handle));
}

static void maybe_stale_access_key()
{
    uint64_t reconnect_wait;
    /**
     * We cannot immediately connect here as we are inside the context of the
     * MQTT event loop so we set a timer that is either short, if we think we
     * might be able to use the access key, or longer if we think both keys
     * might be stale.
    */
    reconnect_wait = RECONNECT_WAIT;
    current_access_key = (1 - current_access_key);
    if (!stale_access_keys)
    {
        stale_access_keys++;
        reconnect_wait = RECONNECT_NOW;
    }
    esp_timer_stop(reconnect_timer_handle);
    ESP_ERROR_CHECK(esp_timer_start_once(reconnect_timer_handle, reconnect_wait));
}

void set_mqtt_config(esp_mqtt_client_handle_t client)
{
    // Note that we only need to change the password here.
    const esp_mqtt_client_config_t mqtt_cfg = {
        .credentials.authentication.password = (char *)sas_token.value
    };
    ESP_LOGD(TAG, "Update MQTT config");
    esp_mqtt_set_config(client, &mqtt_cfg);
}

static void mqtt_event_handler_cb(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    // your_context_t *context = event->context;
    switch (event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            generate_sas_token();
            set_mqtt_config(client);
            break;

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            stale_access_keys = 0;
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            /**
             * The commonest disconnection reason is that the SAS token, which
             * has a time element to it, has expired.  Note we just use the
             * authentication failure callback.
            */
            reconnect_timer_cb(NULL);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGD(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            switch (event->error_handle->error_type)
            {
                case MQTT_ERROR_TYPE_ESP_TLS:
                    ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                    ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                    break;

                case MQTT_ERROR_TYPE_CONNECTION_REFUSED:
                    /**
                     * Some errors, like bad username, cannot be recovered from
                     * but we can retry some.
                    */
                    ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
                    switch (event->error_handle->connect_return_code)
                    {
                        case MQTT_CONNECTION_REFUSE_SERVER_UNAVAILABLE:
                            /**
                             * We will retry this later.
                            */
                            ESP_LOGE(TAG, "MQTT server unavailable.");
                            esp_timer_stop(reconnect_timer_handle);
                            ESP_ERROR_CHECK(esp_timer_start_once(reconnect_timer_handle, RECONNECT_WAIT));
                            break;

                        case MQTT_CONNECTION_REFUSE_PROTOCOL:
                            ESP_LOGE(TAG, "MQTT protocol refused.");
                            break;

                        case MQTT_CONNECTION_REFUSE_ID_REJECTED:
                            ESP_LOGE(TAG, "ID rejected.");
                            break;

                        case MQTT_CONNECTION_REFUSE_BAD_USERNAME:
                            ESP_LOGE(TAG, "Username rejected.");
                            break;

                        case MQTT_CONNECTION_REFUSE_NOT_AUTHORIZED:
                            /**
                             * This implies that perhaps the access key has
                             * become stale.  Note that in web server, keys are
                             * 1-indexed.
                            */
                            ESP_LOGW(TAG, "Access key %d may be invalid.", (current_access_key + 1));
                            maybe_stale_access_key();
                            break;

                        default:
                            ESP_LOGE(TAG, "Unknown connection error.");
                            break;
                    }
                    break;

                default:
                    ESP_LOGE(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
                    break;
            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

static void init_mqtt(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = (char *)(iothub_url.value),
            .verification.crt_bundle_attach = esp_crt_bundle_attach
        },
    };

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle = esp_mqtt_client_init(&mqtt_cfg);
    if (esp_mqtt_client_handle == NULL)
    {
        ESP_LOGE(TAG, "Unable to create MQTT handle");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    esp_mqtt_client_register_event(esp_mqtt_client_handle, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);
    esp_mqtt_client_start(esp_mqtt_client_handle);
}

void cs_mqtt_task(cs_mqtt_create_parms_t *create_parms)
{
    // TODO: Remove this or perhaps replace with config?
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "Init. MQTT task");

    ESP_LOGI(TAG, "Register event handlers");

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    CS_CONFIG_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    CS_TIME_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));

    /* Read initial configuration. */
    read_config();

    /**
     * Configure the reconnect timer.
    */
    esp_timer_create_args_t create_args =
    {
        .callback = reconnect_timer_cb,
        .name = MQTT_TIMER
    };
    ESP_ERROR_CHECK(esp_timer_create(&create_args, &reconnect_timer_handle));

    /**
     * Prepare MQTT
    */
   init_mqtt();

    /**
     * We now have to wait for both the time to be zet (via ZigBee) and
     * a WiFi connection to the AP (router).
    */
}