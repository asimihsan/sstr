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

static int test_format_basic(void)
{
    char buffer[64];
    SStr str;
    sstr_init(&str, buffer, sizeof(buffer));

    /* Test integer formatting */
    int result = sstr_format(&str, "%d", 42);
    TEST_ASSERT(result == 2, "Format result should be 2");
    TEST_ASSERT(str.length == 2, "Length should be 2");
    TEST_ASSERT(strcmp(str.data, "42") == 0, "Content should be '42'");

    /* Test string formatting */
    result = sstr_format(&str, "%s", "test");
    TEST_ASSERT(result == 4, "Format result should be 4");
    TEST_ASSERT(str.length == 4, "Length should be 4");
    TEST_ASSERT(strcmp(str.data, "test") == 0, "Content should be 'test'");

    /* Test mixed formatting */
    result = sstr_format(&str, "Value: %d, %s", 123, "abc");
    TEST_ASSERT(result > 0, "Format result should be positive");
    TEST_ASSERT(str.length > 0, "Length should be positive");
    TEST_ASSERT(strcmp(str.data, "Value: 123, abc") == 0, "Content incorrect");

    return 1;
}

static int test_format_overflow(void)
{
    char small_buffer[10];
    SStr small_str;
    sstr_init(&small_str, small_buffer, sizeof(small_buffer));

/* Test overflow with default policy (SSTR_ERROR) */
#undef SSTR_DEFAULT_POLICY
#define SSTR_DEFAULT_POLICY SSTR_ERROR

    int result = sstr_format(&small_str, "This string is too long to fit");
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should return overflow error");

    /* Create a new version with TRUNCATE policy */
    /* Note: We can't actually change the policy at runtime since our implementation
     * uses #if not if(), but we can simulate a truncate policy by
     * using a string that's only slightly too long */

    /* Test either error or truncation based on compile-time policy */
    sstr_clear(&small_str);

    /* Test a short string first to ensure the buffer is usable */
    result = sstr_format(&small_str, "123");
    TEST_ASSERT(result == 3, "Format should succeed with short string");

    /* Now test the overflow behavior */
    result =
        sstr_format(&small_str, "1234567890123"); /* Buffer size is 10, so this will overflow */

#if SSTR_DEFAULT_POLICY == SSTR_ERROR
    TEST_ASSERT(result == SSTR_ERROR_OVERFLOW, "Should return overflow error with current policy");
#else
    TEST_ASSERT(result >= 0, "Format should succeed with truncation");
    TEST_ASSERT(small_str.length == small_str.capacity, "Length should be capacity");
    TEST_ASSERT(small_str.data[small_str.length] == '\0', "String should be null-terminated");
#endif

/* Reset policy for other tests */
#undef SSTR_DEFAULT_POLICY
#define SSTR_DEFAULT_POLICY SSTR_ERROR

    return 1;
}

static int test_format_edge_cases(void)
{
    char buffer[64];
    SStr str;
    sstr_init(&str, buffer, sizeof(buffer));

    /* Test empty format string */
    int result = sstr_format(&str, "");
    TEST_ASSERT(result == 0, "Empty format should result in zero length");
    TEST_ASSERT(str.length == 0, "Length should be 0");
    TEST_ASSERT(str.data[0] == '\0', "String should be empty");

    /* Test NULL handling */
    result = sstr_format(NULL, "test");
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL string struct");

    result = sstr_format(&str, NULL);
    TEST_ASSERT(result == SSTR_ERROR_NULL, "Should detect NULL format string");

    return 1;
}

