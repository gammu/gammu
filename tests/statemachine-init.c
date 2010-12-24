#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#ifdef WIN32
# define NUL "NUL"
#else
# define NUL "/dev/null"
#endif

GSM_StateMachine *s;

void single_check(const char *device, const char *connection, const char *model, GSM_Error expected)
{
	GSM_Config *smcfg;
	GSM_Error error;
	GSM_Debug_Info *debug_info;

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);

	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);

	smcfg = GSM_GetConfig(s, 0);
	strcpy(smcfg->Model, model);
	smcfg->Device = strdup(device);
	smcfg->UseGlobalDebugFile = TRUE;
	smcfg->Connection = strdup(connection);
	smcfg->PhoneFeatures[0] = F_PBK_ENCODENUMBER;
	smcfg->PhoneFeatures[1] = 0;
	GSM_SetConfigNum(s, 1);

	error = GSM_InitConnection(s, 1);
	test_result(error == expected);

	/* Free state machine */
	GSM_FreeStateMachine(s);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	single_check("/NONEXISTING/DEVICE/NODE", "NONSENSE", "", ERR_UNKNOWNCONNECTIONTYPESTRING);
#ifdef GSM_ENABLE_AT
	single_check("/NONEXISTING/DEVICE/NODE", "at", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at-nodtr", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at19200-nopower", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at115200", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at", "at", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at", "atobex", ERR_DEVICENOTEXIST);
	single_check(NUL, "at", "", ERR_DEVICEOPENERROR);
	single_check(NUL, "at ", "", ERR_DEVICEOPENERROR);
#endif
#ifdef GSM_ENABLE_FBUS2DLR3
	single_check("/NONEXISTING/DEVICE/NODE", "dlr3", "", ERR_DEVICENOTEXIST);
#ifndef WIN32
	single_check("/dev/null ", "dlr3", "", ERR_DEVICEOPENERROR);
#endif
#endif
#ifdef GSM_ENABLE_DKU5FBUS2
	single_check("/NONEXISTING/DEVICE/NODE", "dku5", "", ERR_DEVICENOTEXIST);
#ifndef WIN32
	single_check("/dev/null ", "dku5", "", ERR_DEVICEOPENERROR);
#endif
#endif

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
