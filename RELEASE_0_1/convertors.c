#include "convertors.h"

PyObject *BuildPythonDateTime(const GSM_DateTime *dt) {
    PyObject *pModule;
    PyObject *result;
       
    /* import datetime */
    pModule = PyImport_ImportModule("datetime"); 
    if (pModule == NULL) return NULL;

    /* create datetime object */
    result = PyObject_CallMethod(pModule, 
            "datetime", 
            "iiiiii",
            dt->Year,
            dt->Month,
            dt->Day,
            dt->Hour,
            dt->Minute,
            dt->Second
            );
    
    Py_DECREF(pModule);

    return result;
}

PyObject *BuildPythonTime(const GSM_DateTime *dt) {
    PyObject *pModule;
    PyObject *result;
       
    /* import datetime */
    pModule = PyImport_ImportModule("datetime"); 
    if (pModule == NULL) return NULL;

    /* create datetime object */
    result = PyObject_CallMethod(pModule, 
            "time", 
            "iii",
            dt->Hour,
            dt->Minute,
            dt->Second
            );
    
    Py_DECREF(pModule);

    return result;
}

bool BuildGSMDateTime(PyObject *pydt, GSM_DateTime *dt) {
    PyObject *result;
	static GSM_DateTime	nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;
       
    result = PyObject_GetAttrString(pydt, "year");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "year");
        return false;
    }
    dt->Year = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "month");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "month");
        return false;
    }
    dt->Month = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "day");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "day");
        return false;
    }
    dt->Day = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "hour");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "hour");
        return false;
    }
    dt->Hour = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "minute");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "minute");
        return false;
    }
    dt->Minute = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "second");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "second");
        return false;
    }
    dt->Second = PyInt_AsLong(result);

    return true;
}

bool BuildGSMTime(PyObject *pydt, GSM_DateTime *dt) {
    PyObject *result;
	static GSM_DateTime	nulldt = {0,0,0,0,0,0,0};
    *dt = nulldt;
       
    result = PyObject_GetAttrString(pydt, "year");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "year");
        return false;
    }
    dt->Year = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "month");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "month");
        return false;
    }
    dt->Month = PyInt_AsLong(result);

    result = PyObject_GetAttrString(pydt, "day");
    if (result == NULL) return false;
    if (!PyInt_Check(result)) {
        PyErr_Format(PyExc_ValueError, "Attribute %s doesn't seem to be integer", "day");
        return false;
    }
    dt->Day = PyInt_AsLong(result);

    return true;
}

unsigned char *strPythonToGammu(const Py_UNICODE *src) {
    unsigned char *dest;
    int len = 0, i = 0;

    /* Get string length */
    while (src[len] != 0) len++;

    /* Allocate memory */
    dest = malloc((len + 1) * 2 * sizeof(char));
    if (dest == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }
    
    /* Go here up to len, to copy also last 0 */
    while (i <= len) {
        dest[i*2]       = (src[i] >> 8) & 0xff;
        dest[(i*2)+1]   = src[i] & 0xff;
        i++;
    }

    return dest;
}

Py_UNICODE *strGammuToPython(const unsigned char *src) {
    Py_UNICODE *dest;
    int len = 0, i = 0;

    /* Get string length */
    while (src[len*2] != 0 || src[(len*2)+1] != 0 ) len++;

    /* Allocate memory */
    dest = malloc((len + 2) * sizeof(Py_UNICODE));
    if (dest == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    /* Byte Order Mark */
    dest[0] = 0xfe;
    dest[1] = 0xff;

    /* Go here up to len, to copy also last 0 */
    while (i <= len) {
        dest[i] = (src[2*i] << 8) + src[(2*i) + 1];
        i++;
    }

    return dest;
}

GSM_CategoryType StringToCategoryType(const char *s) {
    if (strcmp(s, "ToDo") == 0)             return Category_ToDo;
    else if (strcmp(s, "Phonebook") == 0)   return Category_Phonebook;
    else {
        PyErr_Format(PyExc_ValueError, "Bad value for category type: '%s'", s);
        return 0;
    }
}

GSM_MemoryType StringToMemoryType(const char *s) {
	if (strcmp(s, "ME") == 0)      return GMT_ME;
	else if (strcmp(s, "SM") == 0) return GMT_SM;
	else if (strcmp(s, "ON") == 0) return GMT_ON;
	else if (strcmp(s, "DC") == 0) return GMT_DC;
	else if (strcmp(s, "RC") == 0) return GMT_RC;
	else if (strcmp(s, "MC") == 0) return GMT_MC;
	else if (strcmp(s, "MT") == 0) return GMT_MT;
	else if (strcmp(s, "FD") == 0) return GMT_FD;
	else if (strcmp(s, "VM") == 0) return GMT_VM;
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
	    case GMT_ME: s[0] = 'M'; s[1] = 'E'; break;
	    case GMT_SM: s[0] = 'S'; s[1] = 'M'; break;
	    case GMT_ON: s[0] = 'O'; s[1] = 'N'; break;
	    case GMT_DC: s[0] = 'D'; s[1] = 'C'; break;
	    case GMT_RC: s[0] = 'R'; s[1] = 'C'; break;
	    case GMT_MC: s[0] = 'M'; s[1] = 'C'; break;
	    case GMT_MT: s[0] = 'M'; s[1] = 'T'; break;
	    case GMT_FD: s[0] = 'F'; s[1] = 'D'; break;
	    case GMT_VM: s[0] = 'V'; s[1] = 'M'; break;
    }
    if (s[0] == 0) {
        PyErr_Format(PyExc_ValueError, "Bad value for MemoryType from Gammu: '%02x'", t);
        free(s);
        return NULL;
    }
    return s;
}

