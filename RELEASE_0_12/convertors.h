/*
 * python-gammu - Phone communication libary
 * Copyright (c) 2003 - 2006 Michal Čihař
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
 */

/* Python includes */
#include <Python.h>

/* Gammu includes */
#include <gammu.h>

#include <limits.h>

/* Needed for Python 2.3, 2.4 has same included */
#ifndef Py_RETURN_NONE
#  define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

/**
 * Invalid value for bool.
 */
#define BOOL_INVALID ((bool)-1)

/**
 * Invalid value for integer.
 */
#define INT_INVALID (INT_MIN)

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
unsigned char *strPythonToGammu(const Py_UNICODE *src);

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
PyObject *UnicodeStringToPythonL(const unsigned char *src, const int len);

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
int GetIntFromDict(PyObject *dict, char *key);

/**
 * Gets bool from dictionary dict with key key.
 */
bool GetBoolFromDict(PyObject *dict, char *key);

/**
 * Gets gammu unicode string from dictionary dict with key key.
 */
unsigned char *GetStringFromDict(PyObject *dict, char *key);

/**
 * Copies unicode string from Python dictionary to Gammu string with length checking
 */
int CopyStringFromDict(PyObject *dict, char *key, int len, unsigned char *dest);

/**
 * Gets GSM_DateTime from dictionary dict with key key.
 */
GSM_DateTime GetDateTimeFromDict(PyObject *dict, char *key);

/**
 * Gets GSM_DateTime from dictionary dict with key key.
 */
GSM_DateTime GetDateFromDict(PyObject *dict, char *key);

/**
 * Gets char* from dictionary dict with key key. (doesn't allocate it!)
 */
char *GetCharFromDict(PyObject *dict, char *key);

/**
 * Gets char* with length (may contain NULs) from dictionary dict with key key. (doesn't allocate it!)
 */
char *GetDataFromDict(PyObject *dict, char *key, int *len);

/**
 * Gets GSM_MemoryType from dictionary dict with key key.
 */
GSM_MemoryType GetMemoryTypeFromDict(PyObject *dict, char *key);

/**
 * Coverts cell broadcast message to Python object.
 */
PyObject *CBToPython(GSM_CBMessage *cb);

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
int SMSCFromPython(PyObject* dict, GSM_SMSC *smsc, bool complete);

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
 * Frees SMS Backup.
 */
void FreeSMSBackup(GSM_SMS_Backup *sms);

/**
 * Converts Todo to Python object.
 */
PyObject *TodoToPython(const GSM_ToDoEntry *entry);

/**
 * Converts Todo from Python object.
 */
int TodoFromPython(PyObject *dict, GSM_ToDoEntry *entry, int needs_location);

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
 * Converts file to Python object.
 */
PyObject *FileToPython(GSM_File *file);

/**
 * Converts file from Python object.
 */
int FileFromPython(PyObject *dict, GSM_File *file, bool check);

/**
 * Converst call to Python object.
 */
PyObject *CallToPython(GSM_Call *call);

/**
 * Converst call type to Python object.
 */
char *CallStatusToString(GSM_CallStatus ct);