static int test_format_complex(void)
{
    char buffer[128];
    SStr str;
    sstr_init(&str, buffer, sizeof(buffer));

    /* Test various format specifiers */
    int result =
        sstr_format(&str, "Int: %d, Hex: 0x%x, Char: %c, String: '%s'", 42, 0xABCD, 'Z', "test");

    TEST_ASSERT(result > 0, "Format should succeed");
    TEST_ASSERT(strstr(str.data, "Int: 42") != NULL, "Int format failed");
    TEST_ASSERT(strstr(str.data, "Hex: 0xabcd") != NULL, "Hex format failed");
    TEST_ASSERT(strstr(str.data, "Char: Z") != NULL, "Char format failed");
    TEST_ASSERT(strstr(str.data, "String: 'test'") != NULL, "String format failed");

    /* Test width and precision */
    result = sstr_format(&str, "Width: [%10s], Prec: [%.3s]", "abc", "abcdef");
    TEST_ASSERT(result > 0, "Format should succeed");
    TEST_ASSERT(strstr(str.data, "Width: [       abc]") != NULL, "Width format failed");
    TEST_ASSERT(strstr(str.data, "Prec: [abc]") != NULL, "Precision format failed");

    return 1;
}

static int test_format_validation(void)
{
    char buffer[128];
    SStr str;
    sstr_init(&str, buffer, sizeof(buffer));
    int result;

#if SSTR_VALIDATE_FORMAT
    /* Test disallowed format specifiers */
    result = sstr_format(&str, "Float: %f", 3.14);
    TEST_ASSERT(result == SSTR_ERROR_FORMAT, "Float format should be rejected");
    
    result = sstr_format(&str, "Pointer: %p", &str);
    TEST_ASSERT(result == SSTR_ERROR_FORMAT, "Pointer format should be rejected");
    
    result = sstr_format(&str, "Scientific: %e", 1.23e-4);
    TEST_ASSERT(result == SSTR_ERROR_FORMAT, "Scientific format should be rejected");
    
    result = sstr_format(&str, "Generic float: %g", 0.0001);
    TEST_ASSERT(result == SSTR_ERROR_FORMAT, "Generic float format should be rejected");
    
    /* Test malformed format strings */
    result = sstr_format(&str, "Incomplete: %");
    TEST_ASSERT(result == SSTR_ERROR_FORMAT, "Incomplete format should be rejected");
    
    result = sstr_format(&str, "Unknown: %v");
    TEST_ASSERT(result == SSTR_ERROR_FORMAT, "Unknown format should be rejected");
    
    /* Test complex flags with allowed specifiers */
    result = sstr_format(&str, "Complex but allowed: %+05d", 42);
    TEST_ASSERT(result > 0, "Format with valid flags should succeed");
    
    /* Test width and precision with allowed specifiers */
    result = sstr_format(&str, "Width and precision: %10.5d", 12345);
    TEST_ASSERT(result > 0, "Format with valid width/precision should succeed");
    
    /* Test different allowed specifiers */
    result = sstr_format(&str, "Allowed: %d %i %u %x %X %s %c %%", 1, 2, 3, 15, 16, "test", 'a');
    TEST_ASSERT(result > 0, "All allowed specifiers should succeed");
    
    /* Test escape sequence */
    result = sstr_format(&str, "Percent: 100%%");
    TEST_ASSERT(result > 0, "Escaped percent should succeed");
    TEST_ASSERT(strcmp(str.data, "Percent: 100%") == 0, "Escaped percent should render correctly");
    
#else
    /* If validation is disabled, just check that it allows a non-standard format */
    result = sstr_format(&str, "No validation: %f", 3.14);
    /* Cannot test the exact result since it depends on whether float is supported */
    
    /* Just check that the function returned a valid result */
    TEST_ASSERT(result >= 0 || result == SSTR_ERROR_FORMAT, 
                "Without validation should either succeed or fail due to float not supported");
#endif

    return 1;
}

int run_format_tests(void)
{
    int passed = 0;
    int total = 0;

    printf("Running format tests...\n");

    total++;
    if (test_format_basic()) {
        passed++;
        printf("PASS: format basic tests\n");
    }

    total++;
    if (test_format_overflow()) {
        passed++;
        printf("PASS: format overflow tests\n");
    }

    total++;
    if (test_format_edge_cases()) {
        passed++;
        printf("PASS: format edge case tests\n");
    }

    total++;
    if (test_format_complex()) {
        passed++;
        printf("PASS: format complex tests\n");
    }
    
    total++;
    if (test_format_validation()) {
        passed++;
        printf("PASS: format validation tests\n");
    }

    printf("Format tests: %d/%d passed\n", passed, total);
    return passed == total;
}
