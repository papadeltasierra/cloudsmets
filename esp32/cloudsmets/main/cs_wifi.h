/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 */
#pragma once

#include <stdio.h>

typedef struct
{
    esp_event_loop_handle_t flash_event_loop_handle;
} cs_wifi_create_parms_t;

extern const char *cs_wifi_task_name;

// The actual task.
void cs_wifi_task(cs_wifi_create_parms_t *arg);

