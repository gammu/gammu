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

#endif
