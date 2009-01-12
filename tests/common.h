#ifndef _test_common_h_
#define _test_common_h_

/**
 * Common functions for test.
 */

#define test_result(val) \
{ \
	if (!(val)) {\
        fprintf(stderr, "Test \"%s\" failed!\n", ""#val); \
        exit(2); \
    } \
}

#define gammu_test_result(error, text) \
{ \
	if (error != ERR_NONE) {\
		fprintf(stderr, "%s\n", GSM_ErrorString(error)); \
        fprintf(stderr, "Test \"%s\" failed!\n", text); \
        exit(2); \
    } \
}

#endif
