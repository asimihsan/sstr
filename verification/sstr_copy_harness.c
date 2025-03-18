/*
 * CBMC Verification Harness for sstr_copy function
 */

#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"
#include <stdlib.h>

#ifndef SSTR_ERROR
#define SSTR_ERROR 0
#endif

#ifndef SSTR_TRUNCATE
#define SSTR_TRUNCATE 1
#endif

#ifndef SSTR_DEFAULT_POLICY
#define SSTR_DEFAULT_POLICY SSTR_ERROR
#endif

/* Helper function to mimic internal behavior of sstr_bounded_strlen
 * but with a fixed unwinding bound */
static size_t strlen_test(const char *str) {
    size_t len = 0;
    
    /* Unroll the loop explicitly for CBMC */
    if (str[0] != '\0') { len++; } else { return len; }
    if (str[1] != '\0') { len++; } else { return len; }
    if (str[2] != '\0') { len++; } else { return len; }
    if (str[3] != '\0') { len++; } else { return len; }
    if (str[4] != '\0') { len++; } else { return len; }
    if (str[5] != '\0') { len++; } else { return len; }
    if (str[6] != '\0') { len++; } else { return len; }
    if (str[7] != '\0') { len++; } else { return len; }
    if (str[8] != '\0') { len++; } else { return len; }
    if (str[9] != '\0') { len++; } else { return len; }
    
    return len;
}

/* CBMC verification harness */
void sstr_copy_harness() {
    /* Create a small buffer for the destination string */
    const size_t DEST_SIZE = 10;
    char dest_buffer[DEST_SIZE];
    SStr dest;
    
    /* Initialize the destination string */
    sstr_init(&dest, dest_buffer, DEST_SIZE);
    
    /* Create a small source string buffer */
    const size_t SRC_SIZE = 10;
    char src_buffer[SRC_SIZE];
    
    /* Static initialization of source buffer - no loops needed */
    src_buffer[0] = 'A';
    src_buffer[1] = 'B';
    src_buffer[2] = 'C';
    src_buffer[3] = 'D';
    src_buffer[4] = 'E';
    src_buffer[5] = 'F';
    src_buffer[6] = 'G';
    src_buffer[7] = 'H';
    src_buffer[8] = 'I';
    src_buffer[9] = 'J';
    
    /* Set a deterministic null terminator at some position */
    size_t null_pos;
    __CPROVER_assume(null_pos < SRC_SIZE);
    src_buffer[null_pos] = '\0';
    
    /* Calculate the actual length using strlen */
    size_t actual_len = strlen_test(src_buffer);
    
    /* Call the function under test */
    SStrResult result = sstr_copy(&dest, src_buffer);
    
    /* Verify post-conditions based on the behavior of sstr_copy */
    if (result == SSTR_SUCCESS) {
        /* The actual length should be either the original string length or the capacity,
         * depending on whether truncation occurred */
        if (actual_len <= dest.capacity) {
            /* No truncation needed, full string fits */
            __CPROVER_assert(dest.length == actual_len, "Length is set correctly with no truncation");
        } else {
            /* Truncation happened or would have returned error */
            if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
                /* Should not succeed in this case */
                __CPROVER_assert(0, "Should have returned error, not success");
            } else {
                /* Truncated to capacity */
                __CPROVER_assert(dest.length == dest.capacity, "Length is set to capacity after truncation");
            }
        }
        
        /* String must be null-terminated */
        __CPROVER_assert(dest.data[dest.length] == '\0', "String is null-terminated after copy");
    } else if (result == SSTR_ERROR_OVERFLOW) {
        /* Must be overflow policy with a string too large */
        __CPROVER_assert(SSTR_DEFAULT_POLICY == SSTR_ERROR, "Error returned only with ERROR policy");
        __CPROVER_assert(actual_len > dest.capacity, "Overflow error implies string too long");
    }
}