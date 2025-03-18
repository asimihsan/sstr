/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/**
 * This test file verifies that the single-include STB-style header works correctly.
 * It contains basic tests for the core functionality when using the SSTR_IMPLEMENTATION
 * approach.
 */

/* Define implementation in this one file */
#define SSTR_IMPLEMENTATION
#include "../single_include/sstr.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 64

void test_initialization(void)
{
    char buffer[BUFFER_SIZE];
    SStr str;

    /* Test normal initialization */
    SStrResult result = sstr_init(&str, buffer, BUFFER_SIZE);
    assert(result == SSTR_SUCCESS);
    assert(str.data == buffer);
    assert(str.capacity == BUFFER_SIZE - 1);
    assert(str.length == 0);
    assert(str.data[0] == '\0');

    /* Test NULL pointer errors */
    result = sstr_init(NULL, buffer, BUFFER_SIZE);
    assert(result == SSTR_ERROR_NULL);

    result = sstr_init(&str, NULL, BUFFER_SIZE);
    assert(result == SSTR_ERROR_NULL);

    /* Test zero-size buffer */
    result = sstr_init(&str, buffer, 0);
    assert(result == SSTR_ERROR_OVERFLOW);

    printf("Initialization tests passed\n");
}

void test_copy_and_append(void)
{
    char buffer[BUFFER_SIZE];
    SStr str;
    SStrResult result;

    /* Initialize string */
    result = sstr_init(&str, buffer, BUFFER_SIZE);
    assert(result == SSTR_SUCCESS);

    /* Test copying */
    result = sstr_copy(&str, "Hello");
    assert(result == SSTR_SUCCESS);
    assert(str.length == 5);
    assert(strcmp(str.data, "Hello") == 0);

    /* Test appending */
    result = sstr_append(&str, ", world!");
    assert(result == SSTR_SUCCESS);
    assert(str.length == 13);
    assert(strcmp(str.data, "Hello, world!") == 0);

    /* Test clearing */
    result = sstr_clear(&str);
    assert(result == SSTR_SUCCESS);
    assert(str.length == 0);
    assert(str.data[0] == '\0');

    printf("Copy and append tests passed\n");
}

void test_formatting(void)
{
    char buffer[BUFFER_SIZE];
    SStr str;
    int result;

    /* Initialize string */
    sstr_init(&str, buffer, BUFFER_SIZE);

    /* Test basic formatting */
    result = sstr_format(&str, "Number: %d", 42);
    assert(result > 0);
    assert(strcmp(str.data, "Number: 42") == 0);

    /* Test multiple arguments */
    result = sstr_format(&str, "%s %d %c", "Test", 123, 'X');
    assert(result > 0);
    assert(strcmp(str.data, "Test 123 X") == 0);

    /* Test format validation (should reject %f) */
    result = sstr_format(&str, "Float: %f", 3.14159);
#if SSTR_VALIDATE_FORMAT
    assert(result == SSTR_ERROR_FORMAT);
#else
    assert(result > 0); /* Validation disabled, should pass */
#endif

    printf("Formatting tests passed\n");
}

int main(void)
{
    printf("Testing single-include STB-style implementation\n");
    
    test_initialization();
    test_copy_and_append();
    test_formatting();
    
    printf("All single-include tests passed!\n");
    return 0;
}
