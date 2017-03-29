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
#define COVERAGE_TEMPLATE "c:\\projects\\gammu\\coberture%s.xml"
#define ARG_SEPARATOR "--separator"
#define ARG_LOG "--log-file="
#define ARG_LOG_LEN 11

int main(int argc, char *argv[])
{
    int i;
    int separator = 0;
    char *logfile = NULL, *testnum, *tmp, *command;
    char logname[1000];
    FILE *handle;
#ifdef WIN32
	BOOL result;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
    DWORD exitcode;
#endif

    // Parse params passed by CTest
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], ARG_LOG, ARG_LOG_LEN) == 0) {
            logfile = argv[i] + ARG_LOG_LEN;
        } else if (strcmp(argv[i], ARG_SEPARATOR) == 0) {
            separator = i + 1;
            break;
        }
    }

    if (logfile == NULL) {
        printf("MISSING PARAMETER: " ARG_LOG "\n");
        return 1;
    }

    if (separator == 0) {
        printf("MISSING PARAMETER: " ARG_SEPARATOR "\n");
        return 1;
    }

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
        "OpenCppCoverage.exe --quiet --export_type cobertura:%s --modules %s --sources %s -- ",
        logname,
        ROOT_DIR,
        ROOT_DIR
    );

    for (i = separator; i < argc; i++) {
        strcat(command, argv[i]);
        strcat(command, " ");
    }
    #ifdef WIN32
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
	free(command);
	if (! result) {
        printf("FAILED TO EXECUTE!: 0x%x\n", (int)GetLastError());
        return 1;
    }
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


/*
$outArgs = @()
$afterSeparator = $false
$logFile = ""

# Parse params passed by CTest
for ( $i = 0; $i -lt $args.count; $i++ ) {
    if ($args[$i] -is [int]) {
        continue
    }
    if ($args[$i] -eq "--separator") {
        $afterSeparator = $true
        continue
    }
    if ($afterSeparator) {
        $outArgs += $args[ $i ]
    }
    if ($args[$i].StartsWith("--log-file=")) {
        $logFile = $args[$i].Substring(11)
    }
}

if ($logFile -eq "") {
    throw "Missing --logfile parameter!"
}

# Create empty file (CTest expects to find it)
$logFileObj = New-Item $logFile -type file

# Figure out test number (it's included in log name)
$testNum = [io.path]::GetFileNameWithoutExtension($logFile).split('.')[1]

# Storage for coverage log
$coverageFile = [System.String]::Format($coverageTemplate, $testNum)

# Coverage command
$args = @(
    '--quiet',
    '--export_type', [System.String]::Format('cobertura:{0}', $coverageFile),
    '--modules', $rootDir,
    '--sources', $rootDir,
    '--'
) + $outArgs

# Execute with code coverage
Start-Process 'OpenCppCoverage.exe' -ArgumentList $args -Wait

# Propagate error code
exit $LASTEXITCODE

*/
