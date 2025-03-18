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
#include <string.h>

SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size)
{
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

SStrResult sstr_clear(SStr *s)
{
    if (s == NULL || s->data == NULL) {
        return SSTR_ERROR_NULL;
    }

    s->length = 0;
    s->data[0] = '\0';

    return SSTR_SUCCESS;
}

SStrResult sstr_copy(SStr *dest, const char *src)
{
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
