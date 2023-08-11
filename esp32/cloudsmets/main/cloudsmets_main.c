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
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "ft_err.h"

#define MAIN "main"
#define TAG "HB"

void app_main(void)
{
    /* Print chip information */
    ByteType_t rc;

    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    ESP_LOGV("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGV("silicon revision %d, ", chip_info.revision);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        ESP_LOGE(msg_flash_failed);
        return;
    }

    ESP_LOGV("%uMB %s flash\n", flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGV("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    // Create the tasks.
    ESP_LOGV(MAIN, msg_creating);
    rc = xTaskCreate(cfg_task, "Cfg", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);
    xTaskCreate(wifi_task, "Wifi", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);
    xTaskCreate(web_task, "Web", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);
    xTaskCreate(azure_task, "Azure", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);
    xTaskCreate(smets_task, "SMETS", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);
    xTaskCreate(ntp_task, "NTP", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);
    xTaskCreate(log_task, "log", 4096, NULL, 10, &myTaskHandle);
    PD_ERROR_CHECK(rc);

    // This now becomes the heartbeat/watchdog.
    ESP_LOGV(TAG, msg_starting);
    s_xQueue = xQueueCreate(STACK_DEPTH, sizeof(CFG_MSG) );
    if (s_xQueue == 0)
    {
        ESP_LOGE(TAG, msg_no_queue);
        // TODO: Reboot?
    }

    ESP_LOGV(TAG, msg_waiting);
    while (1)
    {
        // 6000 ticks is 1min.
        if( xQueueReceive(s_xQueue, &(rxBuffer), (TickType_t)6000))
        {
            ESP_LOGV(TAG, msg_msg_received, 0x1234);
        }
    }
}
