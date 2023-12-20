/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
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
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_app_desc.h"
#include "esp_timer.h"

/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "cs_cfg.h"
#include "cs_ota.h"

#define TAG cs_wifi_task_name

// Only supported SemVer format (-dev can be omitted).
#define SEMVER_FORMAT  "%hu.%hu.%hu-dev%hu"
#define SEMVER_FORMAT_NO_DEV  "%hu.%hu.%hu"

// Task config.
#define CS_TASK_TICKS_TO_WAIT CONFIG_CS_TASK_TICKS_TO_WAIT

static esp_event_loop_handle_t ota_event_loop_handle;

esp_timer_handle_t ota_retry_timer_handle;
esp_timer_handle_t ota_acceptance_timer_handle;

typedef struct
{
    uint16_t    major;
    uint16_t    minor;
    uint16_t    revision;
    uint16_t    dev;
} ota_version_t;

typedef struct
{
    int tag;
    bool allow_dev;
    ota_version_t *version;
} xml_user_data_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    ota_version_t *version;
    esp_err_t esp_rc = ESP_OK;
    int ss_rc;

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
        // Is a SemVer formatted latest revision.
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        version = (ota_version_t *)evt->user_data;
        ss_rc = sscanf(
            evt->data,
            SEMVER_FORMAT,
            &version->major,
            &version->minor,
            &version->revision,
            &version->dev);
        if (ss_rc == 3)
        {
            version->dev = 0;
        }
        if ((ss_rc < 3) || (ss_rc > 4))
        {
            ESP_LOGE(TAG, "Invalid version: %s", evt->data);
            esp_rc = ESP_FAIL;
        }
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
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
    return esp_rc;
}

bool determine_release(ota_version_t *version)
{
    bool success = true;
    uint8_t allow_dev;
    char *query = NULL;
    char *dev = "dev";
    char *buffer;
    size_t len;
    int ss_rc;
    esp_http_client_handle_t esp_http_client_handle;
    esp_err_t esp_rc;

    // If there is a specific release requested then we just try for that.
    cs_cfg_read_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_REL, &buffer, &len);
    if (len > 0)
    {
        // TODO: Common code here?
        ss_rc = sscanf(
            buffer,
            SEMVER_FORMAT,
            &version->major,
            &version->minor,
            &version->revision,
            &version->dev);
        free(buffer);
        if (ss_rc == 3)
        {
            version->dev = 0;
        }
        if ((ss_rc < 3) && (ss_rc > 4))
        {
            ESP_LOGE(TAG, "Revision format invalid: %s", buffer);
            success = false;
        }
    }

    if (success)
    {
        // Are development images acceptable?
        cs_cfg_read_uint8(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_DEV, &allow_dev);

        // TODO: Add length restrictions to the HTML files.
        cs_cfg_read_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_REV_URL, &buffer, &len);

        if (allow_dev)
        {
            ESP_LOGV(TAG, "Dev images are acceptable");
            query = dev;
        }

        esp_http_client_config_t http_client_config = {
            .url = buffer,
            // TODO: Move this string.
            .query = query,
            .event_handler = http_event_handler,
            .auth_type = HTTP_AUTH_TYPE_NONE,
            .user_data = (void *)version
        };

        esp_http_client_handle = esp_http_client_init(&http_client_config);
        esp_rc = esp_http_client_perform(esp_http_client_handle);
        if (esp_rc != ESP_OK)
        {
            // Something went wrong!
            success = false;
        }
    }

    ESP_ERROR_CHECK(esp_http_client_cleanup(esp_http_client_handle));
    free(buffer);

    return success;
}

bool upgrade_to(ota_version_t *version)
{
    char *url;
    size_t len = 1024;
    bool success = true;
    esp_err_t esp_rc;

    cs_cfg_read_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_IMG_URL, &url, &len);

    // Now add the version.
    if (version->dev)
    {
        len += sprintf(&url[len], "/", SEMVER_FORMAT "/esp32/cloudsmets.bin", version->major, version->minor, version->revision, version->dev);
    }
    else
    {
        len += sprintf(&url[len], "/", SEMVER_FORMAT "/esp32/cloudsmets.bin", SEMVER_FORMAT_NO_DEV, version->major, version->minor, version->revision);
    }

    if (len > 1024)
    {
        ESP_LOGE(TAG, "OTA URL overflow: %d", len);
        success = false;
    }

    if (success)
    {
        // TODO: Consider using HTTPS.
        esp_http_client_config_t esp_http_client_config = {
            .url = url,
        // #ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
        //         .crt_bundle_attach = esp_crt_bundle_attach,
        // #else
        //         .cert_pem = (char *)server_cert_pem_start,
        // #endif /* CONFIG_EXAMPLE_USE_CERT_BUNDLE */
            .keep_alive_enable = true,
        };

        esp_https_ota_config_t esp_https_ota_config = {
            .http_config = &esp_http_client_config
        };

        esp_rc = esp_https_ota(&esp_https_ota_config);
        if (esp_rc == ESP_OK)
        {
            ESP_LOGI(TAG, "OTA succeeded; restarting");
            esp_restart();
        }
        else
        {
            ESP_LOGE(TAG, "OTA failed: %d", esp_rc);
            success = false;
        }
    }
    return success;
}

