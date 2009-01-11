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

/* ToDo related conversions */

#include "convertors.h"
#include "misc.h"

char *TodoPriorityToString(GSM_ToDo_Priority p) {
    char *err = "Err";
    char *s = err;

    switch (p) {
        case GSM_Priority_High: s = strdup("High"); break;
        case GSM_Priority_Medium: s = strdup("Medium"); break;
        case GSM_Priority_Low: s = strdup("Low"); break;
        case GSM_Priority_None: s = strdup("None"); break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for TodoPriority from Gammu: '%d'", p);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_ToDo_Priority StringToTodoPriority(const char *s) {
    if (strcmp("High", s) == 0) return GSM_Priority_High;
    else if (strcmp("Medium", s) == 0) return GSM_Priority_Medium;
    else if (strcmp("Low", s) == 0) return GSM_Priority_Low;
    else if (strcmp("None", s) == 0) return GSM_Priority_None;

    PyErr_Format(PyExc_MemoryError, "Bad value for Todo Priority '%s'", s);
    return ENUM_INVALID;
}

PyObject *TodoToPython(const GSM_ToDoEntry *entry) {
    PyObject    *v;
    PyObject    *f;
    PyObject    *r;
    PyObject    *d;
    int         i;
    Py_UNICODE  *s;
    char        *p;
    char        *t;

    v = PyList_New(0);
    if (v == NULL) return NULL;

    for (i=0; i < entry->EntriesNum; i++) {
        f = Py_None;
        switch (entry->Entries[i].EntryType) {
            case TODO_END_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "END_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case TODO_START_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "START_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case TODO_ALARM_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "ALARM_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case TODO_SILENT_ALARM_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "SILENT_ALARM_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case TODO_LAST_MODIFIED:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "LAST_MODIFIED", "Value", d);
                Py_DECREF(d);
                break;
            case TODO_TEXT:
                s = strGammuToPython(entry->Entries[i].Text);
                if (s == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:u}", "Type", "TEXT", "Value", s);
                free(s);
                break;
            case TODO_LOCATION:
                s = strGammuToPython(entry->Entries[i].Text);
                if (s == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:u}", "Type", "LOCATION", "Value", s);
                free(s);
                break;
            case TODO_DESCRIPTION:
                s = strGammuToPython(entry->Entries[i].Text);
                if (s == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:u}", "Type", "DESCRIPTION", "Value", s);
                free(s);
                break;
            case TODO_LUID:
                s = strGammuToPython(entry->Entries[i].Text);
                if (s == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:u}", "Type", "LUID", "Value", s);
                free(s);
                break;
            case TODO_COMPLETED:
                f = Py_BuildValue("{s:s,s:i}", "Type", "COMPLETED", "Value", entry->Entries[i].Number);
                break;
            case TODO_PRIVATE:
                f = Py_BuildValue("{s:s,s:i}", "Type", "PRIVATE", "Value", entry->Entries[i].Number);
                break;
            case TODO_CATEGORY:
                f = Py_BuildValue("{s:s,s:i}", "Type", "CATEGORY", "Value", entry->Entries[i].Number);
                break;
            case TODO_CONTACTID:
                f = Py_BuildValue("{s:s,s:i}", "Type", "CONTACTID", "Value", entry->Entries[i].Number);
                break;
            case TODO_PHONE:
                s = strGammuToPython(entry->Entries[i].Text);
                if (s == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:u}", "Type", "PHONE", "Value", s);
                free(s);
                break;
        }

        if (f == Py_None) {
            Py_DECREF(v);
            PyErr_Format(PyExc_ValueError, "Bad ToDo item type from gammu: %d", entry->Entries[i].EntryType);
            return NULL;
        }

        if (f == NULL) {
            Py_DECREF(v);
            return NULL;
        }

        if (PyList_Append(v, f) != 0) {
            Py_DECREF(f);
            Py_DECREF(v);
            return NULL;
        }
        Py_DECREF(f);
    }

    p = TodoPriorityToString(entry->Priority);
    if (p == NULL) {
        Py_DECREF(v);
        return NULL;
    }

    t = CalendarTypeToString(entry->Type);
    if (t == NULL) {
        free(p);
        Py_DECREF(v);
        return NULL;
    }

    r = Py_BuildValue("{s:i,s:s,s:s,s:O}",
            "Location", entry->Location,
            "Type", t,
            "Priority", p,
            "Entries", v
            );
    free(p);
    free(t);
    Py_DECREF(v);
    return r;
}

