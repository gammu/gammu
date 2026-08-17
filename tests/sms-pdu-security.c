#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gammu.h>

#include "../libgammu/service/gsmlogo.h"
#include "../libgammu/service/sms/gsmmulti.h"
#include "common.h"

static size_t decode_hex(const char *hex, unsigned char *buffer, size_t capacity)
{
	size_t i, length = strlen(hex) / 2;
	unsigned int value;

	test_result(strlen(hex) % 2 == 0);
	test_result(length <= capacity);
	for (i = 0; i < length; i++) {
		test_result(sscanf(hex + i * 2, "%2x", &value) == 1);
		buffer[i] = value;
	}
	return length;
}

static GSM_Error decode_pdu(const unsigned char *buffer, size_t length,
			    GSM_SMSMessage *sms)
{
	size_t final_pos = 0;

	return GSM_DecodePDUFrame(NULL, sms, buffer, length, &final_pos, TRUE);
}

static void test_malformed_frames(void)
{
	GSM_SMSMessage sms;
	unsigned char one[] = {0x00};
	unsigned char long_smsc[201];
	unsigned char long_udl[] = {
		0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x00, 0xa1
	};
	unsigned char truncated[] = {
		0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x00, 0x05, 0x41
	};
	unsigned char bad_udh[] = {
		0x00, 0x41, 0x00, 0x00, 0x81, 0x00, 0x04, 0x04,
		0x03, 0x12, 0x05, 0x00
	};
	unsigned char odd_unicode[] = {
		0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x08, 0x01, 0x41
	};
	unsigned char truncated_unicode_udh[] = {
		0x00, 0x41, 0x00, 0x00, 0x81, 0x00, 0x08, 0x0d,
		0x06, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01,
		0x00, 0x41, 0x00, 0x42
	};
	unsigned char truncated_odd_unicode[] = {
		0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x08, 0x04,
		0x00, 0x41, 0xff
	};
	size_t final_pos = 0;

	test_result(decode_pdu(NULL, 0, &sms) == ERR_CORRUPTED);
	test_result(decode_pdu(one, sizeof(one), &sms) == ERR_CORRUPTED);

	memset(long_smsc, 0x11, sizeof(long_smsc));
	long_smsc[0] = 200;
	long_smsc[1] = 0x81;
	test_result(decode_pdu(long_smsc, sizeof(long_smsc), &sms) == ERR_CORRUPTED);
	test_result(decode_pdu(long_udl, sizeof(long_udl), &sms) == ERR_CORRUPTED);
	test_result(decode_pdu(bad_udh, sizeof(bad_udh), &sms) == ERR_CORRUPTED);
	test_result(decode_pdu(odd_unicode, sizeof(odd_unicode), &sms) == ERR_CORRUPTED);

	/* A complete header with a short user-data tail remains decodable. */
	test_result(decode_pdu(truncated, sizeof(truncated), &sms) == ERR_NONE);
	test_result(sms.Length == 1);

	/* An odd-length UDH can precede a complete UCS-2 tail. */
	test_result(decode_pdu(truncated_unicode_udh,
			       sizeof(truncated_unicode_udh), &sms) == ERR_NONE);
	test_result(sms.UDH.Length == 7);
	test_result(sms.Length == 2);
	test_result(strcmp(DecodeUnicodeString(sms.Text), "AB") == 0);

	/* Consume a discarded trailing octet from a truncated UCS-2 PDU. */
	test_result(GSM_DecodePDUFrame(NULL, &sms, truncated_odd_unicode,
				       sizeof(truncated_odd_unicode), &final_pos,
				       TRUE) == ERR_NONE);
	test_result(final_pos == sizeof(truncated_odd_unicode));
	test_result(sms.Length == 1);
	test_result(strcmp(DecodeUnicodeString(sms.Text), "A") == 0);
}

