/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include "../include/sstr/sstr.h"
#include <stdio.h>

int main(void) {
    char buffer[128];
    SStr str;
    
    /* Initialize string */
    sstr_init(&str, buffer, sizeof(buffer));
    
    /* Format with different data types */
    sstr_format(&str, "Integer: %d\n", 42);
    printf("%s", str.data);
    
    sstr_format(&str, "String: %s\n", "Hello, world!");
    printf("%s", str.data);
    
    sstr_format(&str, "Character: %c\n", 'A');
    printf("%s", str.data);
    
    sstr_format(&str, "Unsigned: %u\n", 123456U);
    printf("%s", str.data);
    
    sstr_format(&str, "Hex: 0x%x\n", 0xABCD);
    printf("%s", str.data);
    
    /* Format with multiple arguments */
    sstr_format(&str, "Multiple values: %d, %s, %c\n", 42, "test", 'X');
    printf("%s", str.data);
    
    /* Format with precision */
    sstr_format(&str, "Precision: %.2f\n", 3.14159);
    printf("%s", str.data);
    
    /* Format with width */
    sstr_format(&str, "Width: [%10s]\n", "test");
    printf("%s", str.data);
    
    /* Format with width and precision */
    sstr_format(&str, "Width and precision: [%10.2f]\n", 3.14159);
    printf("%s", str.data);
    
    return 0;
}
