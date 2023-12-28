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
#include "cs_string.h"
#include "cs_mqtt.h"

/* Extension definitions to make coding simpler. */
#include "mbedtls_err.h"
#include "esp_mqtt_err.h"

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
#define SAS_TOKEN_AND_SE_LEN    (sizeof(SAS_TOKEN_AND_SE) - 1)


#define TAG cs_mqtt_task_name


static esp_mqtt_client_handle_t esp_mqtt_client_handle = NULL;

static esp_timer_handle_t reconnect_timer_handle = NULL;

static esp_event_loop_handle_t mqtt_event_loop_handle = NULL;
static esp_event_loop_handle_t flash_event_loop_handle = NULL;

uint8_t enabled = 0;
// TODO: Proper module and APIs for cs_string.  Alternative in ESP already?
static cs_string access_keys[2] = {{NULL, 0}, {NULL, 0}};
static cs_string iothub_url = {NULL, 0};
static cs_string quoted_device_url = {NULL, 0};
static cs_string sign_key = {NULL, 0};
static cs_string device = {NULL, 0};
static cs_string username = {NULL, 0};
static cs_string sas_token = {NULL, 0};
static int current_access_key = 0;
static int stale_access_keys = 0;

/* We cannot authenticate until we know the correct time or WiFi connected. */
bool time_is_set = false;
bool wifi_ap_connected = false;

// TODO: move prototypes here.
static void init_mqtt(void);

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

/* Escape a string such that it can be used in a URL query. */
static const unsigned char quote_plus_chars[] = "/?#[]@!$&'()*+,;=<>#%%{}|\\^`";
static const unsigned char hex_map[] = "0123456789ABCDEF";

static void quote_plus(cs_string *in_str, cs_string *out_str)
{
    unsigned char *sptr = in_str->value;
    unsigned char *tptr;
    size_t in_len = in_str->length;
    size_t out_len = in_str->length;
    size_t ii;

    ESP_LOGV(TAG, "in_str: %d, %s", in_str->length, in_str->value);

    for (ii = 0; ii < in_len; ii++)
    {
        if (NULL != memchr(quote_plus_chars, (int)(*sptr++), sizeof(quote_plus_chars) - 1))
        {
            // We will have to escape this character.
            out_len += 2;
        }
    }

    out_str->length = out_len;
    if (out_str->length > 0)
    {
        CS_STRING_MALLOC(out_str, (out_len + 1));
    }
    ESP_LOGV(TAG, "out_str len: %u, %p", out_str->length, out_str->value);

    sptr = in_str->value;
    tptr = out_str->value;

    for (ii = 0; ii < in_len; ii++)
    {
        if (NULL != memchr(quote_plus_chars, (int)(*sptr), sizeof(quote_plus_chars) - 1))
        {
            // We will have to escape this character.
            *tptr++ = '%';
            *tptr++ = hex_map[*sptr >> 4];
            *tptr = hex_map[*sptr & 0x0F];
        }
        else if ((*sptr) == ' ')
        {
            *tptr = '+';
        }
        else
        {
            // Not escaped.
            *tptr = *sptr;
        }
        sptr++;
        tptr++;
    }
    *tptr = 0;
    ESP_LOGV(TAG, "out_str: %u, %p, %s", out_str->length, tptr, out_str->value);
    return;
}

/**
 * Note that at present all the inputs to this are global variables or fixed.
*/

/* SHA256 hash is a 32 byte value. */
#define SHA256_LEN 32

/**
 * A 32 byte value can be Base64 encoded into 44 bytes but URL-encoding might
 * have to encode any of these and URL-encoding converts a single byte into
 * 3 bytes ('%hh').  So the URL-encoded length has to be at least 3 x 44 + 1
 * for the NULL terminator = 133 bytes.
 */
#define BASE64_SHA256_LEN 44
#define URLENCODED_BASE64_SHA256_LEN 133

