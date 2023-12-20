/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
// TODO: Are these headers correct?
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/**
 * Allow logging in this file; disabled unless explcitly set.
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "http_parser.h"
#include "cs_web.h"
#include "cs_cfg.h"

#define TAG cs_web_task_name

/* Task configuration */
#define CS_TASK_TICKS_TO_WAIT       CONFIG_CS_TASK_TICKS_TO_WAIT

/* Web server configuration */
#define CS_WEB_LISTEN_PORT          CONFIG_CS_WEB_LISTEN_PORT

/* Link to the web source files, embedded in the image. */
extern const uint8_t index_html_start[]   asm("_binary_index_html_start");
extern const uint8_t index_html_end[]     asm("_binary_index_html_end");
extern const uint8_t menu_html_start[]    asm("_binary_menu_html_start");
extern const uint8_t menu_html_end[]      asm("_binary_menu_html_end");
extern const uint8_t wifi_html_start[]    asm("_binary_wifi_html_start");
extern const uint8_t wifi_html_end[]      asm("_binary_wifi_html_end");
extern const uint8_t web_html_start[]     asm("_binary_web_html_start");
extern const uint8_t web_html_end[]       asm("_binary_web_html_end");
extern const uint8_t ota_html_start[]     asm("_binary_ota_html_start");
extern const uint8_t ota_html_end[]       asm("_binary_ota_html_end");
extern const uint8_t style_css_start[]    asm("_binary_style_css_start");
extern const uint8_t style_css_end[]      asm("_binary_style_css_end");
extern const uint8_t tools_js_start[]     asm("_binary_tools_js_start");
extern const uint8_t tools_js_end[]       asm("_binary_tools_js_end");

// TODO: It appears that length includes a NULL so reduce by 1.
#define index_html_len  (size_t)(index_html_end - index_html_start - 1)
#define menu_html_len   (size_t)(menu_html_end - menu_html_start - 1)
#define wifi_html_len   (size_t)(wifi_html_end - wifi_html_start - 1)
#define web_html_len    (size_t)(web_html_end - web_html_start - 1)
#define ota_html_len    (size_t)(ota_html_end - ota_html_start - 1)
#define style_css_len   (size_t)(style_css_end - style_css_start - 1)
#define getdata_js_len  (size_t)(getdata_js_end - getdata_js_start - 1)
#define tools_js_len    (size_t)(tools_js_end - tools_js_start - 1)

/* Event loops (exccept for the default loop, used by Wifi) */
static esp_event_loop_handle_t web_event_loop_handle = NULL;
static esp_event_loop_handle_t ota_event_loop_handle = NULL;

/* Server configuration. */
static httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();

/* Handle to the web server. */
static httpd_handle_t httpd_server = NULL;


/**
 * We could have a table look-up for pages but it is far simpler to just have
 * a handler per page and be done with it.
*/
static esp_err_t get_index_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET index.html");
    return httpd_resp_send(req, (char *)index_html_start, index_html_len);
}

static esp_err_t get_menu_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET menu.html");
    return httpd_resp_send(req, (char *)menu_html_start, menu_html_len);
}

static esp_err_t get_wifi_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET wifi.html");
    return httpd_resp_send(req, (char *)wifi_html_start, wifi_html_len);
}

static esp_err_t get_web_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET web.html");
    return httpd_resp_send(req, (char *)web_html_start, web_html_len);
}

static esp_err_t get_ota_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET ota.html");
    return httpd_resp_send(req, (char *)ota_html_start, ota_html_len);
}

static esp_err_t get_style_css_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET style.css");
    return httpd_resp_send(req, (char *)style_css_start, style_css_len);
}

static esp_err_t get_tools_js_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET tools.js");
    return httpd_resp_send(req, (char *)tools_js_start, tools_js_len);
}

