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

#define NUM_ITERATIONS 100000
#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    char *format_string = "The answer is %d, and %s is the question.";
    int int_value = 42;
    char *str_value = "life";
    
    if (argc > 1) {
        format_string = argv[1];
    }
    if (argc > 2) {
        int_value = atoi(argv[2]);
    }
    if (argc > 3) {
        str_value = argv[3];
    }

    char buffer[BUFFER_SIZE];

    // Warm up
    for (int i = 0; i < 1000; i++) {
        buffer[0] = '\0';
        snprintf(buffer, BUFFER_SIZE, format_string, int_value, str_value);
    }

    // Perform benchmark
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        buffer[0] = '\0';
        snprintf(buffer, BUFFER_SIZE, format_string, int_value, str_value);
    }

    // Prevent optimization by using the result
    printf("%s\n", buffer);

    return 0;
}