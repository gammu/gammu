/* (c) 2002-2003 by Marcin Wiacek and Michal Cihar */

#include "../common/config.h"
#include "../common/misc/misc.h"
#include "../common/gsmstate.h"

typedef enum {
	H_Call=1,
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
 	H_Other
} HelpCategory;
 
typedef struct {
 	HelpCategory	category;
 	char		*option;
 	char		*description;
} HelpCategoryDescriptions;
  
typedef struct {
 	char		*parameter;
 	int		min_arg;
 	int		max_arg;
 	void		(*Function) (int argc, char *argv[]);	
 	HelpCategory	help_cat[10];
 	char		*help;
} GSM_Parameters;

void	Print_Error		(GSM_Error error);
void	GSM_Init		(bool checkerror);
void	GSM_Terminate		(void);

extern	GSM_StateMachine	s;
extern	GSM_Phone_Functions	*Phone;
extern	GSM_Error		error;

extern volatile bool 		gshutdown;

void interrupt(int sign);

#ifdef GSM_ENABLE_BEEP
void	GSM_PhoneBeep		(void);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
