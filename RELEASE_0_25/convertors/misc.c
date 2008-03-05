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

/* Miscellaneous conversions, that didn't fit elsewhere */

#include "pyg-config.h"
#include "convertors.h"


GSM_CategoryType StringToCategoryType(const char *s) {
    if (strcmp(s, "ToDo") == 0)             return Category_ToDo;
    else if (strcmp(s, "Phonebook") == 0)   return Category_Phonebook;
    else {
        PyErr_Format(PyExc_ValueError, "Bad value for category type: '%s'", s);
        return 0;
    }
}

GSM_MemoryType StringToMemoryType(const char *s) {
    if (strcmp(s, "ME") == 0)      return MEM_ME;
    else if (strcmp(s, "SM") == 0) return MEM_SM;
    else if (strcmp(s, "ON") == 0) return MEM_ON;
    else if (strcmp(s, "DC") == 0) return MEM_DC;
    else if (strcmp(s, "RC") == 0) return MEM_RC;
    else if (strcmp(s, "MC") == 0) return MEM_MC;
    else if (strcmp(s, "MT") == 0) return MEM_MT;
    else if (strcmp(s, "FD") == 0) return MEM_FD;
    else if (strcmp(s, "VM") == 0) return MEM_VM;
    else if (strcmp(s, "SL") == 0) return MEM_SL;
    else {
        PyErr_Format(PyExc_ValueError, "Bad value for memory type: '%s'", s);
        return 0;
    }
}

char *MemoryTypeToString(GSM_MemoryType t) {
    char *s;

    s = malloc(3);
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }
    s[0] = 0;
    s[2] = 0;
    switch (t) {
        case MEM_ME: s[0] = 'M'; s[1] = 'E'; return s;
        case MEM_SM: s[0] = 'S'; s[1] = 'M'; return s;
        case MEM_ON: s[0] = 'O'; s[1] = 'N'; return s;
        case MEM_DC: s[0] = 'D'; s[1] = 'C'; return s;
        case MEM_RC: s[0] = 'R'; s[1] = 'C'; return s;
        case MEM_MC: s[0] = 'M'; s[1] = 'C'; return s;
        case MEM_MT: s[0] = 'M'; s[1] = 'T'; return s;
        case MEM_FD: s[0] = 'F'; s[1] = 'D'; return s;
        case MEM_VM: s[0] = 'V'; s[1] = 'M'; return s;
        case MEM_SL: s[0] = 'S'; s[1] = 'L'; return s;
    }
    PyErr_Format(PyExc_ValueError, "Bad value for MemoryType from Gammu: '%02x'", t);
    free(s);
    return NULL;
}


GSM_MemoryType GetMemoryTypeFromDict(PyObject *dict, char *key) {
    char *s;
    s = GetCharFromDict(dict, key);
    if (s == NULL) return -1;

    return StringToMemoryType(s);
}

PyObject *CBToPython(GSM_CBMessage *cb) {
    return Py_BuildValue("{s:i,s:s}",
            "Channel", cb->Channel,
            "Text", cb->Text
            );
}

GSM_USSDStatus StringToUSSDStatus(const char *s) {
	if (strcmp(s, "Unknown") == 0) return USSD_Unknown;
	else if (strcmp(s, "NoActionNeeded") == 0) return USSD_NoActionNeeded;
	else if (strcmp(s, "ActionNeeded") == 0) return USSD_ActionNeeded;
	else if (strcmp(s, "Terminated") == 0) return USSD_Terminated;
	else if (strcmp(s, "AnotherClient") == 0) return USSD_AnotherClient;
	else if (strcmp(s, "NotSupported") == 0) return USSD_NotSupported;
	else if (strcmp(s, "Timeout") == 0) return USSD_Timeout;

    PyErr_Format(PyExc_ValueError, "Bad value for USSD Status '%s'", s);
    return 0;
}

char *USSDStatusToString(GSM_USSDStatus type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
		case USSD_Unknown:
			s = strdup("Unknown");
			break;
		case USSD_NoActionNeeded:
			s = strdup("NoActionNeeded");
			break;
		case USSD_ActionNeeded:
			s = strdup("ActionNeeded");
			break;
		case USSD_Terminated:
			s = strdup("Terminated");
			break;
		case USSD_AnotherClient:
			s = strdup("AnotherClient");
			break;
		case USSD_NotSupported:
			s = strdup("NotSupported");
			break;
		case USSD_Timeout:
			s = strdup("Timeout");
			break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for USSD Status from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

PyObject *USSDToPython(GSM_USSDMessage *ussd) {
    PyObject *text;
    PyObject *result;
    char        *status;

    status = USSDStatusToString(ussd->Status);
    if (status == NULL) return NULL;

    text = UnicodeStringToPython(ussd->Text);
    if (text == NULL) {
        free(status);
        return NULL;
    }

    result = Py_BuildValue("{s:s,s:O}",
            "Status", status,
            "Text", text
            );

    Py_DECREF(text);
    free(status);
    return result;
}
