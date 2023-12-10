/**
 * Macro which can be used to check the error code,
 * and terminate the program in case the code is not pdTrue.
 * Prints the error code, error location, and the failed statement to serial output.
 *
 * Disabled if assertions are disabled.
 */
#ifdef NDEBUG
#define XML_ERROR_CHECK(x) do {                                         \
        enum XML_Status xml_rc = (x);                                   \
        (void) sizeof(xml_rc);                                          \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define XML_ERROR_CHECK(x) do {                                         \
        enum XML_Status xml_rc = (x);                                   \
        if (unlikely(xml_rc != XML_STATUS_OK)) {                        \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define XML_ERROR_CHECK(x) do {                                         \
        enum XML_Status xml_rc = (x);                                   \
        if (unlikely(xml_rc != XML_STATUS_OK)) {                        \
            _esp_error_check_failed(xml_rc, __FILE__, __LINE__,         \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif