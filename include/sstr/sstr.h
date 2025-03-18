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
 */

#ifndef SSTR_H
#define SSTR_H

#include <stddef.h>
#include <stdarg.h>

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
 * Copy from one SStr to another
 * 
 * @param dest Destination SStr
 * @param src Source SStr
 * @return SSTR_SUCCESS or error code
 */
SStrResult sstr_copy_sstr(SStr *dest, const SStr *src);

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

#endif /* SSTR_H */
