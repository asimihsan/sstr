FROM debian:bullseye-slim

# Install build tools and Valgrind
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Create workspace directory
WORKDIR /app

# Copy the source code
COPY . .

# Build the project
RUN mkdir -p build && cd build && \
    cmake .. && \
    make && \
    make test

# Default command to run tests with Valgrind
ENTRYPOINT ["valgrind", "--leak-check=full", "--error-exitcode=1"]
CMD ["./build/test_runner"]