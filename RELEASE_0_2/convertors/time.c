/* DateTime conversions */

#include "convertors.h"

PyObject *BuildPythonDateTime(const GSM_DateTime *dt) {
    PyObject *pModule;
    PyObject *result;
       
    if (dt->Year == 0) {
        Py_INCREF(Py_None);
        return Py_None;
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
	static GSM_DateTime	nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;
       
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

int BuildGSMTime(PyObject *pydt, GSM_DateTime *dt) {
    PyObject *result;
	static GSM_DateTime	nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;
       
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

