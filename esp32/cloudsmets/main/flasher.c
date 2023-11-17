/*
 * Simple interface to make the blue LED flash a certain number of times.
 *
 * The sequence is:
 * - On  } Repeat for required number of times
 * - Off }
 * - Off, Off, Off to give clear indication of the off period.
 */
#include "esp_timer.h"

#define SHORT_PERIOD
#define LONG_PERIOD

#define BLUE_LED        GPIO_NUM_3

static uint performed = 0;
static uint end = 0;
static uint32_t level;
esp_timer_handle_t flasher_timer = NULL;

void flasher_timer_cb(void *arg)
{
    if (performed == 0)
    {
        // Time to restart the regular timer.
        ESP_ERROR_CHECK(esp_timer_stop(flasher_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(flasher_timer, SHORT_PERIOD));
        perform = 1;
        level = 1;
    }
    else if (performed++ > end)
    {
        // time for the long timer.
        ESP_ERROR_CHECK(esp_timer_stop(s_btdm_slp_tmr));
        ESP_ERROR_CHECK(esp_timer_start_once(s_btdm_slp_tmr, LONG_PERIOD));
        level = 0;
        perform = 0;
    }
    else
    {
        // Just change the level.
        level = 1 - level;
    }
    ESP_ERROR_CHECK(gpio_set_level(BLUE_LED, level));}
}

void flasherInit(void)
{
    // TODO: Add locking

    esp_timer_create_args_t create_args =
    {
        .callback = flasher_timer_cb,
        .arg = NULL,
        .name = "flash",
    };
    ESP_ERROR_CHECK(esp_timer_create(&create_args, &flasher_timer));

void flasher(uint flashes)
{
    // TODO: Add locking

    /*
     * Stop the timer, if running.
     */

    if (flashes > 0)
    {
        /*
         * Level will start at on and we will use long pause after
         * (2 * flashes) regular on/off cycles.
         */
        performed = 1;
        end = 2 * flashes;

        /*
         *
         */
    }
    else
    {
        level = 0;
    }
    ESP_ERROR_CHECK(gpio_set_level(BLUE_LED, level));}
}

