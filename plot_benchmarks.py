#!/usr/bin/env python3
"""
Plot benchmark results from hyperfine outputs
"""

import json
import glob
import matplotlib.pyplot as plt
import numpy as np
import os

def load_benchmark_results(pattern):
    """Load benchmark results from JSON files"""
    results = {}
    for filename in glob.glob(pattern):
        base_name = os.path.basename(filename)
        benchmark_name = os.path.splitext(base_name)[0]
        
        with open(filename, 'r') as f:
            data = json.load(f)
            
        # Extract benchmark results
        benchmark_data = {}
        for result in data['results']:
            name = result['command'].split('/')[-1]  # Extract the executable name
            benchmark_data[name] = {
                'mean': result['mean'],
                'stddev': result['stddev'],
                'min': result['min'],
                'max': result['max'],
            }
            
        results[benchmark_name] = benchmark_data
    
    return results

def create_comparison_plot(results, output_filename):
    """Create comparison plot for mean execution times"""
    benchmarks = sorted(results.keys())
    
    # Group benchmarks by type
    copy_benchmarks = [b for b in benchmarks if b.startswith('results_copy')]
    append_benchmarks = [b for b in benchmarks if b.startswith('results_append')]
    format_benchmarks = [b for b in benchmarks if b.startswith('results_format')]
    
    fig, axes = plt.subplots(3, 1, figsize=(12, 15))
    
    # Plot copy benchmarks
    _plot_benchmark_group(axes[0], results, copy_benchmarks, 'String Copy Performance')
    
    # Plot append benchmarks
    _plot_benchmark_group(axes[1], results, append_benchmarks, 'String Append Performance')
    
    # Plot format benchmarks
    _plot_benchmark_group(axes[2], results, format_benchmarks, 'String Format Performance')
    
    plt.tight_layout()
    plt.savefig(output_filename)
    print(f"Plot saved to {output_filename}")

def _plot_benchmark_group(ax, results, benchmarks, title):
    """Plot a group of benchmarks on a single axis"""
    # Extract benchmark names and mean times
    x_labels = []
    sstr_means = []
    std_means = []
    
    for benchmark in benchmarks:
        x_labels.append(benchmark.split('_', 2)[-1])  # Extract size/type (small, medium, etc.)
        
        # Find the sstr and std implementations
        sstr_impl = None
        std_impl = None
        
        for impl_name in results[benchmark].keys():
            if 'sstr' in impl_name:
                sstr_impl = impl_name
            elif 'std' in impl_name:
                std_impl = impl_name
        
        if sstr_impl and std_impl:
            sstr_means.append(results[benchmark][sstr_impl]['mean'] * 1000)  # Convert to ms
            std_means.append(results[benchmark][std_impl]['mean'] * 1000)    # Convert to ms
    
    # Create bar plot
    x = np.arange(len(x_labels))
    width = 0.35
    
    ax.bar(x - width/2, sstr_means, width, label='sstr', color='blue', alpha=0.7)
    ax.bar(x + width/2, std_means, width, label='std', color='orange', alpha=0.7)
    
    # Calculate percentage difference
    for i in range(len(sstr_means)):
        diff_percent = ((sstr_means[i] - std_means[i]) / std_means[i]) * 100
        ax.text(i, max(sstr_means[i], std_means[i]) + 0.05, 
                f"{diff_percent:.1f}%", 
                ha='center', va='bottom',
                color='green' if diff_percent < 0 else 'red')
    
    ax.set_title(title)
    ax.set_ylabel('Mean execution time (ms)')
    ax.set_xticks(x)
    ax.set_xticklabels(x_labels)
    ax.legend()
    
    # Add a grid for better readability
    ax.grid(axis='y', linestyle='--', alpha=0.7)

if __name__ == "__main__":
    # Load all benchmark results
    results = load_benchmark_results("results_*.json")
    
    # Create comparison plot
    create_comparison_plot(results, "benchmark_results.png")