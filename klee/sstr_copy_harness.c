/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/*
 * Klee Verification Harness for sstr_copy function
 */

#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"
#include <stdlib.h>

// Include Klee header for klee_make_symbolic and klee_assume
#include "klee/klee.h"

#ifndef SSTR_ERROR
#define SSTR_ERROR 0
#endif

#ifndef SSTR_TRUNCATE
#define SSTR_TRUNCATE 1
#endif

#ifndef SSTR_DEFAULT_POLICY
#define SSTR_DEFAULT_POLICY SSTR_ERROR
#endif

/* Helper function to mimic internal behavior of sstr_bounded_strlen
 * but with a fixed unwinding bound */
static size_t strlen_test(const char *str) {
    size_t len = 0;

    // Use a regular loop for Klee (unlike CBMC, Klee can handle loops well)
    for (size_t i = 0; i < 10; i++) {
        if (str[i] == '\0') {
            return len;
        }
        len++;
    }

    return len;
}

int main() {
    /* Create a small buffer for the destination string */
    const size_t DEST_SIZE = 10;
    char dest_buffer[DEST_SIZE];
    SStr dest;

    /* Initialize the destination string */
    sstr_init(&dest, dest_buffer, DEST_SIZE);

    /* Create a small source string buffer */
    const size_t SRC_SIZE = 10;
    char src_buffer[SRC_SIZE];

    /* Make the source buffer symbolic */
    klee_make_symbolic(src_buffer, SRC_SIZE, "src_buffer");

    /* Ensure the buffer has a null terminator somewhere */
    size_t null_pos;
    klee_make_symbolic(&null_pos, sizeof(null_pos), "null_pos");
    klee_assume(null_pos < SRC_SIZE);
    src_buffer[null_pos] = '\0';

    /* Calculate the actual length using strlen */
    size_t actual_len = strlen_test(src_buffer);

    /* Call the function under test */
    SStrResult result = sstr_copy(&dest, src_buffer);

    /* Verify post-conditions based on the behavior of sstr_copy */
    if (result == SSTR_SUCCESS) {
        /* The actual length should be either the original string length or the capacity,
         * depending on whether truncation occurred */
        if (actual_len <= dest.capacity) {
            /* No truncation needed, full string fits */
            klee_assert(dest.length == actual_len && "Length is set correctly with no truncation");
        } else {
            /* Truncation happened or would have returned error */
            if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
                /* Should not succeed in this case */
                klee_assert(0 && "Should have returned error, not success");
            } else {
                /* Truncated to capacity */
                klee_assert(dest.length == dest.capacity && "Length is set to capacity after truncation");
            }
        }

        /* String must be null-terminated */
        klee_assert(dest.data[dest.length] == '\0' && "String is null-terminated after copy");
    } else if (result == SSTR_ERROR_OVERFLOW) {
        /* Must be overflow policy with a string too large */
        klee_assert(SSTR_DEFAULT_POLICY == SSTR_ERROR && "Error returned only with ERROR policy");
        klee_assert(actual_len > dest.capacity && "Overflow error implies string too long");
    }

    return 0;
}
