#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

/**
* @brief CloudSMETS configuration declarations
*
*/
typedef enum {
    CS_FLASH_EVENT_FLASH_TIMER,         /*!< Connected to Azure IotHub? */
} cs_flash_event_t;

ESP_EVENT_DECLARE_BASE(CS_FLASH_EVENT);

typedef struct
{
    esp_event_loop_handle_t flash_event_loop_handle;
} cs_flash_create_parms_t;

void cs_flash_task(cs_flash_create_parms_t *create_parms);

