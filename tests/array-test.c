/**
 * Simple test case for array manipulations.
 */

#include "../helper/array.h"
#include "common.h"
#include <gammu-misc.h>

int main(int argc UNUSED, char **argv UNUSED)
{
    GSM_StringArray array;

    /* Simple new -> free */
    GSM_StringArray_New(&array);
    GSM_StringArray_Free(&array);

    GSM_StringArray_New(&array);
    test_result(GSM_StringArray_Add(&array, "123456"));
    test_result(GSM_StringArray_Add(&array, "654321"));
    test_result(GSM_StringArray_Find(&array, "654321"));
    test_result(GSM_StringArray_Find(&array, "123456"));
    test_result(!GSM_StringArray_Find(&array, "666"));
    GSM_StringArray_Free(&array);
    return 0;
}

