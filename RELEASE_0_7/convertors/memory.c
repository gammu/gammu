/*
 * python-gammu - Phone communication libary
 * Copyright (c) 2003 - 2004 Michal Čihař
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* MemoryEntry conversions */
#undef USE_PREF_UNICODE

#include "convertors.h"

PyObject *MemoryEntryToPython(const GSM_MemoryEntry *entry) {
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
            case PBK_Number_General:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_General", "Value", s);
                free(s);
                break;
            case PBK_Number_Mobile:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_Mobile", "Value", s);
                free(s);
                break;
            case PBK_Number_Work:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_Work", "Value", s);
                free(s);
                break;
            case PBK_Number_Fax:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_Fax", "Value", s);
                free(s);
                break;
            case PBK_Number_Home:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_Home", "Value", s);
                free(s);
                break;
            case PBK_Number_Pager:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_Pager", "Value", s);
                free(s);
                break;
            case PBK_Number_Other:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Number_Other", "Value", s);
                free(s);
                break;
            case PBK_Text_Note:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Note", "Value", s);
                free(s);
                break;
            case PBK_Text_Postal:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Postal", "Value", s);
                free(s);
                break;
            case PBK_Text_Email:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Email", "Value", s);
                free(s);
                break;
            case PBK_Text_Email2:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Email2", "Value", s);
                free(s);
                break;
            case PBK_Text_URL:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_URL", "Value", s);
                free(s);
                break;
            case PBK_Date:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                f = Py_BuildValue("{s:s,s:O}", "Type", "Date", "Value", d);
                Py_DECREF(d);
                break;
            case PBK_Caller_Group:
                f = Py_BuildValue("{s:s,s:i}", "Type", "Caller_Group", "Value", entry->Entries[i].Number);
                break;
            case PBK_Text_Name:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Name", "Value", s);
                free(s);
                break;
            case PBK_Text_LastName:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_LastName", "Value", s);
                free(s);
                break;
            case PBK_Text_FirstName:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_FirstName", "Value", s);
                free(s);
                break;
            case PBK_Text_Company:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Company", "Value", s);
                free(s);
                break;
            case PBK_Text_JobTitle:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_JobTitle", "Value", s);
                free(s);
                break;
            case PBK_Category:
                f = Py_BuildValue("{s:s,s:i}", "Type", "Category", "Value", entry->Entries[i].Number);
                break;
            case PBK_Private:
                f = Py_BuildValue("{s:s,s:i}", "Type", "Private", "Value", entry->Entries[i].Number);
                break;
            case PBK_Text_StreetAddress:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_StreetAddress", "Value", s);
                free(s);
                break;
            case PBK_Text_City:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_City", "Value", s);
                free(s);
                break;
            case PBK_Text_State:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_State", "Value", s);
                free(s);
                break;
            case PBK_Text_Zip:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Zip", "Value", s);
                free(s);
                break;
            case PBK_Text_Country:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Country", "Value", s);
                free(s);
                break;
            case PBK_Text_Custom1:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Custom1", "Value", s);
                free(s);
                break;
            case PBK_Text_Custom2:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Custom2", "Value", s);
                free(s);
                break;
            case PBK_Text_Custom3:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Custom3", "Value", s);
                free(s);
                break;
            case PBK_Text_Custom4:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_Custom4", "Value", s);
                free(s);
                break;
            case PBK_Text_UserID:
                s = strGammuToPython(entry->Entries[i].Text);
                f = Py_BuildValue("{s:s,s:u}", "Type", "Text_UserID", "Value", s);
                free(s);
                break;
            case PBK_RingtoneID:
                f = Py_BuildValue("{s:s,s:O}", "Type", "RingtoneID", "Value", PyLong_FromUnsignedLong(entry->Entries[i].Number));
                break;
            case PBK_PictureID:
                f = Py_BuildValue("{s:s,s:O}", "Type", "PictureID", "Value", PyLong_FromUnsignedLong(entry->Entries[i].Number));
                break;
            case PBK_RingtoneFileSystemID:
                f = Py_BuildValue("{s:s,s:O}", "Type", "RingtoneID", "Value", PyLong_FromUnsignedLong(entry->Entries[i].Number));
                break;
            case PBK_SMSListID:
                f = Py_BuildValue("{s:s,s:O}", "Type", "SMSListID", "Value", PyLong_FromUnsignedLong(entry->Entries[i].Number));
                break;
        }

        if (f == Py_None) {
            Py_DECREF(v);
            PyErr_Format(PyExc_ValueError, "Bad MemoryEntry item type from gammu: %d", entry->Entries[i].EntryType);
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

    t = MemoryTypeToString(entry->MemoryType);

    if (t == NULL) {
        Py_DECREF(v);
        return NULL;
    }

    r = Py_BuildValue("{s:i,s:s,s:O}",
            "Location", entry->Location,
            "MemoryType", t,
            "Entries", v
            );
    free(t);
    Py_DECREF(v);
    return r;
}

