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
#define QUEUE_DEPTH 10

static const char* TAG = "WiFi";

# States
#define WIFI_ST_POWER_ON
#define WIFI_ST_DIRECT_STARTING
#define WIFI_ST_DIRECT_ACTIVE
#define WIFI_ST_STARTING
#define WIFI_ST_ACTIVE
#define NUM_WIFI_ST 5

# Inputs
#define WIFI_IN_START
#define WIFI_IN_CFG_AVAILABLE
#define WIFI_IN_CFG_REMOVED
#define WIFI_IN_WIFI_FAILED
#define WIFI_IN_DISABLE
#define WIFI_IN_ENABLE
#define NUM_WIFI_IN 6

# Actions
#define WIFI_ACT_START_DIRECT
#define WIFI_ACT_START
#define WIFI_ACT_STOP
#define WIFI_ACT_WAIT_5S
#define NUM_WIFI_ACT 4

int wifi_fsm_table[NUM_WIFI_ST][NUM_WIFI_IN] =
    {
        {
            WIFI_ACT_WIFI_ENABLED,
            WIFI_ACT_INVALID,
            WIFI_ACT_INVALID,
            WIFI_ACT_INVALID,
            WIFI_ACT_INVALID,
            WIFI_ACT_INVALID
        }
    }




void wifi_fsm(unsigned long msg)
{
    static fsm_state = WIFI_ST_POWER_ON;
    bool enabled = TRUE;
    bool active = FALSE;

    switch (wifi_fsm_table[fsm_state][input])
    {
        case WIFI_ACT_CHECK_STATE:
            if (enabled) {
            } else if (active) {

            }
}

void wifi(void *arg)
{
    static QueueHandle_t s_xQueue = 0;
    unsigned long msg;

    s_xQueue = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    if (s_xQueue == 0)
    {
        ESP_LOGE(TAG, "failed to create msg queue");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    while (1)
    {
        // 6000 ticks is 1min.
        if( xQueueReceive(s_xQueue, &msg, (TickType_t)6000))
        {
            ESP_LOGV(TAG, "Message received: %lx", msg);
            wifi_fsm(msg);
        }
    }
}