/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 *
 * Note that this API operates on the default NVRAM partition.
 */
#pragma once

#include "esp_log.h"
#include "nvs.h"

const char* TAG = "Cfg";

/*
 * Namespaces, used to partition components.
 */
const char *cfgSoftAp = "softap";
const char *cfgWifi = "wifi";
const char *cfgWeb = "web";
const char *cfgDbg = "debug";
const char *cfgOta = "ota";
const char *cfgAz = "azure";
/*
 * Unsupported at this time.

 * const char *cfgaws = "aws";
 * const char *cfggcp = "gcp";
 */

/*
 * Keys for each namespace.
 * Duplicate keys are commented out after the first definition but shown to
 * explain what keys are available.
 *
 * SoftAP (peer-to-peer WiFi network)
 */
const char *cfgSoftApSsid = "softApSsid";
const char *cfgSoftApPwd = "softApPwd";

/*
 * Station (Wifi to router)
 */
const char *cfgWifiSsid = "wifiSsid";
const char *cfgWifiPwd = "wifiPwd";

/*
 * Web
 */
const char *cfgWebUser = "webUser";
const char *cfgWebPwd = "webPwd";

/*
 * Debug
 */
const char *cfgDbgFunc = "dbgFunc";
const char *cfgDbgBaud = "dbgBaud";
const char *cfgDbgIpPort = "dbgPort";
const char *cfgDbgEsp32c3 = "dbgEsp32c3";
const char *cfgDbgTlsr8258 = "dbgTlsr8258";

/*
 * OTA, Over-the-air upgrade configuration.
 */
const char *cfgOtaFunc = "otaFunc";
const char *cfgOtaUrl = "otaUrl";
const char *cfgOtaRel = "otaRel";

/*
 * Azure
 */
const char *cfgAzFunc = "azFunc";
const char *cfgAzIotHub = "azIotHub";
const char *cfgAzDevice = "azDevice";
const char *cfgAzCnct1 = "azCnct1";
const char *cfgAzCnct2 = "azCnct2";

/*
 * AWS and GCP are unsupported at this time so no configuration required.
 */

void cfgInit(void)
{
    /*
     * No configuration required.
     */
}

void cfgReadUnint8(char *namespace, char *key, uint8_t *value);
    static nvs_handle_t handle;

    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READONLY, &handle);
    ESP_ERROR_CHECK(nvs_get_u8(handle, key, value));
    nvs_close(handle);
}

extern void cfgReadStr(char *namespace, char *key, char *value, size_t *length);
{
    static nvs_handle_t handle;

    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK(nvs_get_u8(handle, key, value, length));
    nvs_close(handle);
}


extern void cfgWriteUint8(char *namespace, char *key, )
{
    static nvs_handle_t handle;

    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK(nvs_set_u8(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}

extern void cfgWriteStr(char *namespace, char *key, char *value)
{
    static nvs_handle_t handle;

    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK(nvs_set_str(handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}
