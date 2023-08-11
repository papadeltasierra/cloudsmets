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

static const char* TAG = "Cfg";
static QueueHandle_t s_xQueue = 0;


typedef struct cfg_msg
{
    unsigned short msg:
    unsigned short field:
    unsigned char *value[1];
} CFG_MSG;

static BaseType_t cfg_send(CFG_MSG *msg)
{
    BaseType_t rc;

    if (0 == s_xQueue)
    {
        ESP_LOGE(TAG, msg_queue_not_ready);
        return pdFAIL;
    }

    rc = xQueueSend(s_xQueue, (void*)msg, (TickType_t)0);
    if (pdPASS != rc)
    {
        ESP_LOGE(TAG, msg_send_failed);
    }
    return rc;
}

void configuration(void *arg)
{
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