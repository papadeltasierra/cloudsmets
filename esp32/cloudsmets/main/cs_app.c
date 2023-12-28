/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Application mainline and heartbeat/watchdog.
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <sys/time.h>
#include "esp_chip_info.h"
#include "esp_flash.h"

/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "driver/gpio.h"

// #include "ft_err.h"
#include "pd_err.h"

// Tasks.
#include "cs_cfg.h"
#include "cs_wifi.h"
#include "cs_web.h"
#include "cs_ota.h"
#include "cs_mqtt.h"
#include "cs_time.h"
#include "cs_zigbee.h"
#include "cs_flash.h"
#include "cs_app.h"

/* Task Configuration */
#define CS_APP_TASK_QUEUE_SIZE          CONFIG_CS_TASK_QUEUE_SIZE
#define CS_APP_TASK_PRIORITY_DEFAULT    CONFIG_CS_TASK_PRIORITY_DEFAULT
#define CS_APP_TASK_STACK_SIZE          CONFIG_CS_TASK_STACK_SIZE

// TODO: Should be common.
#define BLUE_LED            GPIO_NUM_3
#define TLSR8258_POWER      GPIO_NUM_0
#define CLEAR_KEY           GPIO_NUM_2

/* Times. */
#define FIRST_JAN_2023              1672531200
#define FACTORY_RESET_PRESS         10
#define ZIGBEE_RESET_PRESS          4
#define FACTORY_RESET_TIMER_WAIT    ((uint64_t)1000 * 1000 * 2)

#define TAG cs_app_task_name

esp_timer_handle_t factory_reset_timer_handle = NULL;

ESP_EVENT_DEFINE_BASE(CS_APP_EVENT);

static esp_event_loop_handle_t app_event_loop_handle = NULL;
static esp_event_loop_handle_t zigbee_event_loop_handle = NULL;

