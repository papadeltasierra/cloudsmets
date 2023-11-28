/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Allow logging over the WiFi connection.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/app_trace.html#app-trace-logging-to-host
 */
#pragma once

// TODO: Are these headers correct?
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_log.h"

#define LOGGING_POINTER (configNUM_THREAD_LOCAL_STORAGE_POINTERS - 1)
#define LOGGING_BUFFER_SIZE 256

// Message passing stack side.
#define STACK_DEPTH 10

static const char* TAG = "Log";
static QueueHandle_t s_xQueue = 0;
static socket listen4_sock = -1;
static socket listen6_sock = -1;
static socket remote_sock = -1;
static SemaphoreHandle_t logging_mutex = NULL;
static StaticSemaphore_t mutex_buffer;

/*
 * This method has to be thread safe, which is why we allocate thread-specific
 * buffers.
 */
int logger_vprintf(const char *fmt, va_list ap)
{
    TaskHandle_t task_handle;
    char *buffer;
    char *ptr;
    int length;

    /*
     * We cannot easily "list" the current ESP32/FreeRTOS tasks so we cannot
     * preallocate buffers for logging; check here and allocate on the first use.
     */
    task_handle = xTaskGetCurrentTaskHandle
    buffer = (char *)pvTaskGetThreadLocalStoragePointer(task_handle, LOGGING_POINTER);
    if (buffer == NULL)
    {
        buffer = malloc(LOGGING_BUFFER_SIZE);
        if (buffer == NULL)
        {
            // We log here using a simple send then we abort.
            remote_send("Logging malloc() failed!", 24)
            ESP_ERROR_CHECK(ESP_FAIL);
        }
        vTaskSetThreadLocalStoragePointer(task_handle, LOGGING_POINTER, buffer);
    }
    /*
     * Length returned is how much WOULD have been written if the buffer were
     * large enough.*
     */
    length = vsnprintf(buffer, LOGGING_BUFFER_SIZE, fmt, ap);
    if (length > LOGGING_BUFFER_SIZE)
    {
        /*
         * Add "..." to the end of the string to indicate that it was truncated.
         */
        ptr = buffer + (LOGGING_BUFFER_SIZE - 4);
        *ptr++ = '.';
        *ptr++ = '.';
        *ptr++ = '.';
        length = LOGGING_BUFFER_SIZE;
    }
    logger_send(buffer, length);
}

static void remote_send(char *buffer, int length)
{
    socket local_remote_socket;

    PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
    local_remote_sock = remote_sock;
    PD_CHECK_ERROR(xSemaphoreGive(logging_mutex));

    if (local_remote_logging_sock != -1)
    {
        /*
         * Ignore errors since we cannot log anywhere else!
         */
        send(local_remote_sock, buffer, length, 0);
    }
}

