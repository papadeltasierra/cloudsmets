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

#include "cfg.h"
#include "msg.h"

typedef enum msd_id_t
{
    MSG_SMETS_CONNECT = msg_task.SMETS | 0x0001,
    MSG_SMETS_STATUS  = msg_task.SMETS | 0x0002,
    MSG_SMETS_USAGE   = msg_task.SMETS | 0x0003,
} smets_msg_id_t;

unsigned char enum
{
    DISCONNECTED = 0,
    COMNNECTED = 1,
} zigbee_conn_t;

typedef unsigned char zigbee_channel_t;
typedef unsigned char zigbee_key_t[8];
typedef unsigned char zigbee_guid_t[8];

// TODO: How does the webserver manage to query this?
// Sent messages.
typedef struct
{
    zigbee_conn_t connected;
    zigbee_channel_t channel;
    sigbee_guid_t local_guid;
    sigbee_guid_t gateway_guid;
    zigbee_key_t local_key;
    zigbee_key_t gateway_key;
} smets_zigbee_status_t;

typedef struct
{
    char field[1];
} smets_smets_ind_t;

// Received messages.
typedef union
{
    cfg_zigbee_t zigbee;
} smets_rcv_value_t;

typedef struct
{
    common_msg_t common:
    smets_rcv_value_t value;
} smets_rcv_msg_t;
