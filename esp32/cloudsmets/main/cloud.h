/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
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
    MSG_CLOUD_SMETS = msg_task.CLOUD | 0x0001,
    MSG_CLOUD_LOG   = msg_task.CLOUD | 0x0002,
} cloud_msg_id_t;

typedef struct
{
    unsigned short msg:
    unsigned short field:
} msg_cloud_t;
