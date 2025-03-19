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
.PHONY: all
all: $(STATIC_LIB) examples tests

# Benchmarks
.PHONY: benchmarks
benchmarks:
	mkdir -p build
	cd build && cmake .. -DSSTR_BUILD_BENCHMARKS=ON && \
	make bench_copy_sstr bench_copy_std bench_append_sstr bench_append_std bench_format_sstr bench_format_std

.PHONY: run_benchmarks
run_benchmarks: benchmarks
	mise x -- ./run_benchmarks.sh

# Generate the single-include version
.PHONY: single_include
single_include:
	./build_single_include.sh

# Verify the single-include file is up-to-date
.PHONY: verify-single-include
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
.PHONY: examples
examples: $(EXAMPLES)

# Link examples
examples/%: examples/%.o $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -L. -lsstr -o examples/$*

# Build test runner
.PHONY: tests
tests: test_runner

.PHONY: test_runner
test_runner: $(TEST_OBJS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -lsstr -o $@

# Validation test
.PHONY: test_validation
test_validation: tests/test_validation.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $< -L. -lsstr -o $@

# Test the single-include STB-style implementation
.PHONY: test_single_include
test_single_include: single_include
	$(CC) $(CFLAGS) tests/test_single_include.c -o $@

# Run tests - always clean first to ensure consistent builds
.PHONY: check
check: clean single_include
	$(MAKE) $(STATIC_LIB)
	$(MAKE) test_runner
	$(MAKE) test_single_include
	./test_runner
	./test_single_include

# Clean build files
.PHONY: clean
clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS) $(EXAMPLE_OBJS) $(STATIC_LIB) test_runner test_validation test_single_include $(EXAMPLES)

