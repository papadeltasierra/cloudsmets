/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#include <expat.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_netif_ip_addr.h"
#include "esp_http_client.h"
#include "esp_mac.h"
/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "cs_cfg.h"
#include "cs_ota.h"
#include "cs_xml_err.h"

#define TAG cs_wifi_task_name

// Task config.
#define CS_TASK_TICKS_TO_WAIT CONFIG_CS_TASK_TICKS_TO_WAIT

static esp_event_loop_handle_t ota_event_loop_handle;

typedef struct
{
    uint16_t    major;
    uint16_t    minor;
    uint16_t    revision;
    uint16_t    dev;
} ota_version_t;

enum {
    XML_TAG_UNKNOWN,
    XML_TAG_NAME
} xml_tag_e;

typedef struct
{
    int tag;
    bool allow_dev;
    ota_version_t *version;
} xml_user_data_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;

    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;

    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;

    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;

    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        XML_ERROR_CHECK(XML_Parse((XML_Parser)(evt->user_data), evt->data, evt->data_len, false));
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        XML_ERROR_CHECK(XML_Parse((XML_Parser)(evt->user_data), NULL, 0, true));
#if 0
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
#endif
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
#if 0
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
#endif
        break;

    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

static void xml_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    xml_user_data_t *user_data;

    if (0 == strcmp(name, "Name"))
    {
        user_data = (xml_user_data_t *)userData;
        user_data->tag = XML_TAG_NAME;
    }
}

static void xml_end_element(void *userData, const XML_Char *name)
{
    xml_user_data_t *user_data;

    if (0 == strcmp(name, "Name"))
    {
        user_data = (xml_user_data_t *)userData;
        user_data->tag = XML_TAG_UNKNOWN;
    }
}

#define ASCII_LEFT_ANGLE    '<'
#define ASCII_DASH          '-'
#define ASCII_SLASH         '/'

void xml_character_data(void *userData, const XML_Char *s, int len)
{
    ota_version_t version;
    char dash_or_slash;
    char slash;
    xml_user_data_t *user_data = (xml_user_data_t *)userData;
    int rc = 0;
    bool better = false;

    if (user_data->tag == XML_TAG_NAME)
    {
        /**
         * The following expects a string of format:
         * - 1.2.3/ or...
         * - 1.2.3-dev4/
         *
         * So we extract the 3 or 4 integers but also check for the slash and dash.
         * Remember to default the dev to zero in case one is not present.
        */
        ESP_LOGV(TAG, "Found name: %*s", len, s);
        version.dev = 0;

        /**
         * We do not have access to an snscanf() method so so something nasty here!
         * TODO: Explain this - hope more data!
        */
        assert((s[len] == ASCII_LEFT_ANGLE) && "No angle bracket.");
        rc = sscanf(s, "%hu.%hu.%hu%cdev%hu%c", &version.major, &version.minor, &version.revision, &dash_or_slash, &version.dev, &slash);

        if ((rc > 4) && (!user_data->allow_dev))
        {
            // Do not allow dev releases.
            rc = 0;
        }

        if (((rc == 4) && (dash_or_slash == ASCII_SLASH)) ||
            ((rc == 6) && (dash_or_slash == ASCII_DASH) && (slash == ASCII_SLASH)))
        {
            ESP_LOGV(TAG, "Parsed version: %hu.%hu.%hu(-dev%hu)", version.major, version.minor, version.revision, version.dev);
            /**
             * We found a valid version so is it better than the current one?
             * - Major version higher means better
             * - Major same, minor higher means better
             * - Major, minor save, revision higher means better
             * - Major, minor, revision same, current dev not zero, current higher means better.
             *
             * Note that any dev release is WORSE than a full release so...
             *
             * - 1.2.4 > 1.2.3
             * - 1.3.3 > 1.2.3
             * - 2.2.3 > 1.2.3
             * - 1.2.3 > 1.2.3-dev4
             * - 1.2.3-dev5 > 1.2.3-dev4
            */
            if (version.major > user_data->version->major)
            {
                better = true;
            }
            else if (version.major == user_data->version->major)
            {
                if (version.minor > user_data->version->minor)
                {
                    better = true;
                }
                else if (version.minor == user_data->version->minor)
                {
                    if (version.revision > user_data->version->revision)
                    {
                        better = true;
                    }
                    else if (version.revision == user_data->version->revision)
                    {
                        if ((user_data->version->dev != 0) &&
                            ((version.dev == 0) ||
                             (version.dev > user_data->version->dev)))
                        {
                            better = true;
                        }
                    }
                }
            }

            if (better)
            {
                ESP_LOGI(TAG, "This is a better version");
                user_data->version->major = version.major;
                user_data->version->minor = version.minor;
                user_data->version->revision = version.revision;
                user_data->version->dev = version.dev;
            }
        }
    }
}

/**
 * The process will be:
 * - Determine what images are available.
 * - Determine what image we are currently running.
 * - If there is a newer image...
 *   - Perform the OTA download and write
 *   - On start-up of the new image
 *     - Read the current OTA state and spot it is ESP_OTA_IMG_PENDING_VERIFY
 *     - Start the acceptance timer
 *     - If the timer completes before a crash occurs, mark the image as good.
 * TODO: What is ower outage trashes the image?  Will it get downloaded again
 *       and retried?
 */
