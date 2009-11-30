/* Sample code to dump all error codes and their descriptions */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	GSM_Error error;
	gboolean numeric = FALSE, manpage = FALSE;
	int rc = 0;
	const char *errorstring;
	const char *errorname;

	if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
		numeric = TRUE;
	}

	if (argc >= 2 && strcmp(argv[1], "-m") == 0) {
		manpage = TRUE;
	}

	for (error = ERR_NONE; error < ERR_LAST_VALUE; error++) {
		errorstring = GSM_ErrorString(error);
		errorname = GSM_ErrorName(error);
		if (strcmp("Unknown error description.", errorstring) == 0) {
			fprintf(stderr, "Unknown error message for %d!\n", error);
			rc = 1;
		}
		if (errorname == NULL) {
			fprintf(stderr, "Unknown error name for %d!\n", error);
			rc = 1;
		}
		if (numeric) {
			printf("%d. %s - %s\n", error, errorname, errorstring);
		} else if (manpage) {
			printf(".SS %d\n%s\n", 100 + error, errorstring);
		} else {
			printf("# %s - %s\n", errorname, errorstring);
		}
	}

	return rc;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

