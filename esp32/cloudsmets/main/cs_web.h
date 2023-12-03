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

typedef enum msg_id_t
{
    MSG_WEB_REBOOT = msg_task.WEB | 0x0001,
    MSG_WEB_RESET  = msg_task.WEB | 0x0001,
    MSG_WEB_CONFIG = msg_task.WEB | 0x0002,
} msg_web_id_t;

typedef struct
{
    unsigned short msg:
    unsigned short field:
} msg_web_t;