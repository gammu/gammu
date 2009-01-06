#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>

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
	GSM_SetDebugGlobal(true, debug_info);

	smcfg = GSM_GetConfig(s, 0);
	strcpy(smcfg->Model, model);
	smcfg->Device = strdup(device);
	smcfg->UseGlobalDebugFile = true;
	smcfg->Connection = strdup(connection);
	smcfg->PhoneFeatures[0] = F_PBK_ENCODENUMBER;
	smcfg->PhoneFeatures[1] = 0;
	GSM_SetConfigNum(s, 1);

	error = GSM_InitConnection(s, 3);
	test_result(error == expected);

	/* Free state machine */
	GSM_FreeStateMachine(s);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	single_check("/NONEXISTING/DEVICE/NODE", "NONSENSE", "", ERR_UNKNOWNCONNECTIONTYPESTRING);
	single_check("/NONEXISTING/DEVICE/NODE", "at", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at-nodtr", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at19200-nopower", "", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at", "at", ERR_DEVICENOTEXIST);
	single_check("/NONEXISTING/DEVICE/NODE", "at", "atobex", ERR_DEVICENOTEXIST);
	single_check(NUL, "at", "", ERR_DEVICEREADERROR);
	single_check(NUL, "at ", "", ERR_DEVICEREADERROR);
#ifndef WIN32
	single_check("/dev/null ", "at", "", ERR_DEVICEREADERROR);
#endif

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