static void test_special_decoders(void)
{
	static const char *pdus[] = {
		"004000810004000000000000000807120500ffff0000",
		"00400081000400000000000000100b0504158a00000003ce010130017f7f"
	};
	static const unsigned char subject_then_ignored[] = {
		0x01, 0x06, 0x00,
		0x96, 'S', 'u', 'b', 'j', 'e', 'c', 't', 0x00,
		0x98, 'I', 'D', 0x00,
		0x97, 'T', 'o', 0x00
	};
	unsigned char buffer[256];
	GSM_MultiSMSMessage multi;
	GSM_MultiPartSMSInfo info;
	GSM_Error error;
	size_t i, length;

	for (i = 0; i < sizeof(pdus) / sizeof(pdus[0]); i++) {
		memset(&multi, 0, sizeof(multi));
		length = decode_hex(pdus[i], buffer, sizeof(buffer));
		error = decode_pdu(buffer, length, &multi.SMS[0]);
		test_result(error == ERR_NONE);
		multi.Number = 1;
		test_result(!GSM_DecodeMultiPartSMS(NULL, &info, &multi, TRUE));
		GSM_FreeMultiPartSMSInfo(&info);
	}

	memset(&multi, 0, sizeof(multi));
	GSM_SetDefaultSMSData(&multi.SMS[0]);
	multi.Number = 1;
	multi.SMS[0].UDH.Type = UDH_MMSIndicatorLong;
	multi.SMS[0].UDH.Length = 12;
	multi.SMS[0].UDH.Text[10] = 1;
	multi.SMS[0].UDH.Text[11] = 1;
	multi.SMS[0].Coding = SMS_Coding_8bit;
	multi.SMS[0].Length = 205;
	multi.SMS[0].Text[0] = 1;
	multi.SMS[0].Text[1] = 0x06;
	multi.SMS[0].Text[2] = 0;
	multi.SMS[0].Text[3] = 0x96;
	memset(multi.SMS[0].Text + 4, 'A', 201);
	test_result(!GSM_DecodeMultiPartSMS(NULL, &info, &multi, TRUE));
	GSM_FreeMultiPartSMSInfo(&info);

	/* Charset-qualified From values retain the complete sender. */
	memset(&multi, 0, sizeof(multi));
	GSM_SetDefaultSMSData(&multi.SMS[0]);
	multi.Number = 1;
	multi.SMS[0].UDH.Type = UDH_MMSIndicatorLong;
	multi.SMS[0].UDH.Length = 12;
	multi.SMS[0].UDH.Text[10] = 1;
	multi.SMS[0].UDH.Text[11] = 1;
	multi.SMS[0].Coding = SMS_Coding_8bit;
	multi.SMS[0].Length = 10;
	memcpy(multi.SMS[0].Text,
	       "\x01\x06\x00\x89\x05\x80\x03\xea" "A\x00", 10);
	test_result(GSM_DecodeMultiPartSMS(NULL, &info, &multi, TRUE));
	test_result(info.EntriesNum == 1);
	test_result(strcmp(info.Entries[0].MMSIndicator->Sender, "A") == 0);
	GSM_FreeMultiPartSMSInfo(&info);

	/* The same encoded-string form also supports an empty sender. */
	multi.SMS[0].Length = 9;
	memcpy(multi.SMS[0].Text,
	       "\x01\x06\x00\x89\x04\x80\x02\xea\x00", 9);
	test_result(GSM_DecodeMultiPartSMS(NULL, &info, &multi, TRUE));
	test_result(info.EntriesNum == 1);
	test_result(info.Entries[0].MMSIndicator->Sender[0] == 0);
	GSM_FreeMultiPartSMSInfo(&info);

	/* Ignored strings following Subject do not erase its value. */
	multi.SMS[0].Length = sizeof(subject_then_ignored);
	memcpy(multi.SMS[0].Text, subject_then_ignored,
	       sizeof(subject_then_ignored));
	test_result(GSM_DecodeMultiPartSMS(NULL, &info, &multi, TRUE));
	test_result(info.EntriesNum == 1);
	test_result(strcmp(info.Entries[0].MMSIndicator->Title, "Subject") == 0);
	GSM_FreeMultiPartSMSInfo(&info);
}

