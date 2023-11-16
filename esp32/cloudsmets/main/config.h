/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Configuration store using ESP-IDF no-volatile storage.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html?highlight=non%20volatile
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// !!PDS: #include "msg.h"

// !!PDS: #include "wifi.h"

// typedef enum msg_id_t
// {
//     MSG_CFG_READ  = msg_task.CFG | 0x0001,
//     MSG_CFG_WRITE = msg_task.CFG | 0x0002,
//     MSG_CFG_RESET = msg_task.CFG | 0x0003,
// } msg_cfg_id_t;

// Supports up to "123.123.123-dev.123"
typedef char cfg_firmware_version_t[20];

// The following are the values that can be returned.
typedef struct
{
    cfg_firmware_version_t version;
} cfg_cloudsmets_t;

union
{
    char ipv4[4];
    char ipv6[16];
} cfg_addr_t;

typedef unsigned char cfg_ipv4_or_ipv6_t;
enum
{
    ipv4 = 0,
    ipv6 = 1
} cfg_ipv4_or_ipv6_e;

typedef unsigned char cfg_dhcp_t;
enum
{
    dhcp = 0,
    no_dhcp = 1
} cfg_dhcp_e;

typedef struct
{
    char ssid[33];
    char password[64];
    cfg_dhcp_t dhcp;
    cfg_ipv4_or_ipv6_t ipv4_ipv6;
    cfg_ipv4_or_ipv6_t subnet;
    cfg_ipv4_or_ipv6_t bitmask;
    cfg_ipv4_or_ipv6_t address;
} cgf_wifi_t;

typedef struct
{
    // Time, in minutes, between sending updates to the cloud systems.
    unsigned short period;
} cfg_cloud_t;

typedef struct
{
    char iot_hub[64];
    char device_name[64];
    char connection_string[128];
} cfg_azure_t;

typedef struct
{
    unsigned char guid[8];
    unsigned char key[8];
} cfg_zigbee_t;

typedef unsigned char cfg_ota_enabled_t;
enum
{
    disabled = 0,
    enabled = 1
} cfg_ota_enabled_e;

typedef struct {
    char key1[8];
    char key2[8];
} cfg_sign_keys_t;

typedef struct
{
    cfg_ota_enabled_t enabled;
    char server[128];
    cfg_sign_keys_t signing_keys;
    cfg_firmware_version_t force_version;
} cfg_ota_t;

typedef union
{
    cfg_cloudsmets_t cloudsmets;
    // cfg_wifi_t wifi;
    // !!PDS: cfg_cloud_t cloud;
    // !!PDS: cfg_azure_t azure
    // !!PDS: cfg_zigbee_t zigbee;
    // !!PDS: cfg_ota_t ota;
} cfg_value_t;

// The message that the configuration task receives.
typedef struct
{
    unsigned short msg_id;
    unsigned short task;
    cfg_value_t value;
} cfg_recv_msg_t;

// The actual task.
void configuration(void *arg);
