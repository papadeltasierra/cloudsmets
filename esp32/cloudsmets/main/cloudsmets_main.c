/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"


void app_main(void)
{

    /*
     * Perform start-of-day checking.
     *
     * 1. Open the non-volatile RAM (NVRAM) configuration store.
     * 2. Perform start-of-day configuration.
     */
    cfg_open();
    start_of_day();

    /*
     * Now start the processes the form CloudSMETs.
     *
     * - wifi; the WiFi co-ordinator
     * - softAP; the local soft-access-point
     * - stn; the WiFi client that normally connects to your router AP.
     * - zb; the ZigBee handler
     * - cloud; the cloud connection(s) co-ordinator
     * - az; the Azure handler
     * - aws; the Amazon web services handler
     * - gcp; the Google Cloud Platform handler.
     *
     */

    /*
     * Start the ball rolling by asking the wiFi co-ordinator to connect us
     * to the internet or start a softAP for configuration purposes.
     */
}

void start_of_day()
{
    /*
     * - Configure debugging, either via serial port of by creating a socket.
     */
    // How will we reset to factory?  Can we preload the configuration but not
    // overwrite it with OTA updates?

    /*
     * Throw start-of-day debugging, which we may not see if we are using IP.
     */
    esp32_info();
}

/*
 * Use logging to show ESP32c3 information.  We drop this at INFO level so if
 * logging is not enabled to this level, it will not be seen.
 */
void esp32_info()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;

    esp_chip_info(&chip_info);

    ESP_LOGI(TAG,
        "This is %s chip with %d CPU core(s), WiFi%s%s, ",
        CONFIG_IDF_TARGET,
        chip_info.cores,
        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG,
        "silicon revision %d, ", chip_info.revision);

    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        ESP_LOGI(TAG, "Get flash size failed");
        return;
    }

    ESP_LOGI(TAG< "%uMB %s flash\n", flash_size / (1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG,
        "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

