# SStr Library Development Guide

## Build Commands

- Build library: `make`
- Build examples: `make examples`
- Build tests: `make tests`
- Run all tests: `make check`
- Clean: `make clean`

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

## Next Steps

- Run and verify example programs functionality
- Enhance documentation with API details
- Consider CI/CD integration for automated testing
- Test performance in embedded contexts
- Add optional format string validation
- Add additional string manipulation functions:
  - String comparison (sstr_compare)
  - Substring extraction (sstr_substring)
  - String search (sstr_find)
