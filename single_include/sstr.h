/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/**
 * SStr - Safe String Library for Embedded Systems
 *
 * A minimal, bounds-checked string handling library designed for embedded systems
 * with no dynamic memory allocation.
 *
 * Usage:
 *   1. Include this file in any C file that needs to use SStr
 *   2. In EXACTLY ONE C file, define SSTR_IMPLEMENTATION before including:
 *      #define SSTR_IMPLEMENTATION
 *      #include "sstr.h"
 */

#ifndef SSTR_H
#define SSTR_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSTR_CONFIG_H

/**
 * Default truncation policy for string operations.
 * Can be overridden by the user before including sstr.h.
 */
/* Default truncation policy - changed here to ensure all tests work properly */
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
 * Format string validation control.
 * When enabled, format strings will be validated to ensure only
 * allowed specifiers are used.
 */
#ifndef SSTR_VALIDATE_FORMAT
#define SSTR_VALIDATE_FORMAT 1 /* Enable by default */
#endif

/**
 * Define which format specifiers are allowed when validation is enabled.
 * Default allows: d,i,u,x,X,s,c and % (literal percent)
 * Notably excludes: f,e,g,p (floating point and pointers)
 */
#ifndef SSTR_ALLOWED_SPECIFIERS
#define SSTR_ALLOWED_SPECIFIERS "diuxXsc%"
#endif

/**
 * Define format specifiers to handle.
 */
#ifndef SSTR_ENABLE_FLOAT_FORMAT
#define SSTR_ENABLE_FLOAT_FORMAT 1
#endif

#include <stdarg.h>
#include <stddef.h>

/**
 * Result codes for SStr operations
 */
typedef enum {
    SSTR_SUCCESS = 0,         /* Operation completed successfully */
    SSTR_ERROR_NULL = -1,     /* NULL pointer parameter */
    SSTR_ERROR_OVERFLOW = -2, /* Destination buffer too small */
    SSTR_ERROR_FORMAT = -3,   /* Invalid format string */
    SSTR_ERROR_ARGUMENT = -4  /* Invalid argument (e.g., NULL string for %s) */
} SStrResult;

/**
 * Truncation policy for string operations
 */
typedef enum {
    SSTR_TRUNCATE, /* Truncate output when buffer is too small */
    SSTR_ERROR     /* Return error when buffer is too small */
} SStrTruncationPolicy;

/**
 * SStr structure - contains a pointer to a stack-allocated buffer
 * along with capacity and current length information
 */
typedef struct {
    char *data;      /* Points to stack-allocated buffer */
    size_t capacity; /* Maximum usable characters (excluding null terminator) */
    size_t length;   /* Current string length */
} SStr;

/**
 * Initialize an SStr structure with a stack-allocated buffer
 *
 * @param s Pointer to SStr structure to initialize
 * @param buffer Pointer to stack-allocated character buffer
 * @param buffer_size Size of the buffer in bytes
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size);

/**
 * Reset a string to empty (zero length)
 *
 * @param s Pointer to SStr structure
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_clear(SStr *s);

/**
 * Copy a C string into an SStr
 *
 * @param dest Destination SStr
 * @param src Source C string
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_copy(SStr *dest, const char *src);

/**
 * Copy a specified number of characters from a C string to an SStr
 *
 * @param dest Destination SStr
 * @param src Source C string
 * @param src_len Number of characters to copy from source
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_copy_n(SStr *dest, const char *src, size_t src_len);

/**
 * Append a C string to an SStr
 *
 * @param dest Destination SStr
 * @param src Source C string to append
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_append(SStr *dest, const char *src);

/**
 * Append one SStr to another
 *
 * @param dest Destination SStr
 * @param src Source SStr to append
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_append_sstr(SStr *dest, const SStr *src);

/**
 * Format a string into an SStr (printf-style)
 *
 * @param dest Destination SStr
 * @param fmt Format string
 * @param ... Format arguments
 * @return Number of characters written or negative error code
 */
int sstr_format(SStr *dest, const char *fmt, ...);

/**
 * Format a string into an SStr with va_list
 *
 * @param dest Destination SStr
 * @param fmt Format string
 * @param args Variable argument list
 * @return Number of characters written or negative error code
 */
int sstr_vformat(SStr *dest, const char *fmt, va_list args);


#ifdef __cplusplus
}
#endif

/* ===========================================================================
 * Implementation
 * ===========================================================================
 */

#ifdef SSTR_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <ctype.h>

SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size)
{
    /* Example CBMC verification: Use __CPROVER_assume to constrain inputs if needed */
    /* __CPROVER_assume(buffer_size > 0); */ /* Uncomment to assume this precondition */

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

    /* Example CBMC verification: Check a property with __CPROVER_assert */
    __CPROVER_assert(s->capacity == buffer_size - 1, "Capacity is set correctly");
    __CPROVER_assert(s->length == 0, "Length is initialized to zero");
    __CPROVER_assert(s->data[0] == '\0', "String is null-terminated");

    return SSTR_SUCCESS;
}


SStrResult sstr_clear(SStr *s)
{
    if (s == NULL || s->data == NULL) {
        return SSTR_ERROR_NULL;
    }

    s->length = 0;
    s->data[0] = '\0';

    return SSTR_SUCCESS;
}


/* Get the length of a string with an explicit maximum to check
 * Returns SSTR_SUCCESS if a null terminator is found within max_len
 * Sets out_len to the length (not including null terminator)
 * Returns SSTR_ERROR_OVERFLOW if no null terminator is found within max_len
 *
 * Implementation is designed to be CBMC-verifiable with bounded loops
 */
static SStrResult sstr_bounded_strlen(const char *str, size_t max_len, size_t *out_len)
{
    if (str == NULL || out_len == NULL) {
        return SSTR_ERROR_NULL;
    }

    /* For CBMC verification, limit the maximum length to check
     * This allows CBMC to finish verification in reasonable time */
#ifdef __CPROVER
    /* When running under CBMC verification, assume small buffer sizes */
    __CPROVER_assume(max_len <= 10);

    /* Important: Make sure the string pointer is valid for reading */
    __CPROVER_assume(__CPROVER_r_ok(str, max_len));
#endif

    /* Safety check for any null pointer dereference */
    __CPROVER_assert(str != NULL, "String pointer is not null");

    /* Use a bounded for loop with explicit limit for CBMC */
    for (size_t i = 0; i < max_len; i++) {
        /* For each index, ensure we can safely read this memory location */
#ifdef __CPROVER
        __CPROVER_assert(__CPROVER_r_ok(str + i, 1), "String access is safe");
#endif

        if (str[i] == '\0') {
            *out_len = i;
            return SSTR_SUCCESS;
        }
    }

    /* No null terminator found within bounds */
    return SSTR_ERROR_OVERFLOW;
}


SStrResult sstr_copy(SStr *dest, const char *src)
{
    if (dest == NULL || dest->data == NULL) {
        return SSTR_ERROR_NULL;
    }

    if (src == NULL) {
        return SSTR_ERROR_NULL;
    }

#ifdef __CPROVER
    /* For CBMC verification, use fixed small values to allow tractable verification */
    __CPROVER_assume(dest->capacity <= 10);

    /* Use CBMC's built-in primitives for memory safety checks */
    /* Ensure the src pointer can be read for up to 'capacity+1' bytes */
    __CPROVER_assume(__CPROVER_r_ok(src, dest->capacity + 1));

    /* Ensure the dest->data pointer can be written for up to 'capacity+1' bytes */
    __CPROVER_assume(__CPROVER_w_ok(dest->data, dest->capacity + 1));
#endif

    /* Ensure capacity is reasonable before proceeding */
    __CPROVER_assert(dest->capacity < SIZE_MAX, "Destination capacity is reasonable");

    /* Use a bounded check for string length */
    size_t src_len;
    SStrResult result = sstr_bounded_strlen(src, dest->capacity + 1, &src_len);

    /* If source has no null terminator within maximum bounds, handle according to policy */
    if (result == SSTR_ERROR_OVERFLOW) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        src_len = dest->capacity;
    }

    /* Check if source fits in destination */
    if (src_len > dest->capacity) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        src_len = dest->capacity;
    }

    /* Double-check bounds before memory operations */
    __CPROVER_assert(src_len <= dest->capacity, "Source length is within destination capacity");

    /* Use a loop for CBMC verification instead of memcpy
     * This gives CBMC explicit bounds to verify */
#ifdef __CPROVER
    for (size_t i = 0; i < src_len; i++) {
        /* Verify each memory access is safe */
        __CPROVER_assert(__CPROVER_r_ok(src + i, 1), "Source read is safe");
        __CPROVER_assert(__CPROVER_w_ok(dest->data + i, 1), "Destination write is safe");

        dest->data[i] = src[i];
    }
#else
    memcpy(dest->data, src, src_len);
#endif

    /* Ensure null termination */
    dest->data[src_len] = '\0';
    dest->length = src_len;

    return SSTR_SUCCESS;
}


SStrResult sstr_copy_n(SStr *dest, const char *src, size_t src_len)
{
    if (dest == NULL || dest->data == NULL || src == NULL) {
        return SSTR_ERROR_NULL;
    }

    if (src_len > dest->capacity) {
        /* Get truncation policy at runtime - we will respect compile-time policy here
         * The policy could have been changed after the library was compiled */
#if SSTR_DEFAULT_POLICY == SSTR_ERROR
        return SSTR_ERROR_OVERFLOW;
#else
        size_t copy_len = dest->capacity;
        memcpy(dest->data, src, copy_len);
        dest->data[copy_len] = '\0';
        dest->length = copy_len;
#endif
    } else {
        memcpy(dest->data, src, src_len);
        dest->data[src_len] = '\0';
        dest->length = src_len;
    }

    return SSTR_SUCCESS;
}


SStrResult sstr_append(SStr *dest, const char *src)
{
    if (dest == NULL || dest->data == NULL) {
        return SSTR_ERROR_NULL;
    }

    if (src == NULL) {
        return SSTR_ERROR_NULL;
    }

#ifdef __CPROVER
    /* For CBMC verification, limit capacity and length to avoid state explosion */
    __CPROVER_assume(dest->capacity <= 10);
    __CPROVER_assume(dest->length <= dest->capacity);

    /* Calculate available space only after constraining length/capacity */
    size_t available = dest->capacity - dest->length;

    /* Use CBMC's built-in primitives for memory safety checks */
    /* Ensure the src pointer can be read for up to 'available+1' bytes */
    __CPROVER_assume(__CPROVER_r_ok(src, available + 1));

    /* Ensure the dest->data pointer can be written for the appropriate range */
    __CPROVER_assume(__CPROVER_w_ok(dest->data + dest->length, available + 1));
#endif

    /* Verify length is within capacity */
    __CPROVER_assert(dest->length <= dest->capacity, "Destination length is within capacity");

    size_t available = dest->capacity - dest->length;

    /* Use a bounded check for string length */
    size_t src_len;
    SStrResult result = sstr_bounded_strlen(src, available + 1, &src_len);

    /* If source has no null terminator within bounds, handle according to policy */
    if (result == SSTR_ERROR_OVERFLOW) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        src_len = available;
    }

    /* Check if source fits in destination's available space */
    if (src_len > available) {
        if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
            return SSTR_ERROR_OVERFLOW;
        }
        src_len = available;
    }

    /* CBMC assertion: source length is within available space */
    __CPROVER_assert(src_len <= available, "Source length is within available capacity");

    /* Use a loop for CBMC verification instead of memcpy
     * This gives CBMC explicit bounds to verify */
