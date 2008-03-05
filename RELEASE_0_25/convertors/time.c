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

/* DateTime conversions */

#include "pyg-config.h"
#include "convertors.h"

PyObject *BuildPythonDateTime(const GSM_DateTime *dt) {
    PyObject *pModule;
    PyObject *result;

    if (dt->Year == 0) {
        Py_RETURN_NONE;
    }

    /* import datetime */
    pModule = PyImport_ImportModule("datetime");
    if (pModule == NULL) return NULL;

    /* create datetime object */
    result = PyObject_CallMethod(pModule,
            "datetime",
            "iiiiii",
            dt->Year,
            dt->Month,
            dt->Day,
            dt->Hour,
            dt->Minute,
            dt->Second
            );

    Py_DECREF(pModule);

    return result;
}

PyObject *BuildPythonTime(const GSM_DateTime *dt) {
    PyObject *pModule;
    PyObject *result;

    /* import datetime */
    pModule = PyImport_ImportModule("datetime");
    if (pModule == NULL) return NULL;

    /* create datetime object */
    result = PyObject_CallMethod(pModule,
            "time",
            "iii",
            dt->Hour,
            dt->Minute,
            dt->Second
            );

    Py_DECREF(pModule);

    return result;
}

int BuildGSMDateTime(PyObject *pydt, GSM_DateTime *dt) {
    PyObject *result;
    static GSM_DateTime    nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;

    if (pydt == Py_None) return 1;

    result = PyObject_GetAttrString(pydt, "year");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "year");
        return 0;
    }
    dt->Year = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "month");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "month");
        return 0;
    }
    dt->Month = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "day");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "day");
        return 0;
    }
    dt->Day = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "hour");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "hour");
        return 0;
    }
    dt->Hour = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "minute");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "minute");
        return 0;
    }
    dt->Minute = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "second");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "second");
        return 0;
    }
    dt->Second = PyInt_AsLong(result);

    return 1;
}

int BuildGSMDate(PyObject *pydt, GSM_DateTime *dt) {
    PyObject *result;
    static GSM_DateTime    nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;

    if (pydt == Py_None) return 1;

    result = PyObject_GetAttrString(pydt, "year");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "year");
        return 0;
    }
    dt->Year = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "month");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "month");
        return 0;
    }
    dt->Month = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "day");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "day");
        return 0;
    }
    dt->Day = PyInt_AsLong(result);
    return 1;
}

int BuildGSMTime(PyObject *pydt, GSM_DateTime *dt) {
    PyObject *result;
    static GSM_DateTime    nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;

    if (pydt == Py_None) return 1;

    result = PyObject_GetAttrString(pydt, "hour");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "hour");
        return 0;
    }
    dt->Hour = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "minute");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "minute");
        return 0;
    }
    dt->Minute = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "second");
    if (result == NULL) return 0;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "second");
        return 0;
    }
    dt->Second = PyInt_AsLong(result);

    return 1;
}

