/**
 * \file gammu-limits.h
 * \author Michal Čihař
 *
 * Limits definitions.
 */
#ifndef __gammu_limits_h
#define __gammu_limits_h

/**
 * \defgroup Limits Limits
 * Limits definitions.
 */

/**
 * Maximal length of manufacturer name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MANUFACTURER_LENGTH 50

/**
 * Maximal length of model name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MODEL_LENGTH 50

/**
 * Maximal length of version text.
 *
 * \ingroup Limits
 */
#define GSM_MAX_VERSION_LENGTH 200

/**
 * Maximal length of version date.
 *
 * \ingroup Limits
 */
#define GSM_MAX_VERSION_DATE_LENGTH 50

/**
 * Maximal length of IMEI.
 *
 * \ingroup Limits
 */
#define GSM_MAX_IMEI_LENGTH 20

/**
 * Maximal count of SMS messages in folder.
 *
 * \ingroup Limits
 */
#define GSM_PHONE_MAXSMSINFOLDER 600

/**
 * Maximal length of phone number.
 *
 * \ingroup Limits
 */
#define GSM_MAX_NUMBER_LENGTH 50

/**
 * Maximal count of todo and calendar notes.
 *
 * \ingroup Limits
 */
#define GSM_MAXCALENDARTODONOTES 550

/**
 * Maximal length of text in phonebook.
 *
 * \ingroup Limits
 */
#define GSM_PHONEBOOK_TEXT_LENGTH 200

/**
 * Maximal count of phonebook notes.
 *
 * \ingroup Limits
 */
#define GSM_PHONEBOOK_ENTRIES 26

/**
 * Maximal number of calendar entries.
 *
 * \ingroup Limits
 */
#define GSM_CALENDAR_ENTRIES 16

/**
 * Maximal length of text in calendar.
 *
 * \ingroup Limits
 */
#define GSM_MAX_CALENDAR_TEXT_LENGTH 256

/**
 * Maximal number of todo entries.
 *
 * \ingroup Limits
 */
#define GSM_TODO_ENTRIES 7

/**
 * Maximal length of text in todo.
 *
 * \ingroup Limits
 */
#define GSM_MAX_TODO_TEXT_LENGTH 160

/**
 * Maximal length of SMS name.
 */
#define GSM_MAX_SMS_NAME_LENGTH 200

/**
 * Maximal count of other numbes in SMS.
 *
 * \ingroup Limits
 */
#define GSM_SMS_OTHER_NUMBERS 5

/**
 * Number of possible SMS folders.
 */
#define GSM_MAX_SMS_FOLDERS 50

/**
 * Maximal length of SMS folder name.
 */
#define GSM_MAX_SMS_FOLDER_NAME_LEN 20

/**
 * Maximal count of SMSes in multi SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MULTI_SMS 10

/**
 * Maximal length of UDH in SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_UDH_LENGTH 140

/**
 * Maximal length of text in standard SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_SMS_LENGTH 160

/**
 * Maximal length of 8-bit text in SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_8BIT_SMS_LENGTH 140

/**
 * Number of possible MMS folders.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MMS_FOLDERS 10

/**
 * Maximal length of MMS folder name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MMS_FOLDER_NAME_LEN 20

/**
 * Maximal count of MMSes in multi MMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MULTI_MMS 20

/**
 * Maximal length of SMSC name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_SMSC_NAME_LENGTH 50

/**
 * Maximal count of notes in ringtone.
 *
 * \ingroup Limits
 */
#define GSM_MAX_RINGTONE_NOTES 255

/**
 * Maximal length of FM station name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_FMSTATION_LENGTH 18

/**
 * Maximal count of FM stations.
 *
 * \ingroup Limits
 */
#define GSM_MAX_FM_STATION 20

/**
 * Maximal length of security code.
 *
 * \ingroup Limits
 */
#define GSM_SECURITY_CODE_LEN 15

/**
 * Maximal length of category name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_CATEGORY_NAME_LENGTH 60

/**
 * Maximal size of bitmap.
 *
 * \ingroup Limits
 */
#define GSM_BITMAP_SIZE (65+7)/8*96

/**
 * Maximal length of bitmap text.
 *
 * \ingroup Limits
 */
#define GSM_BITMAP_TEXT_LENGTH 128

/**
 * Maximal count of bitmaps in multi bitmap.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MULTI_BITMAP 6

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
