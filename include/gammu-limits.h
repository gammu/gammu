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

#define MAX_MANUFACTURER_LENGTH 50
#define MAX_MODEL_LENGTH 50
#define MAX_VERSION_LENGTH 100
#define MAX_VERSION_DATE_LENGTH 50
#define MAX_IMEI_LENGTH 20
#define PHONE_MAXSMSINFOLDER 600
#define GSM_MAX_NUMBER_LENGTH 50
#define GSM_MAXCALENDARTODONOTES 550
#define GSM_PHONEBOOK_TEXT_LENGTH 200
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
#define MAX_CALENDAR_TEXT_LENGTH 256

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
#define MAX_TODO_TEXT_LENGTH 160

/**
 * Maximal length of SMS name.
 */
#define GSM_MAX_SMS_NAME_LENGTH 200

#define GSM_SMS_OTHER_NUMBERS 5

/**
 * Number of possible SMS folders.
 */
#define GSM_MAX_SMS_FOLDERS 50
/**
 * Maximal length of SMS folder name.
 */
#define GSM_MAX_SMS_FOLDER_NAME_LEN 20

#define MAX_MULTI_SMS 10

#define GSM_MAX_UDH_LENGTH 140
#define GSM_MAX_SMS_LENGTH 160
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

#define MAX_MULTI_MMS 20

#define GSM_MAX_SMSC_NAME_LENGTH 50

#define MAX_RINGTONE_NOTES 255

#define GSM_MAX_FMSTATION_LENGTH 18
#define GSM_MAX_FM_STATION 20

#define GSM_SECURITY_CODE_LEN 15

/**
 * Maximal length of category name.
 *
 * \ingroup Limits
 */
#define GSM_MAX_CATEGORY_NAME_LENGTH 60

#define GSM_BITMAP_SIZE (65+7)/8*96
#define GSM_BITMAP_TEXT_LENGTH 128

#define MAX_MULTI_BITMAP 6

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
