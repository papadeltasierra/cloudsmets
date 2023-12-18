/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Application mainline and heartbeat/watchdog.
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_event.h"
#include "mbedtls/md.h"

// #include "ft_err.h"
#include "pd_err.h"


static const char *cs_mqtt_task = "MQTT";
#define TAG cs_mqtt_task

static esp_mqtt_client_handle_t esp_mqtt_client_handle = NULL;





static bool HMAC_digest(const unsigned char *key, const unsigned char *signed_key, char *hmac)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    const size_t key_length = strlen(len);
    const size_t signed_key_length = strlen(signed_key);
    int mbedtls_rc;

    /**
     * Perform the HMAC calculation by leveraging code intended for TLS usage.
    */
    mbedtls_md_init(&ctx);
    mbedtls_rc = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    if (mbedtls_rc != 0)
    {
        ESP_LOGE(TAG, "mbedtls_md_setup() failed: %d", mbedtls_rc);
        return;
    }
    mbedtls_rc = mbedtls_md_hmac_starts(&ctx, key, keyLength);
    if (mbedtls_rc != 0)
    {
        ESP_LOGE(TAG, "mbedtls_md_hmac_starts() failed: %d", mbedtls_rc);
        return;
    }
    mbedtls_rc = mbedtls_md_hmac_update(&ctx, signed_key, signed_key_length);
    if (mbedtls_rc != 0)
    {
        ESP_LOGE(TAG, "mbedtls_md_hmac_update() failed: %d", mbedtls_rc);
        return;
    }
    mbedtls_rc = mbedtls_md_hmac_finish(&ctx, hmacResult);
    if (mbedtls_rc != 0)
    {
        ESP_LOGE(TAG, "mbedtls_md_hmac_finish() failed: %d", mbedtls_rc);
        return;
    }
    mbedtls_md_free(&ctx);
}

static void url_encode(char *url)
{
}


def generate_sas_token():
    uri = "IoT-Weather-Hub.azure-devices.net/Devices/Display-01"
    key = "D+E0v6/rYM0lQq/DhVmcFppoiYeyQfzGOo5X6YRyLyY="
    expiry = 3600
    ttl = time() + expiry
    sign_key = "%s\n%d" % ((parse.quote_plus(uri)), int(ttl))
    print("Sign key: %2", sign_key)
    signature = b64encode(HMAC(b64decode(key), sign_key.encode('utf-8'), sha256).digest())

    rawtoken = {
        'sr' :  uri,
        'sig': signature,
        'se' : str(int(ttl))
    }


// Escape a string such that it can be used in a URL query.
uint8_t url_quote_plus_esc_chars[] = " <>#%+{}|\\^~[]`;/?:@=&$";
uint8_t url_quote_plus_hex_map[] = "0123456789ABCDEF";

// Logging tag.
static const char *TAG = "IoThub";


char *url_quote_plus(char *string)
{
    char *escaped;
    uint8_t *sptr = query->s;
    uint8_t *tptr;
    size_t in_len = strlen(string);
    size_t out_len = strlen(string);
    size_t ii;

    for (ii = 0; ii < in_len; ii++)
    {
        if (NULL != memchr(url_quote_plus_esc_chars, (int)(*sptr++), sizeof(url_quote_plus_esc_chars)))
        {
            // We will have to escape this character.
            out_len += 2;
        }
    }

    if (out_len == in_len)
    {
        // No escaping required.
        return string;
    }

    escaped = (char *)malloc(len);

    sptr = string;
    tptr = escaped;
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
    free(url);
    return escaped;
}

// Convert a string which might contain extended ASCII characters to utf-8.
coap_string_t *_coap_to_utf_8(coap_string_t *input)
{
    size_t ii;
    size_t len;
    uint8_t *sptr = input->s;
    uint8_t *tptr = input->s;
    coap_string_t *utf8;

    // First count the number of bytes that are in the range 0x80 to 0xff as
    // each of these will be expanded to two bytes.
    len = input->length;
    for (ii = 0; ii < input->length; ii++)
    {
        if (*sptr++ >= 0x80)
        {
            len++;
        }
    }
    if (len == input->length)
    {
        // No extended-ASCII so just return the original string.
        return input;
    }
    utf8 = coap_new_string(len);
    utf8->length = len;
    sptr = input->s;
    tptr = utf8->s;
    for (ii = 0; ii < input->length; ii++)
    {
        if (*sptr >= 0x80)
        {
            // Encode this extended ASCII byte into 2 UTF-8 bytes.
            *tptr++ = 0b11000000 | (*sptr >> 6);
            *tptr++ = 0b10000000 | (*sptr & 0b00111111);
            sptr++;
        }
        else
        {
            // Standard ASCII.
            *tptr++ = *sptr++;
        }
    }
    coap_delete_string(input);
    return(utf8);
}




static void generate_sas_token(char *url, char *base64_key)
{
    uint8_t key[64];
    int b64_rc;
    size_t b64_length;
    char hmac_sig[32];
    char sig[44];
    size_t out_len;
    char *ttl[11];
    int ttl_len;
    time_t expiry = time() + 3600;

    // Decode the Base64 encoded key.
    mbedtls_rc = int mbedtls_base64_decode(key, 64, , &out_len, base64_key, 32);
    if (mbedtls_rc != 0)
    {
        ESP_LOGE(TAG, "mbedtls_base64_decode() failed: %d", mbedtls_rc);
        return;
    }

    /* Caculate the TTL and create the string. */
    ttl_len = ssprintf(ttl, "%lu", expiry);

    /* The URL has to be escaped. */
    url = url_escape_url(url);

    // The SHA256 digest of an HMAC is a 32 byte string.
    HMAC_digest(key, signed_key, hmac_sig)

    // The base64 encoding of a 32 byte string is 44 bytes.
    b64_rc = mbedtls_base64_encode(sig, 44, &b64_length, hmac_sig, 32);
    if (b64_rc != 0)
    {
        ESP_LOGE(TAG, "Enable to BASE64 encode.");
        return;
    }




    token_length = 2 + 1 strlen(uri) + 1 + 3 + 1 + b64_len + 1 + 2 + 1 + ttl_len + 1;
    token = (char *)malloc(token_length);
    ssprintf(token, "sr=%s&sig=%s&se=%s", uri, sig, ttl);

    return(token);

}

/**
 * To connect to Azure we have to create a
*/
static void cs_mqtt_connect()
{
}


static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
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
                cs_mqtt_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                /**
                 * AP disconnected to MQTT cannot work.
                 * Ignore errors if we were never connected.
                */
                ESP_LOGI(TAG, "WiFi AP lost - disconnect");
                esp_mqtt_client_stop(esp_mqtt_client_handle);
                break;

            default:
                ESP_LOGI(TAG, "Other WiFi event: %d", event_id);
                break;
        }
    }
    else if (event_base == CS_TIME)
    {
        ESP_LOGI(TAG, "Time is now set");

    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event_base: %s", event_base);
    }
}

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = CONFIG_BROKER_URI,
            .verification.certificate = (const char *)mqtt_eclipseprojects_io_pem_start
        },
    };

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


void mqtt_main(void)
{
    // TODO: Remove this.
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "Init. MQTT task");

    ESP_LOGI(TAG, "Register event handlers");

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    mqtt_event_handler,
                    NULL,
                    NULL));

    /* */
}

