/**
 * Macro which can be used to check the error code,
 * and terminate the program in case the code is not esp_mqttTrue.
 * Presp_err_ts the error code, error location, and the failed statement to serial output.
 *
 * Disabled if assertions are disabled.
 */
#ifdef NDEBUG
#define ESP_MQTT_ERROR_CHECK(x) do {                                    \
        esp_err_t esp_mqtt_rc = (x);                                    \
        (void) sizeof(esp_mqtt_rc);                                     \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define ESP_MQTT_ERROR_CHECK(x) do {                                    \
        esp_err_t esp_mqtt_rc = (x);                                    \
        if (unlikely(esp_mqtt_rc != ESP_OK)) {                          \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define ESP_MQTT_ERROR_CHECK(x) do {                                    \
        esp_err_t esp_mqtt_rc = (x);                                    \
        if (unlikely(esp_mqtt_rc != ESP_OK)) {                          \
            _esp_error_check_failed(esp_mqtt_rc, __FILE__, __LINE__,    \
                                __ASSERT_FUNC, #x);                     \
        }                                                               \
    } while(0)
#endif