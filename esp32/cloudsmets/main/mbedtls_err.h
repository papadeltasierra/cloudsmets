/**
 * Macro which can be used to check the error code,
 * and terminate the program in case the code is not mbedtlsTrue.
 * Prints the error code, error location, and the failed statement to serial output.
 *
 * Disabled if assertions are disabled.
 */
#ifdef NDEBUG
#define MBEDTLS_ERROR_CHECK(x) do {                                     \
        int mbedtls_rc = (x);                                           \
        (void) sizeof(mbedtls_rc);                                      \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define MBEDTLS_ERROR_CHECK(x) do {                                     \
        int mbedtls_rc = (x);                                           \
        if (unlikely(mbedtls_rc != 0)) {                                \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define MBEDTLS_ERROR_CHECK(x) do {                                     \
        int mbedtls_rc = (x);                                           \
        if (unlikely(mbedtls_rc != 0)) {                                \
            _esp_error_check_failed(mbedtls_rc, __FILE__, __LINE__,     \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif