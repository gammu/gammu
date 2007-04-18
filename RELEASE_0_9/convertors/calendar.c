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

/* Calendar related conversions */

#include "convertors.h"

char *CalendarTypeToString(GSM_CalendarNoteType p) {
    char *err = "Err";
    char *s = err;

    switch (p) {
        case GSM_CAL_REMINDER: s = strdup("REMINDER"); break;
        case GSM_CAL_CALL: s = strdup("CALL"); break;
        case GSM_CAL_MEETING: s = strdup("MEETING"); break;
        case GSM_CAL_BIRTHDAY: s = strdup("BIRTHDAY"); break;
        case GSM_CAL_MEMO: s = strdup("MEMO"); break;
        case GSM_CAL_TRAVEL: s = strdup("TRAVEL"); break;
        case GSM_CAL_VACATION: s = strdup("VACATION"); break;
        case GSM_CAL_T_ATHL: s = strdup("T_ATHL"); break;
        case GSM_CAL_T_BALL: s = strdup("T_BALL"); break;
        case GSM_CAL_T_CYCL: s = strdup("T_CYCL"); break;
        case GSM_CAL_T_BUDO: s = strdup("T_BUDO"); break;
        case GSM_CAL_T_DANC: s = strdup("T_DANC"); break;
        case GSM_CAL_T_EXTR: s = strdup("T_EXTR"); break;
        case GSM_CAL_T_FOOT: s = strdup("T_FOOT"); break;
        case GSM_CAL_T_GOLF: s = strdup("T_GOLF"); break;
        case GSM_CAL_T_GYM: s = strdup("T_GYM"); break;
        case GSM_CAL_T_HORS: s = strdup("T_HORS"); break;
        case GSM_CAL_T_HOCK: s = strdup("T_HOCK"); break;
        case GSM_CAL_T_RACE: s = strdup("T_RACE"); break;
        case GSM_CAL_T_RUGB: s = strdup("T_RUGB"); break;
        case GSM_CAL_T_SAIL: s = strdup("T_SAIL"); break;
        case GSM_CAL_T_STRE: s = strdup("T_STRE"); break;
        case GSM_CAL_T_SWIM: s = strdup("T_SWIM"); break;
        case GSM_CAL_T_TENN: s = strdup("T_TENN"); break;
        case GSM_CAL_T_TRAV: s = strdup("T_TRAV"); break;
        case GSM_CAL_T_WINT: s = strdup("T_WINT"); break;
        case GSM_CAL_ALARM: s = strdup("ALARM"); break;
        case GSM_CAL_DAILY_ALARM: s = strdup("DAILY_ALARM"); break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for CalendarType from Gammu: '%d'", p);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_CalendarNoteType StringToCalendarType(const char *s) {
    if (strcmp("REMINDER", s) == 0) return GSM_CAL_REMINDER;
    else if (strcmp("CALL", s) == 0) return GSM_CAL_CALL;
    else if (strcmp("MEETING", s) == 0) return GSM_CAL_MEETING;
    else if (strcmp("BIRTHDAY", s) == 0) return GSM_CAL_BIRTHDAY;
    else if (strcmp("MEMO", s) == 0) return GSM_CAL_MEMO;
    else if (strcmp("TRAVEL", s) == 0) return GSM_CAL_TRAVEL;
    else if (strcmp("VACATION", s) == 0) return GSM_CAL_VACATION;
    else if (strcmp("T_ATHL", s) == 0) return GSM_CAL_T_ATHL;
    else if (strcmp("T_BALL", s) == 0) return GSM_CAL_T_BALL;
    else if (strcmp("T_CYCL", s) == 0) return GSM_CAL_T_CYCL;
    else if (strcmp("T_BUDO", s) == 0) return GSM_CAL_T_BUDO;
    else if (strcmp("T_DANC", s) == 0) return GSM_CAL_T_DANC;
    else if (strcmp("T_EXTR", s) == 0) return GSM_CAL_T_EXTR;
    else if (strcmp("T_FOOT", s) == 0) return GSM_CAL_T_FOOT;
    else if (strcmp("T_GOLF", s) == 0) return GSM_CAL_T_GOLF;
    else if (strcmp("T_GYM", s) == 0) return GSM_CAL_T_GYM;
    else if (strcmp("T_HORS", s) == 0) return GSM_CAL_T_HORS;
    else if (strcmp("T_HOCK", s) == 0) return GSM_CAL_T_HOCK;
    else if (strcmp("T_RACE", s) == 0) return GSM_CAL_T_RACE;
    else if (strcmp("T_RUGB", s) == 0) return GSM_CAL_T_RUGB;
    else if (strcmp("T_SAIL", s) == 0) return GSM_CAL_T_SAIL;
    else if (strcmp("T_STRE", s) == 0) return GSM_CAL_T_STRE;
    else if (strcmp("T_SWIM", s) == 0) return GSM_CAL_T_SWIM;
    else if (strcmp("T_TENN", s) == 0) return GSM_CAL_T_TENN;
    else if (strcmp("T_TRAV", s) == 0) return GSM_CAL_T_TRAV;
    else if (strcmp("T_WINT", s) == 0) return GSM_CAL_T_WINT;
    else if (strcmp("ALARM", s) == 0) return GSM_CAL_ALARM;
    else if (strcmp("DAILY_ALARM", s) == 0) return GSM_CAL_DAILY_ALARM;

    PyErr_Format(PyExc_MemoryError, "Bad value for Calendar Type '%s'", s);
    return 0;
}

PyObject *CalendarToPython(const GSM_CalendarEntry *entry) {
    PyObject    *v;
    PyObject    *f;
    PyObject    *r;
    PyObject    *d;
    int         i;
    Py_UNICODE  *s;
    char        *t;

    v = PyList_New(0);
    if (v == NULL) return NULL;

    for (i=0; i < entry->EntriesNum; i++) {
        f = Py_None;
        switch (entry->Entries[i].EntryType) {
            case CAL_START_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "START_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case CAL_END_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "END_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case CAL_TONE_ALARM_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "TONE_ALARM_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case CAL_SILENT_ALARM_DATETIME:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "SILENT_ALARM_DATETIME", "Value", d);
                Py_DECREF(d);
                break;
            case CAL_REPEAT_STARTDATE:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "REPEAT_STARTDATE", "Value", d);
                Py_DECREF(d);
                break;
            case CAL_REPEAT_STOPDATE:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "REPEAT_STOPDATE", "Value", d);
                Py_DECREF(d);
                break;

            case CAL_TEXT:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "TEXT", "Value", s);
                free(s);
                break;
            case CAL_LOCATION:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "LOCATION", "Value", s);
                free(s);
                break;
            case CAL_PHONE:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "PHONE", "Value", s);
                free(s);
                break;

            case CAL_PRIVATE:
                f = Py_BuildValue("{s:s,s:i}", "Type", "PRIVATE", "Value", entry->Entries[i].Number);
                break;
            case CAL_CONTACTID:
                f = Py_BuildValue("{s:s,s:i}", "Type", "CONTACTID", "Value", entry->Entries[i].Number);
                break;
            case CAL_REPEAT_DAYOFWEEK:
                f = Py_BuildValue("{s:s,s:i}", "Type", "REPEAT_DAYOFWEEK", "Value", entry->Entries[i].Number);
                break;
            case CAL_REPEAT_DAY:
                f = Py_BuildValue("{s:s,s:i}", "Type", "REPEAT_DAY", "Value", entry->Entries[i].Number);
                break;
            case CAL_REPEAT_WEEKOFMONTH:
                f = Py_BuildValue("{s:s,s:i}", "Type", "REPEAT_WEEKOFMONTH", "Value", entry->Entries[i].Number);
                break;
            case CAL_REPEAT_MONTH:
                f = Py_BuildValue("{s:s,s:i}", "Type", "REPEAT_MONTH", "Value", entry->Entries[i].Number);
                break;
            case CAL_REPEAT_FREQUENCY:
                f = Py_BuildValue("{s:s,s:i}", "Type", "REPEAT_FREQUENCY", "Value", entry->Entries[i].Number);
                break;
        }

        if (f == Py_None) {
            Py_DECREF(v);
            PyErr_Format(PyExc_ValueError, "Bad Calendar item type from gammu: %d", entry->Entries[i].EntryType);
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

    t = CalendarTypeToString(entry->Type);
    if (t == NULL) {
        Py_DECREF(v);
        return NULL;
    }

    r = Py_BuildValue("{s:i,s:s,s:O}",
            "Location", entry->Location,
            "Type", t,
            "Entries", v
            );
    free(t);
    Py_DECREF(v);
    return r;
}

