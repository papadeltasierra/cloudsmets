/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_netif_ip_addr.h"
#include "esp_mac.h"

#include "cs_log.h"
#include "cs_wifi.h"
#include "cs_cfg.h"

#define TAG cs_wifi_task_name

/* AP Configuration */
#define CS_WIFI_AP_MAX_CONN         CONFIG_CS_WIFI_AP_MAX_CONN

/* STA configuration */
#define CS_WIFI_STA_MAX_CONN        CONFIG_CS_WIFI_STA_MAX_CONN
#define CS_WIFI_STA_MAXIMUM_RETRY   CONFIG_CS_WIFI_STA_MAXIMUM_RETRY
#define CS_WIFI_STA_RETRY_INTERVAL  CONFIG_CS_WIFI_STA_RETRY_INTERVAL

typedef struct
{
    esp_netif_t *s_netif_softap;
    esp_netif_t *s_netif_sta;
    esp_timer_handle_t s_netif_sta_timer;
    esp_event_loop_handle_t flash_event_loop_handle;
    int s_retry_num;

} wifi_event_handler_args_t;

// TODO: What about the SoftAP addresses and masks?

static wifi_config_t wifi_sta_config = {
    .sta = {
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .failure_retry_cnt = CS_WIFI_STA_MAXIMUM_RETRY,
        /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
            * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
            * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
        * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
            */
        // .threshold.authmode = WIFI_AUTH_WPA2_WPA3_PSK,
        // TODO: Do we make this cnofigurable?  Certainly want better than this.
        .threshold.authmode = WIFI_AUTH_OPEN,
        .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    },
};

static bool wifi_config_sta()
{
    bool rc = true;
    size_t length;
    uint8_t *ptr;

    ESP_LOGI(TAG, "Config STA");

    length = sizeof(wifi_sta_config.ap.ssid);
    ESP_LOGV(TAG, "B SSID: %d", length);
    ptr = &wifi_sta_config.sta.ssid[0];
    cs_cfg_read_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_STA_SSID, (char **)&ptr, &length);
    ESP_LOGV(TAG, "A SSID: %d", length);
    wifi_sta_config.sta.ssid[length] = 0;
    rc = length > 8 ? rc : false;
    length = sizeof(wifi_sta_config.ap.password);
    ESP_LOGV(TAG, "B PWD: %d", length);
    ptr = &wifi_sta_config.sta.password[0];
    cs_cfg_read_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_STA_PWD, (char **)&ptr, &length);
    ESP_LOGV(TAG, "A PWD: %d", length);
    wifi_sta_config.sta.password[length] = 0;
    rc = length > 8 ? rc : false;

    if (rc)
    {
        ESP_LOGV(TAG, "Have config, start STA");
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config) );
    }
    return rc;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    wifi_event_handler_args_t *wifi_event_handler_args = (wifi_event_handler_args_t *)arg;

    wifi_event_ap_staconnected_t *ap_conn_event;
    wifi_event_ap_stadisconnected_t *ap_disconn_event;

    ESP_LOGI(TAG, "Event: %s, %d", event_base, event_id);

    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_AP_START:
                // esp_wifi_connect();
                ESP_LOGI(TAG, "SoftAP: Started");

                // TODO: Make ticks more realistic and use a #define.
                esp_event_post_to(wifi_event_handler_args->flash_event_loop_handle, event_base, event_id, NULL, 0, 10);
                break;

            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "SoftAP: Stopped");
                esp_event_post_to(wifi_event_handler_args->flash_event_loop_handle, event_base, event_id, NULL, 0, 10);
                break;

            case WIFI_EVENT_AP_STACONNECTED:
                ap_conn_event = (wifi_event_ap_staconnected_t *) event_data;
                ESP_LOGI(TAG, "SoftAP: station "MACSTR" joined, AID=%d",
                    MAC2STR(ap_conn_event->mac), ap_conn_event->aid);
                break;

            case WIFI_EVENT_AP_STADISCONNECTED:
                ap_disconn_event = (wifi_event_ap_stadisconnected_t *) event_data;
                ESP_LOGI(TAG, "SoftAP: Station "MACSTR" left, AID=%d",
                        MAC2STR(ap_disconn_event->mac), ap_disconn_event->aid);
                break;

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "STA: connected to AP (router)");
                wifi_event_handler_args->s_retry_num = 0;

                /*
                 * We are connected to the router so we want traffic to flow
                 * there and not to the SoftAP.  SoftAP connectivity should only
                 * be inbound!
                 *
                 * Set sta as the default interface
                 */
                esp_netif_set_default_netif(wifi_event_handler_args->s_netif_sta);
                esp_event_post_to(wifi_event_handler_args->flash_event_loop_handle, event_base, event_id, NULL, 0, 10);
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                esp_wifi_disconnect();
                if (wifi_event_handler_args->s_retry_num < CS_WIFI_STA_MAXIMUM_RETRY) {
                    ESP_LOGI(TAG, "STA: retry connected from the AP (router)");
                    esp_wifi_connect();
                    wifi_event_handler_args->s_retry_num++;
                } else {
                    ESP_LOGE(TAG, "STA: Connectivity to AP (router) lost.");
                    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
                    esp_netif_set_default_netif(wifi_event_handler_args->s_netif_softap);

                    /* Interval has to be given in microseconds! */
                    uint64_t interval = CS_WIFI_STA_RETRY_INTERVAL * 1000 * 1000;
                    ESP_ERROR_CHECK(esp_timer_start_once(wifi_event_handler_args->s_netif_sta_timer, interval));
                }
                esp_event_post_to(wifi_event_handler_args->flash_event_loop_handle, event_base, event_id, NULL, 0, 10);
                break;

            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "STA: Started");
                // esp_wifi_disconnect();
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_STOP:
                ESP_LOGI(TAG, "STA: Stopped");
                esp_wifi_disconnect();
                break;

            default:
                ESP_LOGI(TAG, "Other WiFi event: %d", event_id);
                break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
            case IP_EVENT_STA_GOT_IP:
                ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
                ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
                break;

            default:
                ESP_LOGE(TAG, "Unexpected IP event: %d", event_id);
                break;
        }
    }
    else if (event_base == CS_CONFIG_EVENT)
    {
        /*
         * Any configuration chnaged means stop and attempt a restart of the
         * STA.
         */
        ESP_LOGI(TAG, "Config. change; restart STA.");
        esp_timer_stop(wifi_event_handler_args->s_netif_sta_timer);
        esp_wifi_disconnect();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        if (wifi_config_sta())
        {
            ESP_LOGV(TAG, "STA config sufficient - try connect");
            wifi_event_handler_args->s_retry_num = 0;
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        }
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event_base: %s", event_base);
    }
}

