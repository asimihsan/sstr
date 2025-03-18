#!/bin/bash
# Script to generate the single-include version of the SStr library

# Exit on error
set -e

# Define source and destination files
OUTPUT_FILE="single_include/sstr.h"
CONFIG_FILE="include/sstr/sstr_config.h"
HEADER_FILE="include/sstr/sstr.h"
IMPLEMENTATION_FILES=("src/sstr.c" "src/sstr_format.c")

# Create output directory if it doesn't exist
mkdir -p "$(dirname "$OUTPUT_FILE")"

# Print a message
echo "Generating single-include header file: $OUTPUT_FILE"

# Start with the header information
cat > "$OUTPUT_FILE" << 'EOF'
/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/**
 * SStr - Safe String Library for Embedded Systems
 * 
 * A minimal, bounds-checked string handling library designed for embedded systems
 * with no dynamic memory allocation.
 * 
 * Usage:
 *   1. Include this file in any C file that needs to use SStr
 *   2. In EXACTLY ONE C file, define SSTR_IMPLEMENTATION before including:
 *      #define SSTR_IMPLEMENTATION
 *      #include "sstr.h"
 */

#ifndef SSTR_H
#define SSTR_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

EOF

# Add the configuration content
echo "Adding configuration from $CONFIG_FILE"
# Skip first 20 lines (copyright, comments, include guards)
tail -n +20 "$CONFIG_FILE" | head -n -1 >> "$OUTPUT_FILE"

# Add the header content (function declarations)
echo "Adding API declarations from $HEADER_FILE"
# Skip first 20 lines (copyright, comments, include guards)
# Also skip the includes for sstr_config.h
awk 'NR>20 && !/sstr_config\.h/' "$HEADER_FILE" | head -n -1 >> "$OUTPUT_FILE"

# Add the C++ closing bracket and transition to implementation
cat >> "$OUTPUT_FILE" << 'EOF'

#ifdef __cplusplus
}
#endif

/* ===========================================================================
 * Implementation
 * ===========================================================================
 */

#ifdef SSTR_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <ctype.h>

EOF

# Add each implementation file
for impl_file in "${IMPLEMENTATION_FILES[@]}"; do
  echo "Adding implementation from $impl_file"
  
  # Extract the function implementations and static helpers
  # Skip includes, copyright, and empty lines at the start
  awk '
    BEGIN { in_function = 0; print_line = 0; }
    /^#include/ { next; }  # Skip includes
    /^\/\*/ { if (!print_line) next; }  # Skip copyright comment blocks at start
    /^$/ { if (!print_line) next; }  # Skip empty lines at start
    /^[a-zA-Z_][a-zA-Z0-9_]* .*\(.*\)/ { in_function = 1; print_line = 1; }  # Start of function
    in_function && /^}/ { print; in_function = 0; print ""; next; }  # End of function
    /^static/ { print_line = 1; }  # Static functions/variables
    print_line { print; }
  ' "$impl_file" >> "$OUTPUT_FILE"
done

# Close the implementation block and file
cat >> "$OUTPUT_FILE" << 'EOF'

#endif /* SSTR_IMPLEMENTATION */

#endif /* SSTR_H */
EOF

echo "Single-include header generated successfully!"