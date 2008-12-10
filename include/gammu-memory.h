/**
 * \file gammu-memory.h
 * \author Michal Čihař
 *
 * Memory data and functions.
 */
#ifndef __gammu_memory_h
#define __gammu_memory_h

/**
 * \defgroup Memory Memory
 * Memory entries manipulations.
 */

#include <gammu-datetime.h>
#include <gammu-limits.h>
#include <gammu-bitmap.h>

/**
 * Enum defines ID for various phone and SIM memories.
 * Phone modules can translate them to values specific for concrete models.
 * Two letter codes (excluding VM and SL) are from GSM 07.07.
 *
 * \ingroup Memory
 */
typedef enum {
	/**
	 * Internal memory of the mobile equipment
	 */
	MEM_ME = 1,
	/**
	 * SIM card memory
	 */
	MEM_SM,
	/**
	 * Own numbers
	 */
	MEM_ON,
	/**
	 * Dialled calls
	 */
	MEM_DC,
	/**
	 * Received calls
	 */
	MEM_RC,
	/**
	 * Missed calls
	 */
	MEM_MC,
	/**
	 * Combined ME and SIM phonebook
	 */
	MEM_MT,
	/**
	 * Fixed dial
	 */
	MEM_FD,

	/**
	 * Voice mailbox
	 */
	MEM_VM,

	/**
	 * Sent SMS logs
	 */
	MEM_SL
} GSM_MemoryType;

/**
 * Returns string representation of a memory type.
 *
 * \param x Memory type to convert
 *
 * \return Static string.
 *
 * \ingroup Memory
 */
#define GSM_MemoryTypeToString(x) ( \
	(x) == MEM_ME ? "ME" :			\
	(x) == MEM_SM ? "SM" :			\
	(x) == MEM_ON ? "ON" :			\
	(x) == MEM_DC ? "DC" :			\
	(x) == MEM_RC ? "RC" :			\
	(x) == MEM_MC ? "MC" :			\
	(x) == MEM_MT ? "MT" :			\
	(x) == MEM_FD ? "FD" :			\
	(x) == MEM_VM ? "VM" :			\
	(x) == MEM_SL ? "SL" : "XX")

/**
 * Structure contains info about number of used/free entries in phonebook
 * memory.
 *
 * \ingroup Memory
 */
typedef struct {
	/**
	 * Number of used entries
	 */
	int MemoryUsed;
	/**
	 * Memory type
	 */
	GSM_MemoryType MemoryType;
	/**
	 * Number of free entries
	 */
	int MemoryFree;
} GSM_MemoryStatus;

/**
 * Type of specific phonebook entry. In parenthesis is specified in which
 * member of @ref GSM_SubMemoryEntry value is stored.
 *
 * \ingroup Memory
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
	PBK_Number_Home, /* 5 */
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
	PBK_Text_Email, /* 10 */
	/*
	 * Second email. (Text)
	 */
	PBK_Text_Email2,
	/**
	 * URL (Text)
	 */
	PBK_Text_URL,
	/**
	 * Date and time of last call.
	 */
	PBK_Date,
	/**
	 * Caller group. (Number)
	 */
	PBK_Caller_Group,
	/**
	 * Name (Text)
	 */
	PBK_Text_Name, /* 15 */
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
	 * Category. (Number, if -1 then text)
	 */
	PBK_Category, /* 20 */
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
	PBK_Text_Zip, /* 25 */
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
	PBK_Text_Custom4, /* 30 */
	/**
	 * Ringtone ID. (Number)
	 */
	PBK_RingtoneID,
	/**
	 * Picture ID. (Number)
	 */
	PBK_PictureID,
	/**
	 * User ID. (Text)
	 */
	PBK_Text_UserID,
	/**
	 * Length of call (Number)
	 */
	PBK_CallLength,
	/**
	 * LUID - Unique Identifier used for synchronisation
	 */
	PBK_Text_LUID, /* 35 */
	/**
	 * Date of last modification
	 */
	PBK_LastModified,
	/**
	 * Nick name (Text)
	 */
	PBK_Text_NickName,
	/**
	 * Formal name (Text)
	 */
	PBK_Text_FormalName,
	/**
	 * Work street address. (Text)
	 */
	PBK_Text_WorkStreetAddress,
	/**
	 * Work city. (Text)
	 */
	PBK_Text_WorkCity, /* 40 */
	/**
	 * Work state. (Text)
	 */
	PBK_Text_WorkState,
	/**
	 * Work zip code. (Text)
	 */
	PBK_Text_WorkZip,
	/**
	 * Work country. (Text)
	 */
	PBK_Text_WorkCountry,
	/**
	 * Complete work postal address. (Text)
	 */
	PBK_Text_WorkPostal,
	/**
	 * Picture name (on phone filesystem). (Text)
	 */
	PBK_Text_PictureName, /* 45 */
 	/**
 	 * Push-to-talk ID (Text)
 	 */
 	PBK_PushToTalkID,
	/**
	 * Favorite messaging number. (Text)
	 */
	PBK_Number_Messaging,
	/**
	 * Photo (Picture).
	 */
	PBK_Photo,
} GSM_EntryType;

