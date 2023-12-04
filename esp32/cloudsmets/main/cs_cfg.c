/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 *
 * Note that this API operates on the default NVRAM partition.
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs.h"
#include "cs_cfg.h"

const char* TAG = "Cfg";

/* Default configuration. */
#define CS_WIFI_AP_CHANNEL          CONFIG_CS_WIFI_AP_CHANNEL
#define CS_WIFI_AP_SSID             CONFIG_CS_WIFI_AP_SSID
#define CS_WIFI_AP_PWD              CONFIG_CS_WIFI_AP_PWD
#define CS_WEB_LISTEN_PORT          CONFIG_CS_WEB_LISTEN_PORT

/**
* @brief Ethernet event base definition
*
*/
ESP_EVENT_DEFINE_BASE(CS_CONFIG_EVENT);

/*
 * Namespaces, used to partition components.
 */
const char *cs_app_task_name = "App";
const char *cs_wifi_task_name = "WiFi";
const char *cs_web_task_name  = "Web";
// const char *cs_cfg__log_task_name  = "Log";
const char *cs_ota_task_name  = "Ota";
// const char *cs_cfg__azure_task_name = "Azure";

/*
 * Keys for each namespace.
 * Duplicate keys are commented out after the first definition but shown to
 * explain what keys are available.
 *
 * SoftAP (peer-to-peer WiFi network)
 */
const char cs_cfg_wifi_ap_chnl[] = "wifiApChnl";
const char cs_cfg_wifi_ap_ssid[] = "wifiApSsid";
const char cs_cfg_wifi_ap_pwd[] = "wifiApPwd";

/*
 * Station (Wifi to router)
 */
const char cs_cfg_wifi_sta_ssid[] = "wifiStaSsid";
const char cs_cfg_wifi_sta_pwd[] = "wifiStaPwd";

/*
 * Web
 */
const char cs_cfg_web_port[] = "webPort";
const char cs_cfg_web_user[] = "webUser";
const char cs_cfg_web_pwd[] = "webPwd";

// /*
//  * Logging
//  */
// const char cs_cfg_Log_func[] = "dbgFunc";
// const char cs_cfg_Log_baud[] = "dbgBaud";
// const char cs_cfg_Log_ip_port[] = "dbgPort";
// const char cs_cfg_Log_esp32c3[] = "dbgEsp32c3";
// const char cs_cfg_Log_tlsr8258[] = "dbgTlsr8258";

/*
 * OTA, Over-the-air upgrade configuration.
 */
const char cs_cfg_ota_func[] = "otaFunc";
const char cs_cfg_ota_url[] = "otaUrl";
const char cs_cfg_ota_rel[] = "otaRel";

// /*
//  * Azure
//  */
// const char *cs_cfg_az_func = "azFunc";
// const char *cs_cfg_az_iotHub = "azIotHub";
// const char *cs_cfg_az_device = "azDevice";
// const char *cs_cfg_az_cnct1 = "azCnct1";
// const char *cs_cfg_az_cnct2 = "azCnct2";

/*
 * Web server helper definitions.
 */
cs_cfg_definitions_t cs_cfg_wifi_definitions[] =
{
    { CS_CFG_KEY_WIFI_AP_CHNL, NVS_TYPE_U8, -1 },
    { CS_CFG_KEY_WIFI_AP_SSID, NVS_TYPE_STR, 63 },
    { CS_CFG_KEY_WIFI_AP_PWD, NVS_TYPE_STR, 32 },
    { CS_CFG_KEY_WIFI_STA_SSID, NVS_TYPE_STR, 63 },
    { CS_CFG_KEY_WIFI_STA_PWD, NVS_TYPE_STR, 32 },
    { NULL, NVS_TYPE_ANY, -1 }
};

cs_cfg_definitions_t cs_cfg_web_definitions[] =
{
    { CS_CFG_KEY_WEB_PORT, NVS_TYPE_U16, -1 },
    { NULL, NVS_TYPE_ANY, -1 }
};

// cs_cfg_definitions_t cs_cfg_log_definitions[] =
// {
//     { CFG_KEY_LOG_FUNC, NVS_TYPE_U8, -1 },
//     { CFG_KEY_LOG_BAUD, NVS_TYPE_U32, -1 },
//     { CFG_KEY_LOG_IP_PORT, NVS_TYPE_U16, -1 },
//     { CFG_KEY_LOG_ESP32C3, NVS_TYPE_U8, -1 },
//     { CFG_KEY_LOG_TLSR8258, NVS_TYPE_U8, -1 },
//     { NULL,NVS_TYPE_ANY -1 }
// };

cs_cfg_definitions_t cs_cfg_ota_definitions[] =
{
    { CS_CFG_KEY_OTA_FUNC, NVS_TYPE_U8, -1 },
    { CS_CFG_KEY_OTA_URL, NVS_TYPE_STR, 256 },
    { CS_CFG_KEY_OTA_REL, NVS_TYPE_STR, 16 },
    { NULL, NVS_TYPE_ANY, -1 }
};

