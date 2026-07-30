/**
 * Simple test case for UTF-8 conversions.
 */

#include "common.h"
#include <gammu.h>
#include <gammu-unicode.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "../libgammu/misc/coding/coding.h"

static gboolean SetUTF8Locale(void)
{
    static const char *locales[] = {
        "",
        "C.UTF-8",
        "C.utf8",
        "en_US.UTF-8",
        ".UTF-8",
    };
    size_t i;
    wchar_t decoded;

    for (i = 0; i < sizeof(locales) / sizeof(locales[0]); i++) {
        if (setlocale(LC_CTYPE, locales[i]) == NULL || MB_CUR_MAX < 2) {
            continue;
        }
        mbtowc(NULL, NULL, 0);
        if (mbtowc(&decoded, "\xc3\xa9", 2) == 2 && decoded == 0xe9) {
            return TRUE;
        }
    }

    return FALSE;
}

int main(int argc UNUSED, char **argv UNUSED)
{
    unsigned char out[200], out2[200];
    gammu_char_t dest;
    size_t i;
    const char input[] = "005400680061006E006B00200079006F0075002E002000570065002000770069006C006C00200063006F006E007400610063007400200079006F007500200073006F006F006E00200078006F0078006F00200078006F0078006F00200061006E0064002000490020D83DDE18D83DDE18D83DDE18D83DDE18D83DDE18D83DDE18D83DDE18D83D";
    const char expected[] = "Thank you. We will contact you soon xoxo xoxo and I 😘😘😘😘😘😘😘�";
    const char invalid_locale[] = {'a', (char)0xc3, 'z', 0};
    const char invalid_utf8[] = {'o', 'k', (char)0xff, 'r', 'e', 's', 't'};
    const char invalid_continuation[] = {(char)0xc3, 'z'};
    const char truncated_utf8[] = {(char)0xc3, (char)0xa9};
    const unsigned char expected_replacement[] = {
        0x00, 0x61, 0xff, 0xfd, 0x00, 0x7a, 0x00, 0x00
    };
    const unsigned char expected_truncated[] = {0xff, 0xfd, 0x00, 0x00};
    const unsigned char expected_utf8_prefix[] = {
        0x00, 0x6f, 0x00, 0x6b, 0x00, 0x00
    };

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

    test_result(DecodeWithUTF8Alphabet("\xf0\x9f\x91\x8d\x00", &dest, 4) == 4);
    test_result(dest == 0x1f44d);

    DecodeUTF8(out, "\xf0\x9f\x91\x8d\x00", 4);

    test_string("\xD8\x3d\xDC\x4d\x00", out, 5);

    DecodeUTF8(out, "ahoj", 4);

    test_string("\x00\x61\x00h\x00o\x00j\x00\x00\x00", out, 10);

    test_result(DecodeUTF8Checked(out, "ahoj", 4));
    test_result(!DecodeUTF8Checked(out, invalid_utf8,
                                   sizeof(invalid_utf8)));
    test_string(expected_utf8_prefix, out, sizeof(expected_utf8_prefix));
    test_result(!DecodeUTF8Checked(out, invalid_continuation,
                                   sizeof(invalid_continuation)));

    /* Decode hex encoded unicode */
    test_result(DecodeHexUnicode(out, input, strlen(input)));
    test_string("\x00T\x00h\x00\x61\x00n\x00k\x00", out, 10);

    /* Convert it to UTF-8 */
    test_result(EncodeUTF8(out2, out));
    test_string(expected, out2, strlen(expected));

    if (SetUTF8Locale()) {
        test_result(!EncodeUnicodeChecked(out, truncated_utf8, 1));
        test_string(expected_truncated, out, sizeof(expected_truncated));
        test_result(!EncodeUnicodeChecked(out, invalid_locale,
                                          sizeof(invalid_locale) - 1));
        test_string(expected_replacement, out,
                    sizeof(expected_replacement));
    }

    return 0;
}
