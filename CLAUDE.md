# SStr Library Development Guide

## Build Commands

- Build library: `make`
- Build examples: `make examples`
- Build tests: `make tests`
- Run all tests: `make check`
- Clean: `make clean`
- Build benchmarks: `cmake -DSSTR_BUILD_BENCHMARKS=ON && make`
- Run benchmarks: `./run_benchmarks.sh`
- Visualize benchmark results: `python plot_benchmarks.py`

## Code Style Guidelines

- C99 standard with pedantic warnings
- Use consistent 4-space indentation
- Null pointer checks at the beginning of functions
- Return error codes via SStrResult enum
- Default truncation policy: SSTR_ERROR
- No dynamic memory allocation (embedded system focus)
- All strings must be null-terminated
- All input parameters must be validated
- Functions should follow consistent parameter order (dest, src)
- Comprehensive error handling for all operations
- Adhere to strict bounds checking for all string operations
- Document all functions with descriptive comments
- Use consistent naming: lowercase with underscores
- Use consistent parameter names across similar functions

## Benchmark System

- All benchmarks use hyperfine for reliable measurements
- Benchmarks compare SStr functions with standard C equivalents:
  - sstr_copy vs strcpy
  - sstr_append vs strcat
  - sstr_format vs snprintf
- Run benchmarks with different input sizes to see scaling behavior
- Benchmark results help quantify the safety/performance tradeoff
- Python visualization script creates charts for easy comparison

## Next Steps

- Run and verify example programs functionality
- Enhance documentation with API details
- Consider CI/CD integration for automated testing
- Test performance in embedded contexts
- Add additional string manipulation functions:
  - String comparison (sstr_compare)
  - Substring extraction (sstr_substring)
  - String search (sstr_find)

## Single-Include Workflow

- Use `make single_include` to regenerate the single header file
- Use `make verify-single-include` to check if it's up-to-date
- CI automatically verifies the single-include file is in sync with the source code
