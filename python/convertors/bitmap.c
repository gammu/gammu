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

/* Bitmaps related conversions */

#include "convertors.h"
#include "misc.h"

char *BitmapTypeToString(GSM_Bitmap_Types type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case GSM_None:
            s = strdup("None");
            break;
        case GSM_ColourStartupLogo_ID:
            s = strdup("ColourStartupLogo_ID");
            break;
        case GSM_StartupLogo:
            s = strdup("StartupLogo");
            break;
        case GSM_ColourOperatorLogo_ID:
            s = strdup("ColourOperatorLogo_ID");
            break;
        case GSM_OperatorLogo:
            s = strdup("OperatorLogo");
            break;
        case GSM_ColourWallPaper_ID:
            s = strdup("ColourWallPaper_ID");
            break;
        case GSM_CallerGroupLogo:
            s = strdup("CallerGroupLogo");
            break;
        case GSM_DealerNote_Text:
            s = strdup("DealerNote_Text");
            break;
        case GSM_WelcomeNote_Text:
            s = strdup("WelcomeNote_Text");
            break;
        case GSM_PictureImage:
            s = strdup("PictureImage");
            break;
        case GSM_PictureBinary:
            s = strdup("PictureBinary");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for BitmapType from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_Bitmap_Types StringToBitmapType(char *s){
    if (strcmp("None", s) == 0) return GSM_None;
    else if (strcmp("ColourStartupLogo_ID", s) == 0) return GSM_ColourStartupLogo_ID;
    else if (strcmp("StartupLogo", s) == 0) return GSM_StartupLogo;
    else if (strcmp("ColourOperatorLogo_ID", s) == 0) return GSM_ColourOperatorLogo_ID;
    else if (strcmp("OperatorLogo", s) == 0) return GSM_OperatorLogo;
    else if (strcmp("ColourWallPaper_ID", s) == 0) return GSM_ColourWallPaper_ID;
    else if (strcmp("CallerGroupLogo", s) == 0) return GSM_CallerGroupLogo;
    else if (strcmp("DealerNote_Text", s) == 0) return GSM_DealerNote_Text;
    else if (strcmp("WelcomeNote_Text", s) == 0) return GSM_WelcomeNote_Text;
    else if (strcmp("PictureImage", s) == 0) return GSM_PictureImage;
    else if (strcmp("PictureBinary", s) == 0) return GSM_PictureBinary;

    PyErr_Format(PyExc_MemoryError, "Bad value for MultiPartSMSID '%s'", s);
    return 0;
}

int BitmapFromPython(PyObject *dict, GSM_Bitmap *entry) {
    char        *s;
    int         i, j;
    int         w, h, cols, chars;
    char        c, black = 0, transp = 0;
    char        buffer[1000];
    int         x, y;
    PyObject    *o;
    PyObject    *item;
    int         len;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "Bitmap is not a dictionary");
        return 0;
    }

    memset(entry, 0, sizeof(GSM_Bitmap));

    s = GetCharFromDict(dict, "Type");
    if (s == NULL) return 0;
    entry->Type = StringToBitmapType(s);
    if (entry->Type == 0) return 0;

    s = GetCharFromDict(dict, "NetworkCode");
    if (s == NULL) {
        PyErr_Clear();
        entry->NetworkCode[0] = 0;
    } else {
        mystrncpy(entry->NetworkCode, s, 6);
    }

    i = GetIntFromDict(dict, "Location");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->Location = i;
    }

    i = GetIntFromDict(dict, "BitmapEnabled");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->BitmapEnabled = i;
    }
     i = GetIntFromDict(dict, "DefaultName");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->DefaultName = i;
    }
     i = GetIntFromDict(dict, "DefaultBitmap");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->DefaultBitmap = i;
    }
     i = GetIntFromDict(dict, "DefaultRingtone");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->DefaultRingtone = i;
    }
     i = GetIntFromDict(dict, "RingtoneID");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->RingtoneID = i;
    }
     i = GetIntFromDict(dict, "ID");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
        entry->ID = i;
    }

    if (!CopyStringFromDict(dict, "Text", GSM_BITMAP_TEXT_LENGTH, entry->Text)) {
        PyErr_Clear();
        entry->Text[0] = 0;
        entry->Text[1] = 0;
    }

    if (!CopyStringFromDict(dict, "Sender", GSM_MAX_NUMBER_LENGTH, entry->Sender)) {
        PyErr_Clear();
        entry->Text[0] = 0;
        entry->Text[1] = 0;
    }

    o = PyDict_GetItemString(dict, "XPM");
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Could not get XPM for bitmap!");
        return 0;
    }

    if (!PyList_Check(o)) {
        PyErr_Format(PyExc_ValueError, "XPM isn't list!");
        return 0;
    }

    len = PyList_Size(o);

    if (len < 3) {
        PyErr_Format(PyExc_ValueError, "XPM list too small!");
        return 0;
    }

