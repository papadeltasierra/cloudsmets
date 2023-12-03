/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#pragma once

#include <stdio.h>

typedef struct
{
    esp_event_loop_handle_t dummy;
} cs_wifi_create_parms_t;

extern const char *cs_wifi_task_name;

// The actual task.
void cs_wifi_task(cs_wifi_create_parms_t *arg);