size_t read_field(char **ptr, size_t available, const char *ns, const cs_cfg_definitions_t *definition)
{
    size_t outlen = 0;
    size_t s_length;
    uint8_t u8_value;
    uint16_t u16_value;
    uint32_t u32_value;

    ESP_LOGV(TAG, "Key, type: %s, %d", definition->key, definition->type);
    switch (definition->type)
    {
        case NVS_TYPE_STR:
            outlen = snprintf(*ptr, available, "\"%s\":\"", definition->key);
            if (outlen > available)
            {
                break;
            }
            available -= outlen;
            *ptr += outlen;
            s_length = available;
            cs_cfg_read_str(ns, definition->key, ptr, &s_length);

            // Length returned out includes the NULL.
            s_length--;
            available -= s_length;
            *ptr+= s_length;
            outlen += s_length;
            if (available < 1)
            {
                break;
            }
            **ptr = '"';
            outlen++;
            break;

        case NVS_TYPE_U8:
            cs_cfg_read_uint8(ns, definition->key, &u8_value);
            outlen = snprintf(*ptr, available, "\"%s\":%hu", definition->key, (unsigned short)u8_value);
            break;

        case NVS_TYPE_U16:
            cs_cfg_read_uint16(ns, definition->key, &u16_value);
            outlen = snprintf(*ptr, available, "\"%s\":%hu", definition->key, u16_value);
            break;

        case NVS_TYPE_U32:
            cs_cfg_read_uint32(ns, definition->key, &u32_value);
            outlen = snprintf(*ptr, available, "\"%s\":%lu", definition->key, u32_value);
            break;

        default:
            ESP_LOGE(TAG, "Unsupported type: %d", definition->type);
            ESP_ERROR_CHECK(ESP_FAIL);
            break;
    }
    return outlen;
}

static esp_err_t get_json_handler(
    httpd_req_t *req,
    const char *ns,
    const cs_cfg_definitions_t *definitions)
{
    cs_cfg_definitions_t *definition = (cs_cfg_definitions_t *)definitions;
    esp_err_t esp_rc = ESP_OK;
    size_t available;
    size_t outlen = 0;
    // TODO: Improve this.
    char *buffer;
    char *ptr;

    ESP_LOGI(TAG, "GET JSON handler");

    buffer = (char *)malloc(1024);
    available = 1024;
    ptr = buffer;
    *ptr++ = '{';
    available--;

    outlen = read_field(&ptr, available, ns, definition);

    while ((available > outlen) && ((++definition)->key != NULL))
    {
        available -= outlen;
        ptr += outlen;
        *ptr++ = ',';
        available--;
        outlen = read_field(&ptr, available, ns, definition);
    }

    if (available > outlen)
    {
        available -= outlen;
        ptr += outlen;
        *ptr++ = '}';
        available--;
    }

    if (available >= outlen)
    {
        outlen = (size_t)(ptr - buffer);
        ESP_LOGV(TAG, "Send successful response: %d", outlen);
        httpd_resp_send(req, buffer, outlen);
    }
    else
    {
        ESP_LOGE(TAG, "Send 500 Internal error");
        httpd_resp_send_500(req);
    }
    free(buffer);

    return esp_rc;
}

static esp_err_t get_wifi_json_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET wifi.json");
    return get_json_handler(req, CS_CFG_NMSP_WIFI, cs_cfg_wifi_definitions);
}

static esp_err_t get_web_json_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET web.json");
    return get_json_handler(req, CS_CFG_NMSP_WEB, cs_cfg_web_definitions);
}

static esp_err_t get_ota_json_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "GET ota.json");
    return get_json_handler(req, CS_CFG_NMSP_OTA, cs_cfg_ota_definitions);
}

