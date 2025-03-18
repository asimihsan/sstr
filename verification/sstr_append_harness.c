/*
 * CBMC Verification Harness for sstr_append function
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

/* Helper function to mimic internal behavior of sstr_bounded_strlen */
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
void sstr_append_harness() {
    /* Create a small buffer for the destination string */
    const size_t DEST_SIZE = 10;
    char dest_buffer[DEST_SIZE];
    SStr dest;
    
    /* Initialize the destination string */
    sstr_init(&dest, dest_buffer, DEST_SIZE);
    
    /* Set a fixed initial length for the destination */
    size_t initial_length = 5; /* Use a fixed value to simplify verification */
    __CPROVER_assume(initial_length < DEST_SIZE);
    dest.length = initial_length;
    
    /* Put some content in the destination string */
    dest_buffer[0] = 'H';
    dest_buffer[1] = 'e';
    dest_buffer[2] = 'l';
    dest_buffer[3] = 'l';
    dest_buffer[4] = 'o';
    dest_buffer[5] = '\0';  /* Null-terminate */
    
    /* Create a source string buffer */
    const size_t SRC_SIZE = 10;
    char src_buffer[SRC_SIZE];
    
    /* Static initialization of source buffer */
    src_buffer[0] = 'W';
    src_buffer[1] = 'o';
    src_buffer[2] = 'r';
    src_buffer[3] = 'l';
    src_buffer[4] = 'd';
    src_buffer[5] = '!';
    src_buffer[6] = '\0';  /* Always null-terminate at a fixed position */
    src_buffer[7] = 'X';   /* Padding */
    src_buffer[8] = 'Y';
    src_buffer[9] = 'Z';
    
    /* Calculate the length of the source string */
    size_t src_len = strlen_test(src_buffer);
    
    /* Keep track of the available space */
    size_t available = dest.capacity - dest.length;
    
    /* Call the function under test */
    SStrResult result = sstr_append(&dest, src_buffer);
    
    /* Verify post-conditions */
    if (result == SSTR_SUCCESS) {
        /* Verify the string was appended properly */
        if (src_len <= available) {
            /* Should have grown by exactly src_len */
            __CPROVER_assert(dest.length == initial_length + src_len, 
                            "Length is correct after append");
                            
            /* Verify the first part of the string is unchanged */
            __CPROVER_assert(dest.data[0] == 'H', "First character preserved");
            __CPROVER_assert(dest.data[1] == 'e', "Second character preserved");
            __CPROVER_assert(dest.data[2] == 'l', "Third character preserved");
            __CPROVER_assert(dest.data[3] == 'l', "Fourth character preserved");
            __CPROVER_assert(dest.data[4] == 'o', "Fifth character preserved");
            
            /* Check the appended characters */
            __CPROVER_assert(dest.data[5] == 'W', "First appended character correct");
            __CPROVER_assert(dest.data[6] == 'o', "Second appended character correct");
        } else {
            /* Policy-dependent behavior */
            if (SSTR_DEFAULT_POLICY == SSTR_ERROR) {
                /* Should not succeed with ERROR policy when overflow */
                __CPROVER_assert(0, "Should have returned error, not success");
            } else {
                /* Truncated to capacity */
                __CPROVER_assert(dest.length == dest.capacity, 
                               "Length uses maximum capacity after truncated append");
            }
        }
        
        /* String should be null-terminated */
        __CPROVER_assert(dest.data[dest.length] == '\0', 
                        "String is null-terminated after append");
    } else if (result == SSTR_ERROR_OVERFLOW) {
        /* Must be overflow policy with a string too large */
        __CPROVER_assert(SSTR_DEFAULT_POLICY == SSTR_ERROR, "Error returned only with ERROR policy");
        __CPROVER_assert(src_len > available, "Overflow error implies string too long");
    }
}