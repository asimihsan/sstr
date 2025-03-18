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
#include "../include/sstr/cbmc_stubs.h"
#include <string.h>
#include <assert.h>

SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size)
{
    /* Example CBMC verification: Use __CPROVER_assume to constrain inputs if needed */
    /* __CPROVER_assume(buffer_size > 0); */  /* Uncomment to assume this precondition */

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
        __CPROVER_assert(__CPROVER_w_ok(dest->data + dest->length + i, 1), "Destination write is safe");
        
        __CPROVER_assert(i < src_len, "Append index is within source length");
        __CPROVER_assert(dest->length + i < dest->capacity, "Append index is within destination capacity");
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
