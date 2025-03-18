#!/bin/bash
set -e

# Format code using clang-format
echo "Formatting code..."
make format

echo "Code formatting completed successfully!"