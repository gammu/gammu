/*
 * python-gammu - Phone communication libary
 * Copyright © 2003 - 2009 Michal Čihař
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

/* MMS and WAP related conversions */
#include "convertors.h"


PyObject *MMSIndicatorToPython(GSM_MMSIndicator *mms) {
    return Py_BuildValue("{s:s,s:s,s:s}",
            "Address",          mms->Address,
            "Title",            mms->Title,
            "Sender",           mms->Sender);
}

int MMSIndicatorFromPython(PyObject* dict, GSM_MMSIndicator *mms) {
    char    *s;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "MMSIndicator is not a dictionary");
        return 0;
    }

    memset(mms, 0, sizeof(mms));

    s = GetCStringFromDict(dict, "Address");
    if (strlen(s) > 499) {
        PyErr_Format(PyExc_ValueError, "Address too long!");
        return 0;
    }
    strcpy(s, mms->Address);

    s = GetCStringFromDict(dict, "Title");
    if (strlen(s) > 499) {
        PyErr_Format(PyExc_ValueError, "Title too long!");
        return 0;
    }
    strcpy(s, mms->Title);

    s = GetCStringFromDict(dict, "Sender");
    if (strlen(s) > 499) {
        PyErr_Format(PyExc_ValueError, "Sender too long!");
        return 0;
    }
    strcpy(s, mms->Sender);

    return 1;
}

PyObject *WAPBookmarkToPython(GSM_WAPBookmark *wap) {
    PyObject            *ret;
    Py_UNICODE          *title, *address;

    title = strGammuToPython(wap->Title);
    if (title  == NULL) return NULL;

    address = strGammuToPython(wap->Address);
    if (address  == NULL) return NULL;

    ret = Py_BuildValue("{s:s,s:s,s:i}",
            "Address",          address,
            "Title",            title,
            "Location",         wap->Location);

    free(title);
    free(address);

    return ret;
}

int WAPBookmarkFromPython(PyObject* dict, GSM_WAPBookmark *wap) {
    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "WAPBookmark is not a dictionary");
        return 0;
    }

    memset(wap, 0, sizeof(wap));

    wap->Location = GetIntFromDict(dict, "Location");
    if (wap->Location == INT_INVALID) return 0;

    if (!CopyStringFromDict(dict, "Address", 255, wap->Address))
        return 0;

    if (!CopyStringFromDict(dict, "Title", 50, wap->Title))
        return 0;

    return 1;
}

