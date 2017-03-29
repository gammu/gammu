/*
 * Wrapper for CTest to execute every test through OpenCPPCoverage.
 *
 * Unfortunately there doesn't seem to be way to hook into the process than
 * pretending to be valgrind memory tester.
 *
 * Usage:
 *
 * cmake \
 *   -DMEMORYCHECK_COMMAND=coveragehelper \
 *   -DMEMORYCHECK_COMMAND_OPTIONS=--separator \
 *   -DMEMORYCHECK_TYPE=Valgrind
 * ctest -D NightlyMemCheck
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif

#define ROOT_DIR "c:\\projects\\gammu"
#define COVERAGE_TEMPLATE "c:\\projects\\gammu\\cobertura%s.xml"
#define ARG_SEPARATOR "--separator"
#define ARG_LOG "--log-file="
#define ARG_LOG_LEN 11

int main(int argc, char *argv[])
{
    int i;
    char *logfile = NULL, *separator=NULL, *testnum, *tmp, *command, *commandline;
    char logname[1000];
    FILE *handle;
#ifdef WIN32
	BOOL result;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
    DWORD exitcode;

    commandline = GetCommandLine();
#else
    commandline = malloc(32768);
    *commandline = '\0';
#endif

    // Parse params passed by CTest
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], ARG_LOG, ARG_LOG_LEN) == 0) {
            logfile = argv[i] + ARG_LOG_LEN;
        }
#ifndef WIN32
        // This does not do proper escaping, but the code
        // does not execute on non WIN32, it's for testing only
        strcat(commandline, argv[i]);
        strcat(commandline, " ");
#endif
    }

    if (logfile == NULL) {
        printf("MISSING PARAMETER: " ARG_LOG "\n");
        return 1;
    }

    separator = strstr(commandline, ARG_SEPARATOR);

    if (separator == NULL) {
        printf("MISSING PARAMETER: " ARG_SEPARATOR "\n");
        return 1;
    }
    separator += strlen(ARG_SEPARATOR);

    // Create empty file (CTest expects to find it)
    handle = fopen(logfile, "w");
    if (handle == NULL) {
        printf("FAILED TO CREATE LOG: %s\n", logfile);
        return 1;
    }
    fclose(handle);

    // Figure out test number (it's included in log name)
    tmp = strrchr(logfile, '/');
    if (tmp == NULL) {
        tmp = strrchr(logfile, '\\');
    }
    if (tmp == NULL) {
        goto fail_number;
    }
    testnum = strchr(logfile, '.');
    if (testnum == NULL) {
        goto fail_number;
    }
    testnum += 1;
    tmp = strchr(testnum, '.');
    if (tmp == NULL) {
        goto fail_number;
    }
    *tmp = '\0';

    // Calculate log name
    sprintf(logname, COVERAGE_TEMPLATE, testnum);

    // Prepare command line
    command = malloc(32768);
    if (command == NULL) {
        printf("FAILED TO ALLOCATE!\n");
        return 1;
    }
    sprintf(
        command,
        "OpenCppCoverage.exe --quiet --export_type cobertura:%s --modules %s --sources %s -- %s",
        logname,
        ROOT_DIR,
        ROOT_DIR,
        separator
    );

#ifdef WIN32
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	result = CreateProcess(NULL,     /* No module name (use command line) */
			command,    	/* Command line */
			NULL,           /* Process handle not inheritable*/
			NULL,           /* Thread handle not inheritable*/
			TRUE,           /* Set handle inheritance to TRUE*/
			0,              /* No creation flags*/
			NULL,           /* Use parent's environment block*/
			NULL,           /* Use parent's starting directory */
			&si,            /* Pointer to STARTUPINFO structure*/
			&pi );           /* Pointer to PROCESS_INFORMATION structure*/
	if (! result) {
        printf("FAILED TO EXECUTE: \"%s\" error=0x%x\n", command, (int)GetLastError());
        return 1;
    }
	free(command);
    // Wait for process
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Get the exit code.
    result = GetExitCodeProcess(pi.hProcess, &exitcode);

    // Close the handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

    if (! result) {
        printf("FAILED TO GET RESULT!\n");
        return 1;
    }
    return exitcode;
#else
    printf("CMD: %s\n", command);
    return 0;
#endif

fail_number:
    printf("FAILED TO PARSE TEST NUMBER: %s\n", logfile);
    return 1;
}
