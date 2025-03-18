/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include "../single_include/sstr.h"
#include <stdio.h>

int main(void)
{
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

    /* Format with width */
    sstr_format(&str, "Width: [%10s]\n", "test");
    printf("%s", str.data);
    
    /* Format with escaped % */
    sstr_format(&str, "Percent sign: 100%%\n");
    printf("%s", str.data);
    
    /* Note: Floating point formats (%f, %e, %g) and pointer (%p) are 
       blocked by default format validation */

    return 0;
}
