/**
 * Test case for GetInt.
 */

#include "../helper/cmdline.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: getint number\n");
        exit(2);
    }
    printf("Result: %ld", GetInt(argv[1]));
    return 0;
}
