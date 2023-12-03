/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#pragma once

// TODO: Are these headers correct?
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "web.h"
#include "cfg.h"

// Message passing stack side.
#define STACK_DEPTH 10

static const char* TAG = "Web";
static QueueHandle_t s_xQueue = 0;

typedef field_t
{
    char *key;
    int type;
}

field_t wifi_map[] = {
    {CFG_KEY_WIFI_SSID, NVS_TYPE_STR},
    {CFG_KEY_WIFI_PWD, NVS_TYPE_STR},
    {NULL, NVS_TYPE_ANY}
};

// TODO: requires URL encode/decode functionality.

esp_err_t get_handler_wifi_html(httpd_req_t *req)
{
    // Where does the file come from?
    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_handler_wifi_json(httpd_req_t *req)
{
    return get_handler_json(req, CFG_NMSP_WWIFI, cfgWiFiDefinitions);
}

esp_err_t get_handler_json(httpd_req_t *req, const char *namespace, const cfgDefinitions_t *definitions)
{
    esp_err_t esp_rc = ESP_OK;
    bool first = TRUE;
    size_t s_length;

    int length;
    char *ptr = buffer;
    u8_t u8_val;

    length = sprintf(ptr, "{\"%s\":", map->key);
    ptr += length;
    do
    {
        switch (definitions->type)
        {
            case NVS_TYPE_STR:
                *ptr++ = '"';
                cfgReadStr(namespace, definitions->key, ptr, &s_length);
                ptr += s_length;
                *ptr++ = '"';
                *ptr++ = ',';
                break;

            case NVS_TYPE_U8:
                cfgReadStr(namespace, definitions->key, &u8_value);
                length = sprintf("%hd,", (unsigned short)u8_value);
                ptr += s_length;
                break;

            case NVS_TYPE_U16:
                cfgReadStr(namespace, definitions->key, &u16_value);
                length = sprintf("%hd,", (unsigned short)u16_value);
                ptr += s_length;
                break;

            case NVS_TYPE_U32:
                cfgReadStr(namespace, definitions->key, &u32_value);
                length = sprintf("%ld,", (unsigned long)u32_value);
                ptr += s_length;
                break;

            default:
                ESP_LOGE(TAG, "Unsupported type: %d", definitions->type);
                httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, NULL);
                rc = ESP_FAIL;
                break;
        }
    } while ((++definitions)->field != NULL);

    if (esp_rc == ESP_OK)
    {
        // Remove the final comma.
        ptr--;
        length = sprintf(ptr,"\"}");
        s_length = (size_t)(ptr - buffer);
        httpd_resp_send(req, buffer, s_length);
    }

    return esp_rc;
}

/* Our URI handler function to be called during POST /uri request */
esp_err_t post_wif_html_handler(httpd_req_t *req)
{
    return post_html_handler(req, CFG_NMSP_WIFI, wifi_map);
}

esp_err_t post_html_handler(httpd_req_t *req, char *namespace, const cfgDefinitions_t *definitions)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    esp_err_t esp_rc = ESP_OK;
    char content[1024];
    char value[1024];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    do
    {
        ESP_ERROR_CHECK(httpd_query_key_value(buffer, definitions->key, value, sizeof(value));
        switch (definitions->type)
        {
            case NVS_TYPE_STR:
                cfgWriteStr(namespace, definitions->field, buffer, ????);
                break;

            case NVS_TYPE_U8:
                sscanf(buffer, "%hd", &u16_value);
                cfgWriteU8(namespace, definitions->field, (u8_t)u16_value);
                break;

            case NVS_TYPE_U16:
                sscanf(buffer, "%hd", &u16_value);
                cfgWriteU16(namespace, definitions->field, (u16_t)u16_value);
                break;

            case NVS_TYPE_U32:
                sscanf(buffer, "%ld", &u32_value);
                cfgWriteU32(namespace, definitions->field, u32_value);
                break;

            default:
                ESP_LOGE(TAG, "Unsupported type: %d", definitions->type);
                httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, NULL);
                rc = ESP_FAIL;
                break;
        }
    } while ((++definitions)->field != NULL);

    if (esp_rc == ESP_OK)
    {
        httpd_resp_send(req, NULL, 0);
    }
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get_wifi_html = {
    .uri      = "/wifi.html",
    .method   = HTTP_GET,
    .handler  = web_get_wifi_html_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get_wifi_json = {
    .uri      = "/wifi.json",
    .method   = HTTP_GET,
    .handler  = web_get_wifi_json_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post_wifi_html = {
    .uri      = "/wifi.html",
    .method   = HTTP_POST,
    .handler  = web_post_wifi_html_handler,
    .user_ctx = NULL
};

// TODO: We want the argument to include the ID of a queue to which we can
//       send responses.  See the API - the variable needs to be static in
//       the "main()" that creats this task.
void webTask(void *arg)
{
    httpd_config_t config =
    {
    };
    httpd_handle_t handle = NULL;

    // Note that we are defining the full size of the message here.
    s_xQueue = xQueueCreate(STACK_DEPTH, sizeof(MSG_WIFI) );
    if (s_xQueue == 0)
    {
        ESP_LOGE(TAG, "Enable to create message queue.");
        // TODO: Reboot?
        reboot?
    }

    while (1)
    {
        // 6000 ticks is 1min.
        if( xQueueReceive(s_xQueue, &(rxBuffer), (TickType_t)6000))
        {
            ESP_LOGV(TAG, "Message received: %lx", 0x1234);
            // Todo: log here.
            switch (msg)
            {
                case WIFI_ACTIVE:
                    ESP_CHECK_ERROR(httpd_start(&handle, &config));
                    httpd_register_uri_handle(handle, &uri_get_wifi_html);
                    httpd_register_uri_handle(handle, &uri_get_wifi_json);
                    httpd_register_uri_handle(handle, &uri_post_wifi_html);
                    break;

                case WIFI_FAILED:
                    httpd_stop(handle);
                    handle = NULL;
                    break;

                default:
                    ESP_LOGE(TAG, "Unrecognised event: %lx", msg);
                    break;
            }
        }
    }
}