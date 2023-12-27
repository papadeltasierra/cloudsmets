/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 *
 * Note that this API operates on the default NVRAM partition.
 */
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(CS_TIME_EVENT);

/** Time event declarations */
typedef enum {
    CS_TIME_EVENT_SET = 0           /**< Time has been set. */
} time_event_t;

