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
#include "esp_event.h"

typedef struct
{
    esp_event_loop_handle_t web_event_loop_handle;
    esp_event_loop_handle_t ota_event_loop_handle;
} cs_web_create_parms_t;

extern const char *cs_web_task_name;

extern void cs_web_task(cs_web_create_parms_t *create_parms);