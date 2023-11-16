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
}
