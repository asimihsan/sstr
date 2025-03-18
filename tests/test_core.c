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
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAIL: %s (%s, line %d)\n", message, __FILE__, __LINE__);                       \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)

static int test_init(void)
{
    char buffer[10];
    SStr str;

    /* Test normal initialization */
    SStrResult result = sstr_init(&str, buffer, sizeof(buffer));
    TEST_ASSERT(result == SSTR_SUCCESS, "Failed to initialize string");
    TEST_ASSERT(str.data == buffer, "Data pointer not set correctly");
    TEST_ASSERT(str.capacity == sizeof(buffer) - 1, "Capacity not set correctly");
    TEST_ASSERT(str.length == 0, "Length not initialized to 0");
    TEST_ASSERT(str.data[0] == '\0', "String not null-terminated");

    /* Test NULL pointer handling */
    result = sstr_init(NULL, buffer, sizeof(buffer));
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL string struct");

    result = sstr_init(&str, NULL, sizeof(buffer));
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL buffer");

    /* Test zero buffer size */
    result = sstr_init(&str, buffer, 0);
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should detect zero buffer size");

    return 1;
}

static int test_clear(void)
{
    char buffer[10];
    SStr str;

    sstr_init(&str, buffer, sizeof(buffer));
    strcpy(buffer, "test");
    str.length = 4;

    SStrResult result = sstr_clear(&str);
    TEST_ASSERT(result == SSTR_SUCCESS, "Clear failed");
    TEST_ASSERT(str.length == 0, "Length not reset to 0");
    TEST_ASSERT(str.data[0] == '\0', "String not null-terminated after clear");

    /* Test NULL handling */
    result = sstr_clear(NULL);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL string struct");

    return 1;
}

static int test_copy(void)
{
    char buffer[10];
    SStr str;

    sstr_init(&str, buffer, sizeof(buffer));

    /* Test normal copy */
    SStrResult result = sstr_copy(&str, "test");
    TEST_ASSERT(result == SSTR_SUCCESS, "Copy failed");
    TEST_ASSERT(str.length == 4, "Length not set correctly");
    TEST_ASSERT(strcmp(str.data, "test") == 0, "String content incorrect");

    /* Test NULL handling */
    result = sstr_copy(NULL, "test");
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL string struct");

    result = sstr_copy(&str, NULL);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL source");

    /* Test overflow (default policy is SSTR_ERROR) */
    result = sstr_copy(&str, "this string is too long");
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should detect overflow");

    return 1;
}

static int test_copy_n(void)
{
    char buffer[10];
    SStr str;

    sstr_init(&str, buffer, sizeof(buffer));

    /* Test normal copy with specified length */
    SStrResult result = sstr_copy_n(&str, "test", 4);
    TEST_ASSERT(result == SSTR_SUCCESS, "Copy failed");
    TEST_ASSERT(str.length == 4, "Length not set correctly");
    TEST_ASSERT(strcmp(str.data, "test") == 0, "String content incorrect");

    /* Test copy with partial string */
    result = sstr_copy_n(&str, "hello", 3);
    TEST_ASSERT(result == SSTR_SUCCESS, "Partial copy failed");
    TEST_ASSERT(str.length == 3, "Length not set correctly");
    TEST_ASSERT(strcmp(str.data, "hel") == 0, "String content incorrect");

    /* Test NULL handling */
    result = sstr_copy_n(NULL, "test", 4);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL dest");

    result = sstr_copy_n(&str, NULL, 4);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL source");

    /* Test overflow (default policy is SSTR_ERROR) */
    result = sstr_copy_n(&str, "1234567890", 10);
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should detect overflow");

    /* Test a controlled overflow case */
    char small_dest_buffer[5];
    SStr small_dest;
    sstr_init(&small_dest, small_dest_buffer, sizeof(small_dest_buffer));

    /* Try to copy more data than fits */
    SStrResult overflow_result = sstr_copy_n(&small_dest, "1234567890", 10);

    /* With the ERROR policy explicitly set, this should return an error */
    TEST_ASSERT(overflow_result == SSTR_ERROR_OVERFLOW, "Should detect overflow");

    return 1;
}

static int test_append(void)
{
    char buffer[10];
    SStr str;

    sstr_init(&str, buffer, sizeof(buffer));

    /* Initial string */
    sstr_copy(&str, "ab");
    TEST_ASSERT(str.length == 2, "Initial length incorrect");

    /* Test normal append */
    SStrResult result = sstr_append(&str, "cd");
    TEST_ASSERT(result == SSTR_SUCCESS, "Append failed");
    TEST_ASSERT(str.length == 4, "Length not updated correctly");
    TEST_ASSERT(strcmp(str.data, "abcd") == 0, "String content incorrect");

    /* Test NULL handling */
    result = sstr_append(NULL, "ef");
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL string struct");

    result = sstr_append(&str, NULL);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL source");

    /* Test overflow (default policy is SSTR_ERROR) */
    result = sstr_append(&str, "efghijklm");
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should detect overflow");

    return 1;
}

static int test_append_sstr(void)
{
    char buffer1[10];
    char buffer2[10];
    SStr str1, str2;

    sstr_init(&str1, buffer1, sizeof(buffer1));
    sstr_init(&str2, buffer2, sizeof(buffer2));

    /* Set up strings */
    sstr_copy(&str1, "ab");
    sstr_copy(&str2, "cd");

    /* Test normal append */
    SStrResult result = sstr_append_sstr(&str1, &str2);
    TEST_ASSERT(result == SSTR_SUCCESS, "Append failed");
    TEST_ASSERT(str1.length == 4, "Length not updated correctly");
    TEST_ASSERT(strcmp(str1.data, "abcd") == 0, "String content incorrect");

    /* Test NULL handling */
    result = sstr_append_sstr(NULL, &str2);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL dest");

    result = sstr_append_sstr(&str1, NULL);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL source");

    /* Test overflow (default policy is SSTR_ERROR) */
    sstr_copy(&str2, "efghijklm");
    result = sstr_append_sstr(&str1, &str2);
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should detect overflow");

    return 1;
}

int run_core_tests(void)
{
    int passed = 0;
    int total = 0;

    printf("Running core tests...\n");

    total++;
    if (test_init()) {
        passed++;
        printf("PASS: init tests\n");
    }

    total++;
    if (test_clear()) {
        passed++;
        printf("PASS: clear tests\n");
    }

    total++;
    if (test_copy()) {
        passed++;
        printf("PASS: copy tests\n");
    }

    total++;
    if (test_copy_n()) {
        passed++;
        printf("PASS: copy_n tests\n");
    }

    total++;
    if (test_append()) {
        passed++;
        printf("PASS: append tests\n");
    }

    total++;
    if (test_append_sstr()) {
        passed++;
        printf("PASS: append_sstr tests\n");
    }

    printf("Core tests: %d/%d passed\n", passed, total);
    return passed == total;
}
