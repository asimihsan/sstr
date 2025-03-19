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

## Pre-commit Hooks

The repository uses pre-commit to ensure code quality before committing. Set up pre-commit with:

```bash
# Install pre-commit
make pre-commit-install

# Run on all files (optional)
make pre-commit-run
```

Pre-commit hooks will automatically:
- Format code using clang-format
- Check for proper code formatting
- Prevent binary files from being committed
- Ensure single-include file is up-to-date
- Fix end-of-file and trailing whitespace issues

## Single-Include Workflow

- Use `make single_include` to regenerate the single header file
- Use `make verify-single-include` to check if it's up-to-date
- CI automatically verifies the single-include file is in sync with the source code

## CBMC Formal Verification

### Overview

CBMC (C Bounded Model Checker) provides formal verification of safety properties through exhaustive symbolic execution. Unlike testing, which examines specific inputs, CBMC mathematically proves properties hold for all possible inputs within defined bounds.

### Running Verification

- `make verify` - Run all CBMC verifications
- Verify specific functions:
  - `make verify-init` - Verify sstr_init function
  - `make verify-copy` - Verify sstr_copy function
  - `make verify-append` - Verify sstr_append function
- List available properties: `make cbmc-properties`

### Core Verification Components

#### 1. Verification Harnesses

Harnesses in `verification/` directory serve as controlled test environments:
- Each harness focuses on one function with specific inputs/preconditions
- They verify postconditions under all possible execution paths
- Example: `verification/sstr_copy_harness.c` tests `sstr_copy`

Structure of a typical harness:
```c
void sstr_function_harness() {
    // 1. Setup input structures with fixed bounds
    char buffer[SIZE];
    SStr s;

    // 2. Apply constraints to make verification tractable
    __CPROVER_assume(condition);

    // 3. Call the function under test
    SStrResult result = sstr_function(...);

    // 4. Verify postconditions with assertions
    __CPROVER_assert(property, "Message explaining the property");
}
```

#### 2. CBMC Special Directives

The library uses CBMC-specific primitives:

- `__CPROVER_assume(condition)`:
  - Constrains inputs to meet preconditions
  - Reduces state space exploration to feasible inputs
  - Example: `__CPROVER_assume(buffer_size <= 10)` limits input size

- `__CPROVER_assert(condition, message)`:
  - Specifies properties that must hold
  - CBMC will try to find a counterexample (any input that violates the property)
  - Example: `__CPROVER_assert(str.data[str.length] == '\0', "String is null-terminated")`

- `__CPROVER_r_ok(pointer, size)` / `__CPROVER_w_ok(pointer, size)`:
  - Checks if memory is safe to read/write
  - Verifies absence of buffer overflows
  - Example: `__CPROVER_r_ok(src, length + 1)` verifies read safety for string + null terminator

- `__CPROVER_OBJECT_SIZE(pointer)`:
  - Gets allocated size of the object
  - Used to verify bounds checks
  - Example: `size_t object_size = __CPROVER_OBJECT_SIZE(buffer)`

#### 3. CBMC Optimization Techniques

These techniques make verification tractable:

- Conditional Compilation:
  ```c
  #ifdef __CPROVER
      // CBMC-specific optimizations
  #else
      // Regular implementation
  #endif
  ```

- Loop Unwinding with Explicit Bounds:
  ```c
  // Replace while loops with bounded for loops
  for (size_t i = 0; i < bound; i++) {
      // Loop body
  }
  ```

- Explicit Loops instead of Library Functions:
  ```c
  // Instead of memcpy(dest, src, len)
  for (size_t i = 0; i < len; i++) {
      dest[i] = src[i];
  }
  ```

- Stubs for CBMC Functions (`include/sstr/cbmc_stubs.h`):
  ```c
  // Provide stubs for regular compilation
  #ifndef __CPROVER_assume
  #define __CPROVER_assume(cond) ((void)0)
  #endif
  ```

### Command-Line Options

