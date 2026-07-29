/* Test that unrelated traffic cannot indefinitely extend reply timeouts. */

#include <gammu.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "../libgammu/gsmstate.h"

#define BUSY_READ_LIMIT 12
#define BUSY_READ_DELAY 150000

static int read_count = 0;

static ssize_t busy_read(GSM_StateMachine *s UNUSED, void *buf, size_t nbytes)
{
	if (read_count >= BUSY_READ_LIMIT || nbytes == 0) {
		return 0;
	}

	usleep(BUSY_READ_DELAY);
	((unsigned char *)buf)[0] = 'x';
	read_count++;
	return 1;
}

static GSM_Error ignore_char(GSM_StateMachine *s UNUSED,
			     unsigned char rx_char UNUSED)
{
	return ERR_NONE;
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Device_Functions device = {NULL, NULL, NULL, NULL, NULL,
				       busy_read, NULL};
	GSM_Protocol_Functions protocol = {NULL, ignore_char, NULL, NULL};
	GSM_Phone_Functions phone;
	GSM_StateMachine *s;
	GSM_Error error;

	memset(&phone, 0, sizeof(phone));

	s = GSM_AllocStateMachine();
	test_result(s != NULL);

	s->opened = TRUE;
	s->Device.Functions = &device;
	s->Protocol.Functions = &protocol;
	s->Phone.Functions = &phone;
	s->Phone.Data.RequestID = ID_GetSignalQuality;

	error = GSM_WaitForOnce(s, NULL, 0, 0, 1);

	gammu_test_result_code(error, "Wait timeout", ERR_TIMEOUT);
	test_result(read_count < BUSY_READ_LIMIT);

	GSM_FreeStateMachine(s);
	return 0;
}