bool do_we_upgrade(ota_version_t *version, bool *upgrade)
{
    // TODO: Better name?
    bool success;
    ota_version_t current;
    const esp_app_desc_t *esp_app_desc;
    int ss_rc;

    (*upgrade) = false;

    // Determine the correct version that we are running.
    esp_app_desc = esp_app_get_description();

    // TODO: Common.
    ss_rc = sscanf(
        esp_app_desc->version,
        SEMVER_FORMAT,
        &current.major,
        &current.minor,
        &current.revision,
        &current.dev);

    if (ss_rc == 3)
    {
        current.dev = 0;
    }
    if ((ss_rc < 3) || (ss_rc > 4))
    {
        ESP_LOGE(TAG, "Invalid version format: %d", esp_app_desc->version);
        success = false;
    }

    if (success)
    {
        /**
         * SemVer versioning is that:
         * - Highest major best or if major is equal
         * - Highest minor is best or if minor is equal
         * - Highest revision is best or if revision is equal
         * - Non-dev is better than dev or if both dev
         * - Highest dev is best.
         */
        if ((version->major > current.major) ||
            ((version->major == current.major) &&
             ((version->minor > current.minor) ||
              ((version->minor == current.minor) &&
               ((version->revision > current.revision) ||
                ((version->revision == current.revision) &&
                 ((current.dev != 0) &&
                   ((version->dev == 0) ||
                    (version->dev > current.dev)))))))))
        {
            ESP_LOGI(TAG,
                "Upgrade from: " SEMVER_FORMAT " to " SEMVER_FORMAT,
                current.major,
                current.minor,
                current.revision,
                current.dev,
                version->major,
                version->minor,
                version->revision,
                version->dev);
            (*upgrade) = true;
        }
    }

    return success;
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
    bool success = true;
    bool do_upgrade;
    uint8_t enabled;
    ota_version_t latest_version = {0};
    uint64_t retry_in;

    // Just in case, stop the timer.
    esp_timer_stop(ota_retry_timer_handle);

    // If OTA is disabled then we are done!
    cs_cfg_read_uint8(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_ENA, &enabled);
    if (!enabled)
    {
        ESP_LOGI(TAG, "OTA is disabled.");
        return;
    }

    success = determine_release(&latest_version);
    if (success)
    {
        success = do_we_upgrade(&latest_version, &do_upgrade);
    }

    if (success)
    {
        success = upgrade_to(&latest_version);
    }

    // We will try again in future but when depends on whether we encountered
    // an error or not.
    //
    // In fact we expect a success to result n a restart!
    if (success)
    {
        // Success so try in 24 hours.
        ESP_LOGV(TAG, "Success - retry in 24hrs");
        retry_in = (uint64_t)24 * 60 * 60 * 1000 * 1000;
    }
    else
    {
        // Error; assume temporary Azure outage and try again in 1 hour.
        ESP_LOGV(TAG, "Error - retry in 1hr");
        retry_in = (uint64_t)1 * 60 * 60 * 1000 * 1000;
    }
    esp_timer_start_once(ota_retry_timer_handle, retry_in);
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

// TODO: Use local function prototypes so can order code nicely.
void ota_retry_timer_callback(void *arg)
{
    // (Re)Start the OTA process.
    start_ota();
}

void ota_acceptance_timer_callback(void *arg)
{
    esp_err_t esp_rc;

    esp_rc = esp_ota_mark_app_valid_cancel_rollback();
    if (esp_rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Unable to acccept OTA: %d", esp_rc);
    }
}

static esp_timer_create_args_t esp_ota_timer_create_args = {
    .callback = ota_retry_timer_callback,
    .name = "OTA timer"
};

static esp_timer_create_args_t esp_acceptance_timer_create_args = {
    .callback = ota_acceptance_timer_callback,
    .name = "OTA acceptance"
};

void cs_ota_task(cs_ota_create_parms_t *create_parms)
{
    ESP_LOGI(TAG, "Creating OTA task");
    ota_event_loop_handle = create_parms->ota_event_loop_handle;
    uint16_t acceptance_interval_mins;
    uint64_t acceptance_interval_mics;
    const esp_partition_t *esp_partition;
    esp_ota_img_states_t esp_ota_img_state;

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

    // Create the acceptance timer and callback.
    ESP_ERROR_CHECK(esp_timer_create(&esp_acceptance_timer_create_args, &ota_acceptance_timer_handle));

    // Create the retry timer and callback.
    ESP_ERROR_CHECK(esp_timer_create(&esp_ota_timer_create_args, &ota_retry_timer_handle));

    // If this is an OTA image and it is not in accepted state, start the acceptance timer.
    esp_partition = esp_ota_get_running_partition();
    if (esp_partition->subtype != ESP_PARTITION_SUBTYPE_APP_FACTORY)
    {
        ESP_ERROR_CHECK(esp_ota_get_state_partition(esp_partition, &esp_ota_img_state));
        if (esp_ota_img_state == ESP_OTA_IMG_VALID)
        {
            cs_cfg_read_uint16(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_ACCEPT, &acceptance_interval_mins);
            acceptance_interval_mics = (uint64_t)acceptance_interval_mins * 60 * 1000 * 1000;
            ESP_ERROR_CHECK(esp_timer_start_once(ota_acceptance_timer_handle, acceptance_interval_mics));
        }
    }
}