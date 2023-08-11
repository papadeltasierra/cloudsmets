/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Watchdog message.
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "msg.h"
#include "cfg.h"

// Sent message values.
typedef enum
{
    MSG_HBWD_ACTIVE = msg_task.HBWD | 0x0001,
} hbwd_msg_id_t;

typedef struct
{
    common_msg_t common;
} hbwd_recv_msg_t;
