#include "cmdline.h"
#include "printing.h"
#include "locales.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

long int GetInt(const char* param)
{
    long int result;
    char *endptr;

    errno = 0;

    result = strtol(param, &endptr, 10);

#ifdef WIN32
    /* Windows do not report correctly errno */
    if (result == LONG_MAX || result == LONG_MIN) {
#else
    if ((errno == ERANGE && (result == LONG_MAX || result == LONG_MIN))) {
#endif
        printf_err(_("Number out of range: %s\n"), param);
        exit(2);
    }

    if (*endptr != '\0') {
        printf_err(_("Parameter is not a number: %s\n"), param);
        exit(2);
    }

    return result;
}
