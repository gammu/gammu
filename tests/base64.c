/**
 * Test case for base 64 encoder/decoder in Gammu
 */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "../libgammu/misc/coding/coding.h"


static const char base64[] =
    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
    "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
    "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
    "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
    "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

static const char text[] = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";


int main(int argc UNUSED, char **argv UNUSED)
{
	char *decoded;
	char *encoded;
	size_t len;

	decoded = malloc(sizeof(base64));
	encoded = malloc(sizeof(base64));


	len = DecodeBASE64(base64, decoded, strlen(base64));
	decoded[len] = 0;

	test_result(len == strlen(text));
	test_result(strcmp(decoded, text) == 0);

	EncodeBASE64(text, encoded, strlen(text));

	test_result(strcmp(base64, encoded) == 0);

	free(encoded);
	free(decoded);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