Common verification options:
- `--bounds-check` - Check array bounds and pointer operations
- `--pointer-check` - Verify pointer safety
- `--unwinding-assertions` - Ensure loops are unwound enough
- `--unwind N` - Set loop unwinding limit (e.g., 10)
- `--function func` - Specify function to verify
- `--stop-on-fail` - Stop at first property violation
- `--slice-formula` - Reduce verification complexity

### Handling Verification Complexity

For complex functions (especially string operations), CBMC may face state explosion. The library addresses this:

1. **Bounded String Operations**:
   - Implement custom `sstr_bounded_strlen` with explicit bounds
   - Replace unbounded library functions like `strlen`/`memcpy`

2. **Input Constraints**:
   - Use `__CPROVER_assume` to limit input size
   - Example: `__CPROVER_assume(src_len <= 10)` makes verification feasible

3. **Incremental Verification**:
   - Start with small bounds, then gradually increase
   - Verify basic properties before complex ones

4. **Performance Optimizations**:
   - Use `--slice-formula` to reduce verification complexity
   - Apply `--stop-on-fail` to focus on first issue

5. **Memory Safety Checks**:
   - Use `__CPROVER_r_ok`/`__CPROVER_w_ok` to ensure memory safety
   - Assert bounds explicitly: `__CPROVER_assert(idx < buffer_size, "Index in bounds")`

If verification hangs, try:
- Reducing bounds further with tighter `__CPROVER_assume` constraints
- Breaking verification into smaller properties
- Using more aggressive slicing with `--slice-formula`
- Increasing the timeout value

### Interpreting Results

Successful verification output includes:
```
VERIFICATION SUCCESSFUL
```

Failed verification shows:
```
VERIFICATION FAILED
Counterexample:
...detailed trace of inputs that violate the property...
Violated property:
...specific assertion that failed...
```

Use the counterexample to understand why verification failed and fix the issue.

## Docker-based Verification

To enhance portability and consistency across environments, the library can run all verification tools through Docker:

### Building the Docker Image

```
make build-docker
```

This builds a Docker image containing CBMC, Klee, and Valgrind, ready for comprehensive verification.

### Docker-based CBMC Verification

Run the following commands to verify the library using CBMC through Docker:

```
make verify-init     # Verify sstr_init function
make verify-copy     # Verify sstr_copy function
make verify-append   # Verify sstr_append function
make verify          # Run all CBMC verifications
```

### Docker-based Klee Verification

Klee is a symbolic execution engine that complements CBMC by exploring execution paths:

```
make klee-prepare         # Set up Klee environment
make klee-compile-init    # Compile sstr_init for Klee
make klee-compile-copy    # Compile sstr_copy for Klee
make klee-compile-append  # Compile sstr_append for Klee
make klee-run-init        # Run Klee on sstr_init
make klee-run-copy        # Run Klee on sstr_copy
make klee-run-append      # Run Klee on sstr_append
make klee-verify          # Run all Klee verifications
```

### Docker-based Valgrind Analysis

Valgrind performs runtime memory analysis to detect leaks and memory errors:

```
make valgrind-docker      # Run tests through Valgrind in Docker
```

### Combined Verification

For comprehensive verification:

```
make verify-all        # Run both CBMC and Klee verifications
make verify-full       # Run all verification tools (CBMC, Klee, and Valgrind)
```

### Key Differences Between CBMC and Klee

1. **Approach**:
   - CBMC performs bounded model checking (exhaustive symbolic analysis)
   - Klee performs symbolic execution (explores execution paths)

2. **Strengths**:
   - CBMC is better at proving properties hold for all inputs within bounds
   - Klee excels at finding concrete test cases that trigger issues

3. **Interpreting Results**:
   - CBMC provides a counterexample when property fails
   - Klee generates test cases for each execution path explored

4. **Limitations**:
   - CBMC may suffer from state explosion on complex code
   - Klee may not explore all paths in large programs

Using both tools provides more comprehensive verification coverage.
