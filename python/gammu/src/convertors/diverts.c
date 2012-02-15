/*
 * python-gammu - Phone communication libary
 * Copyright © 2003 - 2012 Michal Čihař
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

/* Call diverts related conversions */

#include "convertors.h"


GSM_Divert_CallTypes DivertCallTypeFromString(const char *str)
{
    if (strcasecmp(str, "Voice") == 0) {
        return GSM_DIVERT_VoiceCalls;
    } else if (strcasecmp(str, "Fax") == 0) {
        return GSM_DIVERT_FaxCalls;
    } else if (strcasecmp(str, "Data") == 0) {
        return GSM_DIVERT_DataCalls;
    } else if (strcasecmp(str, "All") == 0) {
        return GSM_DIVERT_AllCalls;
    } else {
        PyErr_Format(PyExc_ValueError,
                 "Bad value for GSM_Divert_CallTypes: %s", str);
        return 0;
    }
}

char *DivertCallTypeToString(GSM_Divert_CallTypes ct)
{
    char *s = NULL;

    switch (ct) {
        case GSM_DIVERT_VoiceCalls:
            s = strdup("Voice");
            break;
        case GSM_DIVERT_FaxCalls:
            s = strdup("Fax");
            break;
        case GSM_DIVERT_DataCalls:
            s = strdup("Data");
            break;
        case GSM_DIVERT_AllCalls:
            s = strdup("All");
            break;
    }

    if (s == NULL) {
        PyErr_Format(PyExc_ValueError,
                 "Bad value for GSM_Divert_CallTypes from Gammu: '%d'", ct);
        return NULL;
    }

    return s;
}

GSM_Divert_DivertTypes DivertTypeFromString(const char *str)
{
    if (strcasecmp(str, "Busy") == 0) {
        return GSM_DIVERT_Busy;
    } else if (strcasecmp(str, "NoAnswer") == 0) {
        return GSM_DIVERT_NoAnswer;
    } else if (strcasecmp(str, "OutOfReach") == 0) {
        return GSM_DIVERT_OutOfReach;
    } else if (strcasecmp(str, "AllTypes") == 0) {
        return GSM_DIVERT_AllTypes;
    } else {
        PyErr_Format(PyExc_ValueError,
                 "Bad value for GSM_Divert_DivertTypes: %s", str);
        return 0;
    }
}

char *DivertTypeToString(GSM_Divert_DivertTypes ct)
{
    char *s = NULL;

    switch (ct) {
        case GSM_DIVERT_Busy:
            s = strdup("Busy");
            break;
        case GSM_DIVERT_NoAnswer:
            s = strdup("NoAnswer");
            break;
        case GSM_DIVERT_OutOfReach:
            s = strdup("OutOfReach");
            break;
        case GSM_DIVERT_AllTypes:
            s = strdup("AllTypes");
            break;
    }

    if (s == NULL) {
        PyErr_Format(PyExc_ValueError,
                 "Bad value for GSM_Divert_DivertTypes from Gammu: '%d'", ct);
        return NULL;
    }

    return s;
}

PyObject *CallDivertsToPython(GSM_MultiCallDivert *cd)
{
    int i;
    PyObject *ret, *num, *entry;
    char *dt, *ct;

    ret = PyList_New(0);

    for (i = 0; i < cd->EntriesNum; i++) {
        num = UnicodeStringToPython(cd->Entries[i].Number);
        if (num == NULL) {
            Py_DECREF(ret);
            return NULL;
        }
        dt = DivertTypeToString(cd->Entries[i].DivertType);
        if (dt == NULL) {
            Py_DECREF(ret);
            Py_DECREF(num);
            return NULL;
        }
        ct = DivertCallTypeToString(cd->Entries[i].CallType);
        if (dt == NULL) {
            Py_DECREF(ret);
            Py_DECREF(num);
            free(dt);
            return NULL;
        }
        entry = Py_BuildValue("{s:s,s:s,s:O,s:i}",
            "DivertType", dt,
            "CallType", ct,
            "Number", num,
            "Timeout", cd->Entries[i].Timeout);

        Py_DECREF(num);
        free(dt);
        free(ct);

        if (entry == NULL) {
            Py_DECREF(ret);
            return NULL;
        }
        if (PyList_Append(ret, entry) != 0) {
            Py_DECREF(ret);
            Py_DECREF(entry);
            return NULL;
        }
        Py_DECREF(entry);
    }

    return ret;
}
