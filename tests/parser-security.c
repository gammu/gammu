#include <gammu.h>
#include <stdio.h>
#include <string.h>

#include "../libgammu/service/gsmlogo.h"
#include "common.h"

#ifdef GSM_ENABLE_ATGEN
#include "../libgammu/gsmstate.h"
#include "../libgammu/misc/misc.h"
#include "../libgammu/protocol/protocol.h"

extern GSM_Error SIEMENS_ReplyGetBitmap(GSM_Protocol_Message *msg, GSM_StateMachine *s);
#endif

static void check_rttl(const char *data, size_t length, GSM_Error expected)
{
	const char path[] = "rttl-security-test.rttl";
	GSM_Ringtone ringtone = {0};
	FILE *file = fopen(path, "wb");

	test_result(file != NULL);
	test_result(fwrite(data, 1, length, file) == length);
	test_result(fclose(file) == 0);
	test_result(GSM_ReadRingtoneFile((char *)path, &ringtone) == expected);
	test_result(remove(path) == 0);
}

static void check_bmp(const unsigned char *data, size_t length,
		      GSM_Error expected, int from_file)
{
	GSM_Bitmap bitmap = {0};
	FILE *file = NULL;
	GSM_Error result;

	if (from_file) {
		file = tmpfile();
		test_result(file != NULL);
		test_result(fwrite(data, 1, length, file) == length);
		rewind(file);
		result = BMP2Bitmap(NULL, 0, file, &bitmap);
		test_result(fclose(file) == 0);
	} else {
		result = BMP2Bitmap(data, length, NULL, &bitmap);
	}
	test_result(result == expected);
}

#ifdef GSM_ENABLE_ATGEN
static void check_siemens_bmp(const unsigned char *data, size_t length,
			      GSM_Error expected, int invalid_hex)
{
	static const char hex[] = "0123456789ABCDEF";
	GSM_StateMachine *state = GSM_AllocStateMachine();
	GSM_Protocol_Message message;
	GSM_Bitmap bitmap = {0};
	char *response;
	size_t i;

	test_result(state != NULL);
	response = malloc(length * 2 + 11);
	test_result(response != NULL);
	memcpy(response, "x\r\nbmp\r\n", 8);
	for (i = 0; i < length; i++) {
		response[8 + 2 * i] = hex[data[i] >> 4];
		response[9 + 2 * i] = hex[data[i] & 15];
	}
	if (invalid_hex) response[8] = 'Z';
	memcpy(response + 8 + length * 2, "\r\n", 2);
	response[10 + length * 2] = 0;
	message.Buffer = (unsigned char *)response;
	message.Length = 10 + length * 2;
	message.Type = 0;
	bitmap.Type = GSM_OperatorLogo;
	state->Phone.Data.Bitmap = &bitmap;
	SplitLines(message.Buffer, message.Length,
		   &state->Phone.Data.Priv.ATGEN.Lines, "\r\n", 2, "\"", 1, TRUE);
	test_result(SIEMENS_ReplyGetBitmap(&message, state) == expected);
	FreeLines(&state->Phone.Data.Priv.ATGEN.Lines);
	GSM_FreeStateMachine(state);
	free(response);
}
#endif

int main(void)
{
	char ringtone[2000];
	unsigned char bmp[118] = {0};
#ifdef GSM_ENABLE_ATGEN
	unsigned char long_frame[512] = {0};
#endif

	memset(ringtone, 'A', sizeof(ringtone));
	memcpy(ringtone + 30, ":d=4,o=5,b=63:c", 16);
	check_rttl(ringtone, 46, ERR_NONE);
	ringtone[30] = 'A';
	ringtone[31] = ':';
	check_rttl(ringtone, 47, ERR_FILENOTSUPPORTED);
	memset(ringtone, 'A', sizeof(ringtone));
	ringtone[1999] = ':';
	check_rttl(ringtone, sizeof(ringtone), ERR_FILENOTSUPPORTED);
	ringtone[1999] = 'A';
	check_rttl(ringtone, sizeof(ringtone), ERR_FILENOTSUPPORTED);

	bmp[0] = 'B';
	bmp[1] = 'M';
	bmp[10] = 62;
	bmp[18] = 1;
	bmp[22] = 1;
	bmp[28] = 1;
	bmp[58] = bmp[59] = bmp[60] = 255;
	check_bmp(bmp, 66, ERR_NONE, 0);
	check_bmp(bmp, 66, ERR_NONE, 1);
#ifdef GSM_ENABLE_ATGEN
	check_siemens_bmp(bmp, 66, ERR_NONE, 0);
	check_siemens_bmp(bmp, 65, ERR_UNKNOWN, 0);
	check_siemens_bmp(bmp, 66, ERR_UNKNOWNRESPONSE, 1);
#endif
	check_bmp(bmp, 53, ERR_FILENOTSUPPORTED, 0);
	check_bmp(bmp, 61, ERR_FILENOTSUPPORTED, 0);
	check_bmp(bmp, 65, ERR_FILENOTSUPPORTED, 0);
	check_bmp(bmp, 65, ERR_FILENOTSUPPORTED, 1);
	bmp[10] = 53;
	check_bmp(bmp, sizeof(bmp), ERR_FILENOTSUPPORTED, 0);
	check_bmp(bmp, sizeof(bmp), ERR_FILENOTSUPPORTED, 1);
	bmp[10] = 61;
	check_bmp(bmp, sizeof(bmp), ERR_FILENOTSUPPORTED, 0);
	bmp[10] = 115;
	check_bmp(bmp, sizeof(bmp), ERR_FILENOTSUPPORTED, 0);
	check_bmp(bmp, sizeof(bmp), ERR_FILENOTSUPPORTED, 1);
#ifdef GSM_ENABLE_ATGEN
	check_siemens_bmp(bmp, sizeof(bmp), ERR_UNKNOWN, 0);
#endif
	bmp[10] = 114;
	check_bmp(bmp, 118, ERR_NONE, 0);
	check_bmp(bmp, 118, ERR_NONE, 1);
	check_bmp(bmp, 117, ERR_FILENOTSUPPORTED, 0);
#ifdef GSM_ENABLE_ATGEN
	check_siemens_bmp(long_frame, sizeof(long_frame), ERR_UNKNOWNRESPONSE, 0);
#endif
	return 0;
}
