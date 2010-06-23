#ifndef __gsm_backupvcf_h
#define __gsm_backupvcf_h

#include "backgen.h"

GSM_Error SaveVCard(FILE *file, GSM_Backup *backup);
GSM_Error LoadVCard(char *FileName, GSM_Backup *backup);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
