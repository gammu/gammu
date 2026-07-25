#include <gammu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libgammu/misc/array.h"

extern char *SMSD_RunOnCommand(const char *command, const GSM_StringArray *arguments);

static int check_command(const char *actual, const char *expected)
{
	if (strcmp(actual, expected) == 0) {
		return 0;
	}

	fprintf(stderr, "Command mismatch:\nexpected: %s\nactual: %s\n", expected, actual);
	return 1;
}

int main(void)
{
	GSM_StringArray arguments;
	char *command;
	int result = 0;

	GSM_StringArray_New(&arguments);

	command = SMSD_RunOnCommand("command", &arguments);
	result = check_command(command, "command");
	free(command);
	if (result != 0) {
		return result;
	}

	if (!GSM_StringArray_Add(&arguments, "plain") ||
	    !GSM_StringArray_Add(&arguments, "space value") ||
	    !GSM_StringArray_Add(&arguments, "trail\\")) {
		fprintf(stderr, "Failed to allocate test arguments\n");
		GSM_StringArray_Free(&arguments);
		return 1;
	}

	command = SMSD_RunOnCommand("command", &arguments);
#ifdef WIN32
	result = check_command(command, "command \"plain\" \"space value\" \"trail\\\\\"");
#else
	result = check_command(command, "command \"$@\"");
#endif
	free(command);
	GSM_StringArray_Free(&arguments);

#ifdef WIN32
	{
		const char *unsafe_arguments[] = {
			"x\" & command",
			"%",
			"!",
			"^",
			"&",
			"|",
			"<",
			">",
			"(",
			")",
			"\n",
			"\r",
			"\t",
			"\x7f",
		};
		size_t i;

		for (i = 0; i < sizeof(unsafe_arguments) / sizeof(unsafe_arguments[0]); i++) {
			GSM_StringArray_New(&arguments);
			if (!GSM_StringArray_Add(&arguments, unsafe_arguments[i])) {
				fprintf(stderr, "Failed to allocate unsafe test argument\n");
				GSM_StringArray_Free(&arguments);
				return 1;
			}
			command = SMSD_RunOnCommand("cmd.exe /c hook.bat", &arguments);
			GSM_StringArray_Free(&arguments);
			if (command != NULL) {
				fprintf(stderr, "Unsafe Windows argument was accepted: %s\n", unsafe_arguments[i]);
				free(command);
				return 1;
			}
		}
	}
#endif

	return result;
}
