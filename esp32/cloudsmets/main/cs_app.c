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

#include "esp_event.h"
#include "esp_timer.h"
#include "driver/gpio.h"

/* Routines akin to ESP_ERROR_CHECK() for routines returning pdTrue/pdFalse. */
#include "pd_err.h"

/* CloudSMETS tasks. */
#include "cs_log.h"
#include "cs_gpio.h"
#include "cs_cfg.h"
#include "cs_wifi.h"
#include "cs_web.h"
#include "cs_ota.h"
#include "cs_mqtt.h"
#include "cs_time.h"
#include "cs_zigbee.h"
#include "cs_flash.h"
#include "cs_app.h"

/* Task Configuration set via "idf.py menuconfig". */
#define CS_APP_TASK_QUEUE_SIZE          CONFIG_CS_TASK_QUEUE_SIZE
#define CS_APP_TASK_PRIORITY_DEFAULT    CONFIG_CS_TASK_PRIORITY_DEFAULT
#define CS_APP_TASK_STACK_SIZE          CONFIG_CS_TASK_STACK_SIZE

/* Times or dates (in seconds possibly since Unix epoch, 1-Jan-1970). */
#define FIRST_JAN_2023              ((uint64_t)1672531200)
#define FACTORY_RESET_PRESS         ((uint64_t)10)
#define ZIGBEE_RESET_PRESS          ((uint64_t)4)

/* Delay before ESP32c3 restarts to allow ZigBee reset to be performed. */
#define FACTORY_RESET_DELAY         ((uint64_t)1000 * 1000 * 2)

/* Task name for logging purposes. */
#define TAG cs_app_task_name

esp_timer_handle_t factory_reset_timer_handle = NULL;

ESP_EVENT_DEFINE_BASE(CS_APP_EVENT);

/* Union to save storage for create parameters. */
union cs_create_parms_t {
    cs_flash_create_parms_t flash;
    cs_wifi_create_parms_t wifi;
    cs_web_create_parms_t web;
    cs_ota_create_parms_t ota;
    cs_mqtt_create_parms_t mqtt;
    cs_zigbee_create_parms_t zigbee;
};

/**
 * This interrupt is triggered if the input key is PRESSED or RELEASED.
*/
static void IRAM_ATTR intr_handler(void *arg)
{
    esp_event_loop_handle_t app_event_loop_handle = (esp_event_loop_handle_t)arg;

    esp_event_isr_post_to(
        app_event_loop_handle,
        CS_APP_EVENT,
        CS_APP_EVENT_KEY_ACTION,
        NULL,
        0,
        NULL);
}

/**
 * Note that the input key line is pulled DOWN by a key press.
*/
static void key_action(esp_event_loop_handle_t zigbee_event_loop_handle)
{
    /* Note that timer_t is 64-bit but "Nano" formatting cannot handle 64-bit. */
    static time_t pressed = 0;
    time_t held_for = 0;
    int level;

    level = gpio_get_level(CLEAR_KEY);
    if (level)
    {
        /* Rising line; key has been released. */
        held_for = time(NULL) - pressed;
        if (held_for > FIRST_JAN_2023)
        {
            /**
             * Time has been set whilst we were pressing the key so we cannot
             * tell how long it was held for; have to just ignore it.
            */
            ESP_LOGI(TAG, "Invalid interval - ignore");
        }
        else if (held_for > FACTORY_RESET_PRESS)
        {
            /**
             * Factory resetting the ESP32c3 will happen on a timer to allow
             * the tlsr8258 to be reset first.
            */
            ESP_LOGI(TAG, "Full factory reset");
            ESP_ERROR_CHECK(esp_event_post_to(
                zigbee_event_loop_handle,
                CS_ZIGBEE_EVENT,
                CS_ZIGBEE_EVENT_FACTORY_RESET,
                NULL,
                0,
                10));

            // TODO: Enable this later...
            /* Delay the ESP32c3 reset to allow ZigBee time to reset first. */
            // ESP_ERROR_CHECK(esp_timer_start_once(
            //    factory_reset_timer_handle, FACTORY_RESET_DELAY));
        }
        else if (held_for > ZIGBEE_RESET_PRESS)
        {
            ESP_LOGI(TAG, "ZigBee only reset");
            ESP_ERROR_CHECK(esp_event_post_to(
                zigbee_event_loop_handle,
                CS_ZIGBEE_EVENT,
                CS_ZIGBEE_EVENT_FACTORY_RESET,
                NULL,
                0,
                10));
        }
        else
        {
            ESP_LOGV(TAG, "Short press; ignore: %lu", (uint32_t)held_for);
        }
    }
    else
    {
        /* Falling line; key has been pressed. */
        pressed = time(NULL);
    }
}

