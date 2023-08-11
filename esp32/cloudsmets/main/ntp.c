/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Enable NTP time sychronization.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html?highlight=ntp#sntp-time-synchronization
 */
#pragma once

// TODO: Are these headers correct?
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "web.h"

// Message passing stack side.
#define STACK_DEPTH 10

static const char* TAG = "NTP";
static QueueHandle_t s_xQueue = 0;

void ntp(void *arg)
{
    ESP_LOGV(TAG, msg_starting);
    s_xQueue = xQueueCreate(STACK_DEPTH, sizeof(MSG_WIFI) );
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
            ESP_LOGV(TAG,msg_msg_received, 0x1234);
        }
    }
}