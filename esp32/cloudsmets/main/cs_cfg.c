/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 *
 * Note that this API operates on the default NVRAM partition.
 */

#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cs_log.h"
#include "cs_cfg.h"

const char* TAG = "Cfg";

/* Default configuration. */
#define CS_WIFI_AP_CHANNEL          CONFIG_CS_WIFI_AP_CHANNEL
#define CS_WIFI_AP_SSID             CONFIG_CS_WIFI_AP_SSID
#define CS_WIFI_AP_PWD              CONFIG_CS_WIFI_AP_PWD

#define CS_WEB_LISTEN_PORT          CONFIG_CS_WEB_LISTEN_PORT

#define CS_OTA_ENABLED              CONFIG_CS_OTA_ENABLED
#define CS_OTA_DEV_IMAGES           CONFIG_CS_OTA_DEV_IMAGES
#define CS_OTA_REVISION_SERVER_URL  CONFIG_CS_OTA_REVISION_SERVER_URL
#define CS_OTA_IMAGE_SERVER_URL     CONFIG_CS_OTA_IMAGE_SERVER_URL
#define CS_OTA_ACCEPT               CONFIG_CS_OTA_ACCEPT

/* Define the config event base. */
ESP_EVENT_DEFINE_BASE(CS_CONFIG_EVENT);

/*
 * Namespaces, used to partition components.
 */
const char *cs_app_task_name = "App";
const char *cs_flash_task_name = "Flash";
const char *cs_wifi_task_name = "WiFi";
const char *cs_web_task_name  = "Web";
const char *cs_ota_task_name  = "Ota";
const char *cs_mqtt_task_name = "Mqtt";
const char *cs_zigbee_task_name = "Zb";

// TODO: We are going to have to mutex protect here!
static nvs_handle_t handle;

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

/*
 * OTA, Over-the-air upgrade configuration.
 */
const char cs_cfg_ota_ena[]     = "otaEna";
const char cs_cfg_ota_dev[]     = "otaDev";
const char cs_cfg_ota_rel[]     = "otaRel";
const char cs_cfg_ota_rev_url[] = "otaRevUrl";
const char cs_cfg_ota_img_url[] = "otaImgUrl";
const char cs_cfg_ota_accept[]  = "otaAccept";

/*
 * MQTT
 */
const char cs_cfg_mqtt_ena[]    = "mqttEna";
const char cs_cfg_mqtt_iothub[] = "mqttIotHub";
const char cs_cfg_mqtt_device[] = "mqttDevice";
const char cs_cfg_mqtt_key1[]   = "mqttKey1";
const char cs_cfg_mqtt_key2[]   = "mqttKey2";

/*
 * Web server helper definitions.
 */
const cs_cfg_definitions_t cs_cfg_wifi_definitions[] =
{
    { CS_CFG_KEY_WIFI_AP_CHNL, NVS_TYPE_U8, -1 },
    { CS_CFG_KEY_WIFI_AP_SSID, NVS_TYPE_STR, 63 },
    { CS_CFG_KEY_WIFI_AP_PWD, NVS_TYPE_STR, 32 },
    { CS_CFG_KEY_WIFI_STA_SSID, NVS_TYPE_STR, 63 },
    { CS_CFG_KEY_WIFI_STA_PWD, NVS_TYPE_STR, 32 },
    { NULL, NVS_TYPE_ANY, -1 }
};

const cs_cfg_definitions_t cs_cfg_web_definitions[] =
{
    { CS_CFG_KEY_WEB_PORT, NVS_TYPE_U16, -1 },
    { NULL, NVS_TYPE_ANY, -1 }
};

const cs_cfg_definitions_t cs_cfg_ota_definitions[] =
{
    { CS_CFG_KEY_OTA_ENA, NVS_TYPE_U8, -1 },
    { CS_CFG_KEY_OTA_DEV, NVS_TYPE_U8, -1 },
    { CS_CFG_KEY_OTA_REL, NVS_TYPE_STR, 256 },
    { CS_CFG_KEY_OTA_REV_URL, NVS_TYPE_STR, 256 },
    { CS_CFG_KEY_OTA_IMG_URL, NVS_TYPE_STR, 256 },
    { CS_CFG_KEY_OTA_ACCEPT, NVS_TYPE_U16, -1 },
    { NULL, NVS_TYPE_ANY, -1 }
};

const cs_cfg_definitions_t cs_cfg_mqtt_definitions[] =
{
    { CS_CFG_KEY_MQTT_ENA, NVS_TYPE_U8, -1 },
    { CS_CFG_KEY_MQTT_IOTHUB, NVS_TYPE_STR, 256 },
    { CS_CFG_KEY_MQTT_DEVICE, NVS_TYPE_STR, 256 },
    { CS_CFG_KEY_MQTT_KEY1, NVS_TYPE_STR, 45 },
    { CS_CFG_KEY_MQTT_KEY2, NVS_TYPE_STR, 45 },
    { NULL, NVS_TYPE_ANY, -1 }
};

