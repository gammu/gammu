/*
 * python-gammu - Phone communication libary
 * Copyright © 2003 - 2009 Michal Čihař
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * vim: expandtab sw=4 ts=4 sts=4:
 */

#ifndef __pyg_convertors_h__
#define __pyg_convertors_h__

/* Python includes */
#include <Python.h>

/* Gammu includes */
#include <gammu.h>

#include <limits.h>

#include "misc.h"

/* Needed for Python 2.3, 2.4 has same included */
#ifndef Py_RETURN_NONE
#  define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

/**
 * Invalid value for gboolean.
 */
#define BOOL_INVALID ((gboolean)-1)

/**
 * Invalid value for integer (possibly signed).
 */
#define INT_INVALID (INT_MAX)

/**
 * Invalid value for enums (can not be negative).
 */
#define ENUM_INVALID (99999)

/**
 * Creates Python datetime.datetime from GSM_DateTime.
 */
PyObject *BuildPythonDateTime(const GSM_DateTime *dt);

/**
 * Creates Python datetime.time from GSM_DateTime.
 */
PyObject *BuildPythonTime(const GSM_DateTime *dt);

/**
 * Creates GSM_DateTime from Python datetime.time.
 */
int BuildGSMDateTime(PyObject *pydt, GSM_DateTime *dt);

/**
 * Creates GSM_DateTime from Python datetime.date.
 */
int BuildGSMDate(PyObject *pydt, GSM_DateTime *dt);

/**
 * Creates GSM_DateTime from Python datetime.time.
 */
int BuildGSMTime(PyObject *pydt, GSM_DateTime *dt);

/**
 * Convert Python unicode string to Gammu unicode string.
 */
unsigned char *strPythonToGammu(const Py_UNICODE *src, const size_t len);

/**
 * Converts either object to unicode and returns its value.
 */
unsigned char *StringPythonToGammu(PyObject *o);

/**
 * Convert Gammu unicode string to unicode string that can be read by Python.
 */
Py_UNICODE *strGammuToPython(const unsigned char *src);

/**
 * Convert Gammu unicode string with defined length to unicode string that can be read by Python.
 */
Py_UNICODE *strGammuToPythonL(const unsigned char *src, const int len);

/**
 * Convert Gammu unicode string to python unicode object.
 */
PyObject *UnicodeStringToPython(const unsigned char *src);

/**
 * Convert Gammu unicode string with defined length to python unicode object.
 */
PyObject *UnicodeStringToPythonL(const unsigned char *src, const Py_ssize_t len);

/**
 * Converts locale encoded string to python unicode string.
 */
PyObject *LocaleStringToPython(const char *src);

/**
 * Converts string to CategoryType.
 */
GSM_CategoryType StringToCategoryType(const char *s);

/**
 * Converts string to MemoryType.
 */
GSM_MemoryType StringToMemoryType(const char *s);

/**
 * Converts MemoryType to string.
 */
char *MemoryTypeToString(const GSM_MemoryType t);

/**
 * Builds Python object containing all GSM_MemoryEntry values.
 */
PyObject *MemoryEntryToPython(const GSM_MemoryEntry *entry);

/**
 * Fills GSM_MemoryEntry from Pythons dictionary:.
 */
int MemoryEntryFromPython(PyObject *dict, GSM_MemoryEntry *entry, int needs_location);

/**
 * Gets integer from dictionary dict with key key.
 */
int GetIntFromDict(PyObject *dict, const char *key);

/**
 * Gets gboolean from dictionary dict with key key.
 */
gboolean GetBoolFromDict(PyObject *dict, const char *key);

/**
 * Returns C string with length from dictionary dict with key key.
 *
 * Returns newly allocated memory.
 */
char *GetCStringLengthFromDict(PyObject *dict, const char *key, Py_ssize_t *length);

/**
 * Gets C string from dictionary dict with key key.
 */
char *GetCStringFromDict(PyObject *dict, const char *key);

/**
 * Gets gammu unicode string from dictionary dict with key key.
 */
unsigned char *GetStringFromDict(PyObject *dict, const char *key);

/**
 * Copies unicode string from Python dictionary to Gammu string with length checking
 */
int CopyStringFromDict(PyObject *dict, const char *key, size_t len, unsigned char *dest);

/**
 * Gets GSM_DateTime from dictionary dict with key key.
 */
GSM_DateTime GetDateTimeFromDict(PyObject *dict, const char *key);

/**
 * Gets GSM_DateTime from dictionary dict with key key.
 */
GSM_DateTime GetDateFromDict(PyObject *dict, const char *key);

/**
 * Gets char* from dictionary dict with key key. (doesn't allocate it!)
 */
char *GetCharFromDict(PyObject *dict, const char *key);

/**
 * Gets char* with length (may contain NULs) from dictionary dict with key key. (doesn't allocate it!)
 */
char *GetDataFromDict(PyObject *dict, const char *key, Py_ssize_t *len);

/**
 * Gets GSM_MemoryType from dictionary dict with key key.
 */
GSM_MemoryType GetMemoryTypeFromDict(PyObject *dict, const char *key);

/**
 * Coverts cell broadcast message to Python object.
 */
PyObject *CBToPython(GSM_CBMessage *cb);

/**
 * Coverts USSD message to Python object.
 */
PyObject *USSDToPython(GSM_USSDMessage *ussd);

/**
 * Converts SMSValidity to string.
 */