union cs_create_parms_t {
    cs_flash_create_parms_t flash;
    cs_wifi_create_parms_t wifi;
    cs_web_create_parms_t web;
    cs_ota_create_parms_t ota;
    cs_mqtt_create_parms_t mqtt;
    cs_zigbee_create_parms_t zigbee;
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

/**
 * Note that the line is pull DOWN by a key press.
 * We send ourselves an event to avoid any processing that might not be
 * possible from inside an interrupt handler.
*/
static void intr_handler(void *arg)
{
    time_t pressed = 0;
    time_t released = 0;
    int level;

    level = gpio_get_level(CLEAR_KEY);
    if (level)
    {
        /* Rising line; key has been released. */
        released = time(NULL);
        released = released - pressed;
        if (released > FIRST_JAN_2023)
        {
            /**
             * Time has been set whilst we were pressing the key so we cannot
             * tell how long it was held for; have to just ignore it.
            */
        }
        else if (released > FACTORY_RESET_PRESS)
        {
            /**
             * Factory resetting the ESP32c3 will happen on a timer to allow
             * the tlsr8258 to be reset first.
            */
            ESP_ERROR_CHECK(esp_event_isr_post_to(
                app_event_loop_handle,
                CS_APP_EVENT,
                CS_APP_EVENT_FACTORY_RESET,
                NULL,
                0,
                NULL));
        }
        else if (released > ZIGBEE_RESET_PRESS)
        {
            ESP_ERROR_CHECK(esp_event_isr_post_to(
                app_event_loop_handle,
                CS_APP_EVENT,
                CS_APP_EVENT_FACTORY_RESET,
                NULL,
                0,
                NULL));
        }
    }
    else
    {
        /* Falling line; key has been pressed. */
        pressed = time(NULL);
        released = pressed;
    }
}

static void init_gpio(void)
{
    /* Configure the LED GPIO lines used by T-ZigBee. */
    gpio_config_t gpio_config_data = {
        .pin_bit_mask = (1 << BLUE_LED) | (1 << TLSR8258_POWER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en =  GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_config_data));

    /* Configure the input key GPIO line used by T-ZigBee. */
    gpio_config_data.pin_bit_mask = (1 << CLEAR_KEY);
    gpio_config_data.mode = GPIO_MODE_INPUT;
    gpio_config_data.pull_up_en =  GPIO_PULLUP_ENABLE;
    gpio_config_data.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config_data.intr_type = GPIO_INTR_ANYEDGE;
    ESP_ERROR_CHECK(gpio_config(&gpio_config_data));
    ESP_ERROR_CHECK(gpio_isr_register(intr_handler, NULL, ESP_INTR_FLAG_LEVEL1, NULL));
    ESP_ERROR_CHECK(gpio_intr_enable(CLEAR_KEY));
}

static void factory_reset_timer_cb(void *arg)
{
    cs_cfg_factory_reset();
    esp_restart();
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == CS_APP_EVENT)
    {
        switch (event_id)
        {
            case CS_APP_EVENT_FACTORY_RESET:
                ESP_ERROR_CHECK(esp_event_post_to(
                    zigbee_event_loop_handle,
                    CS_ZIGBEE_EVENT,
                    CS_ZIGBEE_EVENT_FACTORY_RESET,
                    NULL,
                    0,
                    10));

                /* Delay the ESP32c3 reset to allow ZigBee time to reset first. */
                ESP_ERROR_CHECK(esp_timer_start_once(factory_reset_timer_handle, FACTORY_RESET_TIMER_WAIT));
                break;

            case CS_APP_EVENT_ZIGBEE_RESET:
                ESP_ERROR_CHECK(esp_event_post_to(
                    zigbee_event_loop_handle,
                    CS_ZIGBEE_EVENT,
                    CS_ZIGBEE_EVENT_FACTORY_RESET,
                    NULL,
                    0,
                    10));
                break;

            default:
                ESP_LOGE(TAG, "Unexpected flash event: %d", event_id);
                break;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event: %s, %d", event_base, event_id);
    }
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

    /* Turn on the tlsr8258 by setting the GPIO that controls its power. */
    ESP_ERROR_CHECK(gpio_set_level(TLSR8258_POWER, 1));
}

void app_main(void)
{
    union cs_create_parms_t create_parms;
    static esp_event_loop_handle_t flash_event_loop_handle = NULL;
    static esp_event_loop_handle_t web_event_loop_handle = NULL;
    static esp_event_loop_handle_t ota_event_loop_handle = NULL;
    static esp_event_loop_handle_t mqtt_event_loop_handle = NULL;

    // TODO: Remove this.
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    /*
     * Perform start-of-day checking.
     */
    // TODO: Should be cs_cfg_init();  Also very messy having lots of stuff here!
    cs_cfg_init();
    init_gpio();
    start_of_day();

    /**
     * Create various timers.
    */
    esp_timer_create_args_t timer_create_args =
    {
        .callback = factory_reset_timer_cb,
        .name = "F.Reset"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_create_args, &factory_reset_timer_handle));

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
    // .task_stack_size = CS_APP_TASK_STACK_SIZE,
    esp_event_loop_args_t esp_event_loop_args = {
        .queue_size = CS_APP_TASK_QUEUE_SIZE,
        .task_name = cs_app_task_name,
        .task_priority = CS_APP_TASK_PRIORITY_DEFAULT,
        // TODO: Do we really need this big a stack?  Do we care?
        .task_stack_size = 32768,
        .task_core_id = tskNO_AFFINITY
    };
    esp_event_loop_args.task_name = cs_flash_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &flash_event_loop_handle));
    esp_event_loop_args.task_name = cs_web_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &web_event_loop_handle));
    esp_event_loop_args.task_name = cs_ota_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &ota_event_loop_handle));
    esp_event_loop_args.task_name = cs_mqtt_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &mqtt_event_loop_handle));
    esp_event_loop_args.task_name = cs_zigbee_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &zigbee_event_loop_handle));

    ESP_LOGI(TAG, "Creating tasks...");
    // TODO: Create Azure, ZigBee etc.

    /* Create the LED flasher task. */
    create_parms.flash.flash_event_loop_handle = flash_event_loop_handle;
    cs_flash_task(&create_parms.flash);

    /* Web server needs to know about any task that can be configured. */
    create_parms.web.web_event_loop_handle = web_event_loop_handle;
    create_parms.web.ota_event_loop_handle = ota_event_loop_handle;
    create_parms.web.mqtt_event_loop_handle = mqtt_event_loop_handle;
    cs_web_task(&create_parms.web);

    /* Create the OTA task. */
    create_parms.ota.ota_event_loop_handle = ota_event_loop_handle;
    cs_ota_task(&create_parms.ota);

    /* Create the MQTT task. */
    create_parms.mqtt.mqtt_event_loop_handle = mqtt_event_loop_handle;
    create_parms.mqtt.flash_event_loop_handle = flash_event_loop_handle;
    cs_mqtt_task(&create_parms.mqtt);

    /* Create the ZigBee task. */
    create_parms.zigbee.zigbee_event_loop_handle = zigbee_event_loop_handle;
    create_parms.zigbee.mqtt_event_loop_handle = mqtt_event_loop_handle;
    create_parms.zigbee.flash_event_loop_handle = flash_event_loop_handle;
    cs_zigbee_task(&create_parms.zigbee);

    /**
     * Wifi has to start before anything else can happen.
     * starting the Wifi task should also get the entire app rolling.
     */
    create_parms.wifi.flash_event_loop_handle = flash_event_loop_handle;
    cs_wifi_task(&create_parms.wifi);

    /**
     * Listen for events targetted to this app.
    */
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    app_event_loop_handle,
                    CS_APP_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    NULL,
                    NULL));

    // TODO: remove this which was added for testing.
#define NOW  	1703592595
    struct timeval now = { .tv_sec = NOW };
    settimeofday(&now, NULL);
    ESP_LOGV(TAG, "Sending time event");
    ESP_ERROR_CHECK(esp_event_post_to(mqtt_event_loop_handle, CS_TIME_EVENT, 0, NULL, 0, 0));
}