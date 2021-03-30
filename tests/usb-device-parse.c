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

GSM_Error GSM_USB_ParseDevice(GSM_StateMachine *s, int *vendor, int *product, int *bus, int *deviceid, char **serial);

int vendor = -1, product = -1, bus = -1, deviceid = -1;
char *serial;
GSM_StateMachine *s;
GSM_Debug_Info *debug_info;
GSM_Config *cfg;

void single_check(const char *string, int vendor_match, int product_match, int bus_match, int deviceid_match, const char *serial_match)
{
	cfg->Device = strdup(string);
	GSM_USB_ParseDevice(s,  &vendor, &product, &bus, &deviceid, &serial);
	test_result(vendor == vendor_match);
	test_result(product == product_match);
	test_result(bus == bus_match);
	test_result(deviceid == deviceid_match);
	test_result((serial == NULL && serial_match == NULL) || strcmp(serial, serial_match) == 0);
	free(cfg->Device);
	cfg->Device = NULL;
}

int main(int argc UNUSED, char **argv UNUSED)
{
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	s = GSM_AllocStateMachine();
	test_result(s != NULL);

	cfg = GSM_GetConfig(s, -1);

	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);

	single_check("0x1234:0x5678", 0x1234, 0x5678, -1, -1, NULL);
	single_check("1234:5678", 1234, 5678, -1, -1, NULL);
	single_check("0x1234:-1", 0x1234, -1, -1, -1, NULL);
	single_check("1.10", -1, -1, 1, 10, NULL);
	single_check("10", -1, -1, -1, 10, NULL);
	single_check("serial:123456", -1, -1, -1, -1, "123456");
	single_check("serial : 123456", -1, -1, -1, -1, "123456");

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
