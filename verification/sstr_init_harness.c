/*
 * CBMC Verification Harness for sstr_init function
 */

#include "../include/sstr/sstr.h"
#include "../include/sstr/sstr_config.h"

/* CBMC verification harness */
void sstr_init_harness() {
    /* Declare variables for the function parameters */
    SStr str;
    
    /* Use a fixed buffer size to simplify verification */
    const size_t buffer_size = 10;
    
    /* Allocate a buffer of the chosen size */
    char buffer[buffer_size];
    
    /* Call the function under test */
    SStrResult result = sstr_init(&str, buffer, buffer_size);
    
    /* Verify post-conditions */
    if (result == SSTR_SUCCESS) {
        /* Check that the string was initialized properly */
        __CPROVER_assert(str.data == buffer, "Data pointer is set correctly");
        __CPROVER_assert(str.capacity == buffer_size - 1, "Capacity is set correctly");
        __CPROVER_assert(str.length == 0, "Length is set to zero");
        __CPROVER_assert(str.data[0] == '\0', "String is null-terminated");
        
        /* Also check with specific values */
        __CPROVER_assert(str.capacity == 9, "Capacity calculation is correct");
    }
    
    /* Test the null pointer error case */
    SStrResult null_result = sstr_init(NULL, buffer, buffer_size);
    __CPROVER_assert(null_result == SSTR_ERROR_NULL, "Null SStr pointer returns NULL error");
    
    /* Test the null buffer error case */
    SStrResult null_buffer_result = sstr_init(&str, NULL, buffer_size);
    __CPROVER_assert(null_buffer_result == SSTR_ERROR_NULL, "Null buffer pointer returns NULL error");
    
    /* Test the zero buffer size error case */
    SStrResult zero_size_result = sstr_init(&str, buffer, 0);
    __CPROVER_assert(zero_size_result == SSTR_ERROR_OVERFLOW, "Zero buffer size returns OVERFLOW error");
}