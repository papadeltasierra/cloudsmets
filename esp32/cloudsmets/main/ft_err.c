/*
 * Copyright (c) 2023 Paul D.smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * FreeTOS return code error checking macros similar to the ESP_ERROR macros.
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/error-handling.html#error-handling
 */

void _ft_error_check_failed(ByteType_t rc, const char *file, int line, const char *function, const char *expression) __attribute__((noreturn));