static void generate_sas_token()
{
    size_t b64_length;
    unsigned char hmac_sig_value[SHA256_LEN];
    cs_string hmac_sig = {hmac_sig_value, SHA256_LEN};
    unsigned char b64_hmac_sig_value[URLENCODED_BASE64_SHA256_LEN + 1];
    cs_string b64_hmac_sig = {b64_hmac_sig_value, URLENCODED_BASE64_SHA256_LEN};
    // DEBUG: TODO: time_t ttl = time(NULL) + 3600;
    time_t ttl = 1703596659 + 3600;
    unsigned char *sptr;
    int offset;
    cs_string temp_str = {NULL, 0};

    ESP_LOGV(TAG, "Generate SAS token");
    ESP_LOGV(TAG, "SAS 1...");

    /**
     * The sign-key is the escaped URL concatenated with the a newline and the
     * TTL.  We prealllocated this and loaded the URL and newline when we read
     * the configuration.
     */
    ESP_LOGV(TAG, "Debug: %u, %u", quoted_device_url.length, sign_key.length);
    offset = quoted_device_url.length + 1;
    utoa(ttl, (char *)&sign_key.value[offset], 10);
    sign_key.length = strlen((char *)sign_key.value);
    ESP_LOGV(TAG, "sign_key: %s", sign_key.value);

    /* The SHA256 digest of an HMAC is a 32 byte string. */
    HMAC_digest(&access_keys[current_access_key], &sign_key, &hmac_sig);

    /**
     * The BASE64 encoding will need to be escaped before we write it to the
     * SAS token so encoded to a temporary string first.
    */
    MBEDTLS_ERROR_CHECK(mbedtls_base64_encode(b64_hmac_sig_value, (BASE64_SHA256_LEN  +1), &b64_length, hmac_sig.value, 32));
    b64_hmac_sig.length = BASE64_SHA256_LEN;
    ESP_LOGV(TAG, "HMAC(): %s", b64_hmac_sig.value);

    quote_plus(&b64_hmac_sig, &temp_str);
    ESP_LOGV(TAG, "URL(HMAC()): %s", temp_str.value);

    sptr = sas_token.value;
    sptr += SAS_TOKEN_START_LEN + quoted_device_url.length + SAS_TOKEN_AND_SIG_LEN;
    memcpy(sptr, temp_str.value, temp_str.length);
    sptr += temp_str.length;
    memcpy(sptr, SAS_TOKEN_AND_SE, SAS_TOKEN_AND_SE_LEN);
    sptr += SAS_TOKEN_AND_SE_LEN;
    ESP_LOGV(TAG, "SAS 4...");

    /* Finally add the TTL, which will also add the NULL for us! */
    utoa(ttl, (char *)sptr, 10);
    ESP_LOGV(TAG, "SAS token: %s", sas_token.value);
    sas_token.length = strlen((char *)sas_token.value);
}

// TODO: Anyway to avoid global variables?
static void read_config()
{
    cs_string temp_url = {NULL, 0};

    unsigned char *sptr;

    /* Free existing values and read new ones. */
    CS_STRING_FREE(&iothub_url);
    CS_STRING_FREE(&quoted_device_url);
    CS_STRING_FREE(&sign_key);
    CS_STRING_FREE(&device);
    CS_STRING_FREE(&username);
    CS_STRING_FREE(&access_keys[0]);
    CS_STRING_FREE(&access_keys[1]);
    CS_STRING_FREE(&sas_token);

    /**
     * Note that the length of read strings includes the NULL terminator so
     * we reduce the length to "remove" this because the SAS token generation
     * does not want NULLs.
     */
    cs_cfg_read_uint8(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_ENA, &enabled);

    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_IOTHUB, (char **)&iothub_url.value, &iothub_url.length);
    iothub_url.length--;
    ESP_LOGV(TAG, "IH: %d", iothub_url.length);

    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_DEVICE, (char **)&device.value, &device.length);
    device.length--;
    ESP_LOGV(TAG, "D: %d", device.length);

    /**
     * The access keys are Base64 encoded and the first thing we always do is
     * decode them so do that here.
     * Base64 encoding is always 4 bytes for every 3 so the decoded string must
     * fit in the string we start with.
     */
    // TODO: Does the decode output include a NULL or not?
    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_KEY1, (char **)&access_keys[0].value, &access_keys[0].length);
    access_keys[0].length--;
    MBEDTLS_ERROR_CHECK(mbedtls_base64_decode(access_keys[0].value, access_keys[0].length, &access_keys[0].length, access_keys[0].value, access_keys[0].length));
    ESP_LOGV(TAG, "K1: %d", access_keys[0].length);

    cs_cfg_read_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_KEY2, (char **)&access_keys[1].value, &access_keys[1].length);
    access_keys[1].length--;
    MBEDTLS_ERROR_CHECK(mbedtls_base64_decode(access_keys[1].value, access_keys[1].length, &access_keys[1].length, access_keys[1].value, access_keys[1].length));
    ESP_LOGV(TAG, "K2: %d", access_keys[1].length);

    /**
     * We will now try and build some of the SAS token components, but only if
     * we have the components.
    */
#define PROTOCOL_MQTTS          "mqtts://"
#define PROTOCOL_MQTTS_LENGTH   (sizeof(PROTOCOL_MQTTS) - 1)
    if ((iothub_url.length > PROTOCOL_MQTTS_LENGTH) &&
        (device.length > 0))
    {
        /**
         * Username is "<iothub-url-without-protocol>/devicename"
        */
        username.length = iothub_url.length - PROTOCOL_MQTTS_LENGTH + 1+ device.length + 1;
        CS_STRING_MALLOC(&username, username.length);
        sptr = username.value;
        memcpy(sptr, &iothub_url.value[PROTOCOL_MQTTS_LENGTH], (iothub_url.length - PROTOCOL_MQTTS_LENGTH));
        sptr += iothub_url.length - PROTOCOL_MQTTS_LENGTH;
        *sptr++ = '/';
        memcpy(sptr, device.value, device.length);
        sptr += device.length;
        *sptr = 0;
        /**
         * At various times we need the ID of the device which is...
         *   "<IoTHub-URL-without-mqtt//>/Devices/<device-id>"
         * ...and then quote_plus encode this.
        */
#define DEVICES "/Devices/"
#define DEVICES_LEN (sizeof(DEVICES) - 1)
        temp_url.length = iothub_url.length - PROTOCOL_MQTTS_LENGTH + DEVICES_LEN + device.length + 1;
        CS_STRING_MALLOC(&temp_url, temp_url.length);
        sptr = temp_url.value;
        memcpy(sptr, &iothub_url.value[PROTOCOL_MQTTS_LENGTH], (iothub_url.length - PROTOCOL_MQTTS_LENGTH));
        sptr += iothub_url.length - PROTOCOL_MQTTS_LENGTH;
        memcpy(sptr, DEVICES, DEVICES_LEN);
        sptr += DEVICES_LEN;
        memcpy(sptr, device.value, device.length);
        sptr[device.length] = 0;
        temp_url.length--;

        quote_plus(&temp_url, &quoted_device_url);

        /**
         * The final shared access signature (SAS) token use for authentication looks
         * like this:
         *
         * "SharedAccessSignature sr=<uri>&sig=<signature>&se=<ttl>"
         *
         * where:
         * - <uri> is the IotHub URI and the length is known here
         * - <signature> is a URL encoded Base64 encoding of a 32 byte string
         *   which must have length URLENCODED_BASE64_SHA256_LEN (46)
         * - <ttl> is a string encoding of a time_t which has maximum value 0xFFFFFFFF
         *   or base10 value 4,294,967,295 so must have length <= 10.
         *
         * This means we know how long to allocate the SAS token for and we can
         * preload some of the information.  Allow an extra byte for the NULL
         * terminator that we need to add before we pass it to the MQTT library.
        */
        sas_token.length = SAS_TOKEN_START_LEN + quoted_device_url.length + SAS_TOKEN_AND_SIG_LEN + URLENCODED_BASE64_SHA256_LEN + 10 + 1;
        ESP_LOGV(TAG, "ST: %u", sas_token.length);
        CS_STRING_MALLOC(&sas_token, sas_token.length);

        /* Now we preload as much as we can into the token. */
        sptr = sas_token.value;
        ESP_LOGV(TAG, "Debug 1");
        memcpy(sptr, SAS_TOKEN_START, SAS_TOKEN_START_LEN);
        ESP_LOGV(TAG, "Debug 2");
        sptr += SAS_TOKEN_START_LEN;
        memcpy(sptr, quoted_device_url.value, quoted_device_url.length);
        ESP_LOGV(TAG, "Debug 3");
        sptr += quoted_device_url.length;
        memcpy(sptr, SAS_TOKEN_AND_SIG, SAS_TOKEN_AND_SIG_LEN);
        sptr += SAS_TOKEN_AND_SIG_LEN;
        *sptr = 0;
        ESP_LOGV(TAG, "Partial SAS Token: %s", sas_token.value);

        /**
         * We will require a signing key, which is the quoted URL plus a newline
         * plus a stringified time.
        */
        sign_key.length = quoted_device_url.length + 1 + 11;
        CS_STRING_MALLOC(&sign_key, sign_key.length);
        sptr = sign_key.value;
        memcpy(sptr, quoted_device_url.value, quoted_device_url.length);
        sptr += quoted_device_url.length;
        *sptr++ = '\n';
        *sptr = 0;
        ESP_LOGV(TAG, "Partial sign-key: %s", sign_key.value);
    }
}

/**
 * Determine whether we can now start MQTT.
*/
static void maybe_start_mqtt()
{
    ESP_LOGV(TAG, "MQTT capable?: %d, %d", (int)time_is_set, (int)wifi_ap_connected);
    if ((time_is_set) && (wifi_ap_connected))
    {
        /* We could try and connect MQTT if we are configured. */
        ESP_LOGV(TAG, "CloudSMETS is ready...");
        if ((enabled) &&
            (iothub_url.length != 0) &&
            (device.length != 0) &&
            (access_keys[0].length != 0) &&
            (access_keys[1].length != 0))
        {
            ESP_LOGI(TAG, "Start MQTT");

            /**
             * The URL might have changed so reload it.
            */
            // TODO: Don't init like this.
            init_mqtt();

            // TODO: Logic is wrong and we start the client a second time, which fails.
            ESP_ERROR_CHECK(esp_mqtt_client_start(esp_mqtt_client_handle));
        }
    }
}

/**
 * Forward all events to the default event handler to this tasks event handler
 * but ignore the event data since we won't use it.
*/
static void default_event_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data)
{
    esp_event_post_to(mqtt_event_loop_handle, event_base, event_id, NULL, 0, 0);
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

void set_mqtt_config(esp_mqtt_client_handle_t client)
{
    // Note that we only need to change the password here.
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = (char *)(iothub_url.value),
            .verification.crt_bundle_attach = esp_crt_bundle_attach
        },
        // TODO: Needs reworking.  .session.protocol_ver = MQTT_PROTOCOL_V_5,
        // Ref: https://learn.microsoft.com/en-us/azure/iot/iot-mqtt-5-preview
        .credentials = {
            .username = (char *)username.value,
            .client_id = (char *)device.value,
            .authentication.password = (char *)sas_token.value
        },
        .network.reconnect_timeout_ms = 60 * 1000
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
            esp_event_post_to(flash_event_loop_handle, event_base, event_id, NULL, 0, 10);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            /**
             * MQTT client library will try to reconnect in configured time
             * and will generate a new SAS.
            */
            esp_event_post_to(flash_event_loop_handle, event_base, event_id, NULL, 0, 10);
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
                            current_access_key = (1 - current_access_key);
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
    // TODO: We should not allow this if there is no config and we need to recreate
    //       or change config on config changes.
    if (enabled)
    {
        if (esp_mqtt_client_handle == NULL)
        {
            const esp_mqtt_client_config_t mqtt_cfg = {
                .broker = {
                    .address.uri = (char *)(iothub_url.value),
                    .verification.crt_bundle_attach = esp_crt_bundle_attach
                },
                // TODO: Needs reworking for V5. .session.protocol_ver = MQTT_PROTOCOL_V_5,
                .credentials = {
                    .username = (char *)username.value,
                    .client_id = (char *)device.value
                },
                .network.reconnect_timeout_ms = 60 * 1000
            };

            esp_mqtt_client_handle = esp_mqtt_client_init(&mqtt_cfg);
            if (esp_mqtt_client_handle == NULL)
            {
                ESP_LOGE(TAG, "Unable to create MQTT handle");
                ESP_ERROR_CHECK(ESP_FAIL);
            }

            esp_mqtt_client_register_event(esp_mqtt_client_handle, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);
            // esp_mqtt_client_start(esp_mqtt_client_handle);
        }
    }
}

void cs_mqtt_task(cs_mqtt_create_parms_t *create_parms)
{
    // TODO: Remove this or perhaps replace with config?
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "Init. MQTT task");
    mqtt_event_loop_handle = create_parms->mqtt_event_loop_handle;

    ESP_LOGI(TAG, "Register event handlers");

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    default_event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    mqtt_event_loop_handle,
                    WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    mqtt_event_loop_handle,
                    CS_CONFIG_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    mqtt_event_loop_handle,
                    CS_TIME_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));

    /* Read initial configuration. */
    read_config();

    /**
     * Prepare MQTT
    */
   init_mqtt();

    /**
     * We now have to wait for both the time to be zet (via ZigBee) and
     * a WiFi connection to the AP (router).
    */
}