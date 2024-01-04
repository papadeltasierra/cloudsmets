/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 */
#pragma once

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(CS_TIME_EVENT);

/** Time event declarations */
typedef enum {
    CS_TIME_EVENT_SET = 0           /**< Time has been set. */
} time_event_t;

