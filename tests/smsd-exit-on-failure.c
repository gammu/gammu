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

	/* Create new config */
	config = SMSD_NewConfig("test");
	if (config == NULL) {
		fprintf(stderr, "Failed to allocate config\n");
		return 1;
	}

	/*
	 * Test the SMSD_SetExitOnFailure function.
	 * We can't easily verify the actual behavior change in a unit test
	 * (we'd need to trigger a failure and see if exit() is called),
	 * but we can at least verify the function exists and can be called
	 * without crashing.
	 */

	/* Set to FALSE */
	SMSD_SetExitOnFailure(config, FALSE);

	/* Set back to TRUE */
	SMSD_SetExitOnFailure(config, TRUE);

	/* Set to FALSE again */
	SMSD_SetExitOnFailure(config, FALSE);

	/* Cleanup */
	SMSD_FreeConfig(config);

	printf("SMSD_SetExitOnFailure test passed\n");
	return 0;
}
