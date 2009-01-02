#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
	result = fwrite(cleaner, 1, sizeof(cleaner), f);
	assert(result == sizeof(cleaner));
	rewind(f);
}

void Log_Function(const char *text, void *data UNUSED)
{
	printf("msg: %s", text);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	FILE *debug_file;
#ifndef WIN32
	int debug_fd;
#endif
	GSM_Debug_Info *di_sm, *di_global;

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		fail(1);
	}

#ifndef WIN32
	/* Create file for logs */
	debug_fd = mkstemp(debug_filename);
	if (debug_fd == -1) {
		printf("Could not create temporary file!\n");
		fail(2);
	}
	close(debug_fd);
#endif

	/* Get debug handles */
	di_sm = GSM_GetDebug(s);
	di_global = GSM_GetGlobalDebug();

	/*
	 * Test 1 - setting debug level.
	 */
	assert(GSM_SetDebugLevel("NONSENSE", di_sm) == false);
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	assert(GSM_SetDebugLevel("textall", di_sm) == true);
	assert(GSM_SetDebugLevel("textall", di_global) == true);

	/*
	 * Test 2 - global /dev/null, local tempfile, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	check_log(debug_file, true, "NULL,TEMP,FALSE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 3 - global /dev/null, local tempfile, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	check_log(debug_file, false, "NULL,TEMP,TRUE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 4 - global tempfile, local /dev/null, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	check_log(debug_file, true, "TEMP,NULL,TRUE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 5 - global tempfile, local /dev/null, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	check_log(debug_file, false, "TEMP,NULL,FALSE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);

	/*
	 * Test 6 - global /dev/null, local tempfile, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	check_log(debug_file, true, "2:NULL,TEMP,FALSE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 7 - global /dev/null, local tempfile, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	check_log(debug_file, false, "2:NULL,TEMP,TRUE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 8 - global tempfile, local /dev/null, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	check_log(debug_file, true, "2:TEMP,NULL,TRUE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 9 - global tempfile, local /dev/null, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	check_log(debug_file, false, "2:TEMP,NULL,FALSE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);

	/*
	 * Test 10 - functional logging, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	assert(GSM_SetDebugFunction(Log_Function, NULL, di_sm) == ERR_NONE);
	assert(GSM_SetDebugGlobal(false, di_sm) == true);
	check_log(debug_file, false, "10:TEMP,NULL,FALSE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);

	/*
	 * Test 11 - functional logging, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	assert(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	assert(GSM_SetDebugFunction(Log_Function, NULL, di_global) == ERR_NONE);
	assert(GSM_SetDebugGlobal(true, di_sm) == true);
	check_log(debug_file, false, "11:TEMP,NULL,TRUE");
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	assert(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/* Free state machine */
	GSM_FreeStateMachine(s);
	fail(0);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
