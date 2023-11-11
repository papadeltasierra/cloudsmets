/* UART Relay Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/**
 * This is an example which relays any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define RELAY_TEST_TXD0 (CONFIG_EXAMPLE_UART0_TXD)
#define RELAY_TEST_RXD0 (CONFIG_EXAMPLE_UART0_RXD)
#define RELAY_TEST_CTS0 (UART_PIN_NO_CHANGE)
#define RELAY_TEST_RTS0 (UART_PIN_NO_CHANGE)
#define RELAY_TEST_TXD1 (CONFIG_EXAMPLE_UART1_TXD)
#define RELAY_TEST_RXD1 (CONFIG_EXAMPLE_UART1_RXD)
#define RELAY_TEST_CTS1 (UART_PIN_NO_CHANGE)
#define RELAY_TEST_RTS1 (UART_PIN_NO_CHANGE)

#define RELAY_UART_PORT0_NUM     (CONFIG_EXAMPLE_UART0_PORT_NUM)
#define RELAY_UART_PORT1_NUM     (CONFIG_EXAMPLE_UART1_PORT_NUM)
#define RELAY_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define RELAY_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define TLSR8258_POWER  GPIO_NUM_0
#define BLUE_LED        GPIO_NUM_3

// We are not logging.
// static const char *TAG = "UART RELAY";

#define BUF_SIZE (1024)

static void relay_task(void *arg)
{
    // Sleep to give bootloader time to complete otherwise the UART ports
    // seem to get confused.
    vTaskDelay(configTICK_RATE_HZ * 2);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = RELAY_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(RELAY_UART_PORT0_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(RELAY_UART_PORT0_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(RELAY_UART_PORT0_NUM, RELAY_TEST_TXD0, RELAY_TEST_RXD0, RELAY_TEST_RTS0, RELAY_TEST_CTS0));

    ESP_ERROR_CHECK(uart_driver_install(RELAY_UART_PORT1_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(RELAY_UART_PORT1_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(RELAY_UART_PORT1_NUM, RELAY_TEST_TXD1, RELAY_TEST_RXD1, RELAY_TEST_RTS1, RELAY_TEST_CTS1));

    // Give 1/2 seconds for the pins to settle down before we power up the tlsr8258.
    vTaskDelay(configTICK_RATE_HZ / 2);

    // Turn on the power to the tlsr8258
    ESP_ERROR_CHECK(gpio_set_level(TLSR8258_POWER, 1));

    // Turn on the blue LED connected to the ESP32-C3
    ESP_ERROR_CHECK(gpio_set_level(BLUE_LED, 1));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    int len;

    // uart_write_bytes(RELAY_UART_PORT0_NUM, (const char *)"Hello Port 0\r\n", 14);
    // uart_write_bytes(RELAY_UART_PORT1_NUM, (const char *)"Hello Port 1\n\n", 14);

    while (1) {
        // Read data from UART0
        len = uart_read_bytes(RELAY_UART_PORT0_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        // Relay the data to UART1
        if (len)
        {
            uart_write_bytes(RELAY_UART_PORT1_NUM, (const char *) data, len);
            // uart_write_bytes(RELAY_UART_PORT1_NUM, (const char *)"==", 2);
            // uart_write_bytes(RELAY_UART_PORT0_NUM, (const char *)"<<", 2);
        }

        // Read data from UART1
        len = uart_read_bytes(RELAY_UART_PORT1_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        // Relay the data to UART0
        if (len)
        {
            uart_write_bytes(RELAY_UART_PORT0_NUM, (const char *) data, len);
            // uart_write_bytes(RELAY_UART_PORT0_NUM, (const char *)"++", 2);
            // uart_write_bytes(RELAY_UART_PORT1_NUM, (const char *)">>", 2);
        }
    }
}

void app_main(void)
{
    // Configure the GPIO fields used by T-ZigBee.
    gpio_config_t gpio_config_data = {
        .pin_bit_mask = (1 << BLUE_LED) | (1 << TLSR8258_POWER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en =  GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&gpio_config_data));

    // Create the relay task.
    xTaskCreate(relay_task, "uart_relay_task", RELAY_TASK_STACK_SIZE, NULL, 10, NULL);
}