#define GetString(s, x) \
    item = PyList_GetItem(o, x);\
    if (!PyString_Check(item)) {\
        PyErr_Format(PyExc_ValueError, "XPM contains something different than string!");\
        return 0;\
    }\
\
    s = PyString_AsString(item);\
    if (s == NULL) {\
        PyErr_Format(PyExc_ValueError, "XPM contains something different than string!");\
        return 0;\
    }

    GetString(s, 0);

    if (sscanf(s,"%d %d %d %d", &w, &h, &cols, &chars) != 4) {
        PyErr_Format(PyExc_ValueError, "Bad first XPM row");
        return 0;
    }

    if (chars != 1 || cols != 2) {
        PyErr_Format(PyExc_ValueError, "Only two color XPM with one char per pixel supported");
        return 0;
    }

    if (w > 255 || h > 255 || w < 0 || h < 0 || w * h / 8 > GSM_BITMAP_SIZE) {
        PyErr_Format(PyExc_ValueError, "Bad size of bitmap");
        return 0;
    }

    entry->BitmapWidth = w;
    entry->BitmapHeight = h;

    for (i = 1; i < 3; i++) {
        GetString(s, i);

        if (sscanf(s, "%c c %999s", &c, buffer) != 2) {
            PyErr_Format(PyExc_ValueError, "Can not parse XPM line: '%s'", s);
            return 0;
        }

        j = 0;
        while (buffer[j] != 0) {
            buffer[j] = tolower(buffer[j]);
            j++;
        }

        if (
                strcmp(buffer, "none") == 0 ||
                strcmp(buffer, "#fff") == 0 ||
                strcmp(buffer, "#ffffff") == 0 ||
                strcmp(buffer, "white") == 0
           ) transp = c;
        else if (
                strcmp(buffer, "#000") == 0 ||
                strcmp(buffer, "#000000") == 0 ||
                strcmp(buffer, "black") == 0
           ) black = c;
        else {
            PyErr_Format(PyExc_ValueError, "Only black and write are supported (guven was %s)", buffer);
            return 0;
        }
    }
    if (black == 0 || transp == 0) {
        PyErr_Format(PyExc_ValueError, "At least one XPM color was not found");
        return 0;
    }
    for (y = 0; y < h; y++) {
        GetString(s, y + 3);
        if ((ssize_t)strlen(s) != w) {
            PyErr_Format(PyExc_ValueError, "XPM line has bad length: '%s'", s);
            return 0;
        }
        for (x = 0; x < w; x++) {
            if (s[x] == black) GSM_SetPointBitmap(entry, x, y);
            else if (s[x] == transp) GSM_ClearPointBitmap(entry, x, y);
            else {
                PyErr_Format(PyExc_ValueError, "Bad character in XPM data: '%c'", s[x]);
                return 0;
            }
        }
    }

    return 1;
}

