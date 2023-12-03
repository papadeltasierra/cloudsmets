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
static bool wifiSoftAp = FALSE;
static bool wifiSoftApFailed = FALSE;
static bool wifiStn = FALSE;
static bool wifiStnFailed = FALSE;

bool wifiMaybeStn(void)
{
    bool status = FALSE;
    char ssid[64];
    char password[33];
    size_t length;

    // Can we start a connection to a router?
    cfgReadStr(CFG_NM_WIFISTN, CFS_KEY_WIFI_SSID, ssid, &length);
    if (length)
    {
        cfgReadStr(CFG_NM_WIFISTN, CFS_KEY_WIFI_PWD, password, &length);
        if (length)
        {
            // Might need to stop the SoftAP first.
            wifiMaybeStopSoftAP();
        }
    }
    return status;
}


void wifiTask(void *arg)
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
            switch (msg)
            {
                case WIFI_START:
                    /*
                     * System is starting to start WiFi, preferable Station but
                     * SoftAP is not.
                     */
                    ESP_LOGI(TAG, "Starting Wifi.");
                    assert(!wifiSoftAp);
                    assert(!wifiStn);
                    xQueueSend(WIFI_STN_QUEUE, EVT_START, 1000);
                    break;

                case WIFI_STN_STARTED:
                    ESP_LOGI(TAG, "WiFi started in station mode.");
                    wifiStn = TRUE;
                    break;

                case WIFI_STN_FAILED:
                    /*
                     * For some reason we have disconnected from the router and
                     * cannot reconnect so fall back to softAP mode.
                     */
                    ESP_LOGE(TAG, "WiFi station mode failed.");
                    wifiStn = FALSE;
                    xQueueSend(WIFI_SOFTAP_QUEUE, EVT_START, 1000);
                    break;

                case WIFI_SOFTAP_FAILED:
                    /*
                     * SoftAP has failed.  We only run softAP is Station mode 
                     * does not work so all we can try at this point is
                     * rebooting in case there was some error.
                     */
                    ESP_LOGE(TAG, "WiFi SoftAP mode has failed - rebooting.");
                    abort();
                    break;

                case WIFI_SOFTAP_STARTED:
                    /*
                     * SoftAP has started.
                     */
                    ESP_LOGI(TAG, "WiFi started in SoftAP mode.");
                    wifiSoftAp = TRUE;
                    break;

                case WIFI_SOFTAP_STOPPED:
                    /*
                     * SoftAP has stopped, which only happens if we are
                     * about to try and switch to Station mode/
                     */
                    ESP_LOGI(TAG, "WiFi stopped in SoftAP mode; start station mode.");
                    wifiSoftAp = FALSE;
                    xQueueSend(WIFI_STN_QUEUE, EVT_START, 1000);
                    break;

                case WIFI_STN_CONFIG:
                    /*
                     * Station WiFi configuration has changed to we should try
                     * and run Station Wifi with the new configuration.
                     */
                    ESP_LOGI(TAG, "Station mod econfig changed.")
                    if (wifiSoftAp)
                    {
                        xQueueSend(WIFI_STN_QUEUE, EVT_STOP, 1000);
                    }
                    else if (wifiStn)
                    {
                        xQueueSend(WIFI_STN_QUEUE, EVT_RESTRT, 1000);
                    }
                    break;

                case WIFI_SOFTAP_CONFIG:
                    /*
                     * SoftAP WiFI configuration has changed so if we are
                     * currently running the SoftAP, restart it.
                     */
                    if (wifiSoftAp)
                    {
                        xQueueSend(WIFI_STN_QUEUE, EVT_RESTART, 1000);
                    }
                    break;

                default:
                    ESP_LOGE(TAG, "Message type %lx is unrecognised.", %lx);
                    break;
            }
        }
    }
}