static esp_err_t post_html_handler(
    httpd_req_t *req,
    const char *namespace,
    const cs_cfg_definitions_t *definitions,
    esp_event_loop_handle_t event_loop_handle,
    esp_event_base_t event_base,
    int32_t event_id)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    esp_err_t esp_rc = ESP_OK;

    // TODO: Remove this!  This blows stack limits - we should be using allocated memory.
    // TODO: then reduce stack size for both httpd and cs_web.
    char *content;
    char *value;
    uint16_t u16_value;
    uint32_t u32_value;

    ESP_LOGI(TAG, "POST HTML handler");

    content = (char *)malloc(req->content_len);
    value = (char *)malloc(req->content_len);

    /* Truncate if content length larger than the buffer */
    // TODO: Hard-coded value!
    // TODO: No point cnotinuing here as truncated data will cause a crash - return an error instead!
    // TODO: Dynamically allocate based no content length!
    // size_t recv_size = req->content_len < 1024 ? req->content_len : 1024;

    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            ESP_LOGE(TAG, "Timed out receiving request");
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        ESP_LOGE(TAG, "Non-timeout error: %d", ret);
        return ESP_FAIL;
    }

    do
    {
        ESP_LOGV(TAG, "Key: %s", definitions->key);
        // TODO: Need to improve length perhaps?
        ESP_ERROR_CHECK(httpd_query_key_value(content, definitions->key, value, req->content_len));
        ESP_LOGV(TAG, "Value: %s", value);
        switch (definitions->type)
        {
            case NVS_TYPE_STR:
                /* Strings must be NULL terminated. */
                cs_cfg_write_str(namespace, definitions->key, value);
                break;

            case NVS_TYPE_U8:
                sscanf(value, "%hd", &u16_value);
                cs_cfg_write_uint8(namespace, definitions->key, (uint8_t)u16_value);
                break;

            case NVS_TYPE_U16:
                sscanf(value, "%hd", &u16_value);
                cs_cfg_write_uint16(namespace, definitions->key, (uint16_t)u16_value);
                break;

            case NVS_TYPE_U32:
                sscanf(value, "%ld", &u32_value);
                cs_cfg_write_uint32(namespace, definitions->key, u32_value);
                break;

            default:
                ESP_LOGE(TAG, "Unsupported type: %d", definitions->type);
                esp_rc = ESP_FAIL;
                break;
        }
    } while ((++definitions)->key != NULL);

    if (esp_rc == ESP_OK)
    {
        ESP_LOGV(TAG, "Send success response");
        httpd_resp_send(req, NULL, 0);
    }
    else
    {
        ESP_LOGE(TAG, "Send 400 Bad request");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, NULL);
    }

    /* Finally, send a config change notification to the appropriate component. */
    if (event_loop_handle == NULL)
    {
        /* Component uses the default loop. */
        ESP_LOGV(TAG, "Nudge default event loop");
        esp_event_post(CS_CONFIG_EVENT, CS_CONFIG_CHANGE, NULL, 0, CS_TASK_TICKS_TO_WAIT);
    }
    else
    {
        ESP_LOGV(TAG, "Nudge specific event loop");
        esp_event_post_to(event_loop_handle, CS_CONFIG_EVENT, CS_CONFIG_CHANGE, NULL, 0, CS_TASK_TICKS_TO_WAIT);
    }

    free(value);
    free(content);

    return ESP_OK;
}

/* Our URI handler function to be called during POST /uri request */
static esp_err_t post_wifi_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "POST wifi.html");
    return post_html_handler(req, CS_CFG_NMSP_WIFI, cs_cfg_wifi_definitions, NULL, CS_CONFIG_CHANGE, CS_CONFIG_CHANGE);
}

static esp_err_t post_ota_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "POST ota.html");
    return post_html_handler(req, CS_CFG_NMSP_OTA, cs_cfg_ota_definitions, ota_event_loop_handle, CS_CONFIG_CHANGE, CS_CONFIG_CHANGE);
}

static esp_err_t post_web_html_handler(httpd_req_t *req)
{
    ESP_LOGV(TAG, "POST web.html");
    return post_html_handler(req, CS_CFG_NMSP_WEB, cs_cfg_web_definitions, web_event_loop_handle, CS_CONFIG_CHANGE, CS_CONFIG_CHANGE);
}

/* URI handler structures for GET /... */
static httpd_uri_t uri_get_root_html = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_index_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_index_html = {
    .uri      = "/index.html",
    .method   = HTTP_GET,
    .handler  = get_index_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_menu_html = {
    .uri      = "/menu.html",
    .method   = HTTP_GET,
    .handler  = get_menu_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_wifi_html = {
    .uri      = "/wifi.html",
    .method   = HTTP_GET,
    .handler  = get_wifi_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_web_html = {
    .uri      = "/web.html",
    .method   = HTTP_GET,
    .handler  = get_web_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_ota_html = {
    .uri      = "/ota.html",
    .method   = HTTP_GET,
    .handler  = get_ota_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_style_css = {
    .uri      = "/css/style.css",
    .method   = HTTP_GET,
    .handler  = get_style_css_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_tools_js = {
    .uri      = "/tools.js",
    .method   = HTTP_GET,
    .handler  = get_tools_js_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_wifi_json = {
    .uri      = "/wifi.json",
    .method   = HTTP_GET,
    .handler  = get_wifi_json_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_web_json = {
    .uri      = "/web.json",
    .method   = HTTP_GET,
    .handler  = get_web_json_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_get_ota_json = {
    .uri      = "/ota.json",
    .method   = HTTP_GET,
    .handler  = get_ota_json_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
static httpd_uri_t uri_post_wifi_html = {
    .uri      = "/wifi.html",
    .method   = HTTP_POST,
    .handler  = post_wifi_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_post_ota_html = {
    .uri      = "/ota.html",
    .method   = HTTP_POST,
    .handler  = post_ota_html_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_post_web_html = {
    .uri      = "/web.html",
    .method   = HTTP_POST,
    .handler  = post_web_html_handler,
    .user_ctx = NULL
};

void web_start()
{

    /**
     * Read the listen port.
    */
    cs_cfg_read_uint16(CS_CFG_NMSP_WEB, CS_CFG_KEY_WEB_PORT, &httpd_config.server_port);
    ESP_LOGV(TAG, "Listen on port: %hu", httpd_config.server_port);

    /**
     * Stop the server if already running.
     */
    if (httpd_server != NULL)
    {
        ESP_LOGV(TAG, "Stopping existing server");
        httpd_stop(httpd_server);
        httpd_server = NULL;
    }

    /**
     * We are using WiFi STA+SoftAP mode so the Web server should be able to run
     * providing either SoftAP and/or STA are active.
     */
    ESP_LOGI(TAG, "Registering URI handlers");
    ESP_ERROR_CHECK(httpd_start(&httpd_server, &httpd_config));

    // TODO: This is not pretty; we need a better solution!
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_root_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_index_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_menu_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_wifi_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_web_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_ota_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_style_css));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_tools_js));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_wifi_json));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_web_json));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_get_ota_json));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_post_wifi_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_post_web_html));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_server, &uri_post_ota_html));
}

