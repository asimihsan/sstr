# SStr - Safe String Library for Embedded Systems

A minimal, bounds-checked string handling library designed for embedded systems with no dynamic memory allocation.

## Features

- Works exclusively with stack-allocated buffers (no heap allocation)
- Tracks both capacity and length in a single structure
- Performs strict bounds checking on all operations
- Provides safe alternatives to dangerous C string functions
- Implements secure, validated format string operations
- Has minimal standard library dependencies
- Features clear, consistent error handling

## Quick Start

```c
#include "sstr.h"
#include <stdio.h>

int main(void) {
    /* Stack-allocated buffer */
    char buffer[64];
    
    /* Initialize an SStr structure with the buffer */
    SStr str;
    SStrResult result = sstr_init(&str, buffer, sizeof(buffer));
    
    if (result != SSTR_SUCCESS) {
        printf("Failed to initialize string\n");
        return 1;
    }
    
    /* Copy a string */
    result = sstr_copy(&str, "Hello, ");
    
    /* Append another string */
    result = sstr_append(&str, "world!");
    
    /* Print the result */
    printf("Result: %s (length: %zu, capacity: %zu)\n", 
           str.data, str.length, str.capacity);
           
    /* Format a string with validation (only certain format specifiers allowed) */
    int chars_written = sstr_format(&str, "The answer is %d", 42);
    if (chars_written < 0) {
        printf("Format error: %d\n", chars_written);
    }
    
    printf("Formatted: %s\n", str.data);
    
    return 0;
}
```

## Installation

### Using the Single-Include Version

SStr uses the STB-style single-header approach common in popular C libraries:

```c
// In exactly ONE .c file in your project:
#define SSTR_IMPLEMENTATION
#include "sstr.h"

// In all other files:
#include "sstr.h"
```

1. Copy `single_include/sstr.h` to your project
2. Define `SSTR_IMPLEMENTATION` in exactly one C file before including it
3. Use regular includes in all other files

This approach allows:
- Header-only usage when needed (great for embedded)
- Traditional compilation model when preferred (no duplicate code)
- Maximum flexibility for different build systems
- No build-time processing required

#### Maintenance

The single-include file is automatically generated from the source files:

```bash
# Generate or update the single-include file
make single_include

# Verify the single-include file is up-to-date
make verify-single-include
```

CI automatically verifies that the single-include file is in sync with the source code.

### Building from Source

```bash
# Using CMake
mkdir build && cd build
cmake ..
make
make install  # Optional, may require sudo

# Or using the Makefile
make
make install  # Optional, may require sudo
```

## Security Features

SStr includes several security features designed for embedded systems:

### Bounds Checking

All operations check buffer sizes to prevent buffer overflows. When a buffer would overflow:
- If `SSTR_DEFAULT_POLICY` is `SSTR_ERROR`: Returns an error code
- If `SSTR_DEFAULT_POLICY` is `SSTR_TRUNCATE`: Safely truncates the string

### Format Validation

Format string vulnerabilities are a common security issue in C programs. SStr addresses this with format validation:

- By default, only safe format specifiers are allowed: `d i u x X s c %`
- Floating point (`f e g`) and pointer (`p`) specifiers are disabled by default
- Format validation can be customized or disabled at compile time
- Provides compile-time configuration options for allowable format specifiers

This protects against both accidental bugs and potential exploits where untrusted input could be used as format strings.

## API Reference

### Core Data Types

```c
/* Result codes for string operations */
typedef enum {
    SSTR_SUCCESS = 0,            /* Operation completed successfully */
    SSTR_ERROR_NULL = -1,        /* NULL pointer parameter */
    SSTR_ERROR_OVERFLOW = -2,    /* Destination buffer too small */
    SSTR_ERROR_FORMAT = -3,      /* Invalid format string */
    SSTR_ERROR_ARGUMENT = -4     /* Invalid argument (e.g., NULL string for %s) */
} SStrResult;

/* Truncation policy for string operations */
typedef enum {
    SSTR_TRUNCATE,   /* Truncate output when buffer is too small */
    SSTR_ERROR       /* Return error when buffer is too small */
} SStrTruncationPolicy;

/* String structure with bounds checking */
typedef struct {
    char   *data;     /* Points to stack-allocated buffer */
    size_t  capacity; /* Maximum usable characters (excluding null terminator) */
    size_t  length;   /* Current string length */
} SStr;
```

### Core Functions

#### Initialization and Basic Operations

- `SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size)`  
  Initialize a string with a stack buffer

- `SStrResult sstr_clear(SStr *s)`  
  Clear a string (set length to zero)

#### String Copy Operations

- `SStrResult sstr_copy(SStr *dest, const char *src)`  
  Copy a C string to an SStr

- `SStrResult sstr_copy_sstr(SStr *dest, const SStr *src)`  
  Copy between SStr structures

#### String Append Operations

- `SStrResult sstr_append(SStr *dest, const char *src)`  
  Append a C string to an SStr

- `SStrResult sstr_append_sstr(SStr *dest, const SStr *src)`  
  Append one SStr to another

#### Formatting Functions

- `int sstr_format(SStr *dest, const char *fmt, ...)`  
  Format a string (printf-style), returns number of characters written or negative error code

- `int sstr_vformat(SStr *dest, const char *fmt, va_list args)`  
  Format with va_list, returns number of characters written or negative error code

## Configuration Options

You can configure the library by defining these macros before including the header:

### Truncation Policy

```c
/* Set default truncation policy (SSTR_TRUNCATE or SSTR_ERROR) */
#define SSTR_DEFAULT_POLICY SSTR_ERROR
```

### Size Limits

```c
/* Maximum allowed string size (to prevent integer overflow) */
#define SSTR_MAX_SIZE ((size_t)0x7FFFFFFF)
```

### Format String Validation

```c
/* Enable or disable format string validation (1 or 0) */
#define SSTR_VALIDATE_FORMAT 1

/* Define which format specifiers are allowed when validation is enabled */
#define SSTR_ALLOWED_SPECIFIERS "diuxXsc%"
```

### Build-time Configuration

For Makefile builds, you can use:

```bash
# Disable format validation
make NO_FORMAT_VALIDATION=1

# Customize allowed format specifiers
make ALLOWED_SPECIFIERS="dis%"
```

For CMake builds:

```bash
# Disable format validation
cmake -DSSTR_VALIDATE_FORMAT=OFF ..

# Customize allowed format specifiers
cmake -DSSTR_ALLOWED_SPECIFIERS="dis%" ..
```

## Development

### Code Formatting

This project uses clang-format for consistent code style. To format your code:

```bash
# Format all code
make format

# Or use the script
./format_code.sh

# Check if code is properly formatted (without modifying files)
make format-check
```

### Testing

```bash
# Build and run all tests
make check

# Test different format validation configurations
make validation-tests
```

### Memory Testing with Valgrind

```bash
# On Mac or Linux with Docker
make valgrind-docker

# Or using the script
./run_valgrind.sh

# For systems with Valgrind installed natively
make valgrind
```

### Continuous Integration

The project includes a comprehensive CI setup:

```bash
# Run all CI checks locally (build, test, format, copyright, valgrind)
make ci
```

## License

This project is licensed under the Mozilla Public License 2.0. See the [LICENSE](LICENSE) file for details.