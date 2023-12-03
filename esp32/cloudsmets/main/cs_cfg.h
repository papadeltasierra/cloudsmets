/*
 * Namespaces
 */
extern const char *cfgSoftAp;
extern const char *cfgWifi;
extern const char *cfgWeb;
extern const char *cfgDbg;
extern const char *cfgOta;
extern const char *cfgAz;

#define CFG_NMSP_SOFTAP         cfgSoftAo
#define CFG_NMSP_WIFI           cfgWifi
#define CFG_NMSP_WEB            cfgWeb
#define CFG_NMSP_DBG            cfgDbg
#define CFG_NMSP_OTA            cfgOta
#define CFG_NMSP_AZURE          cfgAz

/*
 * Keys
 */
extern const char *cfgSoftApSsid;
extern const char *cfgSoftApPwd;
extern const char *cfgWifiSsid;
extern const char *cfgWifiPwd;
extern const char *cfgWebUser;
extern const char *cfgWebPwd;
extern const char *cfgDbgFunc;
extern const char *cfgDbgBaud;
extern const char *cfgDbgIpPort;
extern const char *cfgDbgEsp32c3;
extern const char *cfgDbgTlsr8258;
extern const char *cfgOtaFunc;
extern const char *cfgOtaUrl;
extern const char *cfgOtaRel;
extern const char *cfgAzFunc;
extern const char *cfgAzIotHub;
extern const char *cfgAzDevice;
extern const char *cfgAzCnct1;
extern const char *cfgAzCnct2;

#define CFG_KEY_SOFTAP_SSID     cfgSoftApSsid
#define CFG_KEY_SOFTAP_PWD      cfgSoftApPwd
#define CFG_KEY_WIFI_SSID       cfgWifiSsid
#define CFG_KEY_WIFI_PWD        cfgWifiPwd
#define CFG_KEY_WEB_USER        cfgWebUser
#define CFG_KEY_WEB_PWD         cfgWebPwd
#define CFG_KEY_DBG_FUNC        cfgDbgFunc
#define CFG_KEY_DBG_BAUD        cfgDbgBaud
#define CFG_KEY_DBG_IP_PORT     cfgDbgIpPort
#define CFG_KEY_DBG_ESP32C3     cfgDbgEsp32c3
#define CFG_KEY_DBG_TLSR8258    cfgDbgTlsr8258
#define CFG_KEY_OTA_FUNC        cfgOtaFunc
#define CFG_KEY_OTA_URL         cfgOtaUrl
#define CFG_KEY_OTA_REL         cfgOtaRel
#define CFG_KEY_AZURE_FUNC      cfgAzFunc
#define CFG_KEY_AZURE_IOTHUB    cfgAzIotHub
#define CFG_KEY_AZURE_DEVICE    cfgAzDevice
#define CFG_KEY_AZURE_CON1      cfgAzCnct1
#define CFG_KEY_AZURE_CON2      cfgAzCnct2

/*
 * Define some structures to make writing the web server simpler.
 */
typedef struct cfgDefinitions_t
{
    const char *key;
    nvs_type_t type;
    size_t length;
};

/*
 * Web server helper definitions.
 */
extern cfgDefinitions_t cfgSoftApDefinitions[];
extern cfgDefinitions_t cfgWifiDefinitions[];
extern cfgDefinitions_t cfgWebDefinitions[];
extern cfgDefinitions_t cfgDbgDefinitions[];
extern cfgDefinitions_t cfgWifiDefinitions[];

/*
 * Interface
 */

extern void cfgInit(void);
extern void cfgDefaultUint8(char *namespace, char *key, uint8_t default);
extern void cfgDefaultStr(char *namespace, char *key, char * default);
extern void cfgReadUnint8(char *namespace, char *key);
extern void cfgReadStr(char *namespace, char *key);
extern void cfgWriteUint8(char *namespace, char *key, );
extern void cfgWriteStr(char *namespace, char *key);
