/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * MQTT communication with the Azure IotHub.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include <sys/time.h>
#include <limits.h>

/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "driver/uart.h"
#include "driver/gpio.h"

/**
 * CloudSMETS own header files.
*/
#include "cs_cfg.h"
#include "cs_time.h"
#include "cs_zigbee.h"

/**
 * The following header files are lifted straight from the Telink SDK.
 * // TODO: Can we use the same ones that we use for the tlsr8258 build?
*/
#include "telink/zbhci.h"
#include "telink/zcl_const.h"
#include "telink/zcl_time.h"
#include "telink/utility.h"

#define TAG cs_zigbee_task_name

/**
 * Define the ZIGBEE events base.
*/
ESP_EVENT_DEFINE_BASE(CS_ZIGBEE_EVENT);

// TODO: What is the size of the biggest message that we can expect to receive?
#define RX_BUFFER_SIZE 256
#define RX_MINIMUM_FRAME_SIZE 10
#define RX_PAYLOAD_LENGTH_MAX 256


static esp_event_loop_handle_t mqtt_event_loop_handle;
static esp_event_loop_handle_t zigbee_event_loop_handle;

/**
 * Calculate the CRC for the received frame.
*/
static bool crc8_calculate(uint16_t data_type, uint16_t length, uint8_t *data)
{
    uint8_t crc;

    crc = data_type & 0x0ff
    crc ^= (data_type >> 8) & 0x0ff
    crc ^= (data_type >> 8) & 0x0ff
    crc ^= length & 0x0ff
    crc ^= (length >> 8) & 0x0ff

    while (length--)
    {
        crc ^= (*data++);
    }

    return crc;
}

// TODO: Validate length and ensure not stupid.
// TODO: Set sensible length for

static void zbhci_forward_attributes(uint16_t payload_length, uint8_t *frame)
{
    ESP_LOGV(TAG, "Fwd attrs to MQTT: %u", payload_length);
    esp_event_post_to(mqtt_event_loop_handle, CS_ZIGBEE_EVENT, CS_ZIGBEE_EVENT_ATTRS, frame, payload_length, 10);
}

/**
 * Structure to make parsing the start of an attribute response simpler.
*/
typedef struct{
    uint8_t num;
    uint8_t attrId16H;
    uint8_t attrId16L;
    uint8_t status;
    uint8_t dataType;
    uint8_t pValue[1];
} _zbhci_attr_t;

/**
 * Most attribute information we just pass straight through to Azure over
 * MQTT but we use the time attribute here to set local time.
*/
static void zbhci_read_attr_rsp(uint16_t command_id, uint16_t payload_length, uint8_t *frame)
{
    static bool time_is_set = false;
    uint8_t num_attr;
    uint8_t data_type;
    uint16_t attr_id;
    uint32_t zigbee_time;
    uint32_t linux_time;
    uint8_t *bptr;
    zbhci_msg_t *msg;
    _zbhci_attr_t *attr;
    struct timeval set_or_adjust = { 0 };

    msg = (zbhci_msg_t *)frame;
    attr = (_zbhci_attr_t *)&msg->pData[0];
    attr_id = (attr->attrId16H << 8) & attr->attrId16L;

    /**
     * When we read time we read a single attribute and if this response contains
     * more than 1 attribute or a different attribute, we just pass it on to
     * Azure via MQTT.
    */
    if ((attr->num != 1) || (attr_id != ZCL_ATTRID_STANDARD_TIME))
    {
        /* Definitely not time so pass to Azure. */
        zbhci_forward_attributes(command_id, payload_length, frame);
        return;
    }

    if (attr->status != ZCL_STA_SUCCESS)
    {
        /* Something is wrong with this time value! */
        ESP_LOGE(TAG, "Attr status bad: %2.2X", attr->status);
        return;
    }

    if (attr->dataType != ZCL_DATA_TYPE_UTC)
    {
        ESP_LOGE(TAG, "Time type bad: %2.2X", attr->dataType);
        return;
    }

    /* Extract the ZigBee time. */
    zigbee_time = BUILD_U32(attr->pValue[3], attr->pValue[2], attr->pValue[1], attr->pValue[0]);

    /**
     * Linux time starts on 1-Jan-1970 but ZigBee time starts on 1-Jan-2000
     * so we have to adjust!  At the Unix epoch timestamp for 00:00:00, 1 Jan 2000.
     */
#define ZIGBEE_TIME_TO_LINUX_TIME   946684800
    linux_time = zigbee_time + ZIGBEE_TIME_TO_LINUX_TIME;
    ESP_LOGV(TAG, "Time: %lu", linux_time);

    if (time_is_set)
    {
        /* Time has been set once so we might adjust it if required. */
        set_or_adjust.tv_sec = linux_time - time();
        adjtime(&set_or_adjust, NULL);
    }
    else
    {
        /* First time info received so adjust and notify MQTT/Azure. */
        time_is_set = true;
        set_or_adjust.tv_sec = linux_time;
        settimeofday(&set_or_adjust, NULL);

        /**
         * Finally let MQTT/Azure component know that time is now set.
        */
       esp_event_post_to(mqtt_event_loop_handle, CS_TIME_EVENT, CS_TIME_EVENT_SET, NULL, 0, 10);
    }
}

