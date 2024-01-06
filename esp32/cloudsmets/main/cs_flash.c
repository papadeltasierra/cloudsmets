/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 *
 * Note that this API operates on the default NVRAM partition.
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "mqtt_client.h"

#include "cs_log.h"
#include "cs_gpio.h"
#include "cs_cfg.h"
#include "cs_flash.h"
#include "cs_zigbee.h"

#define TAG cs_flash_task_name

ESP_EVENT_DEFINE_BASE(CS_FLASH_EVENT);

/* This is a little naughty, but we know it does exist! */
ESP_EVENT_DECLARE_BASE(MQTT_EVENTS);

#define STATUS_WIFI_SOFTAP_STARTED         0x01
#define STATUS_WIFI_AP_CONNECTED           0x02
#define STATUS_MQTT_CONNECTED              0x04
#define STATUS_ZIGBEE_CONNECTED            0x08

#define ALL_ZIGBEE_ACTIVE           0x0F
#define ALL_MQTT_ACTIVE             0x07
#define ALL_WIFI_AP_ACTIVE          0x03
#define ALL_WIFI_SOFTAP_ACTIVE      0x01

static int flashes_per_cycle = 0;

static void led_on()
{
    ESP_ERROR_CHECK(gpio_set_level(BLUE_LED, 1));
}

static void led_off()
{
    ESP_ERROR_CHECK(gpio_set_level(BLUE_LED, 0));
}

#define FLASH_TIME_PERIOD ((uint64_t)1000 * 500)
static void led_flash(esp_timer_handle_t flash_timer_handle, int flashes)
{
    // TODO: Much nicer if we get rid of globals.
    flashes_per_cycle = flashes;
    if (!esp_timer_is_active(flash_timer_handle))
    {
        esp_timer_start_periodic(flash_timer_handle, FLASH_TIME_PERIOD);
    }
}

/**
 * The timer is intended to flash as follows:
 * on, off, off, off
 * on, off, on, off, off, off
 * on, off, on, off, on, off, off, off
 * So a sequence of X (on, off) followed by (off, off).
*/
static void flash_timer_action(void)
{
    static int count = 0;
    static bool lit = false;

    if (lit)
    {
        /* From lit, always off. */
        led_off();
    }
    else
    {
        if (count)
        {
            led_on();
        }
        count--;
        if (count <= -1)
        {
            /* Time for next cycle but LED stays off for this cycle. */
            // TODO: How do we get rid of this global?
            count = flashes_per_cycle;
        }
    }
    lit = !lit;
}

static void flash_timer_cb(void *arg)
{
    esp_event_loop_handle_t flash_event_loop_handle = (esp_event_loop_handle_t)arg;

    esp_event_post_to(flash_event_loop_handle, CS_FLASH_EVENT, CS_FLASH_EVENT_FLASH_TIMER, NULL, 0, 10);
}

static void update_flash_status(esp_timer_handle_t flash_timer_handle, uint8_t status)
{
    ESP_LOGV(TAG, "status: %2.2X", status);
    if ((status & ALL_ZIGBEE_ACTIVE) == ALL_ZIGBEE_ACTIVE)
    {
        esp_timer_stop(flash_timer_handle);
        led_on();
    }
    else if ((status & ALL_MQTT_ACTIVE) == ALL_MQTT_ACTIVE)
    {
        led_flash(flash_timer_handle, 3);
    }
    else if ((status & ALL_WIFI_AP_ACTIVE) == ALL_WIFI_AP_ACTIVE)
    {
        led_flash(flash_timer_handle, 2);
    }
    else if ((status & ALL_WIFI_SOFTAP_ACTIVE) == ALL_WIFI_SOFTAP_ACTIVE)
    {
        led_flash(flash_timer_handle, 1);
    }
    else
    {
        esp_timer_stop(flash_timer_handle);
        led_off();
    }
}

/***
 * Status of the device will be identified as follows:
 * - No light, not even SoftAP is ready.
 * - Single flash, SoftAP web server is available
 * - Double flash, also connected to AP (router)
 * - Triple flash, also connected to Azure IotHub
 * - Solid blue, also receiving and relaying ZigBee messages.
*/
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    esp_timer_handle_t flash_timer_handle = (esp_timer_handle_t)arg;

    static uint8_t status = 0;

    ESP_LOGV(TAG, "Event: %s, %d", event_base, event_id);

    if (event_base == CS_FLASH_EVENT)
    {
        switch (event_id)
        {
            case CS_FLASH_EVENT_FLASH_TIMER:
                flash_timer_action();
                break;

            default:
                ESP_LOGE(TAG, "Unexpected flash event: %d", event_id);
                break;
        }
    }
    else if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_AP_START:
                status |= STATUS_WIFI_SOFTAP_STARTED;
                break;

            case WIFI_EVENT_AP_STOP:
                status &= ~STATUS_WIFI_SOFTAP_STARTED;
                break;

            case WIFI_EVENT_STA_CONNECTED:
                status |= STATUS_WIFI_AP_CONNECTED;
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                status &= ~STATUS_WIFI_AP_CONNECTED;
                break;

            default:
                ESP_LOGE(TAG, "Unexpected WiFi event: %d", event_id);
                break;
        }
    }
    else if (event_base == MQTT_EVENTS)
    {
        switch (event_id)
        {
            case MQTT_EVENT_CONNECTED:
                status |= STATUS_MQTT_CONNECTED;
                break;

            case MQTT_EVENT_DISCONNECTED:
                status &= ~STATUS_MQTT_CONNECTED;
                break;

            default:
                ESP_LOGE(TAG, "Unexpected MQTT event: %d", event_id);
                break;
        }
    }
    else if (event_base == CS_ZIGBEE_EVENT)
    {
        switch (event_id)
        {
            case CS_ZIGBEE_EVENT_CONNECTED:
                status |= STATUS_ZIGBEE_CONNECTED;
                break;

            case CS_ZIGBEE_EVENT_DISCONNECTED:
                status &= ~STATUS_ZIGBEE_CONNECTED;
                break;

            default:
                ESP_LOGE(TAG, "Unexpected ZigBee event: %d", event_id);
                break;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event: %s, %d", event_base, event_id);
    }
    ESP_LOGV(TAG, "Update flasher status");
    update_flash_status(flash_timer_handle, status);
}

void cs_flash_task(cs_flash_create_parms_t *create_parms)
{
    esp_event_loop_handle_t flash_event_loop_handle = create_parms->flash_event_loop_handle;
    esp_timer_handle_t flash_timer_handle = NULL;

    // TODO: Remove this or perhaps replace with config?
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    /**
     * Create the timer that will make the LED flash.
    */
    esp_timer_create_args_t timer_args =
    {
         .callback = flash_timer_cb,
         .name = "Flash timer",
         .arg = flash_event_loop_handle,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &flash_timer_handle));

    /**
     * We can listen for WiFi events but only on the default event loop and
     * we cannot register for MQTT events because they are obfuscated.  So we
     * rely on the various components forwarding events to our handler.
    */
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    flash_event_loop_handle,
                    WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    flash_timer_handle,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    flash_event_loop_handle,
                    MQTT_EVENTS,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    flash_timer_handle,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                    flash_event_loop_handle,
                    CS_ZIGBEE_EVENT,
                    ESP_EVENT_ANY_ID,
                    event_handler,
                    flash_timer_handle,
                    NULL));

}