static void test_link_rejects_malformed_udh(void)
{
	GSM_MultiSMSMessage input;
	GSM_MultiSMSMessage *inputs[] = {&input, NULL};
	GSM_MultiSMSMessage *outputs[2] = {NULL, NULL};

	memset(&input, 0, sizeof(input));
	input.Number = 1;
	GSM_SetDefaultSMSData(&input.SMS[0]);
	input.SMS[0].UDH.Type = UDH_UserUDH;
	input.SMS[0].UDH.Length = 2;
	input.SMS[0].UDH.Text[0] = 1;
	input.SMS[0].UDH.Text[1] = 0;
	test_result(GSM_LinkSMS(NULL, inputs, outputs, TRUE) == ERR_CORRUPTED);
}

static void test_checked_bitmap_decode(void)
{
	GSM_Bitmap bitmap;
	unsigned char data[GSM_BITMAP_SIZE] = {0};

	memset(&bitmap, 0, sizeof(bitmap));
	bitmap.BitmapWidth = 78;
	bitmap.BitmapHeight = 21;
	test_result(!PHONE_DecodeBitmapChecked(GSM_Nokia6510OperatorLogo,
		(const char *)data, 78 * 3 - 1, &bitmap));
	test_result(PHONE_DecodeBitmapChecked(GSM_Nokia6510OperatorLogo,
		(const char *)data, 78 * 3, &bitmap));

	bitmap.BitmapWidth = 96;
	bitmap.BitmapHeight = 65;
	test_result(PHONE_GetBitmapSize(GSM_Nokia7110OperatorLogo, 96, 65) == 780);
	test_result(!PHONE_DecodeBitmapChecked(GSM_Nokia7110OperatorLogo,
		(const char *)data, 205, &bitmap));
	test_result(PHONE_DecodeBitmapChecked(GSM_Nokia7110OperatorLogo,
		(const char *)data, 780, &bitmap));
	test_result(PHONE_GetBitmapSize(GSM_Nokia6510OperatorLogo, 96, 65) ==
		GSM_BITMAP_SIZE);
	test_result(!PHONE_DecodeBitmapChecked(GSM_Nokia6510OperatorLogo,
		(const char *)data, 78 * 3, &bitmap));
	test_result(PHONE_DecodeBitmapChecked(GSM_Nokia6510OperatorLogo,
		(const char *)data, sizeof(data), &bitmap));
}

static GSM_SMSMessageLayout test_layout(void)
{
	GSM_SMSMessageLayout layout;

	memset(&layout, 255, sizeof(layout));
	layout.firstbyte = 0;
	layout.TPDCS = 1;
	layout.TPUDL = 2;
	layout.Text = 3;
	return layout;
}