/**
 * Before reaching here, the ZCL frame has passed all length and CRC checks so
 * should be valid.  What we do now depends on the command_id.
*/
static void zbhci_message(uint16_t command_id, uint16_t payload_length, uint8_t *frame)
{
    switch (command_id)
    {
        case ZBHCI_CMD_BDB_GET_LINK_KEY_RSP:
            /* ZigBee link encryption keys. */
            break;

        case ZBHCI_CMD_DISCOVERY_IEEE_ADDR_RSP:
            /* Discovering useful ZigBee endnodes. */
            break;

        case ZBHCI_CMD_ZCL_ATTR_READ_RSP:
            /* Read-attribute response; the data we really want! */
            zbhci_read_attr_rsp(command_id, payload_length, frame);
            break;

        default:
            ESP_LOGW(TAG, "Unexpected ZBHCI command: %4.4X", command_id);
            break;
    }
}

/**
 * By the time we reach here we should be happy that the frame consists of:
 * - 0x55       "frame-start"
 * - 0x????     command_id
 * - 0x????     payload length
 * - 0x??       CRC byte over command id, payload length and payload
 * - ??         Payload (payload length bytes)
 * - 0xAA       "frame-end"
 *
 * Note that 0x55 and 0xAA can appear inside the frame too which is why the
 * "frame-start/frame-end" are quoted - they are not unique!
 *
 * Returns "true" if the frame was processed or "false" if an error was detected.
*/
static void zbhci_frame(uint16_t payload_length, uint8_t *frame)
{
    uint16_t command_id;
    uint8_t payload;
    uint8_t crc;
    bool rc = true;
    zbhci_msg_t *msg;

    msg = (zbhci_msg_t *)frame;

    /* Extract the command_id. */
    command_id = (msg->msgType16H << 8) & msg->msgType16L;

    /* Calculate and check the CRC. */
    crc = crc_calculate(command_id, payload_length, &msg->pData[0]);
    if (crc != msg->checkSum)
    {
        ESP_LOGE(TAG, "CRC mismatch: %2.2X, %2.2X", crc, msg->checkSum);
        return false;
    }

    /* Process the ZCL message. */
    return zcl_message(command_id, payload_length, frame);
}

uint32_t zbhci_maybe_frame(uint32_t data_length, uint8_t *buffer)
{
    bool processing = true;
    uint32_t data_remains = data_length;
    uint32_t data_accepted = 0;
    uint16_t payload_length;
    zbhci_msg_t *msg;

    while (processing)
    {
        /**
         * We expect to find the message start flag so confirm we do and discard
         * bytes if we do not.
        */
        while ((*buffer++ != ZBHCI_MSG_START_FLAG) && (data_remains--));
        if (data_remains == 0)
        {
            /* Found not start message marker; discard all bytes. */
            return data_length;
        }

        /**
         * Next we check to see if the length makes sense.  Note that we stepped
         * forward a, extra byte in the "start message" loop so step back.
         */
        buffer--;
        msg = (zbhci_msg_t *)buffer;
        payload_length = (msg->msgLen16H << 8) + msg->msgLen16L;
        if (payload_length > RX_PAYLOAD_LENGTH_MAX)
        {
            /**
             * Clearly not a valid message so ignore and look for another
             * start-message marker.
            */
           buffer++;
           data_remains--;
           continue;
        }
        else
        {
            /**
             * Do we have the complete frame of data available?  The frame
             * consists of the payload plus 7 "framing" bytes.
            */
            if (data_remains >= (payload_length + ZBHCI_FRAMING_BYTES))
            {
                /* We have a complete frame; try and process it. */
                zbhci_frame(payload_length, buffer);

                data_accepted = data_length - data_remains + payload_length + ZBHCI_FRAMING_BYTES
                return data_accepted;
            }
            else
            {
                /* Need more data before we can continue. */
                return 0;
            }
        }
    }
}