int CalendarFromPython(PyObject *dict, GSM_CalendarEntry *entry, int needs_location) {
    PyObject        *o;
    PyObject        *item;
    int             len;
    int             i;
    char            *t;
    char            *type;
    char            valuetype;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "Calendar entry is not a dictionary");
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
    if (len > GSM_CALENDAR_ENTRIES) {
        printf("WARNING: using just %i entries from list!", GSM_CALENDAR_ENTRIES);
        len = GSM_CALENDAR_ENTRIES;
    }
    entry->EntriesNum = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(o, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %i in Values is not dictionary", i);
            return 0;
        }
        type = GetCharFromDict(item, "Type");
        if (type == NULL) return 0;

        if (strcmp("START_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = CAL_START_DATETIME;
        } else if (strcmp("END_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = CAL_END_DATETIME;
        } else if (strcmp("TONE_ALARM_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = CAL_TONE_ALARM_DATETIME;
        } else if (strcmp("SILENT_ALARM_DATETIME", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = CAL_SILENT_ALARM_DATETIME;
        } else if (strcmp("REPEAT_STARTDATE", type) == 0) {
            valuetype = 'D';
            entry->Entries[i].EntryType = CAL_REPEAT_STARTDATE;
        } else if (strcmp("REPEAT_STOPDATE", type) == 0) {
            valuetype = 'D';
            entry->Entries[i].EntryType = CAL_REPEAT_STOPDATE;

        } else if (strcmp("TEXT", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = CAL_TEXT;
        } else if (strcmp("LOCATION", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = CAL_LOCATION;
        } else if (strcmp("PHONE", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = CAL_PHONE;

        } else if (strcmp("PRIVATE", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_PRIVATE;
        } else if (strcmp("CONTACTID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_CONTACTID;
        } else if (strcmp("REPEAT_DAYOFWEEK", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_REPEAT_DAYOFWEEK;
        } else if (strcmp("REPEAT_DAY", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_REPEAT_DAY;
        } else if (strcmp("REPEAT_WEEKOFMONTH", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_REPEAT_WEEKOFMONTH;
        } else if (strcmp("REPEAT_MONTH", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_REPEAT_MONTH;
        } else if (strcmp("REPEAT_FREQUENCY", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = CAL_REPEAT_FREQUENCY;

        } else {
            PyErr_Format(PyExc_ValueError, "Element %i in Values has bad type: %s", i, type);
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
            case 'D':
                entry->Entries[i].Date = GetDateFromDict(item, "Value");
                if (entry->Entries[i].Date.Year == -1) return 0;
                break;
            case 'd':
                entry->Entries[i].Date = GetDateTimeFromDict(item, "Value");
                if (entry->Entries[i].Date.Year == -1) return 0;
                break;
        }
    } /* end for */

    return 1;
}



