/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/*
 * Klee Verification Harness for sstr_append function
 */

#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"
#include <stdlib.h>
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

/* Helper function to mimic internal behavior of sstr_bounded_strlen */
static size_t strlen_test(const char *str) {
    size_t len = 0;

    /* Use a regular loop for Klee */
    for (size_t i = 0; i < 10; i++) {
        if (str[i] == '\0') {
            return len;
        }
        len++;
    }

    return len;
}

/* Klee verification harness */
int main() {
    /* Create a small buffer for the destination string */
    const size_t DEST_SIZE = 10;
    char dest_buffer[DEST_SIZE];
    SStr dest;

    /* Initialize the destination string */
    sstr_init(&dest, dest_buffer, DEST_SIZE);

    /* Set a fixed initial length for the destination */
    size_t initial_length = 5; /* Use a fixed value to simplify verification */
    klee_assume(initial_length < DEST_SIZE);
    dest.length = initial_length;

    /* Put some content in the destination string */
    dest_buffer[0] = 'H';
    dest_buffer[1] = 'e';
    dest_buffer[2] = 'l';
    dest_buffer[3] = 'l';
    dest_buffer[4] = 'o';
    dest_buffer[5] = '\0';  /* Null-terminate */

    /* Create a source string buffer */
    const size_t SRC_SIZE = 10;
    char src_buffer[SRC_SIZE];

    /* Make the source buffer symbolic */
    klee_make_symbolic(src_buffer, SRC_SIZE, "src_buffer");

    /* Ensure the buffer has a null terminator somewhere */
    size_t null_pos;
    klee_make_symbolic(&null_pos, sizeof(null_pos), "null_pos");
    klee_assume(null_pos < SRC_SIZE);
    src_buffer[null_pos] = '\0';

    /* Calculate the length of the source string */
    size_t src_len = strlen_test(src_buffer);

    /* Keep track of the available space */
    size_t available = dest.capacity - dest.length;

    /* Call the function under test */
    SStrResult result = sstr_append(&dest, src_buffer);

    /* Verify post-conditions */
    if (result == SSTR_SUCCESS) {
        /* Verify the string was appended properly */
        if (src_len <= available) {
            /* Should have grown by exactly src_len */
            klee_assert(dest.length == initial_length + src_len &&
                        "Length is correct after append");

            /* Verify the first part of the string is unchanged */
            klee_assert(dest.data[0] == 'H' && "First character preserved");
            klee_assert(dest.data[1] == 'e' && "Second character preserved");
            klee_assert(dest.data[2] == 'l' && "Third character preserved");
            klee_assert(dest.data[3] == 'l' && "Fourth character preserved");
            klee_assert(dest.data[4] == 'o' && "Fifth character preserved");
        } else {
            /* Policy-dependent behavior */
            if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
                /* Should not succeed with ERROR policy when overflow */
                klee_assert(0 && "Should have returned error, not success");
            } else {
                /* Truncated to capacity */
                klee_assert(dest.length == dest.capacity &&
                           "Length uses maximum capacity after truncated append");
            }
        }

        /* String should be null-terminated */
        klee_assert(dest.data[dest.length] == '\0' &&
                    "String is null-terminated after append");
    } else if (result == SSTR_ERROR_OVERFLOW) {
        /* Must be overflow policy with a string too large */
        klee_assert(SSTR_DEFAULT_POLICY == SSTR_ERROR && "Error returned only with ERROR policy");
        klee_assert(src_len > available && "Overflow error implies string too long");
    }

    return 0;
}
