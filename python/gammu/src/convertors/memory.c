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

/* MemoryEntry conversions */
#undef USE_PREF_UNICODE

#include "convertors.h"
#include "misc.h"

#define convert_text(name) \
                s = strGammuToPython(entry->Entries[i].Text); \
                if (s == NULL) { \
                    Py_DECREF(v); \
                    return NULL; \
                } \
                f = Py_BuildValue("{s:s,s:u,s:i}", "Type", name, "Value", s, "AddError", (int)entry->Entries[i].AddError); \
                free(s);

#define convert_number(name) \
                convert_text("Number_" name); \
                /* VoiceTag */ \
                PyDict_SetItemString(f, "VoiceTag", PyInt_FromLong((int)entry->Entries[i].VoiceTag)); \
                /* SMSList */ \
                l = PyList_New(0); \
                j = 0; \
                while (entry->Entries[i].SMSList[j] != 0) { \
                    PyList_Append(l, PyInt_FromLong((int)entry->Entries[i].SMSList[j])); \
                    j++; \
                } \
                PyDict_SetItemString(f, "SMSList", l); \
                Py_DECREF(l);

PyObject *MemoryEntryToPython(const GSM_MemoryEntry *entry) {
    PyObject    *v;
    PyObject    *f;
    PyObject    *r;
    PyObject    *d;
    PyObject    *l;
    int         i;
    int         j;
    Py_UNICODE  *s;
    char        *t;
	const GSM_BinaryPicture *bitmap;
    const char *bmptype;

    v = PyList_New(0);
    if (v == NULL) return NULL;

    for (i=0; i < entry->EntriesNum; i++) {
        f = Py_None;
        switch (entry->Entries[i].EntryType) {
            case PBK_Number_General:
                convert_number("General");
                break;
            case PBK_Number_Mobile:
                convert_number("Mobile");
                break;
            case PBK_Number_Mobile_Work:
                convert_number("Mobile_Work");
                break;
            case PBK_Number_Mobile_Home:
                convert_number("Mobile_Home");
                break;
            case PBK_Number_Work:
                convert_number("Work");
                break;
            case PBK_Number_Fax:
                convert_number("Fax");
                break;
            case PBK_Number_Home:
                convert_number("Home");
                break;
            case PBK_Number_Pager:
                convert_number("Pager");
                break;
            case PBK_Number_Other:
                convert_number("Other");
                break;
            case PBK_Text_Note:
                convert_text("Text_Note");
                break;
            case PBK_Text_Postal:
                convert_text("Text_Postal");
                break;
            case PBK_Text_WorkPostal:
                convert_text("Text_WorkPostal");
                break;
            case PBK_Text_Email:
                convert_text("Text_Email");
                break;
            case PBK_Text_Email2:
                convert_text("Text_Email2");
                break;
            case PBK_Text_URL:
                convert_text("Text_URL");
                break;
            case PBK_Date:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "Date", "Value", d);
                Py_DECREF(d);
                break;
            case PBK_LastModified:
                d = BuildPythonDateTime(&(entry->Entries[i].Date));
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                f = Py_BuildValue("{s:s,s:O}", "Type", "LastModified", "Value", d);
                Py_DECREF(d);
                break;
            case PBK_Caller_Group:
                f = Py_BuildValue("{s:s,s:i}", "Type", "Caller_Group", "Value", entry->Entries[i].Number);
                break;
            case PBK_Text_Name:
                convert_text("Text_Name");
                break;
            case PBK_Text_LastName:
                convert_text("Text_LastName");
                break;
            case PBK_Text_FirstName:
                convert_text("Text_FirstName");
                break;
            case PBK_Text_NickName:
                convert_text("Text_NickName");
                break;
            case PBK_Text_FormalName:
                convert_text("Text_FormalName");
                break;
            case PBK_Text_Company:
                convert_text("Text_Company");
                break;
            case PBK_Text_JobTitle:
                convert_text("Text_JobTitle");
                break;
            case PBK_Category:
                if (entry->Entries[i].Number == -1) {
                    convert_text("Category");
                } else {
                    f = Py_BuildValue("{s:s,s:i}", "Type", "Category", "Value", entry->Entries[i].Number);
                }
                break;
            case PBK_Private:
                f = Py_BuildValue("{s:s,s:i}", "Type", "Private", "Value", entry->Entries[i].Number);
                break;
            case PBK_Text_StreetAddress:
                convert_text("Text_StreetAddress");
                break;
            case PBK_Text_City:
                convert_text("Text_City");
                break;
            case PBK_Text_State:
                convert_text("Text_State");
                break;
            case PBK_Text_Zip:
                convert_text("Text_Zip");
                break;
            case PBK_Text_Country:
                convert_text("Text_Country");
                break;
            case PBK_Text_WorkStreetAddress:
                convert_text("Text_WorkStreetAddress");
                break;
            case PBK_Text_WorkCity:
                convert_text("Text_WorkCity");
                break;
            case PBK_Text_WorkState:
                convert_text("Text_WorkState");
                break;
            case PBK_Text_WorkZip:
                convert_text("Text_WorkZip");
                break;
            case PBK_Text_WorkCountry:
                convert_text("Text_WorkCountry");
                break;
            case PBK_Text_Custom1:
                convert_text("Text_Custom1");
                break;
            case PBK_Text_Custom2:
                convert_text("Text_Custom2");
                break;
            case PBK_Text_Custom3:
                convert_text("Text_Custom3");
                break;
            case PBK_Text_Custom4:
                convert_text("Text_Custom4");
                break;
            case PBK_Text_LUID:
                convert_text("Text_LUID");
                break;
            case PBK_Text_UserID:
                convert_text("Text_UserID");
                break;
            case PBK_Text_PictureName:
                convert_text("Text_PictureName");
                break;
            case PBK_RingtoneID:
                f = Py_BuildValue("{s:s,s:O}", "Type", "RingtoneID", "Value", PyLong_FromUnsignedLong(entry->Entries[i].Number));
                break;
            case PBK_PictureID:
                f = Py_BuildValue("{s:s,s:O}", "Type", "PictureID", "Value", PyLong_FromUnsignedLong(entry->Entries[i].Number));
                break;
            case PBK_CallLength:
                f = Py_BuildValue("{s:s,s:i}", "Type", "CallLength", "Value", entry->Entries[i].CallLength);
                break;
            case PBK_Number_Messaging:
                convert_number("Messaging");
                break;
            case PBK_PushToTalkID:
                convert_text("PushToTalkID");
                break;
            case PBK_Photo:
                bitmap = &(entry->Entries[i].Picture);
                d = PyString_FromStringAndSize((char *)bitmap->Buffer, bitmap->Length);
                if (d == NULL) {
                    Py_DECREF(v);
                    return NULL;
                }
                bmptype = "";
                switch (bitmap->Type) {
                    case PICTURE_BMP:
                        bmptype = "BMP";
                        break;
                    case PICTURE_GIF:
                        bmptype = "GIF";
                        break;
                    case PICTURE_JPG:
                        bmptype = "JPG";
                        break;
                    case PICTURE_ICN:
                        bmptype = "ICN";
                        break;
                    case PICTURE_PNG:
                        bmptype = "PNG";
                        break;
                }
                f = Py_BuildValue("{s:s,s:O,s:s}", "Type", "Photo", "Value", d, "PictureType", bmptype);
                Py_DECREF(d);
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
    Py_ssize_t      len, data_len;
    Py_ssize_t      i;
    char            *type;
    char            valuetype;
    const char      *bmptype;

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
    if (entry->MemoryType == ENUM_INVALID || entry->MemoryType == 0) return 0;

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
        pyg_warning("Using just %i entries from list!", GSM_PHONEBOOK_ENTRIES);
        len = GSM_PHONEBOOK_ENTRIES;
    }
    entry->EntriesNum = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(o, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %" PY_FORMAT_SIZE_T "d in Entries is not dictionary", i);
            return 0;
        }
        type = GetCharFromDict(item, "Type");
        if (type == NULL) return 0;

        /* Zero everything, just to be sure */
        entry->Entries[i].Text[0] = 0;
        entry->Entries[i].Text[1] = 0;
        entry->Entries[i].SMSList[0] = 0;
        entry->Entries[i].Number = 0;

        /* Get VoiceTag flag */
        entry->Entries[i].VoiceTag = GetIntFromDict(item, "VoiceTag");
        if (entry->Entries[i].VoiceTag == INT_INVALID) {
            entry->Entries[i].VoiceTag = 0;
            PyErr_Clear();
        }

        /* Get AddError flag */
        entry->Entries[i].AddError = GetIntFromDict(item, "AddError");
        if (entry->Entries[i].AddError == INT_INVALID) {
            entry->Entries[i].AddError = ERR_NONE;
            PyErr_Clear();
        }

        if (strcmp("Number_General", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_General;
        } else if (strcmp("Number_Mobile_Work", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Mobile_Work;
        } else if (strcmp("Number_Mobile_Home", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Mobile_Home;
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
        } else if (strcmp("Number_Messaging", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Number_Messaging;
        } else if (strcmp("Text_Note", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Note;
        } else if (strcmp("Text_Postal", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Postal;
        } else if (strcmp("Text_WorkPostal", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_WorkPostal;
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
        } else if (strcmp("LastModified", type) == 0) {
            valuetype = 'd';
            entry->Entries[i].EntryType = PBK_LastModified;
        } else if (strcmp("Caller_Group", type) == 0) {
            valuetype = 'n';
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
        } else if (strcmp("Text_NickName", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_NickName;
        } else if (strcmp("Text_FormalName", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_FormalName;
        } else if (strcmp("Text_Company", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_Company;
        } else if (strcmp("Text_JobTitle", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_JobTitle;
        } else if (strcmp("Category", type) == 0) {
            valuetype = 'o';
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
        } else if (strcmp("Text_WorkStreetAddress", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_WorkStreetAddress;
        } else if (strcmp("Text_WorkCity", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_WorkCity;
        } else if (strcmp("Text_WorkState", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_WorkState;
        } else if (strcmp("Text_WorkZip", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_WorkZip;
        } else if (strcmp("Text_WorkCountry", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_WorkCountry;
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
        } else if (strcmp("Text_LUID", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_LUID;
        } else if (strcmp("Text_UserID", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_UserID;
        } else if (strcmp("Text_PictureName", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_Text_PictureName;
        } else if (strcmp("RingtoneID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_RingtoneID;
        } else if (strcmp("PictureID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_PictureID;
        } else if (strcmp("CallLength", type) == 0) {
            valuetype = 'c';
            entry->Entries[i].EntryType = PBK_CallLength;
        } else if (strcmp("PushToTalkID", type) == 0) {
            valuetype = 't';
            entry->Entries[i].EntryType = PBK_PushToTalkID;
        } else if (strcmp("Photo", type) == 0) {
            valuetype = 'p';
            entry->Entries[i].EntryType = PBK_Photo;
        } else {
            PyErr_Format(PyExc_ValueError, "Element %" PY_FORMAT_SIZE_T "d in Entries has bad type: %s", i, type);
            return 0;
        }

        switch (valuetype) {
            case 'o':
                entry->Entries[i].Number = GetIntFromDict(item, "Value");
                if (entry->Entries[i].Number == INT_INVALID) {
                    PyErr_Clear();
                    if (!CopyStringFromDict(item, "Value", GSM_PHONEBOOK_TEXT_LENGTH, entry->Entries[i].Text))
                        return 0;
                }
                break;
            case 'n':
                entry->Entries[i].Number = GetIntFromDict(item, "Value");
                if (entry->Entries[i].Number == INT_INVALID) return 0;
                break;
            case 'c':
                entry->Entries[i].CallLength = GetIntFromDict(item, "Value");
                if (entry->Entries[i].CallLength == INT_INVALID) return 0;
                break;
            case 't':
                if (!CopyStringFromDict(item, "Value", GSM_PHONEBOOK_TEXT_LENGTH, entry->Entries[i].Text))
                    return 0;
                break;
            case 'd':
                entry->Entries[i].Date = GetDateTimeFromDict(item, "Value");
                if (entry->Entries[i].Date.Year == -1) return 0;
                break;
            case 'p':
                entry->Entries[i].Picture.Buffer = (unsigned char *)GetCStringLengthFromDict(item, "Value", &data_len);
                entry->Entries[i].Picture.Length = data_len;
                if (entry->Entries[i].Picture.Buffer == NULL) {
                    return 0;
                }
                bmptype = GetCStringFromDict(item, "PictureType");
                if (strcmp(bmptype, "BMP") == 0) {
                    entry->Entries[i].Picture.Type = PICTURE_BMP;
                } else if (strcmp(bmptype, "GIF") == 0) {
                    entry->Entries[i].Picture.Type = PICTURE_GIF;
                } else if (strcmp(bmptype, "JPG") == 0) {
                    entry->Entries[i].Picture.Type = PICTURE_JPG;
                } else if (strcmp(bmptype, "ICN") == 0) {
                    entry->Entries[i].Picture.Type = PICTURE_ICN;
                } else if (strcmp(bmptype, "PNG") == 0) {
                    entry->Entries[i].Picture.Type = PICTURE_PNG;
                } else {
                    entry->Entries[i].Picture.Type = 0;
                }
                break;
        }
    } /* end for */

    return 1;
}

