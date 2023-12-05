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
#include "esp_event.h"

// #include "ft_err.h"
#include "pd_err.h"

// Tasks.
#include "cs_cfg.h"
#include "cs_wifi.h"
#include "cs_web.h"
#include "cs_ota.h"

/* Task Configuration */
#define CS_APP_TASK_QUEUE_SIZE          CONFIG_CS_TASK_QUEUE_SIZE
#define CS_APP_TASK_PRIORITY_DEFAULT    CONFIG_CS_TASK_PRIORITY_DEFAULT
#define CS_APP_TASK_STACK_SIZE          CONFIG_CS_TASK_STACK_SIZE

static const char *cs_app_task = "App";
#define TAG cs_app_task

union cs_create_parms_t {
    cs_wifi_create_parms_t wifi;
    cs_web_create_parms_t web;
    cs_ota_create_parms_t ota;
};

/*
 * Use logging to show ESP32c3 information.  We drop this at INFO level so if
 * logging is not enabled to this level, it will not be seen.
 */
static void esp32_info()
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

    ESP_LOGI(TAG, "%uMB %s flash\n", flash_size / (1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG,
        "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

static void start_of_day()
{
    /*
     * - Configure debugging, either via serial port of by creating a socket.
     */
    // TODO: debugInit();

    // How will we reset to factory?  Can we preload the configuration but not
    // overwrite it with OTA updates?

    /*
     * Throw start-of-day debugging, which we may not see if we are using IP.
     */
    ESP_LOGI(TAG, "start-of-day");
    esp32_info();
}

void app_main(void)
{
    union cs_create_parms_t create_parms;
    static esp_event_loop_handle_t web_event_loop_handle = NULL;
    static esp_event_loop_handle_t ota_event_loop_handle = NULL;

    /*
     * Perform start-of-day checking.
     */
    // TODO: Should be cs_cfg_init();
    cs_cfg_init();
    start_of_day();

    /**
     * Instead of using FreeRTOS tasks and queues, CloudSMETS uses the ESP-IDF
     * Event Loop Library.  Each event loop is instantiated as a FreeRTOS task
     * under-the-covers so by creating an event loop for each sub-component of
     * CloudSMETS, we spread the load across multiple FreeRTOS tasks.
     *
     * The event loops are created here so that they may be passed to the
     * sub-components both for them to handle their own loops, and to allow them
     * to post messages to other loops.
     *
     * Note that certain components, specifically WiFi, are hard-coded to use
     * the default loop and we cannot change that.  For other components we
     * repost the event to break the dependency on the default loop.
     */
    ESP_LOGI(TAG, "Creating event loops...");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_event_loop_args_t esp_event_loop_args = {
        .queue_size = CS_APP_TASK_QUEUE_SIZE,
        .task_name = cs_app_task_name,
        .task_priority = CS_APP_TASK_PRIORITY_DEFAULT,
        .task_stack_size = CS_APP_TASK_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY
    };
    esp_event_loop_args.task_name = cs_web_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &web_event_loop_handle));
    esp_event_loop_args.task_name = cs_ota_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &ota_event_loop_handle));

    ESP_LOGI(TAG, "Creating tasks...");
    // TODO: Create Azure, ZigBee etc.

    /* Web server needs to know about any task that can be configured. */
    create_parms.web.web_event_loop_handle = web_event_loop_handle;
    create_parms.web.ota_event_loop_handle = ota_event_loop_handle;
    cs_web_task(&create_parms.web);

    /* Create the OTA task. */
    create_parms.ota.ota_event_loop_handle = ota_event_loop_handle;
    cs_ota_task(&create_parms.ota);

    /**
     * Wifi has to start before anything else can happen.
     * starting the Wifi task should also get the entire app rolling.
     */
    create_parms.wifi.dummy = NULL;
    cs_wifi_task(&create_parms.wifi);
}

