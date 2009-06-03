/* Sample code to dump all feature codes and their descriptions */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../libgammu/gsmphones.h"


int single_test(const char *string, GSM_Error expected)
{
	GSM_Feature features[GSM_MAX_PHONE_FEATURES + 1];
	GSM_Error real;

	real = GSM_SetFeatureString(features, string);

	if (real != expected) {
		printf("Failed parsing of %s (got %s, expected %s)\n",
				string,
				GSM_FeatureToString(real),
				GSM_FeatureToString(expected));
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int rc = 0;
	GSM_Debug_Info *debug_info;

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	rc |= single_test("CAL33, CAL52,SQWE", ERR_NONE);
	rc |= single_test("CAL33, FOO,SQWE", ERR_BADFEATURE);
	rc |= single_test("CAL33, NO_ATOBEX ,SQWE", ERR_NONE);
	rc |= single_test("CAL33, NO_ATOBEX ,SQWE, CAL33, NO_ATOBEX ,SQWE, CAL52,CAL33, NO_ATOBEX ,SQWE, CAL33, NO_ATOBEX ,SQWE, CAL52,CAL33, NO_ATOBEX ,SQWE, CAL33, NO_ATOBEX ,SQWE, CAL52", ERR_MOREMEMORY);

	return rc;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

