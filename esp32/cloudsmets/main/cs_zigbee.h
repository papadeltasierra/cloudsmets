#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(CS_ZIGBEE_EVENT);

/** Time event declarations */
typedef enum {
    CS_ZIGBEE_EVENT_ATTRS = 0,          /**< Attributes. */
    CS_ZIGBEE_EVENT_TIME,               /**< Query time. */
    CS_ZIGBEE_EVENT_CONNECT_TIMER,      /**< Connect check timer. */
    CS_ZIGBEE_EVENT_CONNECTED,          /**< Receiving msgs over ZigBee. */
    CS_ZIGBEE_EVENT_DISCONNECTED,       /**< No messages seen over ZigBee. */
    CS_ZIGBEE_EVENT_FACTORY_RESET       /**< No messages seen over ZigBee. */
} zigbee_event_t;

typedef struct
{
    esp_event_loop_handle_t zigbee_event_loop_handle;
    esp_event_loop_handle_t mqtt_event_loop_handle;
    esp_event_loop_handle_t flash_event_loop_handle;
} cs_zigbee_create_parms_t;

void cs_zigbee_task(cs_zigbee_create_parms_t *create_parms);