/**
 * Catch events from the default event loop and immediately report to the event
 * loop for this task.
*/
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGV(TAG, "Relay event to web event loop");

    // TODO: Do we really want to pass the data here?  Requires processing!
    ESP_ERROR_CHECK(esp_event_post_to(
        web_event_loop_handle,
        event_base,
        event_id,
        NULL,
        0,
        CS_TASK_TICKS_TO_WAIT));
}

static void web_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    /* STA has gotten an IP address so make sure we are listening on it. */
    ESP_LOGI(TAG, "Restarting HTTP server.");
    if (event_base == CS_CONFIG_EVENT)
    {
        /**
         * A config change means we need to restart the server and listen on
         * a new port.
         */
        web_start();
    }
    else if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_AP_START:
                /**
                 * Signals at least SoftAP up so (re)start the web server.
                */
                web_start();
                break;


            default:
                ESP_LOGE(TAG, "unexpected event: %d", event_id);
                break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
            case IP_EVENT_STA_GOT_IP:
                /**
                 * Ignore for now and hope that the web server listens on the
                 * new socket once we connect to the AP (router).
                */
               break;

            default:
                ESP_LOGE(TAG, "unexpected event: %d", event_id);
                break;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event base: %s, %d", event_base, event_id);
    }
}

/**
 * To catch WiFi events, we have to catch the default event loop but we pass
 * immediately to our local loop to ensure (FreeRTOS) task separation.
*/
void cs_web_task(cs_web_create_parms_t *create_parms)
{
    // TODO: Remove this.
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    /**
     * Increase the number of supported URI handlers.
    */
   // TODO: Do we want to do this?
   httpd_config.max_uri_handlers = 20;

    ESP_LOGI(TAG, "init. Web task");
    /* Save the event handles that we need to send notifications to. */
    web_event_loop_handle = create_parms->web_event_loop_handle;
    ota_event_loop_handle = create_parms->ota_event_loop_handle;

    /**
     * TODO: Is this really true?  Remove this if not.
     *
     * The web server needs to restart each time the STN WiFi server connects
     * so that is a now listening on the new IP address so register for the
     * appropriate events.
    */
    ESP_LOGI(TAG, "Register event handlers");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                IP_EVENT,
                IP_EVENT_STA_GOT_IP,
                wifi_event_handler,
                NULL,
                NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                WIFI_EVENT,
                WIFI_EVENT_AP_START,
                wifi_event_handler,
                NULL,
                NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                web_event_loop_handle,
                IP_EVENT,
                IP_EVENT_STA_GOT_IP,
                web_event_handler,
                NULL,
                NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                web_event_loop_handle,
                WIFI_EVENT,
                WIFI_EVENT_AP_START,
                web_event_handler,
                NULL,
                NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                web_event_loop_handle,
                CS_CONFIG_EVENT,
                CS_CONFIG_CHANGE,
                &web_event_handler,
                NULL,
                NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
                web_event_loop_handle,
                IP_EVENT,
                ESP_NETIF_IP_EVENT_GOT_IP,
                &web_event_handler,
                NULL,
                NULL));

    // TODO: We cannot start the server until there is an IP address.
    // web_start();

    ESP_LOGV(TAG, "Initialization completed");
}