static void socket_task(void *args)
{
    fd_set errorfds;
    socket local_listen4_sock = -1;
    socket local_listen6_sock = -1;
    socket local_remote_sock = -1;
    int sock_rc = -1;
    u16 port = (u16)(*args);
    esp_ip6_addr_t ip6_addr;
    esp_netif_ip_info_t ip4_info;
    esp_err_t esp_rc;
    struct sockaddr_storage sock_addr;
    struct *sockaddr_in = (sockaddr_in *)&sockaddr_ip4;
    struct *sockaddr_in6 = (sockaddr_in6 *)&sockaddr_ip6;

    while (1)
    {
        // We wil listen on both IPv6 and IPv4 ports, if both are supported.
        if (local_listen6_sock == -1)
        {
            // Maybe start listening on IPv6
            esp_rc = esp_netif_get_ip6_global(esp_netif_t *esp_netif, &ip6_addr);
            if (esp_rc == ESP_OK)
            {
                // There is an IPv6 global address so let's listen on it.
                local_listen6_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_IPV6);
                if (local_listen6_sock == -1)
                {
                    ESP_LOGE(TAG, "Failed to create IPv6 listen socket for remote logging: %d", errno);
                    ESP_ERROR_CHECK(ESP_FAIL);
                }

                bzero(sockaddr_ip6->sin6_addr.un, sizeof(sockaddr_ip6->sin6_addr.un));
                sockaddr_ip6->sin6_family = AF_INET6;
                sockaddr_ip6->sin6_port = htons(port);
                sock_rc = bind(local_listen6_socket, sockaddr_ip6, sizeof(sockaddr_in6));
                if (sock_rc == -1)
                {
                    ESP_LOGE(TAG, "Failed to bind listen socket for remote logging: %d", errno);
                    ESP_ERROR_CHECK(ESP_FAIL);
                }

                PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
                listen6_sock = local_listen6_sock;
                PD_CHECK_ERROR(xSemaphoreGive(logging_mutex));

                sock_rc = listen(local_listen6_socket, 1);
                if (sock_rc == -1)
                {
                    ESP_LOGE(TAG, "Failed to listen on IPv6 listen socket for remote logging: %d", errno);
                    ESP_ERROR_CHECK(ESP_FAIL);
                }
            }
        }

        if (local_listen4_scok == -1)
        {
            // Start listening on IPv4.
            ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_t *esp_netif, &ip4_info));
            local_listen4_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (local_listen4_sock == -1)
            {
                ESP_LOGE(TAG, "Failed to create IPv4 listen socket for remote logging: %d", errno);
                ESP_ERROR_CHECK(ESP_FAIL);
            }

            sockaddr_ip->sin_addr.s_addr = htonl(INADDR_ANY);
            sockaddr_ip->sin_family = AF_INET;
            sockaddr_ip->sin_port = htons(port);
            sock_rc = bind(local_listen_socket, sockaddr_ip4, sizeof(sockaddr_in));
            if (sock_rc == -1)
            {
                ESP_LOGE(TAG, "Failed to bind IPv4 listen socket for remote logging: %d", errno);
                ESP_ERROR_CHECK(ESP_FAIL);
            }

            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            listen4_sock = local_listen4_sock;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex));

            sock_rc = listen(local_listen4_socket, 1);
            if (sock_rc == -1)
            {
                ESP_LOGE(TAG, "Failed to listen on IPv4 listen socket for remote logging: %d", errno);
                ESP_ERROR_CHECK(ESP_FAIL);
            }
        }

        FD_ZERO(&readfds);
        FD_ZERO(&errorfds);
        if (local_listen6_sock != -1)
        {
            FD_SET(local_listen_sock, &readfds);
            FD_SET(local_listen_sock, &errorfds);
        }
        FD_SET(local_listen4_sock, &readfds);
        FD_SET(local_listen4_sock, &errorfds);

        // Now we listen to see who gets an incoming connection.
        sock_rc = select(&readfds, NULL, &errorfds);
        if (sock_rc == -1)
        {
            ESP_LOGE(TAG, "Error in connection select: %d", errno);
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        // In the event of an error, just close the socket.
        if (FD_ISSET(errorfds, local_listen4_sock))
        {
            close(local_listen4_sock);
            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            listen_sock = -1;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex, portMAX_DELAY));
        }
        if (FD_ISSET(errorfds, local_listen6_sock))
        {
            close(local_listen6_sock);
            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            listen_sock = -1;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex, portMAX_DELAY));
        }

        // Prefer IPv6 over IPv4.
        if (FD_ISSET(readfds, local_listen6_sock))
        {
            local_remote_sock = accept(local_listen6_sock);
            close(local_listen4_sock);
            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            listen4_sock = -1;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex, portMAX_DELAY));
        }
        else if (FD_ISSET(readfds, local_listen4_sock))
        {
            local_remote_sock = accept(local_listen4_sock);
            close(local_listen6_sock);
            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            listen6_sock = -1;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex, portMAX_DELAY));
        }

        if (local_remote_sock)
        {
            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            remote_sock = local_remote_sock;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex));

            // Switch to remote logging.
            ESP_LOGE(TAG, "Switching to remote logging");
            esp_log_set_vprintf(remote_logging_vsprintf);

            // Wait errors to occur on the socket, probably indicating a disconnect.
            FD_ZERO(errorfds);
            FD_SET(local_remote_sock, &errorfds);
            sock_rc = select(NULL, NULL, errorfds, NULL);

            // Clear the socket used when attempting remote logging.
            // Grab mutex here.
            PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
            remote_sock = -1;
            PD_CHECK_ERROR(xSemaphoreGive(logging_mutex));

            // Ensure that we close the remote logging socket.
            close(local_remote_sock);
            local_remote_sock = -1;

            // Revert to serial port based debugging.
            esp_log_set_vprintf(vsprintf);
            ESP_LOGE(TAG, "Remote logging socket failed");

            if ((sock_rc != 1) || (FD_ISSET(local_listen_sock, errorfds))
            {
                // We have been asked to stop listening.
                break;
            }
        }
    }

    // Close the listen socket if not already closed.
    PD_CHECK_ERROR(xSemaphoreTake(logging_mutex, portMAX_DELAY));
    listen_sock = -1;
    PD_CHECK_ERROR(xSemaphoreGive(logging_mutex));

    if (local_listen_sock != -1)
    {
        close(local_listen_sock);
    }
}

