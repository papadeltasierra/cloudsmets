/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Message types.
 */
#pragma once

typedef short unsigned msg_id_t;

typedef enum msg_id_t =
{
    WEB =   0x0100,
    NTP =   0x0200,
    WIFI =  0x0300,
    SMETS = 0x0400,
    CFG =   0x0500,
    CLOUD = 0x0600,
    AZURE = 0x0700,
    AWS =   0x0800,
    GCS =   0x0900
} msg_task;
