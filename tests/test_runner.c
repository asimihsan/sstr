#include <stdio.h>

/* Test function declarations */
extern int run_core_tests(void);
extern int run_format_tests(void);

int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("=== SStr Library Test Suite ===\n\n");
    
    /* Run core tests */
    total++;
    if (run_core_tests()) {
        passed++;
    } else {
        printf("Some core tests failed\n");
    }
    
    printf("\n");
    
    /* Run format tests */
    total++;
    if (run_format_tests()) {
        passed++;
    } else {
        printf("Some format tests failed\n");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d test groups\n", passed, total);
    
    return (passed == total) ? 0 : 1;
}
