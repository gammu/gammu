
#ifndef __gsm_phonebook_h
#define __gsm_phonebook_h

#include <stdlib.h>

#include "../gsmcomon.h"

/* Define an enum for specifying memory types for retrieving phonebook
 * entries, SMS messages etc. This type is not mobile specific - the model
 * code should take care of translation to mobile specific numbers - see 6110
 * code.
 */
typedef enum {
	/* 01/07/99:    Two letter codes follow GSM 07.07 release 6.2.0 */
	GMT_ME=1,	/* Internal memory of the mobile equipment */
	GMT_SM,		/* SIM card memory */
	GMT_ON,		/* Own numbers */
	GMT_DC,		/* Dialled numbers */
	GMT_RC,		/* Received numbers */
	GMT_MC,		/* Missed numbers */
	GMT_MT,		/* combined ME and SIM phonebook */
	GMT_FD,		/* fix dialling */

	GMT_VM		/* voice mailbox */
} GSM_MemoryType;

/* This data type is used to report the number of used
 * and free positions in memory (sim or internal).
 */
typedef struct {
	GSM_MemoryType	MemoryType;	/* Type of the memory */
	int		Used;		/* Number of used positions */
	int		Free;		/* Number of free positions */
} GSM_MemoryStatus;

typedef enum {
	PBK_Number_General = 1,
	PBK_Number_Mobile,
	PBK_Number_Work,
	PBK_Number_Fax,
	PBK_Number_Home,
	PBK_Number_Pager,
	PBK_Number_Other,
	PBK_Text_Note,
	PBK_Text_Postal, /* Complete postal address */
	PBK_Text_Email,
	PBK_Text_Email2,
	PBK_Text_URL,
	PBK_Name,
	PBK_Date,
	PBK_Caller_Group,
    	PBK_Text_LastName,
    	PBK_Text_FirstName,
    	PBK_Text_Company,
    	PBK_Text_JobTitle,
	PBK_Category,
    	PBK_Private,
    	PBK_Text_StreetAddress,
    	PBK_Text_City,
    	PBK_Text_State,
    	PBK_Text_Zip,
    	PBK_Text_Country,
    	PBK_Text_Custom1,
    	PBK_Text_Custom2,
    	PBK_Text_Custom3,
    	PBK_Text_Custom4,
	PBK_RingtoneID,
	PBK_PictureID
} GSM_EntryType;

/* Limits for sizing of array in GSM_PhonebookEntry.
 * Individual handsets may not support these lengths
 * so they have their own limits set.
 */
#define GSM_PHONEBOOK_TEXT_LENGTH	(62)
#define GSM_PHONEBOOK_ENTRIES		(26)

typedef struct {
	GSM_EntryType		EntryType;
	unsigned char		Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
	GSM_DateTime		Date;
	int			Number;
	int			VoiceTag;
} GSM_SubPhonebookEntry;

/* Define datatype for phonebook entry,
 * used for getting/writing phonebook entries.
 */
typedef struct {
	GSM_MemoryType		MemoryType;			/* Type of memory */
	int			Location;			/* Location */
	int			EntriesNum;			/* Number of entries */
	bool			PreferUnicode;			/* Whether to prefer unicode for storing this entry */
	GSM_SubPhonebookEntry	Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_PhonebookEntry;

void GSM_PhonebookFindDefaultNameNumberGroup(GSM_PhonebookEntry entry, int *Name, int *Number, int *Group);

void NOKIA_EncodeVCARD10SMSText(char *Buffer, int *Length, GSM_PhonebookEntry pbk);
void NOKIA_EncodeVCARD21SMSText(char *Buffer, int *Length, GSM_PhonebookEntry pbk);
void DecodeVCARD21Text(char *VCard, GSM_PhonebookEntry *pbk);

/* This define speed dialing entries. */
typedef struct {
	int 		Location;		/* Which number is used to dialing? 	 */
	GSM_MemoryType 	MemoryType;		/* Memory type of the number. 		 */
	int 		MemoryLocation;		/* Location of the number in MemoryType. */
	int		MemoryNumberID;		/* Which number in location is used	 */
} GSM_SpeedDial;

#endif	/* __gsm_phonebook_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
