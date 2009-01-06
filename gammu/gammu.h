/* (c) 2002-2003 by Marcin Wiacek and Michal Cihar */

#include <gammu-error.h>
#include <gammu-statemachine.h>

#include "common.h"

typedef enum {
	H_Call = 1,
	H_SMS,
	H_Memory,
	H_Filesystem,
	H_Logo,
	H_Ringtone,
	H_Calendar,
	H_ToDo,
	H_Note,
	H_DateTime,
	H_Category,
	H_Tests,
#ifdef GSM_ENABLE_BACKUP
	H_Backup,
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	H_Nokia,
#endif
#ifdef GSM_ENABLE_AT
	H_Siemens,
#endif
	H_Network,
	H_WAP,
	H_MMS,
	H_FM,
	H_Info,
	H_Settings,
#ifdef DEBUG
	H_Decode,
#endif
	H_Gammu,
	H_Obsolete,
	H_Other
} HelpCategory;

typedef struct {
	HelpCategory category;
	const char *option;
	const char *description;
} HelpCategoryDescriptions;

typedef struct {
	const char *parameter;
	int min_arg;
	int max_arg;
	void (*Function) (int argc, char *argv[]);
	HelpCategory help_cat[10];
	const char *help;
} GSM_Parameters;

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
