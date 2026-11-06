#include <gammu.h>
#include <stdio.h>
#include <string.h>

#include "../libgammu/gsmstate.h"
#include "common.h"

extern GSM_Error ALCATEL_EncodeString(GSM_StateMachine *s,
	const unsigned char *buffer, unsigned char *target,
	GSM_Alcatel_FieldType type);

static void check_text(GSM_StateMachine *state, char ch, size_t length,
		       GSM_Error expected)
{
	unsigned char input[(GSM_PHONEBOOK_TEXT_LENGTH + 1) * 2];
	unsigned char output[201];
	char text[GSM_PHONEBOOK_TEXT_LENGTH + 1];

	memset(text, ch, length);
	text[length] = 0;
	EncodeUnicode(input, text, length);
	memset(output, 0xa5, sizeof(output));
	test_result(ALCATEL_EncodeString(state, input, output + 11,
					  Alcatel_string) == expected);
	test_result(output[200] == 0xa5);
	if (expected == ERR_NONE) {
		test_result(output[11] == (ch == '~' ? length * 2 : length));
	}
}

int main(void)
{
	GSM_StateMachine *state = GSM_AllocStateMachine();
	unsigned char unicode_text[142] = {0};
	unsigned char output[201];
	size_t i;

	test_result(state != NULL);
	check_text(state, '~', 50, ERR_NONE);
	check_text(state, '~', 93, ERR_NONE);
	check_text(state, '~', 94, ERR_INVALIDDATA);
	check_text(state, '~', 200, ERR_INVALIDDATA);
	check_text(state, 'A', 150, ERR_NONE);
	state->Phone.Data.Priv.ALCATEL.ProtocolVersion = V_1_1;
	check_text(state, '~', 200, ERR_INVALIDDATA);
	for (i = 0; i < 70; i++) {
		unicode_text[i * 2] = 0x4e;
		unicode_text[i * 2 + 1] = 0x2d;
	}
	memset(output, 0xa5, sizeof(output));
	test_result(ALCATEL_EncodeString(state, unicode_text, output + 11,
					  Alcatel_string) == ERR_NONE);
	test_result(output[11] == 143);
	test_result(output[200] == 0xa5);
	GSM_FreeStateMachine(state);
	return 0;
}
