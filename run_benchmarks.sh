#!/bin/bash
# Benchmark runner for sstr library
# Requires hyperfine to be installed: https://github.com/sharkdp/hyperfine

# Check if hyperfine is installed
if ! command -v hyperfine &> /dev/null; then
    echo "Error: hyperfine is not installed"
    echo "Please install it from https://github.com/sharkdp/hyperfine"
    exit 1
fi

# Build the benchmark executables
echo "Building benchmarks..."
mkdir -p build
cd build
cmake .. -DSSTR_BUILD_BENCHMARKS=ON
make bench_copy_n bench_copy_std bench_append_sstr bench_append_std bench_format_sstr bench_format_std
cd ..

# Define benchmark scenarios
SMALL_STRING="Hello, world!"
MEDIUM_STRING="This is a medium-sized string that will be used for benchmarking the string functions"
LARGE_STRING=$(printf '%.0s-' {1..200})  # 200 character string

# Run the benchmarks
echo "Running string copy benchmarks..."
hyperfine --warmup 3 \
    "./build/bench_copy_n '$SMALL_STRING'" \
    "./build/bench_copy_std '$SMALL_STRING'" \
    --export-markdown results_copy_small.md \
    --export-json results_copy_small.json \
    --export-csv results_copy_small.csv

hyperfine --warmup 3 \
    "./build/bench_copy_n '$MEDIUM_STRING'" \
    "./build/bench_copy_std '$MEDIUM_STRING'" \
    --export-markdown results_copy_medium.md \
    --export-json results_copy_medium.json \
    --export-csv results_copy_medium.csv

hyperfine --warmup 3 \
    "./build/bench_copy_n '$LARGE_STRING'" \
    "./build/bench_copy_std '$LARGE_STRING'" \
    --export-markdown results_copy_large.md \
    --export-json results_copy_large.json \
    --export-csv results_copy_large.csv

echo "Running string append benchmarks..."
hyperfine --warmup 3 \
    "./build/bench_append_sstr 'Hello' ', world!'" \
    "./build/bench_append_std 'Hello' ', world!'" \
    --export-markdown results_append_small.md \
    --export-json results_append_small.json \
    --export-csv results_append_small.csv

hyperfine --warmup 3 \
    "./build/bench_append_sstr 'Start: ' '$MEDIUM_STRING'" \
    "./build/bench_append_std 'Start: ' '$MEDIUM_STRING'" \
    --export-markdown results_append_medium.md \
    --export-json results_append_medium.json \
    --export-csv results_append_medium.csv

echo "Running string format benchmarks..."
hyperfine --warmup 3 \
    "./build/bench_format_sstr 'Value: %d' '42'" \
    "./build/bench_format_std 'Value: %d' '42'" \
    --export-markdown results_format_simple.md \
    --export-json results_format_simple.json \
    --export-csv results_format_simple.csv

hyperfine --warmup 3 \
    "./build/bench_format_sstr 'String: %s, Int: %d, Hex: %x' 'test' '42'" \
    "./build/bench_format_std 'String: %s, Int: %d, Hex: %x' 'test' '42'" \
    --export-markdown results_format_complex.md \
    --export-json results_format_complex.json \
    --export-csv results_format_complex.csv

echo "All benchmarks completed."
echo "Results are saved in Markdown, JSON, and CSV formats."
