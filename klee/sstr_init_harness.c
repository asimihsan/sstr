/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/*
 * Klee Verification Harness for sstr_init function
 */

#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"
#include "klee/klee.h"

int main() {
    /* Declare variables for the function parameters */
    SStr str;

    /* Use a fixed buffer size to simplify verification */
    const size_t buffer_size = 10;

    /* Allocate a buffer of the chosen size */
    char buffer[buffer_size];

    /* Call the function under test */
    SStrResult result = sstr_init(&str, buffer, buffer_size);

    /* Verify post-conditions */
    if (result == SSTR_SUCCESS) {
        /* Check that the string was initialized properly */
        klee_assert(str.data == buffer && "Data pointer is set correctly");
        klee_assert(str.capacity == buffer_size - 1 && "Capacity is set correctly");
        klee_assert(str.length == 0 && "Length is set to zero");
        klee_assert(str.data[0] == '\0' && "String is null-terminated");

        /* Also check with specific values */
        klee_assert(str.capacity == 9 && "Capacity calculation is correct");
    }

    /* Test the null pointer error case - be careful with null pointers in Klee */
    #ifndef SKIP_NULL_TESTS
    SStr* null_str = NULL;
    SStrResult null_result = sstr_init(null_str, buffer, buffer_size);
    klee_assert(null_result == SSTR_ERROR_NULL && "Null SStr pointer returns NULL error");

    /* Test the null buffer error case */
    char* null_buffer = NULL;
    SStrResult null_buffer_result = sstr_init(&str, null_buffer, buffer_size);
    klee_assert(null_buffer_result == SSTR_ERROR_NULL && "Null buffer pointer returns NULL error");
    #endif

    /* Test the zero buffer size error case */
    SStrResult zero_size_result = sstr_init(&str, buffer, 0);
    klee_assert(zero_size_result == SSTR_ERROR_OVERFLOW && "Zero buffer size returns OVERFLOW error");

    return 0;
}