int TodoFromPython(PyObject *dict, GSM_ToDoEntry *entry, int needs_location) {
    PyObject        *o;
    PyObject        *item;
    Py_ssize_t      len;
    Py_ssize_t      i;
    char            *p;
    char            *t;
    char            *type;
    char            valuetype;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "Todo entry is not a dictionary");
        return 0;
    }

    memset(entry, 0, sizeof(entry));

    if (needs_location) {
        entry->Location = GetIntFromDict(dict, "Location");
        if (entry->Location == INT_INVALID) return 0;
    }

    t = GetCharFromDict(dict, "Type");
    if (t == NULL) return 0;
    entry->Type = StringToCalendarType(t);
    if (entry->Type == 0) return 0;

    p = GetCharFromDict(dict, "Priority");
    if (p == NULL) return 0;
    entry->Priority = StringToTodoPriority(p);
    if (entry->Priority == ENUM_INVALID) return 0;

    o = PyDict_GetItemString(dict, "Entries");
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key Values");
        return 0;
    }

    if (!PyList_Check(o)) {
        PyErr_Format(PyExc_ValueError, "Key Values doesn't contain list");
        return 0;
    }

    len = PyList_Size(o);
    if (len > GSM_TODO_ENTRIES) {
        pyg_warning("Using just %i entries from list!", GSM_TODO_ENTRIES);
        len = GSM_TODO_ENTRIES;
    }
    entry->EntriesNum = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(o, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %" PY_FORMAT_SIZE_T "d in Values is not dictionary", i);
            return 0;
        }
        type = GetCharFromDict(item, "Type");
        if (type == NULL) return 0;

        if (strcmp("END_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = TODO_END_DATETIME;
        } else if (strcmp("START_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = TODO_START_DATETIME;
        } else if (strcmp("COMPLETED", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = TODO_COMPLETED;
        } else if (strcmp("ALARM_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = TODO_ALARM_DATETIME;
        } else if (strcmp("SILENT_ALARM_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = TODO_SILENT_ALARM_DATETIME;
        } else if (strcmp("LAST_MODIFIED", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = TODO_LAST_MODIFIED;
        } else if (strcmp("LUID", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = TODO_LUID;
        } else if (strcmp("LOCATION", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = TODO_LOCATION;
        } else if (strcmp("DESCRIPTION", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = TODO_DESCRIPTION;
        } else if (strcmp("TEXT", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = TODO_TEXT;
        } else if (strcmp("PRIVATE", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = TODO_PRIVATE;
        } else if (strcmp("CATEGORY", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = TODO_CATEGORY;
        } else if (strcmp("CONTACTID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = TODO_CONTACTID;
        } else if (strcmp("PHONE", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = TODO_PHONE;
        } else {
            PyErr_Format(PyExc_ValueError, "Element %" PY_FORMAT_SIZE_T "d in Values has bad type: %s", i, type);
            return 0;
        }

        switch (valuetype) {
            case 'n':
                entry->Entries[i].Number = GetIntFromDict(item, "Value");
                if (entry->Entries[i].Number == INT_INVALID) return 0;
                break;
            case 't':
                if (!CopyStringFromDict(item, "Value", GSM_PHONEBOOK_TEXT_LENGTH, entry->Entries[i].Text))
                    return 0;
                break;
            case 'd':
                entry->Entries[i].Date = GetDateTimeFromDict(item, "Value");
                if (entry->Entries[i].Date.Year == -1) return 0;
                break;
        }
    } /* end for */

    return 1;
}



