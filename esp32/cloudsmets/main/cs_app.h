typedef enum {
    CS_APP_EVENT_FACTORY_RESET,         /*!< Reset everything! */
    CS_APP_EVENT_ZIGBEE_RESET,          /*!< Reset ZigBee (new join). */
} cs_app_event_t;

ESP_EVENT_DECLARE_BASE(CS_APP_EVENT);