/**
 * One value of phonebook memory entry.
 *
 * \ingroup Memory
 */
typedef struct {
	/**
	 * Type of entry.
	 */
	GSM_EntryType EntryType;
	/**
	 * Text of entry (if applicable, see @ref GSM_EntryType).
	 */
	GSM_DateTime Date;
	/**
	 * Number of entry (if applicable, see @ref GSM_EntryType).
	 */
	int Number;
	/**
	 * Voice dialling tag.
	 */
	int VoiceTag;
	int SMSList[20];
	int CallLength;

	/**
	 * During adding SubEntry Gammu can return here info, if it was done OK
	 */
	GSM_Error AddError;
	/**
	 * Text of entry (if applicable, see @ref GSM_EntryType).
	 */
	unsigned char Text[(GSM_PHONEBOOK_TEXT_LENGTH + 1) * 2];
	/**
	 * Picture data.
	 */
	GSM_BinaryPicture Picture;
} GSM_SubMemoryEntry;

/**
 * Structure for saving phonebook entries.
 *
 * \ingroup Memory
 */
typedef struct {
	/**
	 * Used memory for phonebook entry
	 */
	GSM_MemoryType MemoryType;
	/**
	 * Used location for phonebook entry
	 */
	int Location;
	/**
	 * Number of SubEntries in Entries table.
	 */
	int EntriesNum;
	/**
	 * Values of SubEntries.
	 */
	GSM_SubMemoryEntry Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_MemoryEntry;

/**
 * Structure for saving speed dials.
 *
 * \ingroup Memory
 */
typedef struct {
	/**
	 * Number of speed dial: 2,3..,8,9
	 */
	int Location;
	/**
	 * ID of phone number used in phonebook entry
	 */
	int MemoryNumberID;
	/**
	 * Memory, where is saved used phonebook entry
	 */
	GSM_MemoryType MemoryType;
	/**
	 * Location in memory, where is saved used phonebook entry
	 */
	int MemoryLocation;
} GSM_SpeedDial;

/**
 * Gets memory (phonebooks or calls) status (eg. number of used and
 * free entries).
 *
 * \param s State machine pointer.
 * \param status Storage for status information, MemoryType has to be
 * set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_GetMemoryStatus(GSM_StateMachine * s, GSM_MemoryStatus * status);

/**
 * Reads entry from memory (phonebooks or calls). Which entry should
 * be read is defined in entry.
 *
 * \param s State machine pointer.
 * \param entry Storage for retrieved entry, MemoryType and Location has
 * to be set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_GetMemory(GSM_StateMachine * s, GSM_MemoryEntry * entry);

/**
 * Reads entry from memory (phonebooks or calls). Which entry should
 * be read is defined in entry. This can be easily used for reading all entries.
 *
 * \param s State machine pointer.
 * \param entry Storage for retrieved entry. MemoryType has to be set
 * for first call (with start set to true), for subsequent calls
 * Location has to stay intact from previous reading.
 * \param start Whether we should start from beginning.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_GetNextMemory(GSM_StateMachine * s, GSM_MemoryEntry * entry,
			    bool start);

/**
 * Sets memory (phonebooks or calls) entry.
 *
 * \param s State machine pointer.
 * \param entry Entry to set, Location and MemoryType has to be set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_SetMemory(GSM_StateMachine * s, GSM_MemoryEntry * entry);

/**
 * Deletes memory (phonebooks or calls) entry.
 *
 * \param s State machine pointer.
 * \param entry Entry to add, Location is ignored, MemoryType has to be
 * set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_AddMemory(GSM_StateMachine * s, GSM_MemoryEntry * entry);

/**
 * Deletes memory (phonebooks or calls) entry.
 *
 * \param s State machine pointer.
 * \param entry Entry to delete, Location and MemoryType has to be set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_DeleteMemory(GSM_StateMachine * s, GSM_MemoryEntry * entry);

/**
 * Deletes all memory (phonebooks or calls) entries of specified type.
 *
 * \param s State machine pointer.
 * \param MemoryType Where to delete all entries.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_DeleteAllMemory(GSM_StateMachine * s, GSM_MemoryType MemoryType);

/**
 * Gets speed dial.
 *
 * \param s State machine pointer.
 * \param Speed Storage for speed dial, Location has to be set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_GetSpeedDial(GSM_StateMachine * s, GSM_SpeedDial * Speed);

/**
 * Sets speed dial.
 *
 * \param s State machine pointer.
 * \param Speed Sspeed dial to set.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_SetSpeedDial(GSM_StateMachine * s, GSM_SpeedDial * Speed);

/**
 * Returns name of entry. It might be possibly concatenated from first
 * and last names.
 *
 * \param entry Entry to process.
 *
 * \return Static unicode string containing name.
 *
 * \ingroup Memory
 */
unsigned char *GSM_PhonebookGetEntryName(const GSM_MemoryEntry * entry);

/**
 * Finds default name, number and group for entry.
 *
 * \param entry Entry to process.
 * \param Name Output index of name.
 * \param Number Output index of number.
 * \param Group Output index of group.
 *
 * \ingroup Memory
 */
void GSM_PhonebookFindDefaultNameNumberGroup(const GSM_MemoryEntry * entry, int *Name,
					     int *Number, int *Group);
/**
 * Types of vCard.
 *
 * \ingroup Memory
 */
typedef enum {
	/**
	 * vCard 1.0 hacked for Nokia.
	 */
	Nokia_VCard10 = 1,
	/**
	 * vCard 2.1 hacked for Nokia.
	 */
	Nokia_VCard21,
	/**
	 * vCard 1.0 hacked for Sony-Ericsson (should be standard
	 * vCard).
	 */
	SonyEricsson_VCard10,
	/**
	 * vCard 2.1 hacked for Sony-Ericsson (should be standard
	 * vCard).
	 */
	SonyEricsson_VCard21,
} GSM_VCardVersion;

/**
 * Encodes memory entry to vCard.
 *
 * \param[out] Buffer Buffer to store vCard text.
 * \param[in] buff_len Size of output buffer.
 * \param[in,out] Pos Position in output buffer.
 * \param[in,out] pbk Phonebook data, AddError will be set on non
 * converted entries.
 * \param[in] header Whether to include vCard header in output.
 * \param[in] Version What vCard version to create.
 *
 * \return Error code.
 *
 * \ingroup Memory
 */
GSM_Error GSM_EncodeVCARD(char *Buffer, const size_t buff_len, size_t *Pos,
			GSM_MemoryEntry * pbk, const bool header,
			const GSM_VCardVersion Version);

/**
 * Decodes memory entry from vCard.
 *
 * \ingroup Memory
 */
GSM_Error GSM_DecodeVCARD(char *Buffer, size_t *Pos,
			  GSM_MemoryEntry * Pbk, const GSM_VCardVersion Version);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
