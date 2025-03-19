#!/bin/bash
#
# KLEE verification script for sstr library (Docker version)
#

set -e  # Exit on error
set -u  # Error on undefined variables

# Configuration
INCLUDE_DIR="/app/include"
SRC_DIR="/app/src"
KLEE_DIR="/app/klee"
BUILD_DIR="/app/klee-build"
CLANG_FLAGS="-g -O0 -Xclang -disable-O0-optnone"

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Function to compile and run KLEE for a specific harness
run_klee() {
    local harness_name="$1"
    echo "Running KLEE verification for $harness_name..."

    # Step 1: Compile each source file separately into LLVM bitcode
    echo "Compiling sstr.c..."
    clang -I"$INCLUDE_DIR" -emit-llvm -c $CLANG_FLAGS -o "$BUILD_DIR/sstr.bc" "$SRC_DIR/sstr.c"

    echo "Compiling sstr_format.c..."
    clang -I"$INCLUDE_DIR" -emit-llvm -c $CLANG_FLAGS -o "$BUILD_DIR/sstr_format.bc" "$SRC_DIR/sstr_format.c"

    echo "Compiling ${harness_name}_harness.c..."
    clang -I"$INCLUDE_DIR" -emit-llvm -c $CLANG_FLAGS -o "$BUILD_DIR/harness.bc" "$KLEE_DIR/${harness_name}_harness.c"

    # Step 2: Link the LLVM bitcode files together using llvm-link
    echo "Linking bitcode files..."
    llvm-link "$BUILD_DIR/sstr.bc" "$BUILD_DIR/sstr_format.bc" "$BUILD_DIR/harness.bc" -o "$BUILD_DIR/${harness_name}_klee.bc"

    # Step 3: Run KLEE on the linked bitcode file
    echo "Running KLEE..."
    klee --output-dir="$BUILD_DIR/klee-${harness_name}-out" "$BUILD_DIR/${harness_name}_klee.bc"

    echo "KLEE verification for $harness_name completed."
    echo "----------------------------------------"
}

# Main execution
echo "Starting KLEE verification for sstr library..."

# Check which harness to run
if [ $# -eq 0 ]; then
    # Run all harnesses if no specific one provided
    run_klee "sstr_init"
    run_klee "sstr_copy"
    run_klee "sstr_append"
else
    # Run only the specified harness
    run_klee "$1"
fi

echo "All KLEE verification completed successfully."
