/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
*/

#ifndef __gsm_phonebook_h
#define __gsm_phonebook_h

#include "gsm-sms.h"

/* Limits for sizing of array in GSM_PhonebookEntry. Individual handsets may
   not support these lengths so they have their own limits set. */

#define GSM_MAX_PHONEBOOK_NAME_LENGTH   (50)   /* For 7110 */
#define GSM_MAX_PHONEBOOK_NUMBER_LENGTH (48)   /* For 7110 */
#define GSM_MAX_PHONEBOOK_TEXT_LENGTH   (60)   /* For 7110 */
#define GSM_MAX_PHONEBOOK_SUB_ENTRIES   (8)    /* For 7110 */
#define GSM_MAX_PHONEBOOK_LOGO_LENGTH   (126)  /* For 7110 */
                                               /* 7110 is able to in one
						* entry 5 numbers and 2
						* texts [email,notice,postal] */

/* Here is a macro for models that do not support caller groups. */

#define GSM_GROUPS_NOT_SUPPORTED -1

/* This data type is used to report the number of used and free positions in
   memory (sim or internal). */

typedef struct {
  GSM_MemoryType MemoryType; /* Type of the memory */
  int Used;                  /* Number of used positions */
  int Free;                  /* Number of free positions */
} GSM_MemoryStatus;

/* Some phones (in this moment 6210/7110) supports extended phonebook
   with additional datas. Here we have structure for them */
typedef enum
{
  GSM_General = 0x0a,
  GSM_Mobile  = 0x03,
  GSM_Work    = 0x06,
  GSM_Fax     = 0x04,
  GSM_Home    = 0x02,
  GSM_SIM     = 0x00
} GSM_Number_Type;

typedef enum
{
  GSM_Number     = 0x0b,
  GSM_Note       = 0x0a,
  GSM_Postal     = 0x09,
  GSM_Email      = 0x08,
  GSM_Name       = 0x07,
  GSM_RingtoneNo = 0x0C,
  GSM_Date       = 0x13,  /* Date is used for DC,RC,etc (last calls) */
  GSM_LogoOn     = 0x1C,
  GSM_GroupLogo  = 0x1B,
  GSM_GroupNo    = 0x1E
} GSM_EntryType;

typedef struct {
    char Number[GSM_MAX_PHONEBOOK_TEXT_LENGTH+1]; /* Number */
    GSM_DateTime Date;                            /* or the last calls list */
} GSM_SubPhonebookEntrydata;

typedef struct {
  GSM_EntryType   EntryType;
  GSM_Number_Type NumberType;
  GSM_SubPhonebookEntrydata data;
  int             BlockNumber;
} GSM_SubPhonebookEntry;

/* Define datatype for phonebook entry, used for getting/writing phonebook
   entries. */

typedef struct {
  bool Empty;                                       /* Is this entry empty? */
  char Name[GSM_MAX_PHONEBOOK_NAME_LENGTH + 1];     /* Plus 1 for
						       nullterminator. */
  char Number[GSM_MAX_PHONEBOOK_NUMBER_LENGTH + 1]; /* Number */
  GSM_MemoryType MemoryType;                        /* Type of memory */
  int Group;                                        /* Group */
  int Location;                                     /* Location */
  GSM_SubPhonebookEntry SubEntries[GSM_MAX_PHONEBOOK_SUB_ENTRIES];
                                                    /* For phones with
						     * additional phonebook
						     * entries */
  int SubEntriesCount;                              /* Should be 0, if extended
                                                       phonebook is not used */
} GSM_PhonebookEntry;

char *GSM_GetVCARD(GSM_PhonebookEntry *entry, int version);

int GSM_SavePhonebookEntryToSMS(GSM_MultiSMSMessage *SMS,
                                GSM_PhonebookEntry *entry, int version);

#endif	/* __gsm_phonebook_h */
