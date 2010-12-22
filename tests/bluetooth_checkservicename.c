/* Test case for bluetooth_checkservicename */
#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libgammu/device/devfunc.h" /* For bluetooth_checkservicename */
#include "../libgammu/gsmstate.h" /* For statemachine internals */
#include "common.h"

GSM_StateMachine *s;

void single_test(GSM_ConnectionType conn, const char *name)
{
	s->ConnectionType = conn;
	if (bluetooth_checkservicename(s, name) == 0) {
		fprintf(stderr, "Test \"%s\" failed!\n", name);
		exit(2);
	}
}

int main(int argc UNUSED, char **argv UNUSED)
{
	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}

	single_test(GCT_BLUEPHONET, "Nokia PC Suite");
	single_test(GCT_BLUEOBEX, "OBEX File Transfer");
	single_test(GCT_BLUEOBEX, "IrMC Sync");
	single_test(GCT_BLUEOBEX, "OBEX");
	single_test(GCT_BLUEAT, "SerialPort1");
	single_test(GCT_BLUEAT, "Dial-up networking Gateway");
	single_test(GCT_BLUEAT, "COM");
	single_test(GCT_BLUEAT, "Serial Server");

	/* Free state machine */
	GSM_FreeStateMachine(s);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
