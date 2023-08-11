/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "msg.h"

typedef enum msd_id_t
{
    MSG_LOG_ENABLE  = msg_task.SMETS | 0x0001,
    MSG_LOG_DISABLE = msg_task.SMETS | 0x0002,
} msg_log_id_t;

typedef struct
{
    unsigned short msg_id:
    unsigned short field:
    // TODO: IP address here
    // TODO: Port here
    // TODO: Can we route error logs to the cloud services?  Probably not since
    //       we cannot see the severity at the point the logs reach here.
} msg_log_t;
