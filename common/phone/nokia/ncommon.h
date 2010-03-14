
#ifndef phone_nokia_common_h
#define phone_nokia_common_h

#include "../../gsmcomon.h"

#define N6110_FRAME_HEADER 0x00, 0x01, 0x00
#define N7110_FRAME_HEADER 0x00, 0x01, 0x01

typedef enum {
	GMT7110_CG=0xf0,	/* Caller groups memory */
	GMT7110_SP=0xf1		/* Speed dial memory */
} GSM_71_65MemoryType;

typedef enum {
	N7110_ENTRYTYPE_SIM_SPEEDDIAL	= 0x04,	/* Speed Dial on SIM			*/
	N7110_ENTRYTYPE_NAME		= 0x07, /* Name (always the only one) 		*/
	N7110_ENTRYTYPE_EMAIL		= 0x08, /* Email Adress   (TEXT) 		*/
	N7110_ENTRYTYPE_POSTAL		= 0x09, /* Postal Address (TEXT) 		*/
	N7110_ENTRYTYPE_NOTE		= 0x0a, /* Note           (TEXT) 		*/
	N7110_ENTRYTYPE_NUMBER 		= 0x0b, /* Phonenumber 				*/
	N7110_ENTRYTYPE_RINGTONE	= 0x0c, /* Ringtone number 			*/
	N7110_ENTRYTYPE_DATE    	= 0x13, /* Date for a Called List 		*/
	N7110_ENTRYTYPE_SPEEDDIAL	= 0x1a,	/* Speed Dial 				*/
	N7110_ENTRYTYPE_GROUPLOGO	= 0x1b,	/* Caller group logo 			*/
	N7110_ENTRYTYPE_LOGOON		= 0x1c,	/* Logo On? 				*/
	N7110_ENTRYTYPE_GROUP		= 0x1e, /* Group number for phonebook entry 	*/
	N6510_ENTRYTYPE_URL		= 0x2c,	/* URL address - DCT4 only (TEXT) 	*/
	N6510_ENTRYTYPE_VOICETAG	= 0x2f	/* Voice tag assigments - DCT4 only	*/
} GSM_71_65_Phonebook_Entries_Types;

typedef enum {
	N7110_NUMBER_HOME	= 0x02,
	N7110_NUMBER_MOBILE	= 0x03,
	N7110_NUMBER_FAX	= 0x04,
	N7110_NUMBER_WORK	= 0x06,
	N7110_NUMBER_GENERAL	= 0x0a,
} GSM_71_65_Phonebook_Number_Types;

typedef struct {
	unsigned char	Location[PHONE_MAXSMSINFOLDER]; /* locations of SMS messages in that folder */
	int		Number;				/* number of SMS messages in that folder */
} GSM_NOKIASMSFolder;

#define NOKIA_MAXCALENDARNOTES	150

typedef struct {
	int		Location[NOKIA_MAXCALENDARNOTES];/* locations of calendar notes */
	int		Number;				 /* number of calendar notes */
} GSM_NOKIACalendarLocations;

#define NOKIA_MAXTODONOTES	150

typedef struct {
	int		Location[NOKIA_MAXTODONOTES];	/* locations of todo notes */
	int		Number;				/* number of todo notes */
} GSM_NOKIAToDoLocations;

#define NOKIA_PRESSPHONEKEY   0x01
#define NOKIA_RELEASEPHONEKEY 0x02

#endif