int MemoryEntryFromPython(PyObject *dict, GSM_MemoryEntry *entry, int needs_location) {
    PyObject        *o;
    PyObject        *item;
    int             len;
    int             i;
    char            *type;
    char            valuetype;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "Memory entry is not a dictionary");
        return 0;
    }

    memset(entry, 0, sizeof(entry));

    if (needs_location) {
        entry->Location = GetIntFromDict(dict, "Location");
        if (entry->Location == INT_INVALID) return 0;
    }

    entry->MemoryType = GetMemoryTypeFromDict(dict, "MemoryType");
    if (entry->MemoryType == -1) return 0;

    o = PyDict_GetItemString(dict, "Entries");
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key Entries");
        return 0;
    }

    if (!PyList_Check(o)) {
        PyErr_Format(PyExc_ValueError, "Key Entries doesn't contain list");
        return 0;
    }

    len = PyList_Size(o);
    if (len > GSM_PHONEBOOK_ENTRIES) {
        printf("WARNING: using just %i entries from list!", GSM_PHONEBOOK_ENTRIES);
        len = GSM_PHONEBOOK_ENTRIES;
    }
    entry->EntriesNum = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(o, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %i in Entries is not dictionary", i);
            return 0;
        }
        type = GetCharFromDict(item, "Type");
        if (type == NULL) return 0;

        if (strcmp("Number_General", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_General;
        } else if (strcmp("Number_Mobile", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Mobile;
        } else if (strcmp("Number_Work", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Work;
        } else if (strcmp("Number_Fax", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Fax;
        } else if (strcmp("Number_Home", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Home;
        } else if (strcmp("Number_Pager", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Pager;
        } else if (strcmp("Number_Other", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Other;
        } else if (strcmp("Text_Note", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Note;
        } else if (strcmp("Text_Postal", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Postal;
        } else if (strcmp("Text_Email", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Email;
        } else if (strcmp("Text_Email2", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Email2;
        } else if (strcmp("Text_URL", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_URL;
        } else if (strcmp("Date", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = PBK_Date;
        } else if (strcmp("Caller_Group", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Caller_Group;
        } else if (strcmp("Text_Name", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Name;
        } else if (strcmp("Text_LastName", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_LastName;
        } else if (strcmp("Text_FirstName", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_FirstName;
        } else if (strcmp("Text_Company", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Company;
        } else if (strcmp("Text_JobTitle", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_JobTitle;
        } else if (strcmp("Category", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_Category;
        } else if (strcmp("Private", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_Private;
        } else if (strcmp("Text_StreetAddress", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_StreetAddress;
        } else if (strcmp("Text_City", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_City;
        } else if (strcmp("Text_State", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_State;
        } else if (strcmp("Text_Zip", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Zip;
        } else if (strcmp("Text_Country", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Country;
        } else if (strcmp("Text_Custom1", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Custom1;
        } else if (strcmp("Text_Custom2", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Custom2;
        } else if (strcmp("Text_Custom3", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Custom3;
        } else if (strcmp("Text_Custom4", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Custom4;
        } else if (strcmp("Text_UserID", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_UserID;
        } else if (strcmp("RingtoneID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_RingtoneID;
        } else if (strcmp("RingtoneFileSystemID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_RingtoneFileSystemID;
        } else if (strcmp("PictureID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_PictureID;
        } else if (strcmp("SMSListID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_SMSListID;
        } else {
            PyErr_Format(PyExc_ValueError, "Element %i in Entries has bad type: %s", i, type);
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

