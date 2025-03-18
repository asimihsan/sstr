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
 * This example demonstrates SStr's format string validation feature.
 * It shows how the library protects against unsafe format specifiers
 * and how to configure the validation behavior.
 */

/* Define SSTR_IMPLEMENTATION in exactly one source file */
#define SSTR_IMPLEMENTATION
#include "../single_include/sstr.h"
#include <stdio.h>

int main(void)
{
    char buffer[128];
    SStr str;
    int result;

    /* Initialize string */
    sstr_init(&str, buffer, sizeof(buffer));

    printf("===== Format String Validation Example =====\n\n");

    /* Safe format specifiers - should succeed */
    printf("Testing safe format specifiers:\n");
    
    result = sstr_format(&str, "Integer: %d\n", 42);
    printf("  %%d result: %d, output: %s", result, result >= 0 ? str.data : "ERROR\n");
    
    result = sstr_format(&str, "String: %s\n", "Hello");
    printf("  %%s result: %d, output: %s", result, result >= 0 ? str.data : "ERROR\n");
    
    result = sstr_format(&str, "Character: %c\n", 'X');
    printf("  %%c result: %d, output: %s", result, result >= 0 ? str.data : "ERROR\n");
    
    result = sstr_format(&str, "Hex: %x\n", 0xABCD);
    printf("  %%x result: %d, output: %s", result, result >= 0 ? str.data : "ERROR\n");
    
    printf("\nTesting unsafe format specifiers:\n");
    
    /* Unsafe format specifier - should fail with SSTR_ERROR_FORMAT */
    result = sstr_format(&str, "Float: %f\n", 3.14159);
    printf("  %%f result: %d (%s)\n", 
           result,
           result == SSTR_ERROR_FORMAT ? "CORRECTLY REJECTED" : "INCORRECTLY ACCEPTED");
    
    /* Unsafe format specifier - should fail with SSTR_ERROR_FORMAT */
    result = sstr_format(&str, "Pointer: %p\n", &str);
    printf("  %%p result: %d (%s)\n",
           result,
           result == SSTR_ERROR_FORMAT ? "CORRECTLY REJECTED" : "INCORRECTLY ACCEPTED");
    
    printf("\n===== End of Example =====\n");
    
    return 0;
}
