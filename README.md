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

## Basic Usage

```c
#include "sstr.h"
#include <stdio.h>

int main() {
    /* Stack-allocated buffer */
    char buffer[64];
    
    /* Initialize an SStr structure with the buffer */
    SStr str;
    SStrResult result = sstr_init(&str, buffer, sizeof(buffer));
    
    /* Copy a string */
    result = sstr_copy(&str, "Hello, ");
    
    /* Append another string */
    result = sstr_append(&str, "world!");
    
    /* Print the result */
    printf("Result: %s (length: %zu, capacity: %zu)\n", 
           str.data, str.length, str.capacity);
           
    /* Format a string */
    int chars_written = sstr_format(&str, "The answer is %d", 42);
    
    printf("Formatted: %s\n", str.data);
    
    return 0;
}
```

## Installation

### Using the Single-Include Version

For most embedded projects, the simplest approach is to use the single-include version:

1. Copy `single_include/sstr.h` to your project
2. Include it in your code: `#include "sstr.h"`

### Building from Source

```
mkdir build && cd build
cmake ..
make
make install  # Optional, may require sudo
```

Or using the Makefile:

```
make
make install  # Optional, may require sudo
```

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

/* String structure with bounds checking */
typedef struct {
    char   *data;     /* Points to stack-allocated buffer */
    size_t  capacity; /* Maximum usable characters (excluding null terminator) */
    size_t  length;   /* Current string length */
} SStr;
```

### Core Functions

- `SStrResult sstr_init(SStr *s, char *buffer, size_t buffer_size)` - Initialize a string with a stack buffer
- `SStrResult sstr_clear(SStr *s)` - Clear a string (set length to zero)
- `SStrResult sstr_copy(SStr *dest, const char *src)` - Copy a C string to an SStr
- `SStrResult sstr_copy_sstr(SStr *dest, const SStr *src)` - Copy between SStr structures
- `SStrResult sstr_append(SStr *dest, const char *src)` - Append a C string to an SStr
- `SStrResult sstr_append_sstr(SStr *dest, const SStr *src)` - Append one SStr to another
- `int sstr_format(SStr *dest, const char *fmt, ...)` - Format a string (printf-style)
- `int sstr_vformat(SStr *dest, const char *fmt, va_list args)` - Format with va_list

## Configuration

You can configure the library by defining these before including the header:

```c
/* Set truncation policy (SSTR_TRUNCATE or SSTR_ERROR) */
#define SSTR_DEFAULT_POLICY SSTR_ERROR

/* Maximum allowed string size (to prevent integer overflow) */
#define SSTR_MAX_SIZE ((size_t)0x7FFFFFFF)

/* Enable/disable floating point format support */
#define SSTR_ENABLE_FLOAT_FORMAT 1
```

## Memory Testing with Valgrind

This project includes a Docker-based setup for running Valgrind memory tests, which works on both Mac and Linux systems.

### Running Valgrind Tests Locally

To run Valgrind tests locally (requires Docker):

```bash
# On Mac or Linux
./run_valgrind.sh
```

This script builds a Docker image with Valgrind and runs the tests inside the container.

### GitHub Actions Integration

The repository is configured with a GitHub Actions workflow that automatically runs Valgrind tests on all PRs and pushes to main.

### Custom Valgrind Options

To run Valgrind with custom options:

```bash
docker build -t sstr-valgrind .
docker run --rm sstr-valgrind valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/test_runner
```

## License

This project is licensed under the Mozilla Public License 2.0. See the [LICENSE](LICENSE) file for details.
