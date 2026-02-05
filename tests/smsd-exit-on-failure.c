/**
 * Test for SMSD_SetExitOnFailure function
 */

#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* Access internal structure for testing */
#include "../smsd/core.h"

int main(int argc, char **argv)
{
	GSM_SMSDConfig *config;
	
	/* Create new config */
	config = SMSD_NewConfig("test");
	if (config == NULL) {
		fprintf(stderr, "Failed to allocate config\n");
		return 1;
	}
	
	/* Check default value is TRUE */
	assert(config->exit_on_failure == TRUE);
	
	/* Set to FALSE */
	SMSD_SetExitOnFailure(config, FALSE);
	assert(config->exit_on_failure == FALSE);
	
	/* Set back to TRUE */
	SMSD_SetExitOnFailure(config, TRUE);
	assert(config->exit_on_failure == TRUE);
	
	/* Cleanup */
	SMSD_FreeConfig(config);
	
	printf("SMSD_SetExitOnFailure test passed\n");
	return 0;
}
