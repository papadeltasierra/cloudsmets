/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#pragma once

// TODO: Are these headers correct?
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Message passing stack side.
#define STACK_DEPTH 10

static const char* TAG = "WiFi";
static QueueHandle_t s_xQueue = 0;

typedef struct wifi_msg
{
    unsigned short msg:
    unsigned short field:
    unsigned char *value[1];
} MSG_WIFI;

static BaseType_t cfg_send(MSG_WIFI *msg)
{
    BaseType_t rc;

    if (0 == s_xQueue)
    {
        ESP_LOGE(TAG, "Queue is not ready");
        return pdFAIL;
    }

    rc = xQueueSend(s_xQueue, (void*)msg, (TickType_t)0);
    if (pdPASS != rc)
    {
        ESP_LOGE(TAG, "Send queue full");
    }
    return rc;
}

void wifi(void *arg)
{
    s_xQueue = xQueueCreate(STACK_DEPTH, sizeof(MSG_WIFI) );
    if (s_xQueue == 0)
    {
        ESP_LOGE(TAG, "Enable to create message queue.");
        // TODO: Reboot?
        reboot?
    }

    while (1)
    {
        // 6000 ticks is 1min.
        if( xQueueReceive(s_xQueue, &(rxBuffer), (TickType_t)6000))
        {
            ESP_LOGV(TAG, "Message received: %lx", 0x1234);
            // Todo: log here.
        }
    }
}