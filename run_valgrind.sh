#!/bin/bash
set -e

# Build the Docker image
echo "Building Docker image..."
docker build -t sstr-valgrind .

# Run Valgrind tests in Docker
echo "Running Valgrind tests..."
docker run --rm sstr-valgrind

echo "Valgrind tests completed successfully!"