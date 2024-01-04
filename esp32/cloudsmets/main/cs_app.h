/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 */
#pragma once

/**
 * The T-ZigBee board has a reset key (which just cuts power when pressed) and
 * a second key (near the tlsr8258 side of the board) that is connected to
 * the ESP32c3's GPIO2 line.
*/
typedef enum {
    CS_APP_EVENT_KEY_ACTION = 0,       /*!< Key has been pressed. */
    CS_APP_EVENT_FACTORY_RESET,        /*!< Complete factory reset. */
} cs_app_event_t;

ESP_EVENT_DECLARE_BASE(CS_APP_EVENT);
