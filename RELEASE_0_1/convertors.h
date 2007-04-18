/* Python includes */
#include <Python.h>

/* Gammu includes */
#include <gammu.h>

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
bool BuildGSMDateTime(PyObject *pydt, GSM_DateTime *dt);

/**
 * Creates GSM_DateTime from Python datetime.time.
 */
bool BuildGSMTime(PyObject *pydt, GSM_DateTime *dt);

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
PyObject *BuildPythonMemoryEntry(const GSM_MemoryEntry *entry);

/**
 * Fills GSM_MemoryEntry from Pythons dictionary:.
 */
bool FillGSMMemoryEntry(PyObject *dict, GSM_MemoryEntry *entry, bool needs_location);
