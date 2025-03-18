/**
 * SStr - Safe String Library for Embedded Systems
 * Single-include version
 * 
 * A minimal, bounds-checked string handling library designed for embedded systems
 * with no dynamic memory allocation.
 */

#ifndef SSTR_H
#define SSTR_H

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Default truncation policy for string operations.
 * Can be overridden by the user before including sstr.h.
 */
#ifndef SSTR_DEFAULT_POLICY
#define SSTR_DEFAULT_POLICY SSTR_ERROR
#endif

/**
 * Maximum allowed size for a single string.
 * This prevents integer overflow in size calculations.
 */
#ifndef SSTR_MAX_SIZE
#define SSTR_MAX_SIZE ((size_t)0x7FFFFFFF)
#endif

/**
 * Define format specifiers to handle.
 */
#ifndef SSTR_ENABLE_FLOAT_FORMAT
#define SSTR_ENABLE_FLOAT_FORMAT 1
#endif

/**
 * Result codes for SStr operations
 */
typedef enum {
    SSTR_SUCCESS = 0,            /* Operation completed successfully */
    SSTR_ERROR_NULL = -1,        /* NULL pointer parameter */
    SSTR_ERROR_OVERFLOW = -2,    /* Destination buffer too small */
    SSTR_ERROR_FORMAT = -3,      /* Invalid format string */
    SSTR_ERROR_ARGUMENT = -4     /* Invalid argument (e.g., NULL string for %s) */
} SStrResult;

/**
 * Truncation policy for string operations
 */
typedef enum {
    SSTR_TRUNCATE,     /* Truncate output when buffer is too small */
    SSTR_ERROR         /* Return error when buffer is too small */
} SStrTruncationPolicy;

/**
 * SStr structure - contains a pointer to a stack-allocated buffer
 * along with capacity and current length information
 */
typedef struct {
    char   *data;     /* Points to stack-allocated buffer */
    size_t  capacity; /* Maximum usable characters (excluding null terminator) */
    size_t  length;   /* Current string length */
} SStr;

/**
 * Initialize an SStr structure with a stack-allocated buffer
 * 
 * @param s Pointer to SStr structure to initialize
 * @param buffer Pointer to stack-allocated character buffer
 * @param buffer_size Size of the buffer in bytes
 * @return SSTR_SUCCESS or error code
 */
static inline SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size) {
    if (s == NULL || buffer == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    if (buffer_size == 0) {
        return SSTR_ERROR_OVERFLOW;
    }
    
    s->data = buffer;
    s->capacity = buffer_size - 1; /* Reserve space for null terminator */
    s->length = 0;
    s->data[0] = '\0';
    
    return SSTR_SUCCESS;
}

/**
 * Reset a string to empty (zero length)
 * 
 * @param s Pointer to SStr structure
 * @return SSTR_SUCCESS or error code
 */
static inline SStrResult sstr_clear(SStr *s) {
    if (s == NULL || s->data == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    s->length = 0;
    s->data[0] = '\0';
    
    return SSTR_SUCCESS;
}

/**
 * Copy a C string into an SStr
 * 
 * @param dest Destination SStr
 * @param src Source C string
 * @return SSTR_SUCCESS or error code
 */
static inline SStrResult sstr_copy(SStr *dest, const char *src) {
    if (dest == NULL || dest->data == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    if (src == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    size_t src_len = strlen(src);
    
    if (src_len > dest->capacity) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        src_len = dest->capacity;
    }
    
    memcpy(dest->data, src, src_len);
    dest->data[src_len] = '\0';
    dest->length = src_len;
    
    return SSTR_SUCCESS;
}

/**
 * Copy from one SStr to another
 * 
 * @param dest Destination SStr
 * @param src Source SStr
 * @return SSTR_SUCCESS or error code
 */
static inline SStrResult sstr_copy_sstr(SStr *dest, const SStr *src) {
    if (dest == NULL || dest->data == NULL || src == NULL || src->data == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    if (src->length > dest->capacity) {
#if SSTR_DEFAULT_POLICY == SSTR_ERROR
        return SSTR_ERROR_OVERFLOW;
#else
        size_t copy_len = dest->capacity;
        memcpy(dest->data, src->data, copy_len);
        dest->data[copy_len] = '\0';
        dest->length = copy_len;
#endif
    } else {
        memcpy(dest->data, src->data, src->length + 1); /* +1 for null terminator */
        dest->length = src->length;
    }
    
    return SSTR_SUCCESS;
}

/**
 * Append a C string to an SStr
 * 
 * @param dest Destination SStr
 * @param src Source C string to append
 * @return SSTR_SUCCESS or error code
 */
static inline SStrResult sstr_append(SStr *dest, const char *src) {
    if (dest == NULL || dest->data == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    if (src == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    size_t src_len = strlen(src);
    size_t available = dest->capacity - dest->length;
    
    if (src_len > available) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        src_len = available;
    }
    
    memcpy(dest->data + dest->length, src, src_len);
    dest->length += src_len;
    dest->data[dest->length] = '\0';
    
    return SSTR_SUCCESS;
}

/**
 * Append one SStr to another
 * 
 * @param dest Destination SStr
 * @param src Source SStr to append
 * @return SSTR_SUCCESS or error code
 */
static inline SStrResult sstr_append_sstr(SStr *dest, const SStr *src) {
    if (dest == NULL || dest->data == NULL || src == NULL || src->data == NULL) {
        return SSTR_ERROR_NULL;
    }
    
    size_t available = dest->capacity - dest->length;
    
    if (src->length > available) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        size_t copy_len = available;
        memcpy(dest->data + dest->length, src->data, copy_len);
        dest->length += copy_len;
        dest->data[dest->length] = '\0';
    } else {
        memcpy(dest->data + dest->length, src->data, src->length);
        dest->length += src->length;
        dest->data[dest->length] = '\0';
    }
    
    return SSTR_SUCCESS;
}

/* Internal helper to safely format strings */
static inline int safe_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
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

/**
 * Format a string into an SStr with va_list
 * 
 * @param dest Destination SStr
 * @param fmt Format string
 * @param args Variable argument list
 * @return Number of characters written or negative error code
 */
static inline int sstr_vformat(SStr *dest, const char *fmt, va_list args) {
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

/**
 * Format a string into an SStr (printf-style)
 * 
 * @param dest Destination SStr
 * @param fmt Format string
 * @param ... Format arguments
 * @return Number of characters written or negative error code
 */
static inline int sstr_format(SStr *dest, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = sstr_vformat(dest, fmt, args);
    va_end(args);
    return result;
}

#ifdef __cplusplus
}
#endif

#endif /* SSTR_H */
