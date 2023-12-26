#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

/**
* @brief CloudSMETS configuration declarations
*
*/
typedef enum {
    CS_CONFIG_CHANGE,               /*!< Some configuration has changed */
} cs_config_event_t;

/**
 * TODO: Comment here
 * TODO: CS_TIME elsewhere.
 */
ESP_EVENT_DECLARE_BASE(CS_CONFIG_EVENT);
ESP_EVENT_DECLARE_BASE(CS_TIME_EVENT);

/*
 * Namespaces and task names.
 */
extern const char *cs_app_task_name;
extern const char *cs_wifi_task_name;
extern const char *cs_web_task_name;
// extern const char *cfgDbg;
extern const char *cs_ota_task_name;
extern const char *cs_mqtt_task_name;

#define CS_CFG_NMSP_WIFI           cs_wifi_task_name
#define CS_CFG_NMSP_WEB            cs_web_task_name
// #define CS_CFG_NMSP_DBG            cfgDbg
#define CS_CFG_NMSP_OTA            cs_ota_task_name
#define CS_CFG_NMSP_MQTT           cs_mqtt_task_name

// TODO: Have to decide whether really supporting anythong other than Azure.

/*
 * Keys
 */
extern const char cs_cfg_wifi_ap_chnl[];
extern const char cs_cfg_wifi_ap_ssid[];
extern const char cs_cfg_wifi_ap_pwd[];
extern const char cs_cfg_wifi_sta_ssid[];
extern const char cs_cfg_wifi_sta_pwd[];
extern const char cs_cfg_web_port[];
// extern const char cs_cfg_web_user[];
// extern const char cs_cfg_web_pwd[];
// extern const char cfg_log_func[];
// extern const char cfg_log_baud[];
// extern const char cfg_log_ip_port[];
// extern const char cfg_log_esp32c3[];
// extern const char cfg_log_tlsr8258[];
extern const char cs_cfg_ota_ena[];
extern const char cs_cfg_ota_rel[];
extern const char cs_cfg_ota_dev[];
extern const char cs_cfg_ota_rev_url[];
extern const char cs_cfg_ota_img_url[];
extern const char cs_cfg_ota_accept[];
extern const char cs_cfg_mqtt_ena[];
extern const char cs_cfg_mqtt_iothub[];
extern const char cs_cfg_mqtt_device[];
extern const char cs_cfg_mqtt_key1[];
extern const char cs_cfg_mqtt_key2[];

#define CS_CFG_KEY_WIFI_AP_CHNL    cs_cfg_wifi_ap_chnl
#define CS_CFG_KEY_WIFI_AP_SSID    cs_cfg_wifi_ap_ssid
#define CS_CFG_KEY_WIFI_AP_PWD     cs_cfg_wifi_ap_pwd
#define CS_CFG_KEY_WIFI_STA_SSID   cs_cfg_wifi_sta_ssid
#define CS_CFG_KEY_WIFI_STA_PWD    cs_cfg_wifi_sta_pwd
#define CS_CFG_KEY_WEB_PORT        cs_cfg_web_port
// #define CS_CFG_KEY_WEB_USER        cs_cfg_webUser
// #define CS_CFG_KEY_WEB_PWD         cs_cfg_webPwd
// #define CS_CFG_KEY_DBG_FUNC        cfgDbgFunc
// #define CS_CFG_KEY_DBG_BAUD        cfgDbgBaud
// #define CS_CFG_KEY_DBG_IP_PORT     cfgDbgIpPort
// #define CS_CFG_KEY_DBG_ESP32C3     cfgDbgEsp32c3
// #define CS_CFG_KEY_DBG_TLSR8258    cfgDbgTlsr8258
#define CS_CFG_KEY_OTA_ENA         cs_cfg_ota_ena
#define CS_CFG_KEY_OTA_DEV         cs_cfg_ota_dev
#define CS_CFG_KEY_OTA_REL         cs_cfg_ota_rel
#define CS_CFG_KEY_OTA_IMG_URL     cs_cfg_ota_img_url
#define CS_CFG_KEY_OTA_REV_URL     cs_cfg_ota_rev_url
#define CS_CFG_KEY_OTA_ACCEPT      cs_cfg_ota_accept

#define CS_CFG_KEY_MQTT_ENA        cs_cfg_mqtt_ena
#define CS_CFG_KEY_MQTT_IOTHUB     cs_cfg_mqtt_iothub
#define CS_CFG_KEY_MQTT_DEVICE     cs_cfg_mqtt_device
#define CS_CFG_KEY_MQTT_KEY1       cs_cfg_mqtt_key1
#define CS_CFG_KEY_MQTT_KEY2       cs_cfg_mqtt_key2

/*
 * Define some structures to make writing the web server simpler.
 */
typedef struct
{
    const char *key;
    nvs_type_t type;
    size_t length;
} cs_cfg_definitions_t;

/*
 * Web server helper definitions.
 */
extern const cs_cfg_definitions_t cs_cfg_wifi_definitions[];
extern const cs_cfg_definitions_t cs_cfg_web_definitions[];
// extern const cs_cfg_definitions_t cs_cfg_log_definitions[];
extern const cs_cfg_definitions_t cs_cfg_ota_definitions[];
extern const cs_cfg_definitions_t cs_cfg_mqtt_definitions[];

/*
 * Interface
 */
extern void cs_cfg_init(void);
extern void cs_cfg_default_uint8(const char *ns, const char *key, uint8_t def);
extern void cs_cfg_default_uint16(const char *ns, const char *key, uint16_t def);
extern void cs_cfg_default_uint32(const char *ns, const char *key, uint32_t def);
extern void cs_cfg_default_str(const char *ns, const char *key, const char * def);
extern void cs_cfg_read_uint8(const char *ns, const char *key, uint8_t *value);
extern void cs_cfg_read_uint16(const char *ns, const char *key, uint16_t *value);
extern void cs_cfg_read_uint32(const char *ns, const char *key, uint32_t *value);
extern void cs_cfg_read_str(const char *ns, const char *key, char **value, size_t *length);
extern void cs_cfg_write_uint8(const char *ns, const char *key, uint8_t value);
extern void cs_cfg_write_uint16(const char *ns, const char *key, uint16_t value);
extern void cs_cfg_write_uint32(const char *ns, const char *key, uint32_t value);
extern void cs_cfg_write_str(const char *ns, const char *key, const char *value);
