typedef struct
{
    esp_event_loop_handle_t ota_event_loop_handle;
    esp_event_loop_handle_t wifi_event_loop_handle;
} cs_ota_create_parms_t;

void cs_ota_task(cs_ota_create_parms_t *create_parms);
