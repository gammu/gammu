/* Python includes */
#include <Python.h>

/* Gammu includes */
#include <gammu.h>

#include <limits.h>

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
 * Creates GSM_DateTime from Python datetime.time.
 */
int BuildGSMTime(PyObject *pydt, GSM_DateTime *dt);

/**
 * Convert Python unicode string to Gammu unicode string.
 */
unsigned char *strPythonToGammu(const Py_UNICODE *src);

/**
 * Convert Gammu unicode string to unicode string that can be read by Python.
 */
Py_UNICODE *strGammuToPython(const unsigned char *src);

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
int SMSCFromPython(PyObject* dict, GSM_SMSC *smsc);

/**
 * Converts SMS to python object.
 */
PyObject *SMSToPython(GSM_SMSMessage *sms);

/**
 * Creates SMS from python object.
 */
int SMSFromPython(PyObject *dict, GSM_SMSMessage *sms, int needslocation, int needsfolder);

/**
 * Converts MultiSMS to list of python object.
 */
PyObject *MultiSMSToPython(GSM_MultiSMSMessage *sms);

/**
 * Converts Todo to Python object.
 */
PyObject *TodoToPython(const GSM_ToDoEntry *entry);

/**
 * Converts Todo from Python object.
 */
int TodoFromPython(PyObject *dict, GSM_ToDoEntry *entry, int needs_location);
