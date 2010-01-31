#include "cmdline.h"
#include "printing.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

long int GetInt(const char* param)
{
    long int result;
    char *endptr;

    errno = 0;

    result = strtol(param, &endptr, 10);

    if ((errno == ERANGE && (result == LONG_MAX || result == LONG_MIN))) {
        printf_err("Number out of range: %s\n", param);
        exit(2);
    }

    if (*endptr != '\0') {
        printf_err("Parameter is not a number: %s\n", param);
        exit(2);
    }

    return result;
}
