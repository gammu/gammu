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
    if (strcmp(s, "ME") == 0)      return MEM_ME;
    else if (strcmp(s, "SM") == 0) return MEM_SM;
    else if (strcmp(s, "ON") == 0) return MEM_ON;
    else if (strcmp(s, "DC") == 0) return MEM_DC;
    else if (strcmp(s, "RC") == 0) return MEM_RC;
    else if (strcmp(s, "MC") == 0) return MEM_MC;
    else if (strcmp(s, "MT") == 0) return MEM_MT;
    else if (strcmp(s, "FD") == 0) return MEM_FD;
    else if (strcmp(s, "VM") == 0) return MEM_VM;
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


