/* Backup related conversions */

#include "convertors.h"

/* FIXME: should better check for errors and support all backup features */

PyObject *BackupToPython(GSM_Backup *backup) {
    PyObject            *phone;
    PyObject            *sim;
    PyObject            *calendar;
    PyObject            *todo;
    PyObject            *o;
    PyObject            *dt;
    int                 i;

    phone = PyList_New(0);
    for (i = 0; backup->PhonePhonebook[i] != NULL; i++) {
        o = MemoryEntryToPython(backup->PhonePhonebook[i]);
        if (o == NULL) {
            Py_DECREF(phone);
            return NULL;
        }
        PyList_Append(phone, o);
        Py_DECREF(o);
    }

    sim = PyList_New(0);
    for (i = 0; backup->SIMPhonebook[i] != NULL; i++) {
        o = MemoryEntryToPython(backup->SIMPhonebook[i]);
        if (o == NULL) {
            Py_DECREF(sim);
            return NULL;
        }
        PyList_Append(sim, o);
        Py_DECREF(o);
    }

    todo = PyList_New(0);
    for (i = 0; backup->ToDo[i] != NULL; i++) {
        o = TodoToPython(backup->ToDo[i]);
        if (o == NULL) {
            Py_DECREF(todo);
            return NULL;
        }
        PyList_Append(todo, o);
        Py_DECREF(o);
    }

    calendar = PyList_New(0);
    for (i = 0; backup->Calendar[i] != NULL; i++) {
        o = CalendarToPython(backup->Calendar[i]);
        if (o == NULL) {
            Py_DECREF(calendar);
            return NULL;
        }
        PyList_Append(calendar, o);
        Py_DECREF(o);
    }

    if (backup->DateTimeAvailable) {
        dt = BuildPythonDateTime(&backup->DateTime);
        if (dt == NULL) {
            PyErr_Clear();
            Py_INCREF(Py_None);
            dt = Py_None;
        }
    } else {
        Py_INCREF(Py_None);
        dt = Py_None;
    }

    o = Py_BuildValue("{s:s,s:s,s:s,s:O,s:O,s:O,s:O,s:O}",
            "IMEI",             backup->IMEI,
            "Model",            backup->Model,
            "Creator",          backup->Creator,
            "PhonePhonebook",   phone,
            "SIMPhonebook",     sim,
            "Calendar",         calendar,
            "ToDo",             todo,
            "DateTime",         dt);
    
    Py_DECREF(phone);
    Py_DECREF(sim);
    Py_DECREF(calendar);
    Py_DECREF(todo);
    Py_DECREF(dt);

    return o;
}

#define GET_ONE(key, data, maxlen, type, conv)\
    o = PyDict_GetItemString(dict, key);\
    if (o == NULL) {\
        backup->data[0] = NULL;\
    } else if (o == Py_None) {\
        backup->data[0] = NULL;\
    } else if (!PyList_Check(o)) {\
        PyErr_Format(PyExc_ValueError, "%s not a list!", key);\
        return 0;\
    } else {\
        len = PyList_Size(o);\
        if (len > maxlen) {\
            printf("Warning: too many entries in %s, truncating to %d from %d!\n", key, maxlen, len);\
            len = maxlen;\
        }\
        for (i = 0; i < len; i++) {\
            backup->data[i] = (type *)malloc(sizeof(type));\
            x = PyList_GetItem(o, i);\
            if (x == NULL) return 0;\
            if (!conv) {\
                return 0;\
            }\
        }\
        backup->data[len] = NULL;\
    }
            
int BackupFromPython(PyObject *dict, GSM_Backup *backup) {
    PyObject            *o;
    PyObject            *x;
    int                 len;
    int                 i;
    char                *s;
   
    if (!PyDict_Check(dict)) {
        PyErr_SetString(PyExc_ValueError, "Backup not a dictionary!");
        return 0;
    }

    GSM_ClearBackup(backup);

    backup->IMEI[0] = 0;
    s = GetCharFromDict(dict, "IMEI");
    if (s == 0) {
        PyErr_Clear();
    } else {
        len = strlen(s);
        if (len > MAX_IMEI_LENGTH - 1) len = MAX_IMEI_LENGTH - 1;
        strncat(backup->IMEI, s, len);
    }

    backup->Model[0] = 0;
    s = GetCharFromDict(dict, "Model");
    if (s == 0) {
        PyErr_Clear();
    } else {
        len = strlen(s);
        if (len > MAX_MODEL_LENGTH + MAX_VERSION_LENGTH - 1) len = MAX_MODEL_LENGTH + MAX_VERSION_LENGTH - 1;
        strncat(backup->Model, s, len);
    }

    backup->Creator[0] = 0;
    s = GetCharFromDict(dict, "Creator");
    if (s == 0) {
        PyErr_Clear();
        strcat(backup->Creator[0], "python-gammu " PYTHON_GAMMU_VERSION);
    } else {
        len = strlen(s);
        if (len > 50 - 1) len = 50 - 1;
        strncat(backup->Creator, s, len);
    }

    GET_ONE("PhonePhonebook", PhonePhonebook, GSM_BACKUP_MAX_PHONEPHONEBOOK, GSM_MemoryEntry, MemoryEntryFromPython(x, backup->PhonePhonebook[i], 1))
    GET_ONE("SIMPhonebook", SIMPhonebook, GSM_BACKUP_MAX_SIMPHONEBOOK, GSM_MemoryEntry, MemoryEntryFromPython(x, backup->SIMPhonebook[i], 1))
    GET_ONE("ToDo", ToDo, GSM_MAXCALENDARTODONOTES, GSM_ToDoEntry, TodoFromPython(x, backup->ToDo[i], 1))
    GET_ONE("Calendar", Calendar, GSM_MAXCALENDARTODONOTES, GSM_CalendarEntry, CalendarFromPython(x, backup->Calendar[i], 1))

    return 1;
}