static void maybe_start_remote_logging()
{
    u8 debug_func;
    static u16 debug_port;
    TaskHandle_t selectTaskHandle = NULL;

    // Read configuration and see if we should run the select socket.
    debug_func = cfgReadUint8(CFG_NMSP_DBG, CFG_KETY_DBG_FUNC);
    debug_port = cfgReadUint16(CFG_NMSP_DBG, CFG_KETY_DBG_PORT);

    // TODO: Get rid of magic value.
    if ((debug_func == 2) && (selectTaskHandle == NULL))
    {
        ESP_LOGI(TAG, "Preparing for remote logging");
        // We will switch to remote logging when a connection is made.

        ??? Pass the port here ???  Maybe IPv6/IPv4 indicator?
        PD_CHECK_ERROR(xTaskCreate(socket_task, "logSock", STACK_DEPTH, &debug_port, 1, &selectTaskHandle));
    }
    return select_task_handle;
}

static TaskHandle_t stop_remote_logging(TaskHandle_t select_task_handle)
{
    int local_listen_sock = -1;
    int local_remote_sock = -1;

    if (select_task_handle != NULL)
    {
        // Switch to serial port based debugging.
        esp_log_set_vprintf(vsprintf);
        ESP_LOGI(TAG, "Stopped remote logging");

        vTaskDelete(select_task_handle);
        select_task_handle = NULL;

        // Ensure that the remote sockets are closed.
        PD_CHECK_ERROR(xSemaphoreTask(logging_mutex, portMAX_DELAY));
        local_listen_sock = listen_sock
        local_remote_sock = remote_sock
        listen_sock = -1;
        remote_sock = -1;
        PD_CHECK_ERROR(xSemaphoreGive(logging_mutex, portMAX_DELAY));

        if (local_remote_sock != -1)
        {
            close(local_remote_sock);
        }
        if (local_listen_sock != 1)
        {
            close(local_listen_sock);
        }
    }
    return select_task_handle;
}

TaskHandle_t config_change(select_task_handle)
{
    // Check configuration.
    if (enable_remote_logging)
    {
        select_task_handle = maybe_start_remote_logging();
    }
    else
    {
        select_task_handle = stop_remote_logging(select_task_handle);
    }
    return select_task_handle;
}

void logging_task()
{
    TaskHandle_t select_task_handle = null;

    // Create a mutex here.
    logging_mutex = xSemaphoreCreateMutexStatic(&mutex_buffer);
    if (logging_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create logging mutex");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    while (1)
    {
        if( xQueueReceive(s_xQueue, &(rxBuffer), (TickType_t)6000))
        {
            ESP_LOGV(TAG,msg_msg_received, 0x1234);
            switch (msgType)
            {
                case WIFI_DOWN:
                    select_task_handle = stop_debugging(select_task_handle);
                    break;

                case WIFI_UP:
                    select_task_handle = maybe_start_debugging();
                    break;

                case CONFIG:
                    select_task_handle = config_change(select_task_handle);
                    break;

                default:
                    break;
            }
        }
    }
}

