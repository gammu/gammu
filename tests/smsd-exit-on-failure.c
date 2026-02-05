/**
 * Test for SMSD_SetExitOnFailure function
 */

#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* We do not want to push another header, so we need to copy definiton of UNUSED */
#ifndef UNUSED
# if __GNUC__
#  define UNUSED __attribute__ ((unused))
# else
#  define UNUSED
# endif
#endif

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_SMSDConfig *config;
	GSM_SMSDStatus status;
	GSM_Error error;
	
	/* Create new config */
	config = SMSD_NewConfig("test");
	if (config == NULL) {
		fprintf(stderr, "Failed to allocate config\n");
		return 1;
	}
	
	/* 
	 * Test 1: With exit_on_failure=TRUE (default), we can't easily test
	 * the exit behavior in a unit test, so we skip this case.
	 */
	
	/* 
	 * Test 2: Set exit_on_failure to FALSE and verify it doesn't exit
	 * when calling a function that would normally fail
	 */
	SMSD_SetExitOnFailure(config, FALSE);
	
	/* Try to get status (this will fail because SMSD is not running) */
	/* If exit_on_failure was still TRUE, this would call exit() */
	error = SMSD_GetStatus(config, &status);
	
	/* If we reach here, exit_on_failure must be FALSE (otherwise we'd have exited) */
	if (error == ERR_NONE) {
		fprintf(stderr, "Unexpected success from GetStatus on non-running SMSD\n");
		SMSD_FreeConfig(config);
		return 1;
	}
	
	/* Test 3: Set back to TRUE */
	SMSD_SetExitOnFailure(config, TRUE);
	
	/* Cleanup */
	SMSD_FreeConfig(config);
	
	printf("SMSD_SetExitOnFailure test passed\n");
	return 0;
}
