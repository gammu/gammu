/**
 * Simple test case for backup comments
 */

#include "common.h"
#include <gammu.h>
#include <stdio.h>

extern GSM_Error SaveTextComment(FILE *file, unsigned char *comment);

int main(int argc UNUSED, char **argv UNUSED)
{
    GSM_Error err;
    unsigned char buf[2048];
    const char src[] = "Text\n\nwith\n\rseveral\n\nnew\n\nlines.";
    EncodeUnicode(buf, src, sizeof(src));
    err = SaveTextComment(stdout, buf);
    gammu_test_result(err, "SaveTextComment");
	return 0;
}
