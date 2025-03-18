#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* Internal helper to safely format strings */
static int safe_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    if (str == NULL || format == NULL) {
        return -1;
    }
    
    /* Ensure null termination even if buffer size is 0 */
    if (size > 0) {
        str[0] = '\0';
    }
    
    int result = vsnprintf(str, size, format, ap);
    
    /* Ensure null termination even if vsnprintf implementation is broken */
    if (result >= 0 && (size_t)result >= size && size > 0) {
        str[size - 1] = '\0';
    }
    
    return result;
}

int sstr_vformat(SStr *dest, const char *fmt, va_list args) {
    if (dest == NULL || dest->data == NULL || fmt == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    va_list args_copy;
    va_copy(args_copy, args);
    
    /* First, try the format to get the required length */
    int needed_length = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    if (needed_length < 0) {
        return SSTR_ERROR_FORMAT;
    }
    
    /* Check if there's enough space */
    if ((size_t)needed_length > dest->capacity) {
#if SSTR_DEFAULT_POLICY == SSTR_ERROR
        return SSTR_ERROR_OVERFLOW;
#else
        /* Truncate the output */
        int result = safe_vsnprintf(dest->data, dest->capacity + 1, fmt, args);
        dest->length = dest->capacity;
        return result;
#endif
    }
    
    /* Format into the buffer */
    int result = safe_vsnprintf(dest->data, dest->capacity + 1, fmt, args);
    
    if (result >= 0) {
        dest->length = (size_t)result;
    }
    
    return result;
}

int sstr_format(SStr *dest, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = sstr_vformat(dest, fmt, args);
    va_end(args);
    return result;
}