PyObject *BitmapToPython(GSM_Bitmap *bitmap) {
    char        buffer[1000];
    size_t      x,y;
    PyObject    *xpmval;
    PyObject    *s;
    char        *t;
    Py_UNICODE  *txt;
    Py_UNICODE  *sendr;
    PyObject    *val;

    xpmval = PyList_New(0);
    if (xpmval == NULL) return NULL;

#if 0
    /* Not needed as BitmapWidth is char */
    if (bitmap->BitmapWidth > 999) {
        PyErr_SetString(PyExc_MemoryError, "Maximal supported bitmap width is 999 for now!");
        return NULL;
    }
#endif

    snprintf(buffer, 99, "%i %i 2 1", (int)bitmap->BitmapWidth, (int)bitmap->BitmapHeight);
    s = PyString_FromString(buffer);
    if (s == NULL) return NULL;
    if (PyList_Append(xpmval, s) != 0) {
        Py_DECREF(xpmval);
        Py_DECREF(s);
        return NULL;
    }
    Py_DECREF(s);

    s = PyString_FromString("# c Black");
    if (s == NULL) return NULL;
    if (PyList_Append(xpmval, s) != 0) {
        Py_DECREF(xpmval);
        Py_DECREF(s);
        return NULL;
    }
    Py_DECREF(s);

    s = PyString_FromString("  c None");
    if (s == NULL) return NULL;
    if (PyList_Append(xpmval, s) != 0) {
        Py_DECREF(xpmval);
        Py_DECREF(s);
        return NULL;
    }
    Py_DECREF(s);

    buffer[bitmap->BitmapWidth] = 0;

    for (y = 0; y < bitmap->BitmapHeight; y++) {
        for (x = 0; x < bitmap->BitmapWidth; x++) {
            buffer[x] = GSM_IsPointBitmap(bitmap,x,y) ? '#' : ' ';
        }
        s = PyString_FromString(buffer);
        if (s == NULL) return NULL;
        if (PyList_Append(xpmval, s) != 0) {
            Py_DECREF(xpmval);
            Py_DECREF(s);
            return NULL;
        }
        Py_DECREF(s);
    }

    t = BitmapTypeToString(bitmap->Type);
    if (t == NULL) {
        Py_DECREF(xpmval);
        return NULL;
    }

    txt = strGammuToPython(bitmap->Text);
    if (txt == NULL) {
        Py_DECREF(xpmval);
        free(t);
        return NULL;
    }

    sendr = strGammuToPython(bitmap->Sender);
    if (sendr == NULL) {
        Py_DECREF(xpmval);
        free(t);
        free(txt);
        return NULL;
    }

    val = Py_BuildValue("{s:s,s:i,s:u,s:i,s:i,s:i,s:i,s:i,s:i,s:O,s:u,s:s}",
            "Type",             t,
            "Location",         (int)bitmap->Location,
            "Text",             txt,
            "Enabled",          (int)bitmap->BitmapEnabled,
            "DefaultName",      (int)bitmap->DefaultName,
            "DefaultBitmap",    (int)bitmap->DefaultBitmap,
            "DefaultRingtone",  (int)bitmap->DefaultRingtone,
            "RingtoneID",       (int)bitmap->RingtoneID,
            "ID",               (int)bitmap->ID,
            "XPM",              xpmval,
            "Sender",           sendr,
            "NetworkCode",      bitmap->NetworkCode
            );

    Py_DECREF(xpmval);
    free(t);
    free(txt);
    free(sendr);

    return val;
}

int MultiBitmapFromPython(PyObject *list, GSM_MultiBitmap *bmp) {
    PyObject    *item;
    Py_ssize_t  len;
    Py_ssize_t  i;

    if (!PyList_Check(list)) {
        PyErr_Format(PyExc_ValueError, "Multi bitmap is not a list");
        return 0;
    }

    len = PyList_Size(list);

    if (len > GSM_MAX_MULTI_BITMAP) {
        pyg_warning("Truncating Multi Bitmap entries to %d entries! (from %" PY_FORMAT_SIZE_T "d))\n", GSM_MAX_MULTI_BITMAP, len);
        len = GSM_MAX_MULTI_BITMAP;
    }
    bmp->Number = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(list, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %" PY_FORMAT_SIZE_T "d in Bitmaps is not dictionary", i);
            return 0;
        }
        if (!BitmapFromPython(item, &(bmp->Bitmap[i]))) return 0;
    }

    return 1;
}

PyObject *MultiBitmapToPython(GSM_MultiBitmap *bmp) {
    PyObject    *val;
    PyObject    *item;
    int         i;

    val = PyList_New(0);
    if (val == NULL) return NULL;

    for (i=0; i < bmp->Number; i++) {
        item = BitmapToPython(&(bmp->Bitmap[i]));
        if (item == NULL) {
            Py_DECREF(val);
            return NULL;
        }
        if (PyList_Append(val, item) != 0) {
            Py_DECREF(item);
            Py_DECREF(val);
            return NULL;
        }
        Py_DECREF(item);
    }

    return val;
}
