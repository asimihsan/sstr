#!/bin/bash
set -e

# Run Valgrind tests using the Makefile target
echo "Running Valgrind tests in Docker..."
make valgrind-docker

echo "Valgrind tests completed successfully!"
