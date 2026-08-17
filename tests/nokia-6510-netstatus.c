/* Test unsolicited NETSTATUS frames reported by newer Nokia S40 phones. */

#include <gammu.h>
#include <stdio.h>

#include "common.h"
#include "../libgammu/gsmstate.h"
#include "../libgammu/phone/nokia/dct4s40/6510/n6510.h"

static void check_notification(GSM_StateMachine *s, unsigned char *buffer,
			       size_t length)
{
	GSM_Protocol_Message msg;
	GSM_Error error;

	msg.Type = 0x0A;
	msg.Length = length;
	msg.Buffer = buffer;

	s->Phone.Data.RequestID = ID_GetSMSC;
	s->Phone.Data.RequestMsg = &msg;

	error = GSM_DispatchMessage(s);
	gammu_test_result_code(error, "NETSTATUS notification", ERR_NONE);
	test_result(s->Phone.Data.RequestID == ID_GetSMSC);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	/* gammu/gammu#10, Nokia 112 */
	unsigned char subtype_b6[] = {
		0x01, 0x8A, 0xFF, 0xB6, 0x64, 0x52, 0x00, 0x00,
		0x00, 0x02, 0xE8, 0x04, 0x00, 0x01, 0xE9, 0x04,
		0x00, 0x60
	};
	/* gammu/gammu#113, Nokia 3120 Classic */
	unsigned char subtype_2c[] = {
		0x01, 0x83, 0x00, 0x2C, 0x00, 0x01, 0x28, 0x04,
		0x29, 0xA7
	};
	unsigned char subtype_3f[] = {
		0x01, 0x83, 0x00, 0x3F, 0x00, 0x01, 0x3C, 0x08,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	/* gammu/gammu#368, Nokia C1-02 */
	unsigned char subtype_42[] = {
		0x01, 0x80, 0xFF, 0x42, 0x00, 0x01, 0x46, 0x14,
		0x7F, 0x59, 0x00, 0x00, 0x26, 0xEE, 0x00, 0x00,
		0x00, 0x02, 0x62, 0xF2, 0x30, 0x00, 0x00, 0x00,
		0x00, 0x00
	};
	unsigned char subtype_e2[] = {
		0x01, 0x80, 0x00, 0xE2, 0x00, 0x03, 0x00, 0x18,
		0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x45,
		0x00, 0x2D, 0x00, 0x50, 0x00, 0x6C, 0x00, 0x75,
		0x00, 0x73, 0x01, 0x00, 0x01, 0x00, 0x09, 0x18,
		0x7F, 0x59, 0x00, 0x00, 0x26, 0xEE, 0x62, 0xF2,
		0x30, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE3, 0x14,
		0x0A, 0x00, 0x00, 0x07, 0x00, 0x6F, 0x00, 0x32,
		0x00, 0x20, 0x00, 0x2D, 0x00, 0x20, 0x00, 0x64,
		0x00, 0x65
	};
	/* Keep genuinely unknown NETSTATUS subtypes visible. */
	unsigned char unknown[] = {
		0x01, 0x80, 0xFF, 0x99, 0x00, 0x00
	};
	GSM_Protocol_Message msg;
	GSM_StateMachine *s;
	GSM_Error error;
	unsigned char operator_logo_request[1000] = {0};
	size_t operator_logo_length;

	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	s->Phone.Functions = &N6510Phone;

	test_result(N6510_EncodeOperatorLogoFrameLengths(operator_logo_request,
							 234 + 28,
							 234, &operator_logo_length));
	test_result(operator_logo_length == 262);
	test_result(operator_logo_request[19] == 244);
	test_result(operator_logo_length == (size_t)18 + operator_logo_request[19]);
	test_result(operator_logo_request[22] == 0);
	test_result(operator_logo_request[23] == 234);
	test_result(operator_logo_request[24] == 0);
	test_result(operator_logo_request[25] == 234);
	test_result(!N6510_EncodeOperatorLogoFrameLengths(operator_logo_request,
							  sizeof(operator_logo_request),
							  246, &operator_logo_length));

	check_notification(s, subtype_b6, sizeof(subtype_b6));
	check_notification(s, subtype_2c, sizeof(subtype_2c));
	check_notification(s, subtype_3f, sizeof(subtype_3f));
	check_notification(s, subtype_42, sizeof(subtype_42));
	check_notification(s, subtype_e2, sizeof(subtype_e2));

	msg.Type = 0x0A;
	msg.Length = sizeof(unknown);
	msg.Buffer = unknown;
	s->Phone.Data.RequestID = ID_GetSMSC;
	s->Phone.Data.RequestMsg = &msg;
	error = GSM_DispatchMessage(s);
	gammu_test_result_code(error, "Unknown NETSTATUS subtype", ERR_TIMEOUT);
	test_result(s->Phone.Data.RequestID == ID_GetSMSC);

	GSM_FreeStateMachine(s);
	return 0;
}
