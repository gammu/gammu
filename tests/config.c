/**
 * Config file parsing tests.
 */


#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

int main(int argc, char **argv)
{
    GSM_Error error;
    GSM_Config cfg  = {"", "", NULL, NULL, FALSE, FALSE, NULL, FALSE, FALSE, "", "", "", "", "", {0}};
    INI_Section *ini = NULL;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: config config_file\n");
		return 1;
	}

	error = GSM_FindGammuRC(&ini, argv[1]);
	gammu_test_result(error, "GSM_FindGammuRC");

	error = GSM_ReadConfig(ini, &cfg, 0);
	gammu_test_result(error, "GSM_ReadConfig");

	/* Free config file structures */
	INI_Free(ini);

    printf("DEBUG_LEVEL: '%s'\n", cfg.DebugLevel);

    free(cfg.Device);
    free(cfg.Connection);
    free(cfg.DebugFile);

    return 0;
}
