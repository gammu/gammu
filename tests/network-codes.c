/* Sample code to test network codes decoding */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../libgammu/gsmphones.h"


int single_test(const char *string, const char* expected)
{
	const char *ret;
	ret = GSM_GetNetworkName(string);
	if (strcmp(DecodeUnicodeConsole(ret), expected) != 0) {
		printf("Result %s did not match %s\n", DecodeUnicodeConsole(ret), expected);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int rc = 0;

	rc |= single_test("247 01", "LMT LV");
	rc |= single_test("24701", "LMT LV");
	rc |= single_test("99999", "NasraTel");
	rc |= single_test("00000", "unknown");

	return rc;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

