/* (c) 2002-2004 by Marcin Wiacek */

#ifndef n6510_h
#define n6510_h

#include "../ncommon.h"
#include "../../../service/sms/gsmsms.h"

typedef enum {
	N6510_MMS_SETTINGS = 0x01,
	N6510_CHAT_SETTINGS,

	N6510_WAP_SETTINGS,
	N6510_SYNCML_SETTINGS
} N6510_Connection_Settings;

typedef enum {
	N6510_LIGHT_DISPLAY = 0x01,
	N6510_LIGHT_KEYPAD  = 0x03,
	N6510_LIGHT_TORCH   = 0x10
} N6510_PHONE_LIGHTS;

typedef struct {
	int				LastCalendarYear;
	int				LastCalendarPos;
	GSM_NOKIACalToDoLocations	LastCalendar;
	int				FirstCalendarPos;
	unsigned char			CalendarIcons[10];
	GSM_CalendarNoteType		CalendarIconsTypes[10];
	int				CalendarIconsNum;

	GSM_NOKIASMSFolder		LastSMSFolder;
	GSM_SMSFolders			LastSMSFolders;

	GSM_NOKIACalToDoLocations	LastToDo;

	GSM_NOKIACalToDoLocations	LastNote;

	unsigned char			RingtoneID;	/* When set with preview */

	int				FilesLocations[1000];
	int				FilesLevels[1000];
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;
	int				FileToken;
	int				ParentID;
	int				FileCheckSum;

	unsigned char			FMStatus[4000];
	int				FMStatusLength;

	unsigned char			GPRSPoints[4000];
	int				GPRSPointsLength;

	int				BearerNumber;

	unsigned char			PhoneMode;
} GSM_Phone_N6510Data;

void N6510_EncodeFMFrequency(double freq, unsigned char *buff);
void N6510_DecodeFMFrequency(double *freq, unsigned char *buff);

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif
#ifndef GSM_USED_FBUS2DLR3
#  define GSM_USED_FBUS2DLR3
#endif
#ifndef GSM_USED_FBUS2DKU5
#  define GSM_USED_FBUS2DKU5
#endif
#ifndef GSM_USED_PHONETBLUE
#  define GSM_USED_PHONETBLUE
#endif
#ifndef GSM_USED_IRDAPHONET
#  define GSM_USED_IRDAPHONET
#endif
#ifndef GSM_USED_BLUEPHONET
#  define GSM_USED_BLUEPHONET
#endif
#ifndef GSM_USED_FBUS2PL2303
#  define GSM_USED_FBUS2PL2303
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