/**
 * Enable GPIO which here is:
 * - Prepare the blue LED and tlsr8258 power GPIOs for output
 * - Ensure that the blue LED is off.
 * - Turn on the tlsr8258 power
 * - Prepare the input key GPIO for input and use pull-up since the key
 *   connects the GPIO to ground.
*/
static void init_gpio(esp_event_loop_handle_t app_event_loop_handle)
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

    /* Turn on the tlsr8258 by setting the GPIO that controls its power. */
    ESP_ERROR_CHECK(gpio_set_level(BLUE_LED, 0));
    ESP_ERROR_CHECK(gpio_set_level(TLSR8258_POWER, 1));

    /* Install the interrupt handler that will be triggered by the input key.*/
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1));
    ESP_ERROR_CHECK(gpio_isr_handler_add(CLEAR_KEY, intr_handler, app_event_loop_handle));

    /* Configure the input key GPIO line used by T-ZigBee. */
    gpio_config_data.pin_bit_mask = (1 << CLEAR_KEY);
    gpio_config_data.mode = GPIO_MODE_INPUT;
    gpio_config_data.pull_up_en =  GPIO_PULLUP_ENABLE;
    gpio_config_data.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config_data.intr_type = GPIO_INTR_ANYEDGE;
    ESP_ERROR_CHECK(gpio_config(&gpio_config_data));
}

/* Callback used as part of delaying the ESP32c3 factory reset. */
static void factory_reset_timer_cb(void *arg)
{
    esp_event_loop_handle_t app_event_loop_handle = (esp_event_loop_handle_t)arg;

    esp_event_post_to(app_event_loop_handle, CS_APP_EVENT, CS_APP_EVENT_FACTORY_RESET, NULL, 0, 10);
}

/**
 * All of what happens after the initial start-up is driven by events being
 * received to this event handler.
*/
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    esp_event_loop_handle_t zigbee_event_loop_handle = (esp_event_loop_handle_t)arg;

    if (event_base == CS_APP_EVENT)
    {
        switch (event_id)
        {
            case CS_APP_EVENT_KEY_ACTION:
                key_action(zigbee_event_loop_handle);
                break;

            case CS_APP_EVENT_FACTORY_RESET:
                cs_cfg_factory_reset();
                esp_restart();
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

void app_main(void)
{
    union cs_create_parms_t create_parms;
    esp_event_loop_handle_t app_event_loop_handle = NULL;
    esp_event_loop_handle_t flash_event_loop_handle = NULL;
    esp_event_loop_handle_t web_event_loop_handle = NULL;
    esp_event_loop_handle_t ota_event_loop_handle = NULL;
    esp_event_loop_handle_t mqtt_event_loop_handle = NULL;
    esp_event_loop_handle_t zigbee_event_loop_handle = NULL;


    // TODO: Remove this.
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

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
        .task_stack_size = 4096,
        .task_core_id = tskNO_AFFINITY
    };
    esp_event_loop_args.task_name = cs_app_task_name;
    ESP_ERROR_CHECK(esp_event_loop_create(&esp_event_loop_args, &app_event_loop_handle));
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

    /*
     * Perform start-of-day checking.
     */
    // TODO: Should be cs_cfg_init();  Also very messy having lots of stuff here!
    cs_cfg_init();
    init_gpio(app_event_loop_handle);

    ESP_LOGI(TAG, "Creating tasks...");
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
    create_parms.zigbee.web_event_loop_handle = web_event_loop_handle;
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
#define NOW  	1703870896
    struct timeval now = { .tv_sec = NOW };
    settimeofday(&now, NULL);

    // TODO: Do minimal work in timer callbacks.
    // REf: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html?highlight=time
    ESP_ERROR_CHECK(esp_event_post_to(mqtt_event_loop_handle, CS_TIME_EVENT, 0, NULL, 0, 0));
}