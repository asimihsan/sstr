/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/sstr/sstr.h"

#define NUM_ITERATIONS 100000
#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    char *input_string = "Hello, world! This is a benchmark for sstr_copy performance testing.";
    if (argc > 1) {
        input_string = argv[1];
    }

    char buffer[BUFFER_SIZE];
    SStr dest;
    sstr_init(&dest, buffer, BUFFER_SIZE);

    // Warm up
    for (int i = 0; i < 1000; i++) {
        sstr_clear(&dest);
        sstr_copy(&dest, input_string);
    }

    // Perform benchmark
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sstr_clear(&dest);
        sstr_copy(&dest, input_string);
    }

    // Prevent optimization by using the result
    printf("%s\n", dest.data);

    return 0;
}