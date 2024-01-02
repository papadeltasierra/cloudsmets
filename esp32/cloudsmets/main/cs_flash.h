/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
*/

typedef enum {
    CS_FLASH_EVENT_FLASH_TIMER,         /* Event from the timer callback. */
} cs_flash_event_t;

ESP_EVENT_DECLARE_BASE(CS_FLASH_EVENT);

typedef struct
{
    esp_event_loop_handle_t flash_event_loop_handle;
} cs_flash_create_parms_t;

void cs_flash_task(cs_flash_create_parms_t *create_parms);