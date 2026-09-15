/* Windows system errors must retain their code and produce UTF-8 diagnostics. */
#include <gammu.h>

#include <windows.h>
#include <string.h>

#include "../libgammu/gsmstate.h"
#include "common.h"

static char output[8192];

static void capture(const char *text, void *data UNUSED)
{
	test_result(strlen(output) + strlen(text) < sizeof(output));
	strcat(output, text);
	/* Logging callbacks can change the thread's last error. */
	SetLastError(ERROR_ACCESS_DENIED);
}

int main(void)
{
	GSM_StateMachine *s = GSM_AllocStateMachine();
	GSM_Debug_Info *di;

	test_result(s != NULL);
	di = GSM_GetDebug(s);
	test_result(GSM_SetDebugGlobal(FALSE, di));
	test_result(GSM_SetDebugLevel("textall", di));
	test_result(GSM_SetDebugFunction(capture, NULL, di) == ERR_NONE);

	SetLastError(ERROR_INVALID_PARAMETER);
	GSM_OSErrorInfo(s, "serial setup");
	test_result(GetLastError() == ERROR_INVALID_PARAMETER);
	test_result(strstr(output, "serial setup, 87, \"") != NULL);
	test_result(strstr(output, "message unavailable") == NULL);
	test_result(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		output, -1, NULL, 0) > 0);
	test_result(strchr(output, '\r') == NULL);
	test_result(strchr(output, '\n') == output + strlen(output) - 1);
	test_result(strstr(output, " \"]") == NULL);

	output[0] = '\0';
	SetLastError(0xffffffffUL);
	GSM_OSErrorInfo(s, "unknown error");
	test_result(GetLastError() == 0xffffffffUL);
	test_result(strstr(output, "unknown error, 4294967295, \"System error message unavailable\"") != NULL);

	output[0] = '\0';
	SetLastError(ERROR_SUCCESS);
	GSM_OSErrorInfo(s, "no error");
	test_result(output[0] == '\0');
	test_result(GetLastError() == ERROR_SUCCESS);

	test_result(GSM_SetDebugLevel("nothing", di));
	SetLastError(ERROR_INVALID_PARAMETER);
	GSM_OSErrorInfo(s, "disabled logging");
	test_result(output[0] == '\0');
	test_result(GetLastError() == ERROR_INVALID_PARAMETER);

	GSM_FreeStateMachine(s);
	return 0;
}
