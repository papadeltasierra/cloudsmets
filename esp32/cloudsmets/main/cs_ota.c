/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_netif_ip_addr.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "cs_cfg.h"
#include "cs_ota.h"

#define TAG cs_wifi_task_name

void cs_ota_task(cs_ota_create_parms_t *create_parms)
{
    ESP_LOGI(TAG, "Creating OTA task");
    create_parms = create_parms;

    // TODO: OTA confirmation perdio should perhaps be configurable.
}
