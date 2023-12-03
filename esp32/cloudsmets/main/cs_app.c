/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Application mainline and heartbeat/watchdog.
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"

// #include "ft_err.h"
#include "pd_err.h"

// Tasks.
#include "config.h"

#define MAIN "main"
#define TAG "HB"
#define QUEUE_DEPTH 10

#define Q_ERROR_CHECK(QUEUE, TASK)                                            \
    if ((QUEUE) == 0)                                                         \
    {                                                                         \
        ESP_LOGE(TAG, "Queue creation for '%s' task failed.", (TASK));        \
        ESP_ERROR_CHECK(ESP_FAIL);                                            \
    }



typedef struct
{
    int msgType;
    QueueHandle_t cloud_q;
    QueueHandle_t main_q;
} CloudCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t azure_q;
    QueueHandle_t cloud_q;
} AzureCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t zigbee_q;
    QueueHandle_t cloud_q;
} ZigBeeCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t zbhci_q;
    QueueHandle_t zigbee_q;
} ZbhciCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t stn_q;
    QueueHandle_t wifi_q;
} StnCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t softap_q;
    QueueHandle_t wifi_q;
} SoftApCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t wifi_q;
    QueueHandle_t softap_q;
    QueueHandle_t station_q;
    QueueHandle_t web_q;
} WiFiCreateMsg_t;

typedef struct
{
    int msgType;
    QueueHandle_t stn_q;
    QueueHandle_t web_q;
    QueueHandle_t softap_q;
    QueueHandle_t azure_q;
} WebCreateMsg_t;

typedef struct {
    int msgType;
} StartMsg_t;

union {
    CloudCreateMsg_t cloud;
    AzureCreateMsg_t azure;
    ZigBeeCreateMsg_t zigbee;
    ZbhciCreateMsg_t zbhic;
    StnCreateMsg_t stn;
    SoftApCreateMsg_t softap;
    WiFiCreateMsg_t wifi;
    WebCreateMsg_t web;
    StartMsg_t start;
} Msg_t;


void app_main(void)
{
    static SOMETHING webTaskParms;
    static QueueHandle_t main_q = 0;
    static QueueHandle_t wifi_q = 0;
    Msg_t msg;
    /*
     * The remaining queue handles are only used during initialization.
     */
    QueueHandle_t
    /*
     * Perform start-of-day checking.
     */
    flasher(1);
    cfgOpen();
    startOfDay();

    /*
     * Now start the processes that form CloudSMETs.  To do this we use this process:
     * - Create all the queues on which the tasks will listen
     * - On creation, pass each task the queue that it will listen on plus
     *   any queues that it needs to send to in oder to communicate with other
     *   processes.
     */
    ESP_LOGI(TAG, "Creating queues...");
    main_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    cloud_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    azure_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    zigbee_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    zbhci_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    wifi_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    stn_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    softap_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    web_q = xQueueCreate(QUEUE_DEPTH, sizeof(unsigned long) );
    Q_ERROR_CHECK(cloud_q, "Cloud");
    Q_ERROR_CHECK(azure_q, "Azure");
    Q_ERROR_CHECK(zigbee_q, "ZigBee");
    Q_ERROR_CHECK(zbhci_q, "zbhci");
    Q_ERROR_CHECK(wifi_q, "Wifi");
    Q_ERROR_CHECK(stn_q, "Stn");
    Q_ERROR_CHECK(softap_q, "SoftAP");
    Q_ERROR_CHECK(web_q, "Web");

    /*
     * Note that we don't care about the task handles.
     */
    ESP_LOGI(TAG, "Creating tasks...");
    msg.cloud.type = MSG_TASK_CREATE;
    msg.cloud.cloud_q = cloud_q;
    msg.cloud.main_q = main_q;
    PD_ERROR_CHECK(xTaskCreate(cloudTask, "Cloud", 4096, &msg, 10, &myTaskHandle));

    msg.azure.azure_q = azure_q;
    msg.azure.cloud_q = cloud_q;
    PD_ERROR_CHECK(xTaskCreate(azTask, "Azure", 4096, &msg, 10, &myTaskHandle));

    msg.zigbee.zigbee_q = zigbee_q;
    msg.zigbee.cloud_q = cloud_q;
    PD_ERROR_CHECK(xTaskCreate(zbTask, "ZigBee", 4096, &msg, 10, &myTaskHandle));

    msg.wifi.wifi_q = wifi_q;
    msg.wifi.main_q = main_q;
    PD_ERROR_CHECK(xTaskCreate(wifiTask, "Wifi", 4096, &msg, 10, &myTaskHandle));

    msg.softap.softap_q = azure_q;
    msg.softap.wifi_q = cloud_q;
    PD_ERROR_CHECK(xTaskCreate(softApTask, "SoftAp", 4096, &msg, 10, &myTaskHandle));

    msg.stn.stn_q = azure_q;
    msg.stn.wifi_q = cloud_q;
    PD_ERROR_CHECK(xTaskCreate(stnTask, "Stn", 4096, &msg, 10, &myTaskHandle));

    /*
     * Web server needs to know about any task that can be configured.
     */
    msg.web.web_q = web_q;
    msg.web.stn_q = stn_q;
    msg.web.softap_q = softap_q;
    msg.web.azure_q = azure_q;
    PD_ERROR_CHECK(xTaskCreate(webTask, "Web", 4096, &msg, 10, &myTaskHandle));

    /*
     * Start the ball rolling by starting WiFi.
     */
    msg.start.type = MSG_TYPE_START;
    xQueueSend(wifi_q, &msg, 1000);
}

void startOfDay()
{
    /*
     * - Configure debugging, either via serial port of by creating a socket.
     */
    debugInit();

    // How will we reset to factory?  Can we preload the configuration but not
    // overwrite it with OTA updates?

    /*
     * Throw start-of-day debugging, which we may not see if we are using IP.
     */
    esp32_info();
}

/*
 * Use logging to show ESP32c3 information.  We drop this at INFO level so if
 * logging is not enabled to this level, it will not be seen.
 */
void esp32Info()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;

    esp_chip_info(&chip_info);

    ESP_LOGI(TAG,
        "This is %s chip with %d CPU core(s), WiFi%s%s, ",
        CONFIG_IDF_TARGET,
        chip_info.cores,
        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG,
        "silicon revision %d, ", chip_info.revision);

    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        ESP_LOGI(TAG, "Get flash size failed");
        return;
    }

    ESP_LOGI(TAG< "%uMB %s flash\n", flash_size / (1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG,
        "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

