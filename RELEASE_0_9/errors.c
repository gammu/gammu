/*
 * python-gammu - Phone communication libary
 * Copyright (c) 2003 - 2005 Michal Čihař
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
 */

#include "errors.h"

PyObject *GammuError;
PyObject *PyERR_DEVICEOPENERROR;
PyObject *PyERR_DEVICELOCKED;
PyObject *PyERR_DEVICENOTEXIST;
PyObject *PyERR_DEVICEBUSY;
PyObject *PyERR_DEVICENOPERMISSION;
PyObject *PyERR_DEVICENODRIVER;
PyObject *PyERR_DEVICENOTWORK;
PyObject *PyERR_DEVICEDTRRTSERROR;
PyObject *PyERR_DEVICECHANGESPEEDERROR;
PyObject *PyERR_DEVICEWRITEERROR;
PyObject *PyERR_DEVICEREADERROR;
PyObject *PyERR_DEVICEPARITYERROR;
PyObject *PyERR_TIMEOUT;
PyObject *PyERR_FRAMENOTREQUESTED;
PyObject *PyERR_UNKNOWNRESPONSE;
PyObject *PyERR_UNKNOWNFRAME;
PyObject *PyERR_UNKNOWNCONNECTIONTYPESTRING;
PyObject *PyERR_UNKNOWNMODELSTRING;
PyObject *PyERR_SOURCENOTAVAILABLE;
PyObject *PyERR_NOTSUPPORTED;
PyObject *PyERR_EMPTY;
PyObject *PyERR_SECURITYERROR;
PyObject *PyERR_INVALIDLOCATION;
PyObject *PyERR_NOTIMPLEMENTED;
PyObject *PyERR_FULL;
PyObject *PyERR_UNKNOWN;
PyObject *PyERR_CANTOPENFILE;
PyObject *PyERR_MOREMEMORY;
PyObject *PyERR_PERMISSION;
PyObject *PyERR_EMPTYSMSC;
PyObject *PyERR_INSIDEPHONEMENU;
PyObject *PyERR_NOTCONNECTED;
PyObject *PyERR_WORKINPROGRESS;
PyObject *PyERR_PHONEOFF;
PyObject *PyERR_FILENOTSUPPORTED;
PyObject *PyERR_BUG;
PyObject *PyERR_CANCELED;
PyObject *PyERR_NEEDANOTHERANSWER;
PyObject *PyERR_OTHERCONNECTIONREQUIRED;
PyObject *PyERR_WRONGCRC;
PyObject *PyERR_INVALIDDATETIME;
PyObject *PyERR_MEMORY;
PyObject *PyERR_INVALIDDATA;
PyObject *PyERR_FILEALREADYEXIST;
PyObject *PyERR_FILENOTEXIST;
PyObject *PyERR_SHOULDBEFOLDER;
PyObject *PyERR_SHOULDBEFILE;
PyObject *PyERR_NOSIM;