/* Initialize soft AP */
static esp_netif_t *wifi_init_softap()
{
    // TODO: How do we set the network mask?  Or at last force to always 192.168.4.xxx
    size_t length;
    uint8_t *ptr;
    esp_netif_t *s_netif_softap = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_ap_config = {
        .ap = {
            .max_connection = CS_WIFI_STA_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    ESP_LOGV(TAG, "Read configuration");

    // Read SoftAP configuration.
    cs_cfg_read_uint8(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_CHNL, &wifi_ap_config.ap.channel);
    length = sizeof(wifi_ap_config.ap.ssid);
    ptr = &wifi_ap_config.ap.ssid[0];
    cs_cfg_read_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_SSID, (char **)&ptr, &length);
    wifi_ap_config.ap.ssid_len = (uint8_t)length;
    length = sizeof(wifi_ap_config.ap.password);
    ptr = &wifi_ap_config.ap.password[0];
    cs_cfg_read_str(CS_CFG_NMSP_WIFI, CS_CFG_KEY_WIFI_AP_PWD, (char **)&ptr, &length);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    esp_netif_set_default_netif(s_netif_softap);

    ESP_LOGV(TAG, "AP configured");

    return s_netif_softap;
}

/* Try and reconnect the STA WiFi connection. */
static void esp_netif_timer_cb(void *arg)
{
    int *s_retry_num = (int *)arg;

    ESP_LOGI(TAG, "Timer retrying connect for STA");
    (*s_retry_num) = 0;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
}

/* Initialize wifi station */
static esp_netif_t *wifi_init_sta(void)
{
    esp_netif_t *s_netif_sta = esp_netif_create_default_wifi_sta();

    if (wifi_config_sta())
    {
        ESP_LOGI(TAG, "Try initial connect");
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    }

    return s_netif_sta;
}

/**
 * As noted elsewhere, the esp-idf library hard-codes WiFi to use the default
 * event loop and we cannot change that.
*/
void cs_wifi_task(cs_wifi_create_parms_t *create_parms)
{
    static wifi_event_handler_args_t wifi_event_handler_args;

    // TODO: Remove this.
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "Init. WiFi task");

    wifi_event_handler_args.flash_event_loop_handle = create_parms->flash_event_loop_handle;

    /*Initialize WiFi */
    ESP_LOGI(TAG, "Init. WiFi");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Initialize AP */
    ESP_LOGI(TAG, "Init ESP_WIFI_MODE_AP");
    wifi_event_handler_args.s_netif_softap = wifi_init_softap();

    /* Initialize STA */
    ESP_LOGI(TAG, "Init ESP_WIFI_MODE_STA");
    wifi_event_handler_args.s_netif_sta = wifi_init_sta();

    ESP_LOGI(TAG, "Register event handlers");
    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    wifi_event_handler,
                    &wifi_event_handler_args,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    wifi_event_handler,
                    &wifi_event_handler_args,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                    CS_CONFIG_EVENT,
                    CS_CONFIG_CHANGE,
                    wifi_event_handler,
                    &wifi_event_handler_args,
                    NULL));


    /* We will use a timer to try and reconnect when the connection fails. */
    ESP_LOGI(TAG, "Create STA reconnect timer");
    esp_timer_create_args_t esp_timer_create_args = {
        .callback = esp_netif_timer_cb,
        .arg = &wifi_event_handler_args.s_retry_num,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "WiFiStaTimer",
        .skip_unhandled_events = false
    };
    ESP_ERROR_CHECK(esp_timer_create(&esp_timer_create_args, &wifi_event_handler_args.s_netif_sta_timer));

    ESP_LOGI(TAG, "initialize NETIF");
    ESP_ERROR_CHECK(esp_netif_init());


    /* Start WiFi */
    ESP_LOGI(TAG, "start WiFi");
    ESP_ERROR_CHECK(esp_wifi_start());
}