void cs_cfg_default_uint8(const char *ns, const char *key, uint8_t def)
{
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

void cs_cfg_default_uint32(const char *ns, const char *key, uint32_t def)
{
    uint32_t existing;
    esp_err_t err_rc;

    ESP_LOGV(TAG, "Default u32 value: %s, %s: %hu", ns, key, def);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    err_rc = nvs_get_u32(handle, key, &existing);
    if (err_rc == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_ERROR_CHECK(nvs_set_u32(handle, key, def));
        err_rc = nvs_commit(handle);
    }
    nvs_close(handle);
    ESP_ERROR_CHECK(err_rc);
}

void cs_cfg_default_str(const char *ns, const char *key, const char * def)
{
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
    ESP_LOGV(TAG, "Read u8 value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
    ESP_ERROR_CHECK(nvs_get_u8(handle, key, value));
    ESP_LOGV(TAG, "Value: %u", (uint)(*value));
    nvs_close(handle);
}

void cs_cfg_read_uint16(const char *ns, const char *key, uint16_t *value)
{
    ESP_LOGV(TAG, "Read u16 value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
    ESP_ERROR_CHECK(nvs_get_u16(handle, key, value));
    ESP_LOGV(TAG, "Value: %hu", (*value));
    nvs_close(handle);
}

void cs_cfg_read_uint32(const char *ns, const char *key, uint32_t *value)
{
    ESP_LOGV(TAG, "Read u32 value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
    ESP_ERROR_CHECK(nvs_get_u32(handle, key, value));
    ESP_LOGV(TAG, "Value: %lu", (*value));
    nvs_close(handle);
}

/**
 * How to use this method:
 * - If value is NULL, just returns the length
 * - if value is not NULL but (*value) is NULL, allocates memory for the string
 * - If value and (*value) are not NULL, copies string into the provided memory.
*/
void cs_cfg_read_str(const char *ns, const char *key, char **value, size_t *length)
{
    ESP_LOGV(TAG, "Read str value: %s, %s", ns, key);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    if (value != NULL)
    {
        if ((*value) == NULL)
        {
            // User requests we allocate buffer.
            ESP_ERROR_CHECK(nvs_get_str(handle, key, NULL, length));
            if (*length)
            {
                (*value) = (char *)malloc(*length);
                if ((*value) == NULL)
                {
                    ESP_LOGE(TAG, "malloc failed: %u", (*length));
                    ESP_ERROR_CHECK(ESP_FAIL);
                }
            }
        }
    }
    ESP_ERROR_CHECK(nvs_get_str(handle, key, *value, length));
    ESP_LOGV(TAG, "Value: %s", *value);
    nvs_close(handle);
}

void cs_cfg_write_uint8(const char *ns, const char *key, uint8_t value)
{
    ESP_LOGV(TAG, "Write u8 value: %s, %s: %u", ns, key, (uint)value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_u8(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

void cs_cfg_write_uint16(const char *ns, const char *key, uint16_t value)
{
    ESP_LOGV(TAG, "Write u16 value: %s, %s: %hu", ns, key, value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_u16(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

void cs_cfg_write_uint32(const char *ns, const char *key, uint32_t value)
{
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
    ESP_LOGV(TAG, "Write u32 value: %s, %s: %s", ns, key, value);
    ESP_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_str(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

/**
 * Default configuration required to start CloudSMETS.  Note that existing
 * configuration is not changed.
*/
// TODO: If crap gets into the NVS, how do we recover from it?
static void cs_cfg_default(void)
{
    /* Default web server listen port. */
    cs_cfg_default_uint16(CS_CFG_NMSP_WEB, CS_CFG_KEY_WEB_PORT, CS_WEB_LISTEN_PORT);

    /* Default SoftAP. */
    cs_cfg_default_uint8(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_CHNL, CS_WIFI_AP_CHANNEL);
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_SSID, CS_WIFI_AP_SSID);
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_PWD, CS_WIFI_AP_PWD);

    /* Default STA is no SSID or Password. */
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_STA_SSID, "");
    cs_cfg_default_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_STA_PWD, "");

#define OTA_DISABLED 0
#define OTA_PROD_ONLY 0
    cs_cfg_default_uint8(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_ENA, OTA_DISABLED);
    cs_cfg_default_uint8(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_DEV, OTA_PROD_ONLY);
    cs_cfg_default_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_REL, "");
    cs_cfg_default_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_REV_URL, CS_OTA_REVISION_SERVER_URL);
    cs_cfg_default_str(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_IMG_URL, CS_OTA_IMAGE_SERVER_URL);
    cs_cfg_default_uint16(CS_CFG_NMSP_OTA, CS_CFG_KEY_OTA_ACCEPT, CS_OTA_ACCEPT);

#define MQTT_DISABLED 0
    cs_cfg_default_uint8(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_ENA, MQTT_DISABLED);
    cs_cfg_default_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_IOTHUB, "");
    cs_cfg_default_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_DEVICE, "");
    cs_cfg_default_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_KEY1, "");
    cs_cfg_default_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_KEY2, "");

    cs_cfg_write_uint8(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_ENA, MQTT_DISABLED);
    cs_cfg_write_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_KEY1, "");
    cs_cfg_write_str(CS_CFG_NMSP_MQTT, CS_CFG_KEY_MQTT_KEY2, "");
}

/**
 * Wipe all configuration from the NVS.  We expect an immediate reboot after
 * this happens.
*/
void cs_cfg_factory_reset(void)
{
    nvs_flash_erase();
}

void cs_cfg_init(void)
{
    // TODO: Remove this.
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

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