/*
 * python-gammu - Phone communication libary
 * Copyright © 2003 - 2008 Michal Čihař
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

/* Basic getters from Python objects */

#include "convertors.h"
#include "misc.h"

bool GetBoolFromDict(PyObject *dict, const char *key) {
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

int GetIntFromDict(PyObject *dict, const char *key) {
    PyObject        *o;
    char            *s;
    int             i;

    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return INT_INVALID;
    }

    if (PyLong_Check(o)) {
        /* Well we loose here something, but it is intentional :-) */
        return PyLong_AsLongLong(o);
    }

    if (PyInt_Check(o)) {
        return PyInt_AsLong(o);
    }

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

char *GetCStringLengthFromDict(PyObject *dict, const char *key, Py_ssize_t *length) {
    PyObject        *o;
    char *result, *data;

    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }
    PyString_AsStringAndSize(o, &data, length);
    result = (char *)malloc(*length);
    if (result == NULL) {
        PyErr_Format(PyExc_ValueError, "Failed to allocate memory!");
        return NULL;
    }
    memcpy(result, data, *length);

	return result;
}

char *GetCStringFromDict(PyObject *dict, const char *key) {
    PyObject        *o;

    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }

	return PyString_AsString(o);
}

unsigned char *GetStringFromDict(PyObject *dict, const char *key) {
    PyObject        *o;

    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }

	return StringPythonToGammu(o);
}

int CopyStringFromDict(PyObject *dict, const char *key, size_t len, unsigned char *dest) {
    unsigned char   *s;

    s = GetStringFromDict(dict, key);
    if (s == NULL) return 0;
    if (UnicodeLength(s) > len) {
        pyg_warning("Truncating text %s to %d chars!\n", key, len);
        s[2*len]        = 0;
        s[(2*len) + 1]  = 0;
    }
    CopyUnicodeString(dest, s);
    free(s);
    return 1;
}

GSM_DateTime GetDateTimeFromDict(PyObject *dict, const char *key) {
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

GSM_DateTime GetDateFromDict(PyObject *dict, const char *key) {
    PyObject        *o;
    GSM_DateTime    dt;

    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        dt.Year = -1;
        return dt;
    }

    if (!BuildGSMDate(o, &dt)) {
        dt.Year = -1;
    }
    return dt;
}

char *GetCharFromDict(PyObject *dict, const char *key) {
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

char *GetDataFromDict(PyObject *dict, const char *key, Py_ssize_t *len) {
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
