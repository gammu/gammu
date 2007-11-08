/* Sample code to dump all error codes and their descriptions */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	GSM_Error error;
	bool numeric = false;
	int rc = 0;
	const char *errorstring;

	if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
		numeric = true;
	}

	for (error = ERR_NONE; error < ERR_LAST_VALUE; error++) {
		if (strcmp("Unknown error description.", errorstring) == 0) {
			fprintf(stderr, "Unknown error message for %d!\n", error);
			rc = 1;
		}
		errorstring = GSM_ErrorString(error);
		if (numeric) {
			printf("%d. %s\n", error, errorstring);
		} else {
			printf("# %s\n", errorstring);
		}
	}

	return rc;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

