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

#ifndef __pyg_errors_h__
#define __pyg_errors_h__

#include <Python.h>
#include <gammu.h>

extern PyObject *GammuError;
extern PyObject *PyERR_DEVICEOPENERROR;
extern PyObject *PyERR_DEVICELOCKED;
extern PyObject *PyERR_DEVICENOTEXIST;
extern PyObject *PyERR_DEVICEBUSY;
extern PyObject *PyERR_DEVICENOPERMISSION;
extern PyObject *PyERR_DEVICENODRIVER;
extern PyObject *PyERR_DEVICENOTWORK;
extern PyObject *PyERR_DEVICEDTRRTSERROR;
extern PyObject *PyERR_DEVICECHANGESPEEDERROR;
extern PyObject *PyERR_DEVICEWRITEERROR;
extern PyObject *PyERR_DEVICEREADERROR;
extern PyObject *PyERR_DEVICEPARITYERROR;
extern PyObject *PyERR_TIMEOUT;
extern PyObject *PyERR_FRAMENOTREQUESTED;
extern PyObject *PyERR_UNKNOWNRESPONSE;
extern PyObject *PyERR_UNKNOWNFRAME;
extern PyObject *PyERR_UNKNOWNCONNECTIONTYPESTRING;
extern PyObject *PyERR_UNKNOWNMODELSTRING;
extern PyObject *PyERR_SOURCENOTAVAILABLE;
extern PyObject *PyERR_NOTSUPPORTED;
extern PyObject *PyERR_EMPTY;
extern PyObject *PyERR_SECURITYERROR;
extern PyObject *PyERR_INVALIDLOCATION;
extern PyObject *PyERR_NOTIMPLEMENTED;
extern PyObject *PyERR_FULL;
extern PyObject *PyERR_UNKNOWN;
extern PyObject *PyERR_CANTOPENFILE;
extern PyObject *PyERR_MOREMEMORY;
extern PyObject *PyERR_PERMISSION;
extern PyObject *PyERR_EMPTYSMSC;
extern PyObject *PyERR_INSIDEPHONEMENU;
extern PyObject *PyERR_NOTCONNECTED;
extern PyObject *PyERR_WORKINPROGRESS;
extern PyObject *PyERR_PHONEOFF;
extern PyObject *PyERR_FILENOTSUPPORTED;
extern PyObject *PyERR_BUG;
extern PyObject *PyERR_CANCELED;
extern PyObject *PyERR_NEEDANOTHERANSWER;
extern PyObject *PyERR_OTHERCONNECTIONREQUIRED;
extern PyObject *PyERR_WRONGCRC;
extern PyObject *PyERR_INVALIDDATETIME;

int checkError(GSM_StateMachine *s, GSM_Error error, const char *where);
int gammu_create_errors(PyObject *d);

#endif
