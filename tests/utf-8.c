/**
 * Simple test case for UTF-8 conversions.
 */

#include "common.h"
#include <gammu.h>
#include <gammu-unicode.h>

int main(int argc UNUSED, char **argv UNUSED)
{
    unsigned char out[20];

    test_result(EncodeWithUTF8Alphabet(0x24, out) == 1);
    test_result(out[0] == 0x24);

    test_result(EncodeWithUTF8Alphabet(0xa2, out) == 2);
    test_result(out[0] == 0xc2);
    test_result(out[1] == 0xa2);

    test_result(EncodeWithUTF8Alphabet(0x20ac, out) == 3);
    test_result(out[0] == 0xe2);
    test_result(out[1] == 0x82);
    test_result(out[2] == 0xac);

    test_result(EncodeWithUTF8Alphabet(0x10348, out) == 4);
    test_result(out[0] == 0xf0);
    test_result(out[1] == 0x90);
    test_result(out[2] == 0x8d);
    test_result(out[3] == 0x88);

    test_result(EncodeWithUTF8Alphabet(0x1F44D, out) == 4);
    test_result(out[0] == 0xf0);
    test_result(out[1] == 0x9f);
    test_result(out[2] == 0x91);
    test_result(out[3] == 0x8d);

    test_result(EncodeUTF8(out, "\xD8\x3d\xDC\x4d\x00\x00"));
    test_result(out[0] == 0xf0);
    test_result(out[1] == 0x9f);
    test_result(out[2] == 0x91);
    test_result(out[3] == 0x8d);
    test_result(out[4] == 0x00);

	return 0;
}

