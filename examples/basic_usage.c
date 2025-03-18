#include "../include/sstr/sstr.h"
#include <stdio.h>

int main(void) {
    /* Stack-allocated buffer */
    char buffer[64];
    
    /* Initialize an SStr structure with the buffer */
    SStr str;
    SStrResult result = sstr_init(&str, buffer, sizeof(buffer));
    
    if (result != SSTR_SUCCESS) {
        printf("Failed to initialize string\n");
        return 1;
    }
    
    /* Copy a string */
    result = sstr_copy(&str, "Hello, ");
    if (result != SSTR_SUCCESS) {
        printf("Failed to copy string\n");
        return 1;
    }
    
    /* Append another string */
    result = sstr_append(&str, "world!");
    if (result != SSTR_SUCCESS) {
        printf("Failed to append string\n");
        return 1;
    }
    
    /* Print the result */
    printf("Result: %s (length: %zu, capacity: %zu)\n", 
           str.data, str.length, str.capacity);
    
    /* Format a string */
    int chars_written = sstr_format(&str, "The answer is %d", 42);
    if (chars_written < 0) {
        printf("Failed to format string\n");
        return 1;
    }
    
    printf("Formatted: %s (length: %zu, capacity: %zu)\n", 
           str.data, str.length, str.capacity);
    
    /* Test overflow handling (default is SSTR_ERROR policy) */
    char small_buffer[5];
    SStr small_str;
    sstr_init(&small_str, small_buffer, sizeof(small_buffer));
    
    result = sstr_copy(&small_str, "This string is too long for the buffer");
    printf("Copy result with overflow: %d\n", result);
    printf("Small buffer content: %s (length: %zu, capacity: %zu)\n", 
           small_str.data, small_str.length, small_str.capacity);
    
    return 0;
}