static void test_single_part_encoding(void)
{
	GSM_SMSMessage sms;
	GSM_SMSMessageLayout layout = test_layout();
	unsigned char output[256];
	char text[161];
	int length;
	GSM_Error error;

	memset(text, '^', 80);
	text[80] = 0;
	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_Default_No_Compression;
	EncodeUnicode(sms.Text, text, 80);
	error = GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE);
	gammu_test_result_code(error, "escaped default alphabet encoding", ERR_NONE);

	memset(text, '^', 160);
	text[160] = 0;
	EncodeUnicode(sms.Text, text, 160);
	test_result(GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE) == ERR_INVALIDDATA);

	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_8bit;
	sms.Length = 141;
	test_result(GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE) == ERR_INVALIDDATA);

	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_8bit;
	sms.UDH.Type = UDH_UserUDH;
	sms.UDH.Length = 4;
	sms.UDH.Text[0] = 3;
	sms.UDH.Text[1] = 0x12;
	sms.UDH.Text[2] = 5;
	test_result(GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE) == ERR_INVALIDDATA);

	/* A structurally valid header can occupy all 140 user-data octets. */
	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_Default_No_Compression;
	sms.UDH.Type = UDH_UserUDH;
	sms.UDH.Length = GSM_MAX_UDH_LENGTH;
	sms.UDH.Text[0] = GSM_MAX_UDH_LENGTH - 1;
	sms.UDH.Text[1] = 0x70;
	sms.UDH.Text[2] = GSM_MAX_UDH_LENGTH - 3;
	error = GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE);
	gammu_test_result_code(error, "maximum UDH encoding", ERR_NONE);
	test_result(length == layout.Text + GSM_MAX_UDH_LENGTH);
	test_result(output[layout.TPUDL] == GSM_MAX_SMS_CHARS_LENGTH);

	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_Unicode_No_Compression;
	memset(text, 'a', 71);
	text[71] = 0;
	EncodeUnicode(sms.Text, text, 71);
	test_result(GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE) == ERR_INVALIDDATA);

	/* Four source tildes compact to two UCS-2 code units. */
	memset(text, 'a', 68);
	memcpy(text + 68, "~~~~", 4);
	text[72] = 0;
	EncodeUnicode(sms.Text, text, 72);
	memset(output, 0xa5, sizeof(output));
	error = GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE);
	gammu_test_result_code(error, "compacted Unicode encoding", ERR_NONE);
	test_result(length == layout.Text + GSM_MAX_8BIT_SMS_LENGTH);
	test_result(output[layout.TPUDL] == GSM_MAX_8BIT_SMS_LENGTH);
	test_result(output[layout.Text + 136] == 0x00);
	test_result(output[layout.Text + 137] == 0x7e);
	test_result(output[layout.Text + 138] == 0x00);
	test_result(output[layout.Text + 139] == 0x7e);

	/* Header-only 7-bit messages retain the UDH padding octet. */
	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_Default_No_Compression;
	sms.UDH.Type = UDH_DisableVoice;
	GSM_EncodeUDHHeader(NULL, &sms.UDH);
	memset(output, 0xa5, sizeof(output));
	error = GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE);
	gammu_test_result_code(error, "header-only SMS encoding", ERR_NONE);
	test_result(sms.UDH.Length == 5);
	test_result(output[layout.TPUDL] == 6);
	test_result(length == layout.Text + 6);
	test_result(output[layout.Text + 5] == 0x00);

	/* Compaction does not admit more than 70 encoded code units. */
	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Submit;
	sms.Coding = SMS_Coding_Unicode_No_Compression;
	memset(text, 'a', 69);
	memcpy(text + 69, "~~~~", 4);
	text[73] = 0;
	EncodeUnicode(sms.Text, text, 73);
	test_result(GSM_EncodeSMSFrame(NULL, &sms, output, layout, &length, TRUE) == ERR_INVALIDDATA);
}

