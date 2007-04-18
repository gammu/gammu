/* Basic getters from Python objects */

#include "convertors.h"

bool GetBoolFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    char            *s;
    int             i;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return BOOL_INVALID;
    }
    
    if (!PyBool_Check(o)) {
        if (PyInt_Check(o)) {
            i = PyInt_AsLong(o);
            if (i == 0) return false;
            else return true;
        }
        if (PyString_Check(o)) {
            s = PyString_AsString(o);
            if (isdigit(s[0])) {
                i = atoi(s);
                if (i == 0) return false;
                else return true;
            } else {
                PyErr_Format(PyExc_ValueError, "Value of '%s' doesn't seem to be bool", key);
                return BOOL_INVALID;
            }
        }

        PyErr_Format(PyExc_ValueError, "Value of '%s' doesn't seem to be bool", key);
        return BOOL_INVALID;
    }

    if (Py_False == o) return false;
    else if (Py_True == o) return true;

    PyErr_Format(PyExc_ValueError, "Value of '%s' doesn't seem to be bool", key);
    return BOOL_INVALID;
}

int GetIntFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    char            *s;
    int             i;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return INT_INVALID;
    }
    
    if (!PyInt_Check(o)) {
        if (PyString_Check(o)) {
            s = PyString_AsString(o);
            if (isdigit(s[0])) {
                i = atoi(s);
                return i;
            } else {
                PyErr_Format(PyExc_ValueError, "Value of '%s' doesn't seem to be integer", key);
            }
        }

        PyErr_Format(PyExc_ValueError, "Value of '%s' doesn't seem to be integer", key);
        return INT_INVALID;
    }

    return PyInt_AsLong(o);
}

unsigned char *GetStringFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    PyObject        *u;
    Py_UNICODE      *ps;
    unsigned char   *gs;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }

    u = PyObject_Unicode(o);
    if (u == NULL) {
        PyErr_Format(PyExc_ValueError, "Value for key '%s' can not be converted to unicode object", key);
        return NULL;
    }

    ps = PyUnicode_AsUnicode(u);
    if (ps == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get unicode value for key %s", key);
        return NULL;
    }
    gs = strPythonToGammu(ps);
    Py_DECREF(u);
    return gs;
}

int CopyStringFromDict(PyObject *dict, char *key, int len, unsigned char *dest) {
    unsigned char   *s;

    s = GetStringFromDict(dict, key);
    if (s == NULL) return 0;
    if (UnicodeLength(s) > len) {
        printf("WARNING: Truncating text to %d chars!", len);
        s[2*len]        = 0;
        s[(2*len) + 1]  = 0;
    }
    CopyUnicodeString(dest, s);
    free(s);
    return 1;
}

GSM_DateTime GetDateTimeFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    GSM_DateTime    dt;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        dt.Year = -1;
        return dt;
    }

    if (!BuildGSMDateTime(o, &dt)) {
        dt.Year = -1;
    }
    return dt;
}

char *GetCharFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    char            *ps;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }

    ps = PyString_AsString(o);
    if (ps == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key %s", key);
        return NULL;
    }
    return ps;
}

char *GetDataFromDict(PyObject *dict, char *key, int *len) {
    PyObject        *o;
    char            *ps;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }
    if (PyString_AsStringAndSize(o, &ps, len) != 0) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key %s", key);
        return NULL;
    }
    return ps;
}
