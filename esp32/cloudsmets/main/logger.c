/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Allow logging over the WiFi connection.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/app_trace.html#app-trace-logging-to-host
 */
#pragma once

// TODO: Are these headers correct?
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// TODO: This has to be thread safe somehow.  Message passing is probably not good enough!
int logger_vprintf(const char *fmt, va_list ap);


// Message passing stack side.
#define STACK_DEPTH 10

static const char* TAG = "Log";
static QueueHandle_t s_xQueue = 0;

void logger_task(void *arg)
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