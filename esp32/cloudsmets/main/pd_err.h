/**
 * Macro which can be used to check the error code,
 * and terminate the program in case the code is not pdTrue.
 * Prints the error code, error location, and the failed statement to serial output.
 *
 * Disabled if assertions are disabled.
 */
#ifdef NDEBUG
#define PD_ERROR_CHECK(x) do {                                          \
        BaseType_t pd_rc = (x);                                         \
        (void) sizeof(pd_rc);                                           \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define PD_ERROR_CHECK(x) do {                                          \
        BaseType_t pd_rc = (x);                                         \
        if (unlikely(pd_rc != pdTRUE)) {                                \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define PD_ERROR_CHECK(x) do {                                          \
        BaseType_t pd_rc = (x);                                         \
        if (unlikely(pd_rc != pdTRUE)) {                                \
            _esp_error_check_failed(pd_rc, __FILE__, __LINE__,          \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif