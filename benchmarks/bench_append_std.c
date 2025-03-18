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
    char *base_string = "Hello, world!";
    char *append_string = " This is a benchmark for strcat performance testing.";
    
    if (argc > 1) {
        base_string = argv[1];
    }
    if (argc > 2) {
        append_string = argv[2];
    }

    char buffer[BUFFER_SIZE];

    // Warm up
    for (int i = 0; i < 1000; i++) {
        buffer[0] = '\0';
        strcpy(buffer, base_string);
        strcat(buffer, append_string);
    }

    // Perform benchmark
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        buffer[0] = '\0';
        strcpy(buffer, base_string);
        strcat(buffer, append_string);
    }

    // Prevent optimization by using the result
    printf("%s\n", buffer);

    return 0;
}