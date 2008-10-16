#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>

GSM_StateMachine *s;

#ifdef WIN32
# define NUL "NUL"
#else
# define NUL "/dev/null"
#endif

char debug_filename[] = "gammu-debug-test-XXXXXX";

NORETURN void fail(int errcode)
{
	unlink(debug_filename);
	exit(errcode);
}

void check_log(FILE *f, bool match, const char *test_name)
{
	char buff[100];
	char test_message[] = "T3ST M3S5AG3";
	char cleaner[] = "XXXXXXXXXXXXXXXXX";
	int result;

	rewind(f);
	GSM_LogError(s, test_message, ERR_MOREMEMORY);
	rewind(f);
	result = fread(buff, 1, sizeof(test_message), f);
	if (match && result != sizeof(test_message)) {
		printf("%s: Read failed (%d)!\n", test_name, result);
		fail(10);
	}
	if (!match && result != sizeof(test_message)) {
		goto done;
	}
	result = strncmp(test_message, buff, sizeof(test_message) - 1);
	if (match && result != 0) {
		printf("%s: Match failed!\n", test_name);
		fail(11);
	}
	if (!match && result == 0) {
		printf("%s: Matchech but should not!\n", test_name);
		fail(12);
	}
done:
	rewind(f);
	fwrite(cleaner, 1, sizeof(cleaner), f);
	rewind(f);
}


int main(int argc UNUSED, char **argv UNUSED)
{
	FILE *debug_file;
	int debug_fd;
	GSM_Debug_Info *di_sm, *di_global;

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		fail(1);
	}

	/* Create file for logs */
	debug_fd = mkstemp(debug_filename);
	if (debug_fd == -1) {
		printf("Could not create temporary file!\n");
		fail(2);
	}
	debug_file = fdopen(debug_fd, "w+");

	/* Get debug handles */
	di_sm = GSM_GetDebug(s);
	di_global = GSM_GetGlobalDebug();

	/*
	 * Test 1 - setting debug level.
	 */
	if (GSM_SetDebugLevel("NONSENSE", di_sm)) {
		printf("Failed to not set debug level!\n");
		fail(3);
	}
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (!GSM_SetDebugLevel("textall", di_sm)) {
		printf("Failed to set debug level!\n");
		fail(4);
	}
	if (!GSM_SetDebugLevel("textall", di_global)) {
		printf("Failed to set debug level!\n");
		fail(5);
	}

	/*
	 * Test 2 - global /dev/null, local tempfile, do not use global
	 */

	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFile(NUL, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(debug_file, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}
	check_log(debug_file, true, "NULL,TEMP,FALSE");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}


	/*
	 * Test 3 - global /dev/null, local tempfile, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFile(NUL, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(debug_file, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}
	if (!GSM_SetDebugGlobal(true, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	check_log(debug_file, false, "NULL,TEMP,TRUE");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}


	/*
	 * Test 4 - global tempfile, local /dev/null, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFile(NUL, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(debug_file, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}
	if (!GSM_SetDebugGlobal(true, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	check_log(debug_file, true, "TEMP,NULL,TRUE");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}


	/*
	 * Test 5 - global tempfile, local /dev/null, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFile(NUL, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(debug_file, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	check_log(debug_file, false, "TEMP,NULL,FALSE");
	if (!GSM_SetDebugGlobal(false, di_sm)) {
		printf("Failed to set global flag!\n");
		fail(6);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_sm) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(7);
	}
	if (GSM_SetDebugFileDescriptor(NULL, di_global) != ERR_NONE) {
		printf("Failed to set debug file!\n");
		fail(8);
	}


	/* Free state machine */
	GSM_FreeStateMachine(s);
	fail(0);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
