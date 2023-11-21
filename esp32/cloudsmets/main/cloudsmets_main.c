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
#include "esp_log.h"

// #include "ft_err.h"
#include "pd_err.h"

// Tasks.
#include "config.h"

#define MAIN "main"
#define TAG "HB"
#define QUEUE_DEPTH 10


void app_main(void)
{
    static SOMETHING webTaskParms;
    /*
     * Perform start-of-day checking.
     */
    flasher(1);
    cfgOpen();
    startOfDay();

    /*
     * Now start the processes that form CloudSMETs.
     *
     * - wifi; the WiFi co-ordinator
     * - softAP; the local soft-access-point
     * - stn; the WiFi client that normally connects to your router AP.
     * - web; the web server used to configure CloudSMETS
     * - zb; the ZigBee handler
     * - cloud; the cloud connection(s) co-ordinator
     * - az; the Azure handler
     * - aws; the Amazon web services handler
     * - gcp; the Google Cloud Platform handler.
     *
     */
    ESP_LOGI(MAIN, "Creating tasks...");
    // PD_ERROR_CHECK(xTaskCreate(wifiTask, "Wifi", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(softApTask, "SoftAp", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(stnTask, "Stn", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(webTask, "Web", 4096, &webTaskParms, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(zbTask, "ZigBee", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(cloudTask, "Cloud", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(azTask, "Azure", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(awsTask, "AWS", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(gcpTask, "GCP", 4096, NULL, 10, &myTaskHandle));

    /*
     * Start the ball rolling by asking the wiFi co-ordinator to connect us
     * to the internet or start a softAP for configuration purposes.
     */

    // Do we want a heartbeat monitor and if so, for what?  Probably the
    // ZigBee and cloud services that are active?
}

void startOfDay()
{
    /*
     * - Configure debugging, either via serial port of by creating a socket.
     */
    debugInit();

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
void esp32Info()
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