static void start_ota()
{
    char *url;
    uint8_t enabled;
    uint8_t allow_dev;
    esp_http_client_handle_t client;
    ota_version_t latest_version = {0};
    XML_Parser XMLCALL xml_parser;
    size_t len;

    xml_user_data_t xml_user_data = {
        // TODO: Use same syntax as ESP tag definitions.
        .tag = XML_TAG_UNKNOWN,
        .version = &latest_version,
    };

    // If OTA is disabled then we are done!
    cs_cfg_read_uint8(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_FUNC, &enabled);
    if (!enabled)
    {
        ESP_LOGI(TAG, "OTA is disabled.");
        return;
    }
    // Are development images acceptable?
    cs_cfg_read_uint8(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_IMAGE, &allow_dev);

// TODO: Common config used in various places.
#define OTA_MAX_URL_LENGTH 1024
    url = (char *)malloc(OTA_MAX_URL_LENGTH);
    len = OTA_MAX_URL_LENGTH;
    cs_cfg_read_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_URL, url, &len);

    xml_parser = XML_ParserCreate(NULL);

    // !!PDS: Has to carry more!
    XML_SetUserData(xml_parser, &xml_user_data);
    XML_SetElementHandler(xml_parser, xml_start_element, xml_end_element);
    XML_SetCharacterDataHandler(xml_parser, xml_character_data);

    esp_http_client_config_t http_client_config = {
        .url = url,
        // TODO: Move this string.
        .query = "restype=container&comp=list&delimiter=%2F",
        .event_handler = http_event_handler,
        .auth_type = HTTP_AUTH_TYPE_NONE,
        .user_data = (void *)xml_parser
    };

    client = esp_http_client_init(&http_client_config);
    ESP_ERROR_CHECK(esp_http_client_perform(client));
    XML_ParserFree(xml_parser);

#if 0
    // We should now have the XML listing of the available versions.
    https://components.espressif.com/components/espressif/expat

    esp_app_desc = esp_app_get_description()
                       esp_add_desc->version;
    esp_add_desc->project_name;

    free(buffer);
#endif
    ESP_ERROR_CHECK(esp_http_client_cleanup(client));
    free(url);
}

// TODO: How are we ensuring that OTA stuff is checked regularly?

/**
 * Catch events from the default event loop and immediately report to the event
 * loop for this task.
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGV(TAG, "Relay event to ota event loop");

    // We don't care about the data so just ignore it.
    ESP_ERROR_CHECK(esp_event_post_to(
        ota_event_loop_handle,
        event_base,
        event_id,
        NULL,
        0,
        CS_TASK_TICKS_TO_WAIT));
}

static void ota_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_CONNECTED:
            /**
             * We are connected so check for OTA update.
             */
            start_ota();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            /**
             * We are disconnected so abort any OTA update.
             */
            break;

        default:
            ESP_LOGE(TAG, "unexpected WiFi event: %d", event_id);
            break;
        }
    }
    else if (event_base == CS_CONFIG_EVENT)
    {
        /*
         * Any configuration chnaged means that we:
         * - Maybe abort any running OTA
         * - Maybe start a new OTA is not previously enabled
         * - Maybe schedule another OTA if settings have changed.
         */
        ESP_LOGI(TAG, "Config. change");
        // TODO: Add code here.
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event base: %s, %d", event_base, event_id);
    }
}

void cs_ota_task(cs_ota_create_parms_t *create_parms)
{
    ESP_LOGI(TAG, "Creating OTA task");
    ota_event_loop_handle = create_parms->ota_event_loop_handle;

    // We want to know when we are connected to an AP (router) and when not.
    ESP_LOGI(TAG, "Register event handlers");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_STA_CONNECTED,
        wifi_event_handler,
        NULL,
        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        wifi_event_handler,
        NULL,
        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
        ota_event_loop_handle,
        WIFI_EVENT,
        WIFI_EVENT_STA_CONNECTED,
        ota_event_handler,
        NULL,
        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
        ota_event_loop_handle,
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        ota_event_handler,
        NULL,
        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
        ota_event_loop_handle,
        CS_CONFIG_EVENT,
        CS_CONFIG_CHANGE,
        wifi_event_handler,
        NULL,
        NULL));
}

#if 0
    // TODO: OTA confirmation perdio should perhaps be configurable.



    Ideally we want a self-test...
    // TODO: On timer.... ESP_ERROR_CHECK(esp_ota_mark_app_valid_cancel_rollback())

    esp_ota_set_boot_partition()
    esp_restart()

// Get curent application information.
esp_app_get_description()
#endif

#if 0
// We have to create an ESP HTTP client to do the download.

bool image_header_was_checked = false;
while (1) {
    int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
    ...
    if (data_read > 0) {
        if (image_header_was_checked == false) {
            esp_app_desc_t new_app_info;
            if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                // check current version with downloading
                if (esp_efuse_check_secure_version(new_app_info.secure_version) == false) {
                    ESP_LOGE(TAG, "This a new app can not be downloaded due to a secure version is lower than stored in efuse.");
                    http_cleanup(client);
                    task_fatal_error();
                }

                image_header_was_checked = true;

                esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
            }
        }
        esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
    }
}
...
ESP_ERROR_CHECK(esp_ota_end());

// Add these to a status page.
esp_ota_get_boot_partition()
esp_ota_get_running_partition()
#endif