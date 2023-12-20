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

#include "cfg.h"
#include "smets.h"

typedef union
{
    // No value is actually sent.
    char dummy[1];
} azure_snd_value_t;

// Received messages.
typedef union
{
    cfg_wifi_t cfg;
    smets_smets_ind_t smets;
} azure_rcv_value_t;

typedef struct
{
    msg_common_t common:
    azure_value_t field:
} azure_rcv_msg_t;

