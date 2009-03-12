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

/* Miscellaneous conversions, that didn't fit elsewhere */

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
    GSM_MemoryType type;

    type = GSM_StringToMemoryType(s);

    if (type == 0) {
        PyErr_Format(PyExc_ValueError, "Bad value for memory type: '%s'", s);
    }

    return type;
}

char *MemoryTypeToString(GSM_MemoryType t) {
    char *s;

    s = strdup(GSM_MemoryTypeToString(t));
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }
    if (strcmp("XX", s) == 0) {
        PyErr_Format(PyExc_ValueError, "Bad value for MemoryType from Gammu: '%02x'", t);
        free(s);
        s = NULL;
    }
    return s;
}


GSM_MemoryType GetMemoryTypeFromDict(PyObject *dict, const char *key) {
    char *s;
    s = GetCharFromDict(dict, key);
    if (s == NULL) return ENUM_INVALID;

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
    char *s = NULL;

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

    if (s == NULL) {
        PyErr_Format(PyExc_ValueError, "Bad value for USSD Status from Gammu: '%d'", type);
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
