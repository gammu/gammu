
#ifndef __gsm_phonebook_h
#define __gsm_phonebook_h

#include <stdlib.h>

#include "../gsmcomon.h"
#include "gsmmisc.h"

/* This data type is used to report the number of used
 * and free positions in memory (sim or internal).
 */
typedef struct {
	/**
	 * Type of the memory
	 */
	GSM_MemoryType  MemoryType;
	/**
	 * umber of used positions
	 */
	int	     	Used;
	/**
	 * Number of free positions
	 */
	int	     	Free;
} GSM_MemoryStatus;

/**
 * Type of specific phonebook entry. In parenthesis is specified in which
 * member of @ref GSM_SubMemoryEntry value is stored.
 */
typedef enum {
	/**
	 * General number. (Text)
	 */
	PBK_Number_General = 1,
	/**
	 * Mobile number. (Text)
	 */
	PBK_Number_Mobile,
	/**
	 * Work number. (Text)
	 */
	PBK_Number_Work,
	/**
	 * Fax number. (Text)
	 */
	PBK_Number_Fax,
	/**
	 * Home number. (Text)
	 */
	PBK_Number_Home,
	/**
	 * Pager number. (Text)
	 */
	PBK_Number_Pager,
	/**
	 * Other number. (Text)
	 */
	PBK_Number_Other,
	/**
	 * Note. (Text)
	 */
	PBK_Text_Note,
	/**
	 * Complete postal address. (Text)
	 */
	PBK_Text_Postal,
	/**
	 * Email. (Text)
	 */
	PBK_Text_Email,
	/**
	 * Second email. (Text)
	 */
	PBK_Text_Email2,
	/**
	 * URL (Text)
	 */
	PBK_Text_URL,
	/**
	 * Date and time. FIXME: describe better (Date)
	 */
	PBK_Date,
	/**
	 * Caller group. (Text)
	 */
	PBK_Caller_Group,
	/**
	 * Name (Text)
	 */
	PBK_Text_Name,
	/**
	 * Last name. (Text)
	 */
	PBK_Text_LastName,
	/**
	 * First name. (Text)
	 */
	PBK_Text_FirstName,
	/**
	 * Company. (Text)
	 */
	PBK_Text_Company,
	/**
	 * Job title. (Text)
	 */
	PBK_Text_JobTitle,
	/**
	 * Category. (Number)
	 */
	PBK_Category,
	/**
	 * Whether entry is private. (Number)
	 */
	PBK_Private,
	/**
	 * Street address. (Text)
	 */
	PBK_Text_StreetAddress,
	/**
	 * City. (Text)
	 */
	PBK_Text_City,
	/**
	 * State. (Text)
	 */
	PBK_Text_State,
	/**
	 * Zip code. (Text)
	 */
	PBK_Text_Zip,
	/**
	 * Country. (Text)
	 */
	PBK_Text_Country,
	/**
	 * Custom information 1. (Text)
	 */
	PBK_Text_Custom1,
	/**
	 * Custom information 2. (Text)
	 */
	PBK_Text_Custom2,
	/**
	 * Custom information 3. (Text)
	 */
	PBK_Text_Custom3,
	/**
	 * Custom information 4. (Text)
	 */
	PBK_Text_Custom4,
	/**
	 * Ringtone ID. (Text)
	 */
	PBK_RingtoneID,
	/**
	 * Picture ID. (Number)
	 */
	PBK_PictureID
} GSM_EntryType;

/* Limits for sizing of array in GSM_MemoryEntry.
 * Individual handsets may not support these lengths
 * so they have their own limits set.
 */
#define GSM_PHONEBOOK_TEXT_LENGTH       (200)
#define GSM_PHONEBOOK_ENTRIES	   	(26)

/**
 * One value of memory entry.
 */
typedef struct {
	/**
	 * Type of entry.
	 */
	GSM_EntryType	   	EntryType;
	/**
	 * Text of entry (if applicable, see @ref GSM_EntryType).
	 */
	unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
	/**
	 * Text of entry (if applicable, see @ref GSM_EntryType).
	 */
	GSM_DateTime	    	Date;
	/**
	 * Number of entry (if applicable, see @ref GSM_EntryType).
	 */
	int		     	Number;
	/**
	 * Voice dialling tag.
	 */
	int		     	VoiceTag;
} GSM_SubMemoryEntry;

/* Define datatype for phonebook entry,
 * used for getting/writing phonebook entries.
 */
typedef struct {
	/**
	 * Type of memory.
	 */
	GSM_MemoryType	  	MemoryType;
	/**
	 * Location.
	 */
	int		     	Location;
	/**
	 * Number of entries.
	 */
	int		     	EntriesNum;
	/**
	 * Whether to prefer unicode for storing this entry.
	 */
	bool		    	PreferUnicode;
	/**
	 * Values for this entry.
	 */
	GSM_SubMemoryEntry      Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_MemoryEntry;

typedef enum {
	Nokia_VCard10 = 1,
	Nokia_VCard21,
	SonyEricsson_VCard10,
	SonyEricsson_VCard21
} GSM_VCardVersion;

void GSM_PhonebookFindDefaultNameNumberGroup(GSM_MemoryEntry *entry, int *Name, int *Number, int *Group);
unsigned char *GSM_PhonebookGetEntryName (GSM_MemoryEntry *entry);

void      GSM_EncodeVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version);
GSM_Error GSM_DecodeVCARD(unsigned char *Buffer, int *Pos, GSM_MemoryEntry *Pbk, GSM_VCardVersion Version);

void DecodeVCARD21Text(char *VCard, GSM_MemoryEntry *pbk);

/**
 * This define speed dialing entries.
 */
typedef struct {
	/**
	 * Which number is used to dialing?
	 */
	int	     	Location;
	/**
	 * Memory type of the number.
	 */
	GSM_MemoryType  MemoryType;
	/**
	 * Location of the number in MemoryType.
	 */
	int	     	MemoryLocation;
	/**
	 * Which number in location is used
	 */
	int	     	MemoryNumberID;
} GSM_SpeedDial;

#endif  /* __gsm_phonebook_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
