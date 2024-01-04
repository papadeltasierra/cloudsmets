/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 */
#pragma once

typedef struct
{
    esp_event_loop_handle_t mqtt_event_loop_handle;
    esp_event_loop_handle_t flash_event_loop_handle;
} cs_mqtt_create_parms_t;

void cs_mqtt_task(cs_mqtt_create_parms_t *create_parms);