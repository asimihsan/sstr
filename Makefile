CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -DSSTR_DEFAULT_POLICY=SSTR_ERROR
INCLUDES = -Iinclude

# Format validation
ifdef NO_FORMAT_VALIDATION
    CFLAGS += -DSSTR_VALIDATE_FORMAT=0
else
    CFLAGS += -DSSTR_VALIDATE_FORMAT=1
endif

# Customize allowed format specifiers
ifdef ALLOWED_SPECIFIERS
    CFLAGS += -DSSTR_ALLOWED_SPECIFIERS=\"$(ALLOWED_SPECIFIERS)\"
endif

# Debug build
ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG
else
    CFLAGS += -O2
endif

# Library objects
LIB_SRCS = src/sstr.c src/sstr_format.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Test objects
TEST_SRCS = tests/test_runner.c tests/test_core.c tests/test_format.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Example objects
EXAMPLE_SRCS = examples/basic_usage.c examples/formatting.c examples/format_validation.c
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.c=.o)
EXAMPLES = $(EXAMPLE_SRCS:.c=)

# Static library
STATIC_LIB = libsstr.a

# Default target
all: $(STATIC_LIB) examples tests

# Benchmarks
benchmarks:
	mkdir -p build
	cd build && cmake .. -DSSTR_BUILD_BENCHMARKS=ON && \
	make bench_copy_sstr bench_copy_std bench_append_sstr bench_append_std bench_format_sstr bench_format_std

run_benchmarks: benchmarks
	mise x -- ./run_benchmarks.sh

# Generate the single-include version
single_include:
	./build_single_include.sh

# Verify the single-include file is up-to-date
verify-single-include: single_include
	@if git diff --quiet single_include/sstr.h; then \
		echo "✅ single_include/sstr.h is up-to-date"; \
	else \
		echo "❌ single_include/sstr.h is out of date. Run 'make single_include' to update."; \
		git --no-pager diff single_include/sstr.h; \
		exit 1; \
	fi

# Compile library objects
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Create static library
$(STATIC_LIB): $(LIB_OBJS)
	ar rcs $@ $^

# Build all examples
examples: $(EXAMPLES)

# Link examples
examples/%: examples/%.o $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -L. -lsstr -o examples/$*

# Build test runner
tests: test_runner

test_runner: $(TEST_OBJS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -lsstr -o $@

# Validation test
test_validation: tests/test_validation.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $< -L. -lsstr -o $@

# Test the single-include STB-style implementation
test_single_include: tests/test_single_include.c
	$(CC) $(CFLAGS) $< -o $@

# Run tests
check: test_runner test_single_include
	./test_runner
	./test_single_include

# Clean build files
clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS) $(EXAMPLE_OBJS) $(STATIC_LIB) test_runner test_validation test_single_include $(EXAMPLES)

