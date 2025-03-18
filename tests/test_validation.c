#include "sstr/sstr.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    char buffer[64];
    SStr str;
    int result;

    sstr_init(&str, buffer, sizeof(buffer));

    /* Test valid format */
    result = sstr_format(&str, "Test %s %d", "string", 42);
    printf("Valid format result: %d, content: %s\n", result, str.data);
    assert(result > 0);

    /* Test invalid format */
    result = sstr_format(&str, "Test %f", 3.14);
    printf("Invalid format result: %d\n", result);

#if SSTR_VALIDATE_FORMAT
    /* When validation is enabled, float formats should be rejected */
    assert(result == SSTR_ERROR_FORMAT);
    printf("Format validation is ENABLED - properly rejected float format\n");
#else
    /* When validation is disabled, result depends on whether floats are supported */
    printf("Format validation is DISABLED\n");
    /* We can't guarantee the exact result since it depends on vsnprintf implementation */
#endif

    printf("All tests passed!\n");
    return 0;
}
