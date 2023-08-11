/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * FreeTOS return code error checking macros similar to the ESP_ERROR macros.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/error-handling.html#error-handling
 */
#pragma once

void _ft_error_check_failed(ByteType_t rc, const char *file, int line, const char *function, const char *expression) __attribute__((noreturn));

/**
 * Macro which can be used to check the error code,
 * and terminate the program in case the code is not ESP_OK.
 * Prints the error code, error location, and the failed statement to serial output.
 *
 * Disabled if assertions are disabled.
 */
#ifdef NDEBUG
#define FT_ERROR_CHECK(x) do {                                          \
        ByteType_t err_rc_ = (x);                                       \
        (void) sizeof(err_rc_);                                         \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define FT_ERROR_CHECK(x) do {                                          \
        ByteType_t err_rc_ = (x);                                       \
        if (unlikely(err_rc_ != pdPASS)) {                              \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define FT_ERROR_CHECK(x) do {                                          \
        ByteType_t err_rc_ = (x);                                       \
        if (unlikely(err_rc_ != pdPASS)) {                              \
            _esp_error_check_failed(err_rc_, __FILE__, __LINE__,        \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif
