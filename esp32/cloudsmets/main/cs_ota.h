ESP_EVENT_DECLARE_BASE(CS_OTA_EVENT);

/** Time event declarations */
typedef enum {
    CS_OTA_EVENT_ACCEPTANCE_TIMER = 0,          /**< Attributes. */
    CS_OTA_EVENT_START_OTA_TIMER,          /**< Attributes. */
} ota_event_t;

typedef struct
{
    esp_event_loop_handle_t ota_event_loop_handle;
    esp_event_loop_handle_t wifi_event_loop_handle;
} cs_ota_create_parms_t;

void cs_ota_task(cs_ota_create_parms_t *create_parms);