PyObject *BuildPythonMemoryEntry(const GSM_MemoryEntry *entry) {
    PyObject    *v;
    PyObject    *f;
    PyObject    *r;
    PyObject    *d;
    int         i;
    Py_UNICODE  *s;
    char        *t;

    v = Py_BuildValue("[]");
    if (v == NULL) return NULL;
    
	for (i=0;i<entry->EntriesNum;i++) {
        f = NULL;
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
            case PBK_RingtoneID:
                f = Py_BuildValue("{s:s,s:i}", "Type", "RingtoneID", "Value", entry->Entries[i].Number);
                break;
            case PBK_PictureID:
                f = Py_BuildValue("{s:s,s:i}", "Type", "PictureID", "Value", entry->Entries[i].Number);
                break;
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
    
    r = Py_BuildValue("{s:i,s:s,s:i,s:O}",
            "Location", entry->Location,
            "MemoryType", t,
            "PreferUnicode", entry->PreferUnicode,
            "Values", v
            );
    free(t);
    Py_DECREF(v);
    return r;
}

long GetIntFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return -1;
    }
    
    if (!PyInt_Check(o)) {
        PyErr_Format(PyExc_ValueError, "Key %s doesn't seem to be integer", key);
        return -1;
    }

    return PyInt_AsLong(o);
}

unsigned char *GetStringFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    PyObject        *u;
    Py_UNICODE      *ps;
    unsigned char   *gs;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }

    u = PyUnicode_FromObject(o);
    if (u == NULL) {
        PyErr_Format(PyExc_ValueError, "Value for key %s can not be converted to unicode object", key);
        return NULL;
    }

    ps = PyUnicode_AsUnicode(u);
    if (ps == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get unicode value for key %s", key);
        return NULL;
    }
    gs = strPythonToGammu(ps);
    Py_DECREF(u);
    return gs;
}

GSM_DateTime GetDateTimeFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    GSM_DateTime    dt;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        dt.Year = -1;
        return dt;
    }

    if (!BuildGSMDateTime(o, &dt)) {
        dt.Year = -1;
    }
    return dt;
}

char *GetCharFromDict(PyObject *dict, char *key) {
    PyObject        *o;
    char            *ps;
    
    o = PyDict_GetItemString(dict, key);
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing key in dictionary: %s", key);
        return NULL;
    }

    ps = PyString_AsString(o);
    if (ps == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key %s", key);
        return NULL;
    }
    return ps;
}

GSM_MemoryType GetMemoryTypeFromDict(PyObject *dict, char *key) {
    char *s;
    s = GetCharFromDict(dict, key);
    if (s == NULL) return -1;
    
    return StringToMemoryType(s);
}


bool FillGSMMemoryEntry(PyObject *dict, GSM_MemoryEntry *entry, bool needs_location) {
    PyObject        *o;
    PyObject        *item;
    int             len;
    int             i;
    char            *type;
    char            valuetype;
    unsigned char   *s;
    
    if (needs_location) {
        entry->Location = GetIntFromDict(dict, "Location");
        if (entry->Location == -1) return false;
    }
    
    entry->Location = GetIntFromDict(dict, "PreferUnicode");
    if (entry->PreferUnicode == -1) {
        PyErr_Clear();
        entry->PreferUnicode = false;
    }
    
    entry->Location = GetMemoryTypeFromDict(dict, "MemoryType");
    if (entry->Location == -1) return false;
    
    o = PyDict_GetItemString(dict, "Values");
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key Values");
        return false;
    }
    
    if (!PyList_Check(o)) {
        PyErr_Format(PyExc_ValueError, "Key Values doesn't contain list");
        return false;
    }

    len = PyList_Size(o);
    if (len > GSM_PHONEBOOK_ENTRIES) {
        printf("WARNING: using just %i entries from list!", GSM_PHONEBOOK_ENTRIES);
        len = GSM_PHONEBOOK_ENTRIES;
    }
    entry->EntriesNum = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(o, i);
        if (item == NULL) return false;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %i in Values is not dictionary", i);
            return false;
        }
        type = GetCharFromDict(item, "Type");
        if (type == NULL) return false;

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
        } else if (strcmp("RingtoneID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_RingtoneID;
        } else if (strcmp("PictureID", type) == 0) {
            valuetype = 'n';
            entry->Entries[i].EntryType = PBK_PictureID;
        } else {
            PyErr_Format(PyExc_ValueError, "Element %i in Values has bad type: %s", i, type);
            return false;
        }

        switch (valuetype) {
            case 'n': 
                entry->Entries[i].Number = GetIntFromDict(item, "Value");
                if (entry->Entries[i].Number == -1) return false;
                break;
            case 't': 
                s = GetStringFromDict(item, "Value");
                if (s == NULL) return false;
                if (UnicodeLength(s) > GSM_PHONEBOOK_TEXT_LENGTH) {
                    printf("WARNING: Truncating text to %d chars!", GSM_PHONEBOOK_TEXT_LENGTH);
                    s[2*GSM_PHONEBOOK_TEXT_LENGTH]          = 0;
                    s[(2*GSM_PHONEBOOK_TEXT_LENGTH) + 1]    = 0;
                }
                CopyUnicodeString(entry->Entries[i].Text, s);
                free(s);
                break;
            case 'd': 
                entry->Entries[i].Date = GetDateTimeFromDict(item, "Value");
                if (entry->Entries[i].Date.Year == -1) return false;
                break;
        }
    } /* end for */
    
    return true;
}
