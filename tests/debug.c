#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

GSM_StateMachine *s;

#ifdef WIN32
# define NUL "NUL"
char debug_filename[] = ".\\gammu-debug-test.log";
#else
# define NUL "/dev/null"
char debug_filename[] = "./gammu-debug-test.log";
#endif


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
	test_result(result == sizeof(cleaner));
	rewind(f);
}

void Log_Function(const char *text, void *data UNUSED)
{
	printf("msg: %s", text);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	FILE *debug_file;
	GSM_Debug_Info *di_sm, *di_global;

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		fail(1);
	}

	/* Get debug handles */
	di_sm = GSM_GetDebug(s);
	di_global = GSM_GetGlobalDebug();

	/*
	 * Test 1 - setting debug level.
	 */
	test_result(GSM_SetDebugLevel("NONSENSE", di_sm) == false);
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	test_result(GSM_SetDebugLevel("textall", di_sm) == true);
	test_result(GSM_SetDebugLevel("textall", di_global) == true);

	/*
	 * Test 2 - global /dev/null, local tempfile, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	check_log(debug_file, true, "2. NULL,TEMP,FALSE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 3 - global /dev/null, local tempfile, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	check_log(debug_file, false, "3. NULL,TEMP,TRUE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 4 - global tempfile, local /dev/null, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	check_log(debug_file, true, "4. TEMP,NULL,TRUE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 5 - global tempfile, local /dev/null, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	check_log(debug_file, false, "5. TEMP,NULL,FALSE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);

	/*
	 * Test 6 - global /dev/null, local tempfile, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	check_log(debug_file, true, "6. NULL,TEMP,FALSE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 7 - global /dev/null, local tempfile, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	check_log(debug_file, false, "7. NULL,TEMP,TRUE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 8 - global tempfile, local /dev/null, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	check_log(debug_file, true, "8. TEMP,NULL,TRUE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/*
	 * Test 9 - global tempfile, local /dev/null, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	check_log(debug_file, false, "9. TEMP,NULL,FALSE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);

	/*
	 * Test 10 - functional logging, do not use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFunction(Log_Function, NULL, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugGlobal(false, di_sm) == true);
	check_log(debug_file, false, "10. TEMP,NULL,FALSE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);

	/*
	 * Test 11 - functional logging, use global
	 */
	debug_file = fopen(debug_filename, "w+");
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	test_result(GSM_SetDebugFile(NUL, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(debug_file, true, di_global) == ERR_NONE);
	test_result(GSM_SetDebugFunction(Log_Function, NULL, di_global) == ERR_NONE);
	test_result(GSM_SetDebugGlobal(true, di_sm) == true);
	check_log(debug_file, false, "11. TEMP,NULL,TRUE");
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_sm) == ERR_NONE);
	test_result(GSM_SetDebugFileDescriptor(NULL, false, di_global) == ERR_NONE);


	/* Free state machine */
	GSM_FreeStateMachine(s);
	fail(0);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
