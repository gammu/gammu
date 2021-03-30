/**
 * INI file parsing tests.
 */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

int main(int argc, char **argv)
{
	GSM_Error error;
	INI_Section *ini = NULL;
    int intval;
    gboolean boolval;
    char *strval;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: inifile ini_file\n");
		return 1;
	}

	error = INI_ReadFile(argv[1], FALSE, &ini);
    gammu_test_result(error, "INI_ReadFile");
    test_result(ini != NULL);

	intval = INI_GetInt(ini, "section", "intval", -1);
    test_result(intval == 65536);

    boolval = INI_GetBool(ini, "section", "trueval", FALSE);
    test_result(boolval == TRUE);

    boolval = INI_GetBool(ini, "section", "falseval", TRUE);
    test_result(boolval == FALSE);

    boolval = INI_GetBool(ini, "section", "notexistingval", TRUE);
    test_result(boolval == TRUE);

    boolval = INI_GetBool(ini, "section", "intval", FALSE);
    test_result(boolval == FALSE);

    boolval = INI_GetBool(ini, "section", "intval", TRUE);
    test_result(boolval == TRUE);

    strval = INI_GetValue(ini, "section", "val1", FALSE);
    test_result(strval != NULL);
    test_result(strcmp(strval, "ABCDE abcde") == 0);

	INI_Free(ini);

	return 0;
}