# Install
PREFIX ?= /usr/local
.PHONY: install
install: $(STATIC_LIB)
	mkdir -p $(PREFIX)/include/sstr
	mkdir -p $(PREFIX)/lib
	cp include/sstr/*.h $(PREFIX)/include/sstr/
	cp $(STATIC_LIB) $(PREFIX)/lib/
	cp single_include/sstr.h $(PREFIX)/include/

# Uninstall
.PHONY: uninstall
uninstall:
	rm -rf $(PREFIX)/include/sstr
	rm -f $(PREFIX)/include/sstr.h
	rm -f $(PREFIX)/lib/$(STATIC_LIB)

.PHONY: copyright
copyright:
	fd -e c -e h | xargs addlicense -f copyright.tmpl -c "Asim Ihsan" -v -s

.PHONY: copyright-check
copyright-check:
	fd -e c -e h | xargs addlicense -f copyright.tmpl -c "Asim Ihsan" -v -s -check

# Format code using clang-format
.PHONY: format
format:
	find src include tests examples -name "*.c" -o -name "*.h" | xargs clang-format -i

# Check if code is properly formatted
.PHONY: format-check
format-check:
	find src include tests examples -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror

# Docker build for verification tools
# Build Docker images for verification
.PHONY: build-docker
build-docker:
	docker buildx build \
	  --progress=plain \
	  --load \
	  -t sstr-verification \
	  --platform linux/arm64,linux/amd64 \
	  .

# Docker runner for CBMC (use native ARM64 for better performance)
.PHONY: docker-run-cbmc
docker-run-cbmc:
	docker run --platform $(shell uname -m | grep -q "arm64" && echo "linux/arm64" || echo "linux/amd64") --rm -v $(shell pwd):/app -w /app sstr-verification $(CMD)

# Docker runner for Klee (must use AMD64 as Klee doesn't support ARM64)
.PHONY: docker-run-klee
docker-run-klee:
	docker run --platform linux/amd64 --rm -v $(shell pwd):/app -w /app sstr-verification $(CMD)

# Default Docker runner - use ARM64 for performance when not using Klee
.PHONY: docker-run
docker-run:
	docker run --platform linux/arm64 --rm -v $(shell pwd):/app -w /app sstr-verification $(CMD)

# Test with different format validation configurations
.PHONY: validation-tests
validation-tests: clean
	@echo "Testing with format validation enabled (default)..."
	$(MAKE) clean
	$(MAKE) single_include
	$(MAKE) $(STATIC_LIB)
	$(MAKE) test_validation
	$(MAKE) test_single_include
	./test_validation
	./test_single_include
	@echo "\nTesting with format validation disabled..."
	$(MAKE) clean
	$(MAKE) single_include
	$(MAKE) NO_FORMAT_VALIDATION=1 $(STATIC_LIB)
	$(MAKE) test_validation NO_FORMAT_VALIDATION=1
	$(MAKE) test_single_include NO_FORMAT_VALIDATION=1
	./test_validation
	./test_single_include
	@echo "\nTesting with custom allowed specifiers..."
	$(MAKE) clean
	$(MAKE) single_include
	$(MAKE) ALLOWED_SPECIFIERS="ds" $(STATIC_LIB)
	$(MAKE) test_validation ALLOWED_SPECIFIERS="ds"
	$(MAKE) test_single_include ALLOWED_SPECIFIERS="ds"
	./test_validation
	./test_single_include

# Run Valgrind in Docker
.PHONY: valgrind-verify
valgrind-verify:
	$(MAKE) docker-run CMD="bash -c 'make clean && make single_include && make test_runner && /usr/bin/valgrind --leak-check=full --error-exitcode=1 ./test_runner'"

# Docker-based CBMC verification targets (use ARM64 for speed)
.PHONY: cbmc-verify-init
cbmc-verify-init:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c verification/sstr_init_harness.c --function sstr_init_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail"

.PHONY: cbmc-verify-copy
cbmc-verify-copy:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c verification/sstr_copy_harness.c --function sstr_copy_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail --slice-formula"

.PHONY: cbmc-verify-append
cbmc-verify-append:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c verification/sstr_append_harness.c --function sstr_append_harness --bounds-check --pointer-check --unwind 10 --unwinding-assertions --stop-on-fail --slice-formula"

# Show available properties for a function
.PHONY: cbmc-properties
cbmc-properties:
	$(MAKE) docker-run-cbmc CMD="cbmc src/sstr.c src/sstr_format.c --function sstr_init --show-properties"

# Run all CBMC verifications
.PHONY: cbmc-verify
cbmc-verify: cbmc-verify-init cbmc-verify-copy cbmc-verify-append

# Klee verification targets (use AMD64 as required by Klee)
# Klee verification targets using the script (local)
.PHONY: klee-init
klee-init:
	./run_klee.sh sstr_init

.PHONY: klee-copy
klee-copy:
	./run_klee.sh sstr_copy

.PHONY: klee-append
klee-append:
	./run_klee.sh sstr_append

.PHONY: klee-all
klee-all:
	./run_klee.sh

# Klee verification targets (Docker)
.PHONY: klee-docker-init
klee-docker-init:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh sstr_init"

.PHONY: klee-docker-copy
klee-docker-copy:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh sstr_copy"

.PHONY: klee-docker-append
klee-docker-append:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh sstr_append"

.PHONY: klee-docker-all
klee-docker-all:
	$(MAKE) docker-run-klee CMD="./run_klee_docker.sh"

# Top-level verify targets
.PHONY: klee-verify
klee-verify: klee-all

.PHONY: klee-verify-docker
klee-verify-docker: klee-docker-all

# Display KLEE test results in human-readable format
.PHONY: klee-results
klee-results:
	@echo "Displaying KLEE verification results..."
	@if [ -d "./klee-build" ]; then \
		for dir in ./klee-build/klee-*-out; do \
			if [ -d "$$dir" ]; then \
				echo "\n📊 Results for $$(basename $$dir | sed 's/klee-\(.*\)-out/\1/')"; \
				echo "--------------------------------------------"; \
				echo "📈 Test statistics:"; \
				cat "$$dir/info" 2>/dev/null | grep -E "instructions|completed paths|generated tests" || echo "No info file found"; \
				echo "\n💥 Errors found:"; \
				if [ -d "$$dir/test-errors" ] && ls "$$dir/test-errors" 2>/dev/null | grep -q "\.err"; then \
					for err in "$$dir/test-errors"/*.err; do \
						echo "  - $$(basename $$err | sed 's/\.err//'): $$(cat $$err)"; \
					done; \
				else \
					echo "  ✅ No errors found"; \
				fi; \
				echo ""; \
			fi; \
		done; \
	else \
		echo "❌ KLEE build directory not found. Run klee-verify or klee-verify-docker first."; \
	fi

# Docker-based KLEE results
.PHONY: klee-results-docker
klee-results-docker:
	$(MAKE) docker-run-klee CMD="bash -c 'make klee-results'"

# Run all verifications
.PHONY: verify-all
verify-all: clean cbmc-verify klee-verify valgrind-verify

.PHONY: setup
setup:
	mise x -- pre-commit install
	mise x -- pre-commit autoupdate

.PHONY: pre-commit-run
pre-commit-run:
	mise x -- pre-commit run --all-files

# CI targets - All Docker-based for consistency across platforms
.PHONY: ci
ci: clean single_include verify-single-include format-check copyright-check pre-commit-run
	$(MAKE) docker-run CMD="make clean && make check"
	$(MAKE) cbmc-verify
	$(MAKE) klee-docker-all
	$(MAKE) valgrind-verify

# Note: .PHONY declarations have been moved to each target definition
