/* Regression test for postal addresses dropped when writing Nokia contacts (#129). */

#include <gammu.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "../libgammu/gsmstate.h"
#include "../libgammu/gsmphones.h"
#include "../libgammu/phone/nokia/nfunc.h"

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_StateMachine *s = GSM_AllocStateMachine();
	GSM_MemoryEntry entry = {0};
	unsigned char buffer[5000] = {0};
	/* Postal block containing the UTF-16BE string "Berlin". */
	const unsigned char expected[] = {
		0x09, 0x00, 0x00, 0x14, 0x01, 0x0e,
		0x00, 'B', 0x00, 'e', 0x00, 'r',
		0x00, 'l', 0x00, 'i', 0x00, 'n', 0x00, 0x00
	};
	size_t blocks, length;

	test_result(s != NULL);
	entry.EntriesNum = 1;
	entry.Entries[0].EntryType = PBK_Text_Postal;
	EncodeUnicode(entry.Entries[0].Text, "Berlin", 6);

	/* The 6310i supports postal addresses. */
	s->Phone.Data.ModelInfo = GetModelData(NULL, NULL, "NPL-1", NULL);
	length = N71_65_EncodePhonebookFrame(s, buffer, &entry, &blocks, TRUE, FALSE);
	test_result(length == sizeof(expected));
	test_result(blocks == 1);
	test_result(memcmp(buffer, expected, sizeof(expected)) == 0);
	gammu_test_result(entry.Entries[0].AddError, "Encode postal address");

	/* The 2600 has F_PBKNOPOSTAL and must omit the unsupported field. */
	s->Phone.Data.ModelInfo = GetModelData(NULL, NULL, "RM-340", NULL);
	length = N71_65_EncodePhonebookFrame(s, buffer, &entry, &blocks, TRUE, FALSE);
	test_result(length == 0);
	test_result(blocks == 0);
	gammu_test_result_code(entry.Entries[0].AddError,
		"Reject unsupported postal address", ERR_NOTSUPPORTED);

	GSM_FreeStateMachine(s);
	return 0;
}