int checkError(GSM_StateMachine *s, GSM_Error error, char *where) {
    PyObject    *val;
    PyObject    *err = GammuError;
    char        *msg;

    switch (error) {
        case ERR_NONE: return 1;
        case ERR_DEVICEOPENERROR: err = PyERR_DEVICEOPENERROR; break;
        case ERR_DEVICELOCKED: err = PyERR_DEVICELOCKED; break;
        case ERR_DEVICENOTEXIST: err = PyERR_DEVICENOTEXIST; break;
        case ERR_DEVICEBUSY: err = PyERR_DEVICEBUSY; break;
        case ERR_DEVICENOPERMISSION: err = PyERR_DEVICENOPERMISSION; break;
        case ERR_DEVICENODRIVER: err = PyERR_DEVICENODRIVER; break;
        case ERR_DEVICENOTWORK: err = PyERR_DEVICENOTWORK; break;
        case ERR_DEVICEDTRRTSERROR: err = PyERR_DEVICEDTRRTSERROR; break;
        case ERR_DEVICECHANGESPEEDERROR: err = PyERR_DEVICECHANGESPEEDERROR; break;
        case ERR_DEVICEWRITEERROR: err = PyERR_DEVICEWRITEERROR; break;
        case ERR_DEVICEREADERROR: err = PyERR_DEVICEREADERROR; break;
        case ERR_DEVICEPARITYERROR: err = PyERR_DEVICEPARITYERROR; break;
        case ERR_TIMEOUT: err = PyERR_TIMEOUT; break;
        case ERR_FRAMENOTREQUESTED: err = PyERR_FRAMENOTREQUESTED; break;
        case ERR_UNKNOWNRESPONSE: err = PyERR_UNKNOWNRESPONSE; break;
        case ERR_UNKNOWNFRAME: err = PyERR_UNKNOWNFRAME; break;
        case ERR_UNKNOWNCONNECTIONTYPESTRING: err = PyERR_UNKNOWNCONNECTIONTYPESTRING; break;
        case ERR_UNKNOWNMODELSTRING: err = PyERR_UNKNOWNMODELSTRING; break;
        case ERR_SOURCENOTAVAILABLE: err = PyERR_SOURCENOTAVAILABLE; break;
        case ERR_NOTSUPPORTED: err = PyERR_NOTSUPPORTED; break;
        case ERR_EMPTY: err = PyERR_EMPTY; break;
        case ERR_SECURITYERROR: err = PyERR_SECURITYERROR; break;
        case ERR_INVALIDLOCATION: err = PyERR_INVALIDLOCATION; break;
        case ERR_NOTIMPLEMENTED: err = PyERR_NOTIMPLEMENTED; break;
        case ERR_FULL: err = PyERR_FULL; break;
        case ERR_UNKNOWN: err = PyERR_UNKNOWN; break;
        case ERR_CANTOPENFILE: err = PyERR_CANTOPENFILE; break;
        case ERR_MOREMEMORY: err = PyERR_MOREMEMORY; break;
        case ERR_PERMISSION: err = PyERR_PERMISSION; break;
        case ERR_EMPTYSMSC: err = PyERR_EMPTYSMSC; break;
        case ERR_INSIDEPHONEMENU: err = PyERR_INSIDEPHONEMENU; break;
        case ERR_NOTCONNECTED: err = PyERR_NOTCONNECTED; break;
        case ERR_WORKINPROGRESS: err = PyERR_WORKINPROGRESS; break;
        case ERR_PHONEOFF: err = PyERR_PHONEOFF; break;
        case ERR_FILENOTSUPPORTED: err = PyERR_FILENOTSUPPORTED; break;
        case ERR_BUG: err = PyERR_BUG; break;
        case ERR_CANCELED: err = PyERR_CANCELED; break;
        case ERR_NEEDANOTHERANSWER: err = PyERR_NEEDANOTHERANSWER; break;
        case ERR_OTHERCONNECTIONREQUIRED: err = PyERR_OTHERCONNECTIONREQUIRED; break;
        case ERR_WRONGCRC: err = PyERR_WRONGCRC; break;
        case ERR_INVALIDDATETIME: err = PyERR_INVALIDDATETIME; break;
        case ERR_MEMORY: err = PyERR_MEMORY; break;
        case ERR_INVALIDDATA: err = PyERR_INVALIDDATA; break;
        case ERR_FILEALREADYEXIST: err = PyERR_FILEALREADYEXIST; break;
        case ERR_FILENOTEXIST: err = PyERR_FILENOTEXIST; break;
        case ERR_SHOULDBEFOLDER: err = PyERR_SHOULDBEFOLDER; break;
        case ERR_SHOULDBEFILE: err = PyERR_SHOULDBEFILE; break;
        case ERR_NOSIM: err = PyERR_NOSIM; break;
    }


    if (s == NULL) {
        msg = print_error(error, di.df, NULL);
    } else {
        msg = print_error(error, s->di.df, s->msg);
    }

    val = Py_BuildValue("{s:s,s:s,s:i}",
            "Text", msg,
            "Where", where,
            "Code", error);

    if (val == NULL) {
        PyErr_Format(err, "GSM Error %d (%s) in %s", error, msg, where);
    } else {
        PyErr_SetObject(err, val);
    }


    return 0;
}