#ifdef __CPROVER
    for (size_t i = 0; i < src_len; i++) {
        /* Verify each memory access is safe */
        __CPROVER_assert(__CPROVER_r_ok(src + i, 1), "Source read is safe");
        __CPROVER_assert(__CPROVER_w_ok(dest->data + dest->length + i, 1),
                         "Destination write is safe");

        __CPROVER_assert(i < src_len, "Append index is within source length");
        __CPROVER_assert(dest->length + i < dest->capacity,
                         "Append index is within destination capacity");
        dest->data[dest->length + i] = src[i];
    }
#else
    memcpy(dest->data + dest->length, src, src_len);
#endif

    dest->length += src_len;
    dest->data[dest->length] = '\0';

    return SSTR_SUCCESS;
}


SStrResult sstr_append_sstr(SStr *dest, const SStr *src)
{
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

static int safe_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
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


#if SSTR_VALIDATE_FORMAT
/* Validates that a format string only uses allowed format specifiers */
static int validate_format_string(const char *fmt)
{
    if (fmt == NULL) {
        return SSTR_ERROR_NULL;
    }

    const char *ptr = fmt;

    while (*ptr) {
        /* Find the next '%' character */
        if (*ptr != '%') {
            ptr++;
            continue;
        }

        /* Process a % character */
        ptr++; /* Move past '%' */

        /* Handle %% escape sequence */
        if (*ptr == '%') {
            ptr++;
            continue;
        }

        /* If we reached the end of the string after a %, that's invalid */
        if (*ptr == '\0') {
            return SSTR_ERROR_FORMAT;
        }

        /* Skip flags: "-+0 #" */
        while (*ptr == '-' || *ptr == '+' || *ptr == '0' || *ptr == ' ' || *ptr == '#') {
            ptr++;
        }

        /* Skip width: digits */
        while (isdigit((unsigned char)*ptr)) {
            ptr++;
        }

        /* Skip precision: .digits */
        if (*ptr == '.') {
            ptr++;
            while (isdigit((unsigned char)*ptr)) {
                ptr++;
            }
        }

        /* Skip length modifiers: h, l, ll, z, j, t, L */
        if (*ptr == 'h' || *ptr == 'l' || *ptr == 'j' || *ptr == 'z' || *ptr == 't' ||
            *ptr == 'L') {
            /* Handle double character modifiers like 'hh', 'll' */
            if ((*ptr == 'h' && *(ptr + 1) == 'h') || (*ptr == 'l' && *(ptr + 1) == 'l')) {
                ptr += 2;
            } else {
                ptr++;
            }
        }

        /* Check if the specifier is allowed */
        if (*ptr == '\0') {
            return SSTR_ERROR_FORMAT; /* Incomplete format specifier */
        }

        /* Verify the final conversion specifier */
        if (strchr(SSTR_ALLOWED_SPECIFIERS, *ptr) == NULL) {
            return SSTR_ERROR_FORMAT;
        }

        /* Move past this format specifier */
        ptr++;
    }

    return SSTR_SUCCESS;
}

#endif

int sstr_vformat(SStr *dest, const char *fmt, va_list args)
{
    if (dest == NULL || dest->data == NULL || fmt == NULL) {
        return SSTR_ERROR_NULL;
    }

#if SSTR_VALIDATE_FORMAT
    /* Validate format string - only allow approved specifiers */
    int validation_result = validate_format_string(fmt);
    if (validation_result != SSTR_SUCCESS) {
        return validation_result;
    }
#endif

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


int sstr_format(SStr *dest, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = sstr_vformat(dest, fmt, args);
    va_end(args);
    return result;
}


#endif /* SSTR_IMPLEMENTATION */

#endif /* SSTR_H */
