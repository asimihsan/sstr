# SStr Benchmarks

This directory contains benchmark code to compare the performance of the SStr library against standard C string functions.

## Benchmarks

The benchmarks compare the following operations:

1. **String Copy**
   - `sstr_copy` and `sstr_copy_n` vs `strcpy`
   - Tests with small, medium, and large strings

2. **String Append**
   - `sstr_append` vs `strcat`
   - Tests with small and medium strings

3. **String Formatting**
   - `sstr_format` vs `snprintf`
   - Tests with simple and complex format strings

## Running the Benchmarks

Requirements:
- [Hyperfine](https://github.com/sharkdp/hyperfine) for running benchmarks
- CMake and a C compiler
- Python with matplotlib (for visualization)

To run all benchmarks:

```bash
./run_benchmarks.sh
```

The script will:
1. Build all benchmark executables
2. Run multiple benchmark comparisons with hyperfine
3. Save the results in Markdown, JSON, and CSV formats

## Visualizing Results

After running the benchmarks, you can visualize the results:

```bash
python plot_benchmarks.py
```

This will generate a `benchmark_results.png` file with bar charts comparing the performance of SStr functions vs standard C functions.

## Benchmark Details

Each benchmark program:
- Performs a warm-up phase (1,000 iterations)
- Runs the actual benchmark (100,000 iterations)
- Allows customizing input via command line arguments

## Interpreting Results

The benchmark results show the performance overhead (if any) introduced by SStr's safety features compared to standard C string functions. Key factors to consider:

- **Bounds checking**: SStr performs bounds checking which may add overhead
- **Format validation**: When enabled, sstr_format validates format strings for security
- **Error handling**: SStr checks for NULL pointers and other error conditions

Keep in mind that SStr prioritizes safety over raw performance, making it ideal for embedded systems where security and reliability are critical.
