CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -DSSTR_DEFAULT_POLICY=SSTR_ERROR
INCLUDES = -Iinclude

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
EXAMPLE_SRCS = examples/basic_usage.c examples/formatting.c
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.c=.o)
EXAMPLES = $(EXAMPLE_SRCS:.c=)

# Static library
STATIC_LIB = libsstr.a

# Default target
all: $(STATIC_LIB) examples tests

# Compile single-include version
single_include: 
	@echo "Single-include library is already available at single_include/sstr.h"

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
	$(CC) $(CFLAGS) $< -L. -lsstr -o $@

# Build test runner
tests: test_runner

test_runner: $(TEST_OBJS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -lsstr -o $@

# Run tests
check: test_runner
	./test_runner

# Clean build files
clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS) $(EXAMPLE_OBJS) $(STATIC_LIB) test_runner $(EXAMPLES)

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

.PHONY: all clean check examples tests single_include install uninstall copyright copyright-check