#define CREATE_ERROR(x) \
    Py##x = PyErr_NewException("gammu." #x, GammuError, NULL);\
    if (Py##x == NULL)\
        return 0;\
    help_text = PyString_FromFormat("Exception corresponding to gammu error " #x ".\nVerbose error description: %s", print_error(x, NULL, 0)); \
    PyObject_SetAttrString(Py##x, "__doc__", help_text); \
    Py_DECREF(help_text); \
    PyDict_SetItemString(d, "" #x, Py##x); \
    Py_DECREF(Py##x);

int gammu_create_errors(PyObject *d) {
    PyObject *help_text;

    GammuError = PyErr_NewException("gammu.GSMError", NULL, NULL);
    if (GammuError == NULL)
        return 0;
    help_text = PyString_FromString("Generic class as parent for all gammu exceptions. This is never raised directly.");
    PyObject_SetAttrString(GammuError, "__doc__", help_text);
    Py_DECREF(help_text);
    PyDict_SetItemString(d, "GSMError", GammuError);
    Py_DECREF(GammuError);

    CREATE_ERROR(ERR_DEVICEOPENERROR)
    CREATE_ERROR(ERR_DEVICELOCKED)
    CREATE_ERROR(ERR_DEVICENOTEXIST)
    CREATE_ERROR(ERR_DEVICEBUSY)
    CREATE_ERROR(ERR_DEVICENOPERMISSION)
    CREATE_ERROR(ERR_DEVICENODRIVER)
    CREATE_ERROR(ERR_DEVICENOTWORK)
    CREATE_ERROR(ERR_DEVICEDTRRTSERROR)
    CREATE_ERROR(ERR_DEVICECHANGESPEEDERROR)
    CREATE_ERROR(ERR_DEVICEWRITEERROR)
    CREATE_ERROR(ERR_DEVICEREADERROR)
    CREATE_ERROR(ERR_DEVICEPARITYERROR)
    CREATE_ERROR(ERR_TIMEOUT)
    CREATE_ERROR(ERR_FRAMENOTREQUESTED)
    CREATE_ERROR(ERR_UNKNOWNRESPONSE)
    CREATE_ERROR(ERR_UNKNOWNFRAME)
    CREATE_ERROR(ERR_UNKNOWNCONNECTIONTYPESTRING)
    CREATE_ERROR(ERR_UNKNOWNMODELSTRING)
    CREATE_ERROR(ERR_SOURCENOTAVAILABLE)
    CREATE_ERROR(ERR_NOTSUPPORTED)
    CREATE_ERROR(ERR_EMPTY)
    CREATE_ERROR(ERR_SECURITYERROR)
    CREATE_ERROR(ERR_INVALIDLOCATION)
    CREATE_ERROR(ERR_NOTIMPLEMENTED)
    CREATE_ERROR(ERR_FULL)
    CREATE_ERROR(ERR_UNKNOWN)
    CREATE_ERROR(ERR_CANTOPENFILE)
    CREATE_ERROR(ERR_MOREMEMORY)
    CREATE_ERROR(ERR_PERMISSION)
    CREATE_ERROR(ERR_EMPTYSMSC)
    CREATE_ERROR(ERR_INSIDEPHONEMENU)
    CREATE_ERROR(ERR_NOTCONNECTED)
    CREATE_ERROR(ERR_WORKINPROGRESS)
    CREATE_ERROR(ERR_PHONEOFF)
    CREATE_ERROR(ERR_FILENOTSUPPORTED)
    CREATE_ERROR(ERR_BUG)
    CREATE_ERROR(ERR_CANCELED)
    CREATE_ERROR(ERR_NEEDANOTHERANSWER)
    CREATE_ERROR(ERR_OTHERCONNECTIONREQUIRED)
    CREATE_ERROR(ERR_WRONGCRC)
    CREATE_ERROR(ERR_INVALIDDATETIME)
    CREATE_ERROR(ERR_MEMORY)
    CREATE_ERROR(ERR_INVALIDDATA)
    CREATE_ERROR(ERR_FILEALREADYEXIST)
    CREATE_ERROR(ERR_FILENOTEXIST)
    CREATE_ERROR(ERR_SHOULDBEFOLDER)
    CREATE_ERROR(ERR_SHOULDBEFILE)
    CREATE_ERROR(ERR_NOSIM)

    return 1;
}