/*
 * AWS and GCP are unsupported at this time so no configuration required.
 */

void cs_cfg_default_uint8(const char *ns, const char *key, uint8_t def)
{
    nvs_handle_t handle;
    uint8_t existing;
    esp_err_t err_rc;

    ESP_LOGV(TAG, "Default u8 value: %s, %s: %u", ns, key, (uint)def);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    err_rc = nvs_get_u8(handle, key, &existing);
    if (err_rc == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_ERROR_CHECK(nvs_set_u8(handle, key, def));
        err_rc = nvs_commit(handle);
    }
    nvs_close(handle);
    ESP_ERROR_CHECK(err_rc);
}

void cs_cfg_default_uint16(const char *ns, const char *key, uint16_t def)
{
    nvs_handle_t handle;
    uint16_t existing;
    esp_err_t err_rc;

    ESP_LOGV(TAG, "Default u8 value: %s, %s: %hu", ns, key, def);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    err_rc = nvs_get_u16(handle, key, &existing);
    if (err_rc == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_ERROR_CHECK(nvs_set_u16(handle, key, def));
        err_rc = nvs_commit(handle);
    }
    nvs_close(handle);
    ESP_ERROR_CHECK(err_rc);
}

void cs_cfg_default_str(const char *ns, const char *key, const char * def)
{
    nvs_handle_t handle;
    size_t length;
    esp_err_t err_rc;

    ESP_LOGV(TAG, "Default str value: %s, %s: %s", ns, key, def);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    err_rc = nvs_get_str(handle, key, NULL, &length);
    if (err_rc == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_ERROR_CHECK(nvs_set_str(handle, key, def));
        err_rc = nvs_commit(handle);
    }
    nvs_close(handle);
    ESP_ERROR_CHECK(err_rc);
}

void cs_cfg_read_uint8(const char *ns, const char *key, uint8_t *value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Read u8 value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
    ESP_ERROR_CHECK(nvs_get_u8(handle, key, value));
    ESP_LOGV(TAG, "Value: %u", (uint)value);
    nvs_close(handle);
}

void cs_cfg_read_uint16(const char *ns, const char *key, uint16_t *value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Read u16 value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
    ESP_ERROR_CHECK(nvs_get_u16(handle, key, value));
    ESP_LOGV(TAG, "Value: %hu", value);
    nvs_close(handle);
}

void cs_cfg_read_uint32(const char *ns, const char *key, uint32_t *value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Read u32 value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
    ESP_ERROR_CHECK(nvs_get_u32(handle, key, value));
    ESP_LOGV(TAG, "Value: %lu", value);
    nvs_close(handle);
}

void cs_cfg_read_str(const char *ns, const char *key, char *value, size_t *length)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Read str value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_get_str(handle, key, value, length));
    ESP_LOGV(TAG, "Value: %s", value);
    nvs_close(handle);
}


void cs_cfg_write_uint8(const char *ns, const char *key, uint8_t value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Write u8 value: %s, %s: %u", ns, key, (uint)value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_u8(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

void cs_cfg_write_uint16(const char *ns, const char *key, uint16_t value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Write u16 value: %s, %s: %hu", ns, key, value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_u16(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

void cs_cfg_write_uint32(const char *ns, const char *key, uint32_t value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Write u32 value: %s, %s: %lu", ns, key, value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_u32(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

/**
 * Note that strings have to be NULL terminated.  Note also that strings are
 * stored in blocks of 32 bytes.
 */
void cs_cfg_write_str(const char *ns, const char *key, const char *value)
{
    nvs_handle_t handle;

    ESP_LOGV(TAG, "Write u32 value: %s, %s: %s", ns, key, value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_str(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

/**
 * Default configuration required to start CLoudSMETS.  Note that existing
 * configuration is not changed.
*/
static void cs_cfg_default(void)
{
    cs_cfg_default_uint16(CS_CFG_NMSP_WEB, CS_CFG_KEY_WEB_PORT, CS_WEB_LISTEN_PORT);
    cs_cfg_default_uint8(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_CHNL, CS_WIFI_AP_CHANNEL);
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_SSID, CS_WIFI_AP_SSID);
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_PWD, CS_WIFI_AP_PWD);

    /* Default STA is no SSID or Password. */
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_SSID, "");
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_PWD, "");
}

void cs_cfg_init(void)
{
    esp_err_t esp_rc;
    nvs_stats_t nvs_stats;

    /**
     * Test read NVS stats to confirm that the partition is correctly set-up.
    */
    esp_rc = nvs_get_stats(NULL, &nvs_stats);
    if (esp_rc == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGI(TAG, "Initialize NVS");
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /**
     * Default the NVS.
     */
    cs_cfg_default();

    /**
     * Read stats again so that we can log them.
    */
    ESP_ERROR_CHECK(nvs_get_stats(NULL, &nvs_stats));
    ESP_LOGI(TAG, "Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)",
          nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);
}

