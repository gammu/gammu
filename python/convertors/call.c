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

/* Call related conversions */

#include "pyg-config.h"
#include "convertors.h"

char *CallStatusToString(GSM_CallStatus ct) {
    char *err = "Err";
    char *s = err;

    switch (ct) {
        case GSM_CALL_IncomingCall: s = strdup("IncomingCall"); break;
        case GSM_CALL_OutgoingCall: s = strdup("OutgoingCall"); break;
        case GSM_CALL_CallStart: s = strdup("CallStart"); break;
        case GSM_CALL_CallEnd: s = strdup("CallEnd"); break;
        case GSM_CALL_CallRemoteEnd: s = strdup("CallRemoteEnd"); break;
        case GSM_CALL_CallLocalEnd: s = strdup("CallLocalEnd"); break;
        case GSM_CALL_CallEstablished: s = strdup("CallEstablished"); break;
        case GSM_CALL_CallHeld: s = strdup("CallHeld"); break;
        case GSM_CALL_CallResumed: s = strdup("CallResumed"); break;
        case GSM_CALL_CallSwitched: s = strdup("CallSwitched"); break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for CallStatus from Gammu: '%d'", ct);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

PyObject *CallToPython(GSM_Call *call) {
    PyObject *number;
    PyObject *result;
    char *status;

    number = UnicodeStringToPython(call->PhoneNumber);
    if (number == NULL) {
        return NULL;
    }

    status = CallStatusToString(call->Status);
    if (status == NULL) {
        Py_DECREF(number);
        return NULL;
    }

    if (call->CallIDAvailable) {
        result = Py_BuildValue("{s:s,s:O,s:i,s:i}",
                "Status", status,
                "Number", number,
                "CallID", call->CallID,
                "StatusCode", call->StatusCode);
    } else {
        result = Py_BuildValue("{s:s,s:O,s:i}",
                "Status", status,
                "Number", number,
                "StatusCode", call->StatusCode);
    }
    Py_DECREF(number);
    free(status);
    return result;
}
