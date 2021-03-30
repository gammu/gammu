/* Test for vcard LUID and index parsing of OBEX driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

#include "common.h"

extern GSM_Error OBEXGEN_InitLUID(GSM_StateMachine *s, const char *Name,
		const gboolean Recalculate,
		const char *Header,
		char **Data, int **Offsets, int *Count,
		char ***LUIDStorage, int *LUIDCount,
		int **IndexStorage, int *IndexCount);

int main(int argc, char **argv)
{
	GSM_Debug_Info *debug_info;
	char *buffer;
    struct stat sb;
	FILE *f;
	size_t len;
	GSM_StateMachine *s;
	GSM_Error error;
    int *Offsets;
    int Count;
	char **LUIDStorage;
    int LUIDCount;
	int *IndexStorage;
    int IndexCount;

	/* Check parameters */
	if (argc != 3) {
		printf("Not enough parameters!\nUsage: obex-vcard-split data num-entries\n");
		return 1;
    }

    if (stat(argv[1], &sb) != 0) {
		printf("Could not stat %s\n", argv[1]);
		return 1;
    }

    buffer = malloc(sb.st_size + 1);
    if (buffer == NULL) {
        printf("Failed to malloc %ld bytes\n", (long)sb.st_size);
        return 1;
    }

	/* Open file */
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Could not open %s\n", argv[1]);
		return 1;
	}

	/* Read data */
	len = fread(buffer, 1, sb.st_size + 1, f);
	if (!feof(f)) {
		printf("Could not read whole file %s\n", argv[1]);
		fclose(f);
		return 1;
	}
	/* Zero terminate data */
	buffer[len] = 0;

	/* Close file */
	fclose(f);

	/* Configure state machine */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);

	/* Parse data */
	error = OBEXGEN_InitLUID(s, "", TRUE, "BEGIN:VCARD", &buffer, &Offsets, &Count, &LUIDStorage, &LUIDCount, &IndexStorage, &IndexCount);

	/* Free state machine */
	GSM_FreeStateMachine(s);

    free(buffer);

	gammu_test_result(error, "OBEXGEN_InitLUID");
    test_result(atoi(argv[2]) == Count);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
