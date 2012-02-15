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
 * Maximal length of version text. This needs to hold all information we
 * receive, so buffer has to be big enough.
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
#define GSM_MAX_IMEI_LENGTH 35

/**
 * Maximal length of various phone information. Just a shorcut for
 * functions doing whole identification and use one buffer.
 *
 * \ingroup Limits
 */
#define GSM_MAX_INFO_LENGTH MAX(MAX(MAX(GSM_MAX_VERSION_LENGTH, \
	GSM_MAX_IMEI_LENGTH), GSM_MAX_MODEL_LENGTH), \
	GSM_MAX_MANUFACTURER_LENGTH)

/**
 * Maximal count of SMS messages in folder.
 *
 * \ingroup Limits
 */
#define GSM_PHONE_MAXSMSINFOLDER 100000

/**
 * Maximal length of phone number.
 *
 * \ingroup Limits
 */
#define GSM_MAX_NUMBER_LENGTH 200

/**
 * Maximal count of todo and calendar notes.
 *
 * \ingroup Limits
 */
#define GSM_MAXCALENDARTODONOTES 1000

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
#define GSM_PHONEBOOK_ENTRIES 40

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
#define GSM_MAX_CALENDAR_TEXT_LENGTH 4096

/**
 * Maximal length of text in calendar.
 *
 * \ingroup Limits
 */
#define GSM_MAX_NOTE_TEXT_LENGTH 4096

/**
 * Maximal number of todo entries.
 *
 * \ingroup Limits
 */
#define GSM_TODO_ENTRIES 16

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
#define GSM_MAX_SMS_FOLDER_NAME_LEN 100

/**
 * Maximal count of SMSes in multi SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_MULTI_SMS 50

/**
 * Maximal length of UDH in SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_UDH_LENGTH 140

/**
 * Maximal length of text in SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_SMS_LENGTH 650

/**
 * Maximal length of text in standard 7-bit SMS.
 *
 * \ingroup Limits
 */
#define GSM_MAX_SMS_CHARS_LENGTH 160

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
 * Maximal length of ringtone name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_RINGTONE_NAME_LENGTH 30

/**
 * Maximal length of FM station name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_FMSTATION_LENGTH 30

/**
 * Maximal count of FM stations.
 *
 * \ingroup Limits
 */
#define GSM_MAX_FM_STATION 30

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

/**
 * Maximal length of file name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_FILENAME_LENGTH 256

/**
 * Maximal length of full file name (including path).
 *
 * \ingroup Limits
 */
#define GSM_MAX_FILENAME_ID_LENGTH 4096

/**
 * Maximal number of features per phone.
 *
 * \ingroup Limits
 */
#define GSM_MAX_PHONE_FEATURES 20

/**
 * Maximal length of USSD text.
 *
 * \ingroup Limits
 */
#define GSM_MAX_USSD_LENGTH 999

/**
 * Maximal length of GPRS access point URL.
 *
 * \ingroup Limits
 */
#define GSM_MAX_GPRS_AP_NAME_LENGTH 149

/**
 * Maximal length of GPRS access point URL.
 *
 * \ingroup Limits
 */
#define GSM_MAX_GPRS_AP_URL_LENGTH 249

/**
 * Maximal number of call diverts.
 *
 * \ingroup Limits
 */
#define GSM_MAX_CALL_DIVERTS 20


#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
