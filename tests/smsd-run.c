#include <gammu.h>
#include <gammu-smsd.h>

#include <stdio.h>
#include <stdlib.h>

#include "../libgammu/misc/array.h"

extern gboolean SMSD_RunOn(const char *command, GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, const GSM_StringArray *arguments, const char *event);

static gboolean add_argument(GSM_StringArray *arguments, const char *value)
{
	if (GSM_StringArray_Add(arguments, value)) {
		return TRUE;
	}

	fprintf(stderr, "Failed to allocate test argument\n");
	return FALSE;
}

static gboolean add_marker_argument(GSM_StringArray *arguments, const char *format, const char *marker)
{
	char value[4096];

	snprintf(value, sizeof(value), format, marker);
	return add_argument(arguments, value);
}

int main(int argc, char **argv)
{
	GSM_SMSDConfig *config;
	GSM_StringArray arguments;
	FILE *file;
	gboolean result;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s HELPER MARKER\n", argv[0]);
		return 1;
	}

	remove(argv[2]);
	setenv("SMSD_RUN_HELPER", argv[1], 1);
	setenv("SMSD_TEST_MARKER", argv[2], 1);
	unsetenv("SMSD_TEST_EMPTY");
	unsetenv("SMSD_TEST_SINGLE");

	config = SMSD_NewConfig("test");
	if (config == NULL) {
		fprintf(stderr, "Failed to allocate SMSD config\n");
		return 1;
	}

	GSM_StringArray_New(&arguments);
	if (!add_argument(&arguments, "normal") ||
	    !add_marker_argument(&arguments, "$(touch %s)", argv[2]) ||
	    !add_marker_argument(&arguments, "`touch %s`", argv[2]) ||
	    !add_marker_argument(&arguments, "; touch %s", argv[2]) ||
	    !add_marker_argument(&arguments, "& touch %s", argv[2]) ||
	    !add_marker_argument(&arguments, "(touch %s)", argv[2]) ||
	    !add_argument(&arguments, "value with spaces\tand a tab") ||
	    !add_argument(&arguments, "line one\nline two") ||
	    !add_argument(&arguments, "$PATH") ||
	    !add_argument(&arguments, "\"'") ||
	    !add_argument(&arguments, "")) {
		GSM_StringArray_Free(&arguments);
		SMSD_FreeConfig(config);
		return 1;
	}

	result = SMSD_RunOn("exec \"$SMSD_RUN_HELPER\"", NULL, config, &arguments, "test");
	GSM_StringArray_Free(&arguments);
	if (!result) {
		fprintf(stderr, "RunOn command failed\n");
		SMSD_FreeConfig(config);
		return 1;
	}

	file = fopen(argv[2], "r");
	if (file != NULL) {
		fclose(file);
		fprintf(stderr, "Injected command created marker file\n");
		remove(argv[2]);
		SMSD_FreeConfig(config);
		return 1;
	}

	GSM_StringArray_New(&arguments);
	setenv("SMSD_TEST_EMPTY", "1", 1);
	result = SMSD_RunOn("exec \"$SMSD_RUN_HELPER\"", NULL, config, &arguments, "test");
	GSM_StringArray_Free(&arguments);
	unsetenv("SMSD_TEST_EMPTY");
	if (!result) {
		fprintf(stderr, "RunOn command without arguments failed\n");
		SMSD_FreeConfig(config);
		return 1;
	}

	GSM_StringArray_New(&arguments);
	setenv("SMSD_TEST_SINGLE", "1", 1);
	if (!add_argument(&arguments, "single argument")) {
		GSM_StringArray_Free(&arguments);
		SMSD_FreeConfig(config);
		return 1;
	}
	result = SMSD_RunOn("exec \"$SMSD_RUN_HELPER\"", NULL, config, &arguments, "test");
	GSM_StringArray_Free(&arguments);
	unsetenv("SMSD_TEST_SINGLE");
	SMSD_FreeConfig(config);

	if (!result) {
		fprintf(stderr, "RunOn command with one argument failed\n");
		return 1;
	}

	return 0;
}
