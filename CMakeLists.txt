cmake_minimum_required(VERSION 3.10)
project(sstr VERSION 0.1.0 LANGUAGES C)

# Options
option(SSTR_BUILD_TESTS "Build tests" ON)
option(SSTR_BUILD_EXAMPLES "Build examples" ON)
option(SSTR_BUILD_BENCHMARKS "Build benchmarks" ON)
option(SSTR_VALIDATE_FORMAT "Enable format string validation" ON)
set(SSTR_ALLOWED_SPECIFIERS "diuxXsc%" CACHE STRING "Allowed format specifiers")

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler warnings and configure options
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Default configuration
add_compile_definitions(SSTR_DEFAULT_POLICY=SSTR_ERROR)

# Configure format validation
if(SSTR_VALIDATE_FORMAT)
    add_compile_definitions(SSTR_VALIDATE_FORMAT=1)
    add_compile_definitions(SSTR_ALLOWED_SPECIFIERS="${SSTR_ALLOWED_SPECIFIERS}")
else()
    add_compile_definitions(SSTR_VALIDATE_FORMAT=0)
endif()

# Include directories
include_directories(include)

# Library source files
set(SSTR_SOURCES
    src/sstr.c
    src/sstr_format.c
)

# Create library
add_library(sstr ${SSTR_SOURCES})
target_include_directories(sstr PUBLIC include)

# Examples
if(SSTR_BUILD_EXAMPLES)
    add_executable(basic_usage examples/basic_usage.c)
    target_link_libraries(basic_usage sstr)
    
    add_executable(formatting examples/formatting.c)
    target_link_libraries(formatting sstr)
endif()

# Tests
if(SSTR_BUILD_TESTS)
    add_executable(test_runner 
        tests/test_runner.c
        tests/test_core.c
        tests/test_format.c
    )
    target_link_libraries(test_runner sstr)
    
    # Add test
    enable_testing()
    add_test(NAME test_runner COMMAND test_runner)
endif()

# Benchmarks
if(SSTR_BUILD_BENCHMARKS)
    # Copy benchmark
    add_executable(bench_copy_n benchmarks/bench_copy_n.c)
    target_link_libraries(bench_copy_n sstr)
    
    add_executable(bench_copy_std benchmarks/bench_copy_std.c)
    
    # Append benchmark
    add_executable(bench_append_sstr benchmarks/bench_append_sstr.c)
    target_link_libraries(bench_append_sstr sstr)
    
    add_executable(bench_append_std benchmarks/bench_append_std.c)
    
    # Format benchmark
    add_executable(bench_format_sstr benchmarks/bench_format_sstr.c)
    target_link_libraries(bench_format_sstr sstr)
    
    add_executable(bench_format_std benchmarks/bench_format_std.c)
endif()

# Installation
install(TARGETS sstr
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/sstr/
    DESTINATION include/sstr
    FILES_MATCHING PATTERN "*.h"
)

install(FILES single_include/sstr.h
    DESTINATION include
)

# CBMC verification target
find_program(CBMC_EXECUTABLE cbmc QUIET)
if(CBMC_EXECUTABLE)
    add_custom_target(verify
        COMMAND ${CBMC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/src/sstr.c
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running CBMC verification"
    )
    message(STATUS "CBMC found: ${CBMC_EXECUTABLE}")
else()
    message(STATUS "CBMC not found, verification target not available")
endif()