char *SMSValidityToString(GSM_SMSValidity Validity);

/**
 * Creates SMSValidity from string.
 */
GSM_SMSValidity StringToSMSValidity(char *s);

/**
 * Converts SMSFormat to string.
 */
char *SMSFormatToString(GSM_SMSFormat f);

/**
 * Converts string to SMSFormat.
 */
GSM_SMSFormat StringToSMSFormat(char *s);

/**
 * Builds python object from GSM_SMSC.
 */
PyObject *SMSCToPython(GSM_SMSC *smsc);

/**
 * Parses python object into GMS_SMSC.
 */
int SMSCFromPython(PyObject* dict, GSM_SMSC *smsc, gboolean complete);

/**
 * Converts SMS to python object.
 */
PyObject *SMSToPython(GSM_SMSMessage *sms);

/**
 * Creates SMS from python object.
 */
int SMSFromPython(PyObject *dict, GSM_SMSMessage *sms, int needslocation, int needsfolder, int needsnumber);

/**
 * Converts MultiSMS to list of python object.
 */
PyObject *MultiSMSToPython(GSM_MultiSMSMessage *sms);

/**
 * Converts MultiSMS from list of python objects.
 */
int MultiSMSFromPython(PyObject *list, GSM_MultiSMSMessage *sms);

/**
 * Converts MultiSMS list from list of python objects.
 */
int MultiSMSListFromPython(PyObject *list, GSM_MultiSMSMessage ***sms);

/**
 * Converts MultiSMS list to list of python objects.
 */
PyObject *MultiSMSListToPython(GSM_MultiSMSMessage **sms);

/**
 * Converts SMS backup to list of Python objects.
 */
PyObject *SMSBackupToPython(GSM_SMS_Backup *sms);

/**
 * Converts SMS backup from list of Python objects.
 */
int SMSBackupFromPython(PyObject *list, GSM_SMS_Backup *sms);

/**
 * Converts Todo to Python object.
 */
PyObject *TodoToPython(const GSM_ToDoEntry *entry);

/**
 * Converts Todo from Python object.
 */
int TodoFromPython(PyObject *dict, GSM_ToDoEntry *entry, int needs_location);

/**
 * Convert calendar type to string.
 */
char *CalendarTypeToString(GSM_CalendarNoteType p);

/**
 * Convert string to calendar type.
 */
GSM_CalendarNoteType StringToCalendarType(const char *s);

/**
 * Converts Calendar to Python object.
 */
PyObject *CalendarToPython(const GSM_CalendarEntry *entry);

/**
 * Converts Calendar from Python object.
 */
int CalendarFromPython(PyObject *dict, GSM_CalendarEntry *entry, int needs_location);

/**
 * Converts SMSInfo to Python object.
 */
PyObject *SMSInfoToPython(GSM_MultiPartSMSInfo *smsinfo);

/**
 * Converts SMSInfo from Python object.
 */
int SMSInfoFromPython(PyObject *dict, GSM_MultiPartSMSInfo *entry);

/**
 * Converts Bitmap to Python object.
 */
PyObject *BitmapToPython(GSM_Bitmap *bitmap);

/**
 * Converts multiple Bitmaps to list of Python objects.
 */
PyObject *MultiBitmapToPython(GSM_MultiBitmap *bmp);

/**
 * Converts Bitmap from Python object.
 */
int BitmapFromPython(PyObject *data, GSM_Bitmap *bitmap);

/**
 * Converts multiple Bitmaps from list of Python objects.
 */
int MultiBitmapFromPython(PyObject *list, GSM_MultiBitmap *bmp);

/**
 * Converts SMS folders structure to list of Python objects.
 */
PyObject *SMSFoldersToPython(GSM_SMSFolders *folders);

/**
 * Converts ringtone to Python object.
 */
PyObject *RingtoneToPython(GSM_Ringtone *ring);

/**
 * Converts ringtone from Python object.
 */
int RingtoneFromPython(PyObject *dict, GSM_Ringtone *ring);

/**
 * Converts backup to Python object.
 */
PyObject *BackupToPython(GSM_Backup *backup);

/**
 * Converts backup from Python object.
 */
int BackupFromPython(PyObject *dict, GSM_Backup *backup);

/**
 * Converts backup format from string to Gammu.
 */
int BackupFormatFromString(const char *s, GSM_BackupFormat *format);

/**
 * Converts file to Python object.
 */
PyObject *FileToPython(GSM_File *file);

/**
 * Converts file from Python object.
 */
int FileFromPython(PyObject *dict, GSM_File *file, gboolean check);

/**
 * Converts call to Python object.
 */
PyObject *CallToPython(GSM_Call *call);

/**
 * Converts call type to Python object.
 */
char *CallStatusToString(GSM_CallStatus ct);

/**
 * Converts MMS indicator to Python object.
 */
PyObject *MMSIndicatorToPython(GSM_MMSIndicator *mms);

/**
 * Converts MMS indicator from Python object.
 */
int MMSIndicatorFromPython(PyObject* dict, GSM_MMSIndicator *mms);

/**
 * Converts WAP bookmark to Python object.
 */
PyObject *WAPBookmarkToPython(GSM_WAPBookmark *wap);

/**
 * Converts WAP bookmark from Python object.
 */
int WAPBookmarkFromPython(PyObject* dict, GSM_WAPBookmark *wap);

#endif
