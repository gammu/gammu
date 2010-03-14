
#include <string.h>

#include "gsmcal.h"
#include "gsmmisc.h"
#include "../misc/coding.h"

void ReadVCALDateTime(char *Buffer, GSM_DateTime *dt);
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);

void SaveVCALText(char *Buffer, int *Length, char *Text, char *Start);
bool ReadVCALText(char *Buffer, char *Start, char *Value);

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