# Install
PREFIX ?= /usr/local
install: $(STATIC_LIB)
	mkdir -p $(PREFIX)/include/sstr
	mkdir -p $(PREFIX)/lib
	cp include/sstr/*.h $(PREFIX)/include/sstr/
	cp $(STATIC_LIB) $(PREFIX)/lib/
	cp single_include/sstr.h $(PREFIX)/include/

# Uninstall
uninstall:
	rm -rf $(PREFIX)/include/sstr
	rm -f $(PREFIX)/include/sstr.h
	rm -f $(PREFIX)/lib/$(STATIC_LIB)

copyright:
	fd -e c -e h | xargs addlicense -f copyright.tmpl -c "Asim Ihsan" -v -s

copyright-check:
	fd -e c -e h | xargs addlicense -f copyright.tmpl -c "Asim Ihsan" -v -s -check

# Format code using clang-format
format:
	find src include tests examples -name "*.c" -o -name "*.h" | xargs clang-format -i

# Check if code is properly formatted
format-check:
	find src include tests examples -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror

# Docker build for verification tools
# Build Docker images for verification
build-docker:
	docker buildx build \
	  --progress=plain \
	  --load \
	  -t sstr-verification-arm64 \
	  --platform linux/arm64,linux/amd64 \
	  .

# Docker runner for CBMC (use native ARM64 for better performance)
docker-run-cbmc:
	docker run --platform linux/arm64 --rm -v $(shell pwd):/app -w /app sstr-verification-arm64 $(CMD)

# Docker runner for Klee (must use AMD64 as Klee doesn't support ARM64)
docker-run-klee:
	docker run --platform linux/amd64 --rm -v $(shell pwd):/app -w /app sstr-verification-amd64 $(CMD)

# Default Docker runner - use ARM64 for performance when not using Klee
docker-run:
	docker run --platform linux/arm64 --rm -v $(shell pwd):/app -w /app sstr-verification-arm64 $(CMD)

# Test with different format validation configurations
validation-tests: test_validation test_single_include
	@echo "Testing with format validation enabled (default)..."
	./test_validation
	./test_single_include
	@echo "\nTesting with format validation disabled..."
	$(MAKE) clean
	$(MAKE) NO_FORMAT_VALIDATION=1
	$(MAKE) test_validation NO_FORMAT_VALIDATION=1
	$(MAKE) test_single_include NO_FORMAT_VALIDATION=1
	./test_validation
	./test_single_include
	@echo "\nTesting with custom allowed specifiers..."
	$(MAKE) clean
	$(MAKE) ALLOWED_SPECIFIERS="ds"
	$(MAKE) test_validation
	$(MAKE) test_single_include ALLOWED_SPECIFIERS="ds"
	./test_validation
	./test_single_include

# Run Valgrind in Docker
valgrind-verify:
	$(MAKE) docker-run CMD="bash -c 'make clean && make test_runner && /usr/bin/valgrind --leak-check=full --error-exitcode=1 ./test_runner'"

# CBMC verification targets with bounded loop unwinding and timeout
# Local CBMC commands (kept for reference)
cbmc-local-init:
	cbmc src/sstr.c src/sstr_format.c verification/sstr_init_harness.c --function sstr_init_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail

cbmc-local-copy:
	cbmc src/sstr.c src/sstr_format.c verification/sstr_copy_harness.c --function sstr_copy_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail --slice-formula

cbmc-local-append:
	cbmc src/sstr.c src/sstr_format.c verification/sstr_append_harness.c --function sstr_append_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail --slice-formula

# Docker-based CBMC verification targets (use ARM64 for speed)
cbmc-verify-init:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c verification/sstr_init_harness.c --function sstr_init_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail"

cbmc-verify-copy:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c verification/sstr_copy_harness.c --function sstr_copy_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail --slice-formula"

cbmc-verify-append:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c verification/sstr_append_harness.c --function sstr_append_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail --slice-formula"

# Show available properties for a function
cbmc-properties:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c --function sstr_init --show-properties"

# Run all CBMC verifications
cbmc-verify: cbmc-verify-init cbmc-verify-copy cbmc-verify-append

# Klee verification targets (use AMD64 as required by Klee)
# Klee verification targets using the script (local)
klee-init:
	./run_klee.sh sstr_init

klee-copy:
	./run_klee.sh sstr_copy

klee-append:
	./run_klee.sh sstr_append

klee-all:
	./run_klee.sh

# Klee verification targets (Docker)
klee-docker-init:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh sstr_init"

klee-docker-copy:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh sstr_copy"

klee-docker-append:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh sstr_append"

klee-docker-all:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh"

# Top-level verify targets
klee-verify: klee-all
klee-verify-docker: klee-docker-all

# Run all verifications
verify-all: cbmc-verify klee-verify valgrind-verify

setup:
	mise x -- pre-commit install
	mise x -- pre-commit autoupdate

pre-commit-run:
	mise x -- pre-commit run --all-files

# CI target that runs all checks
ci: all tests check format-check copyright-check validation-tests verify-single-include verify-all pre-commit-run

# Define groups of PHONY targets
PHONY_MAIN = all clean check examples tests
PHONY_BUILD = benchmarks run_benchmarks single_include verify-single-include
PHONY_INSTALL = install uninstall
PHONY_CODING = copyright copyright-check format format-check
PHONY_TESTING = valgrind-verify validation-tests test_validation test_single_include ci
PHONY_DOCKER = docker-run build-docker docker-run-cbmc docker-run-klee
PHONY_CBMC = cbmc-verify cbmc-verify-init cbmc-verify-copy cbmc-verify-append cbmc-properties
PHONY_KLEE = klee-init klee-copy klee-append klee-all \
            klee-docker-init klee-docker-copy klee-docker-append klee-docker-all \
            klee-verify klee-verify-docker
PHONY_ALL = verify-all

# Combine all PHONY targets
.PHONY: $(PHONY_MAIN) $(PHONY_BUILD) $(PHONY_INSTALL) $(PHONY_CODING) \
        $(PHONY_TESTING) $(PHONY_DOCKER) $(PHONY_CBMC) $(PHONY_KLEE) $(PHONY_ALL)
