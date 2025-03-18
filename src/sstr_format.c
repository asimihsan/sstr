/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Internal helper to safely format strings */
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
