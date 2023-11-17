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
<<<<<<< HEAD
    /* Print chip information then start-up */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    QueueHandle_t s_xQueue;
    cfg_recv_msg_t *rxBuffer;

    esp_chip_info(&chip_info);
    ESP_LOGV(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGV(TAG, "silicon revision %d, ", chip_info.revision);
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &flash_size));

    ESP_LOGV(TAG, "%uMB %s flash\n", flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGV(TAG, "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    // Create the tasks.
    ESP_LOGV(MAIN, "Creating tasks...");
    // PD_ERROR_CHECK(xTaskCreate(configuration, "Cfg", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(wifi_task, "Wifi", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(web_task, "Web", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(azure_task, "Azure", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(smets_task, "SMETS", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(ntp_task, "NTP", 4096, NULL, 10, &myTaskHandle));
    // PD_ERROR_CHECK(xTaskCreate(log_task, "log", 4096, NULL, 10, &myTaskHandle));

    // This now becomes the heartbeat/watchdog.
    ESP_LOGV(TAG, "Creating heartbeat monitor queue...");
    s_xQueue = xQueueCreate(QUEUE_DEPTH, sizeof(cfg_recv_msg_t) );
    if (s_xQueue == 0)
    {
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    ESP_LOGV(TAG, "Heatbeat monitor started.");
    while (1)
    {
        // 6000 ticks is 1min.
        if( xQueueReceive(s_xQueue, &(rxBuffer), (TickType_t)6000))
        {
            ESP_LOGV(TAG, "Heartbeat received.");
        }
    }
=======

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
>>>>>>> dev/docs
}