static void test_multipart_encoding(void)
{
	GSM_MultiPartSMSInfo info;
	GSM_MultiSMSMessage sms;
	GSM_SMSMessageLayout layout = test_layout();
	unsigned char *unicode;
	unsigned char compacted[146];
	unsigned char output[256];
	char *text;
	char compacted_text[73];
	int frame_length;
	size_t length = 40000;

	memset(compacted_text, 'a', 68);
	memcpy(compacted_text + 68, "~~~~", 4);
	compacted_text[72] = 0;
	EncodeUnicode(compacted, compacted_text, 72);
	GSM_ClearMultiPartSMSInfo(&info);
	info.EntriesNum = 1;
	info.Entries[0].ID = SMS_Text;
	info.Entries[0].Buffer = compacted;
	info.UnicodeCoding = TRUE;
	test_result(GSM_EncodeMultiPartSMS(NULL, &info, &sms) == ERR_NONE);
	test_result(sms.Number == 1);
	test_result(UnicodeLength(sms.SMS[0].Text) == 72);
	sms.SMS[0].PDU = SMS_Submit;
	test_result(GSM_EncodeSMSFrame(NULL, &sms.SMS[0], output, layout,
				       &frame_length, TRUE) == ERR_NONE);
	test_result(output[layout.TPUDL] == GSM_MAX_8BIT_SMS_LENGTH);

	text = malloc(length + 1);
	unicode = malloc(length * 2 + 2);
	test_result(text != NULL && unicode != NULL);
	memset(text, 'A', length);
	text[length] = 0;
	EncodeUnicode(unicode, text, length);

	GSM_ClearMultiPartSMSInfo(&info);
	info.EntriesNum = 1;
	info.Entries[0].ID = SMS_ConcatenatedTextLong;
	info.Entries[0].Buffer = unicode;
	info.UnicodeCoding = FALSE;
	test_result(GSM_EncodeMultiPartSMS(NULL, &info, &sms) == ERR_INVALIDDATA);

	EncodeUnicode(unicode, text, 7500);
	test_result(GSM_EncodeMultiPartSMS(NULL, &info, &sms) == ERR_NONE);
	test_result(sms.Number <= GSM_MAX_MULTI_SMS);

	EncodeUnicode(unicode, text, 8000);
	test_result(GSM_EncodeMultiPartSMS(NULL, &info, &sms) == ERR_INVALIDDATA);
	info.Entries[0].Bold = TRUE;
	test_result(GSM_EncodeMultiPartSMS(NULL, &info, &sms) == ERR_INVALIDDATA);

	free(unicode);
	free(text);
}

static void test_ems_udh_boundary(void)
{
	GSM_MultiSMSMessage sms;
	GSM_SMSMessageLayout layout = test_layout();
	unsigned char formatting[] = {0x0a, 0x03, 0x00, 0x00, 0x00};
	unsigned char output[256];
	char text[155];
	size_t used_text = 0, copied_text = 0, copied_sms_text = 0;
	int frame_length;
	GSM_Error error;

	memset(&sms, 0, sizeof(sms));
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	GSM_SetDefaultSMSData(&sms.SMS[1]);
	sms.SMS[0].Coding = SMS_Coding_Default_No_Compression;
	sms.SMS[1].Coding = SMS_Coding_Default_No_Compression;
	memset(text, 'A', 154);
	text[154] = 0;
	EncodeUnicode(sms.SMS[0].Text, text, 154);
	sms.SMS[0].Length = 154;

	error = GSM_AddSMS_Text_UDH(NULL, &sms,
				    SMS_Coding_Default_No_Compression,
				    (char *)formatting, sizeof(formatting), TRUE,
				    &used_text, &copied_text, &copied_sms_text);
	gammu_test_result_code(error, "exact-fit EMS UDH placement", ERR_NONE);
	test_result(sms.Number == 1);
	test_result(sms.SMS[0].UDH.Length == 0);
	test_result(sms.SMS[1].UDH.Length == 6);

	sms.SMS[0].PDU = SMS_Submit;
	error = GSM_EncodeSMSFrame(NULL, &sms.SMS[0], output, layout,
				   &frame_length, TRUE);
	gammu_test_result_code(error, "full EMS text frame", ERR_NONE);
	sms.SMS[1].PDU = SMS_Submit;
	error = GSM_EncodeSMSFrame(NULL, &sms.SMS[1], output, layout,
				   &frame_length, TRUE);
	gammu_test_result_code(error, "moved EMS UDH frame", ERR_NONE);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	test_malformed_frames();
	test_special_decoders();
	test_link_rejects_malformed_udh();
	test_checked_bitmap_decode();
	test_single_part_encoding();
	test_multipart_encoding();
	test_ems_udh_boundary();
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
