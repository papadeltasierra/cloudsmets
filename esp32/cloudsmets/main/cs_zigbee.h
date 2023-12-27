#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(CS_ZIGBEE_EVENT);

/** Time event declarations */
typedef enum {
    CS_ZIGBEE_EVENT_ATTRS = 0           /**< Attributes. */
} zigbee_event_t;

typedef struct
{
    esp_event_loop_handle_t zigbee_event_loop_handle;
    esp_event_loop_handle_t mqtt_event_loop_handle;
} cs_zigbee_create_parms_t;

void cs_zigbee_task(cs_mqtt_create_parms_t *create_parms);