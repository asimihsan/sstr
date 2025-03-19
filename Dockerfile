# syntax=docker/dockerfile:1.4
FROM ubuntu:22.04

# These ARGs are automatically set by buildx
ARG TARGETPLATFORM
ARG BUILDPLATFORM
ARG TARGETOS
ARG TARGETARCH

# Print build info for debugging
RUN echo "Building for $TARGETPLATFORM (arch: $TARGETARCH)"

# Set environment variable for architecture detection
ENV TARGETARCH=${TARGETARCH:-arm64}

# Install essential build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    flex \
    bison \
    git \
    curl \
    patch \
    valgrind \
    clang-format \
    ninja-build \
    graphviz \
    gnuplot \
    universal-ctags \
    wget \
    unzip \
    python3 \
    python3-pip \
    libtcmalloc-minimal4 \
    libgoogle-perftools-dev \
    zlib1g-dev \
    libboost-all-dev \
    python3 \
    perl \
    minisat \
    software-properties-common \
    gnupg \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install uv - the modern Python package installer
RUN <<EOF
curl -fsSL https://astral.sh/uv/install.sh | sh
EOF

ENV PATH="/root/.local/bin:${PATH}"

# Create workspace directory
WORKDIR /build

# Create a virtual environment for Litani dependencies
RUN uv venv /build/litani-venv

# Set up environment variables for the virtual environment
ENV VIRTUAL_ENV=/build/litani-venv
ENV PATH="/build/litani-venv/bin:${PATH}"
ENV PYTHONPATH="/build/litani-venv/lib/python3.10/site-packages:/build"

# Build Litani from source with cache mount for Git operations
RUN --mount=type=cache,target=/root/.cache/git \
    git clone --depth 1 --single-branch https://github.com/awslabs/aws-build-accumulator.git litani-src && \
    cd litani-src && \
    # Install Python dependencies with uv in the virtual environment
    uv pip install jinja2 voluptuous && \
    # Create a bin directory for the litani scripts
    mkdir -p /usr/local/bin && \
    # Copy the main litani script to bin
    cp litani /usr/local/bin/ && \
    # Copy the lib directory to the right location in PYTHONPATH
    mkdir -p /build/lib && \
    cp -r lib/* /build/lib/ && \
    # Make the litani script executable
    chmod +x /usr/local/bin/litani && \
    # Test that it's working
    litani --help && \
    # Clean up source
    cd .. && rm -rf litani-src

# Build CBMC from source with cache for Git clone and build artifacts
RUN --mount=type=cache,target=/root/.cache/git \
    --mount=type=cache,target=/build/.cache \
    git clone --depth 1 --branch cbmc-6.5.0 https://github.com/diffblue/cbmc.git cbmc-src && \
    cd cbmc-src && \
    git submodule update --init --depth 1 && \
    # Use cache mount for CMake build
    cmake -S . -Bbuild -DCMAKE_BUILD_TYPE=Release -DWITH_JBMC=OFF && \
    cmake --build build -j$(nproc) && \
    # Install CBMC binaries
    cp build/bin/cbmc /usr/local/bin/ && \
    cp build/bin/goto-cc /usr/local/bin/ && \
    cp build/bin/goto-instrument /usr/local/bin/ && \
    cp build/bin/goto-analyzer /usr/local/bin/ && \
    # Verify installation
    cbmc --version && \
    # Clean up source
    cd .. && rm -rf cbmc-src

# Create a new virtual environment for Python tools
RUN uv venv /opt/tools-venv
ENV VIRTUAL_ENV=/opt/tools-venv
ENV PATH="/opt/tools-venv/bin:${PATH}"

# Install cbmc-viewer and cbmc-starter-kit with uv
RUN uv pip install cbmc-viewer cbmc-starter-kit

# Install only what's needed for both platforms
RUN uv pip install lit

# Only build KLEE and dependencies on AMD64
RUN if [ "$TARGETARCH" = "amd64" ]; then \
    # Install LLVM 13 and related packages for AMD64
    apt-get update && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-13 main" && \
    apt-get update && \
    apt-get install -y clang-13 llvm-13 llvm-13-dev llvm-13-tools llvm-13-runtime && \
    rm -rf /var/lib/apt/lists/* && \
    # Create symlinks for clang, clang++, and llvm-link
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-13 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-13 100 && \
    update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-13 100 && \
    update-alternatives --install /usr/bin/llvm-link llvm-link /usr/bin/llvm-link-13 100 && \
    # Install STP constraint solver for KLEE
    git clone https://github.com/stp/stp.git && \
    cd stp && \
    git checkout tags/2.3.3 && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    cd ../.. && \
    rm -rf stp && \
    # Download and extract Google Test for KLEE
    curl -OL https://github.com/google/googletest/archive/release-1.11.0.zip && \
    unzip release-1.11.0.zip && \
    rm release-1.11.0.zip && \
    # Build klee-uclibc for POSIX support
    git clone https://github.com/klee/klee-uclibc.git && \
    cd klee-uclibc && \
    ./configure --make-llvm-lib --with-cc clang-13 --with-llvm-config llvm-config-13 && \
    make -j$(nproc) && \
    cd .. && \
    # Build KLEE
    git clone https://github.com/klee/klee.git && \
    cd klee && \
    mkdir build && \
    cd build && \
    cmake -DENABLE_SOLVER_STP=ON \
          -DENABLE_POSIX_RUNTIME=ON \
          -DKLEE_UCLIBC_PATH=/build/klee-uclibc \
          -DENABLE_UNIT_TESTS=ON \
          -DGTEST_SRC_DIR=/build/googletest-release-1.11.0 \
          -DLLVM_CONFIG_BINARY=/usr/bin/llvm-config-13 \
          -DLLVMCC=/usr/bin/clang-13 \
          -DLLVMCXX=/usr/bin/clang++-13 \
          .. && \
    make -j$(nproc) && \
    make install && \
    # Add KLEE binaries to PATH
    echo 'export PATH="/build/klee/build/bin:$PATH"' >> /root/.bashrc; \
else \
    # On ARM64, just install a minimal toolchain with clang
    apt-get update && \
    apt-get install -y clang && \
    rm -rf /var/lib/apt/lists/* && \
    # Create a dummy klee command for ARM64 that explains it's not available
    echo '#!/bin/bash' > /usr/local/bin/klee && \
    echo 'echo "KLEE is not available on ARM64 architecture. Please use the AMD64 image for KLEE verification."' >> /usr/local/bin/klee && \
    chmod +x /usr/local/bin/klee; \
fi

# Add KLEE binaries to PATH (will be a real path on AMD64, dummy on ARM64)
ENV PATH="/build/klee/build/bin:${PATH}"

# Set the stack size to unlimited for STP
RUN echo "ulimit -s unlimited" >> /root/.bashrc

# Copy run_klee_docker.sh to the container and make it executable
COPY run_klee_docker.sh /app/
RUN chmod +x /app/run_klee_docker.sh
