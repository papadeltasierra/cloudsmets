/**
 * It is helpful to have a string-with-length structure.  Note that the length
 * is the length WITHOUT NULL but we always NULL terminate so we can trace
 * values.
*/
typedef struct
{
    uint8_t *value;
    size_t length;
} cs_string;

#define CS_STRING_FREE(X)                                                   \
        if ((X)->value != NULL)                                             \
        {                                                                   \
            free((X)->value);                                               \
            (X)->value = NULL;                                              \
            (X)->length = 0;                                                \
        }

#define CS_STRING_MALLOC(DST, LEN)                                          \
    (DST)->value = malloc(LEN);                                             \
    if ((DST)->value == NULL)                                               \
    {                                                                       \
        ESP_LOGE(TAG, "malloc failed for length: %d", (LEN));               \
        ESP_ERROR_CHECK(ESP_FAIL);                                          \
    }