void cs_zigbee_task(cs_zigbee_create_parms_t *create_parms)
{
    uint32_t bytes_to_read = RX_BUFFER_SIZE;
    uint32_t bytes_present = 0;
    uint32_t bytes_read;
    uint8_t  *buffer;
    uint8_t  *bPtr;

    // TODO: Remove this or perhaps replace with config?
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    /* Allocate a buffer for receipt. */
    buffer = (uint8_t *)malloc(RX_BUFFER_SIZE);

    ESP_LOGI(TAG, "Init. MQTT task");
    zigbee_event_loop_handle = create_parms->zigbee_event_loop_handle;

    ESP_LOGI(TAG, "Register event handlers");

    // TODO: Any use for a loop handler?
    // /* Register Event handler */
    // ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
    //                 zigbee_event_loop_handle,
    //                 CS_ZIGBEE_EVENT,
    //                 ESP_EVENT_ANY_ID,
    //                 event_handler,
    //                 NULL,
    //                 NULL));

    /**
     * Processing below follows the order show in example:
     *   peripherals/uart/uart_events/main/uart_events_example_main.c
    */
#define TLSR8258_BAUD_RATE 115200
    const uart_port_t uart_num = UART_NUM_0;
    const uart_config_t uart_config = {
        .baud_rate = TLSR8258_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

QueueHandle_t uart_queue_handle;
// TODO: Understand this.
#define TLSR8258_MIN_MSG_SIZE   1
#define TLSR8258_MAX_MSG_SIZE   2
#define UART_QUEUE_DEPTH 5
    /* Configure interupts, just for RX. */
    ESP_ERROR_RCHECK(uart_driver_install(
        UART_NUM_0, TLSR8258_MAX_MSG_SIZE, TLSR8258_MAX_MSG_SIZE, UART_QUEUE_DEPTH, &uart_queue_handle, 0));

    /* Configure UART parameters. */
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    /**
     * Set UART pins.  Note that these are reversed from the T-ZigBee documentation
     * because we are talking to the tlsr8258 pins and connections are nominally
     * RX/RX, TX/TX so we switch the ESP32 TX/RX.
     */
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, GPIO_NUM_18, GPIO_NUM_19, -1, -1));

    /**
     * Initially assume we don't need an interrupt handler and can run things
     * using a tight loop and time-outs.
    */
#ifdef UART_INTR
    const uart_intr_config_t uart_intr_config = {
        .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M |    // Rx data available
                            UART_RXFIFO_OVF_INT_ENA_M,      // Rx FIFO oveflow
        .rxfifo_full_thresh = TLSR8258_MIN_MSG_SIZE
    };

    ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM_0));
    ESP_ERROR_CHECK(esp_intr_alloc(
        EST_UART_INTR_SOURCE, 0, zigbee_rx_intr_handler, NULL, NULL));
#endif

/* We simply tight-loop here with the read timeout set to 1s. */
#define TICKS_TO_WAIT   configTICK_RATE_HZ

    /**
     * Now we need to...
     *
     * - Periodically send requests to the ZigBee network.
     * - Accept responses from the ZigBee network and process them.
     * - Forward responses to MQQ except for time responses which we process
     *   locally to set/update time.
     *
     * We will send requests independently using timers.
     *
     * We will just accept responses and pass them along, assuming we recognise
     * them.
    */
    while (true)
    {
        bPtr = buffer + bytes_present;
        bytes_to_read = RX_BUFFER_SIZE - bytes_present;
        bytes_read = uart_read_bytes(UART_NUM_0, bPtr, bytes_to_read, TICKS_TO_WAIT);
        if (bytes_present == -1)
        {
            ESP_LOGE(TAG, "Rx error.");
            ESP_ERROR_CHECK(ESP_FAIL);
        }
        bytes_present += bytes_read;
        if ((bytes_read) && (bytes_present > RX_MINIMUM_FRAME_SIZE))
        {
            /**
             * Processing a possible frame returns the number of bytes
             * that were accepted, which might be zero if there is insufficient
             * bytes for the complete frame.
            */
            bytes_read = zbhci_maybe_frame(buffer);
            bytes_present -= bytes_read;

            if (bytes_present)
            {
                /* Shuffle data down ready for the next frame. */
                memmove(buffer, &buffer[bytes_read], bytes_present);
            }
        }
    }
}