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

/* File related conversions */

#include "convertors.h"
#include "misc.h"

char *FileTypeToString(GSM_FileType type) {
    char *s = NULL;

    if (type == 0) {
        /* 0 can be valid for folders? If handled inside switch, gcc complains */
        s = strdup("");
    } else {
        switch (type) {
            case GSM_File_Java_JAR:
                s = strdup("Java_JAR");
                break;
            case GSM_File_Image_JPG:
                s = strdup("Image_JPG");
                break;
            case GSM_File_Image_BMP:
                s = strdup("Image_BMP");
                break;
            case GSM_File_Image_GIF:
                s = strdup("Image_GIF");
                break;
            case GSM_File_Image_PNG:
                s = strdup("Image_PNG");
                break;
            case GSM_File_Image_WBMP:
                s = strdup("Image_WBMP");
                break;
            case GSM_File_Video_3GP:
                s = strdup("Video_3GP");
                break;
            case GSM_File_Sound_AMR:
                s = strdup("Sound_AMR");
                break;
            case GSM_File_Sound_NRT:
                s = strdup("Sound_NRT");
                break;
            case GSM_File_Sound_MIDI:
                s = strdup("Sound_MIDI");
                break;
            case GSM_File_MMS:
                s = strdup("MMS");
                break;
            case GSM_File_Other:
                s = strdup("Other");
                break;
        }
    }

    if (s == NULL) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_FileType from Gammu: '%d'", type);
        return NULL;
    }

    return s;
}

GSM_FileType StringToFileType(char *s){
    if (strcmp("Java_JAR", s) == 0) return GSM_File_Java_JAR;
    else if (strcmp("Image_JPG", s) == 0) return GSM_File_Image_JPG;
    else if (strcmp("Image_BMP", s) == 0) return GSM_File_Image_BMP;
    else if (strcmp("Image_GIF", s) == 0) return GSM_File_Image_GIF;
    else if (strcmp("Image_PNG", s) == 0) return GSM_File_Image_PNG;
    else if (strcmp("Image_WBMP", s) == 0) return GSM_File_Image_WBMP;
    else if (strcmp("Video_3GP", s) == 0) return GSM_File_Video_3GP;
    else if (strcmp("Sound_AMR", s) == 0) return GSM_File_Sound_AMR;
    else if (strcmp("Sound_NRT", s) == 0) return GSM_File_Sound_NRT;
    else if (strcmp("Sound_MIDI", s) == 0) return GSM_File_Sound_MIDI;
    else if (strcmp("MMS", s) == 0) return GSM_File_MMS;
    else if (strcmp("Other", s) == 0) return GSM_File_Other;
    else if (strlen(s) == 0) return 0;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_FileType '%s'", s);
    return ENUM_INVALID;
}

PyObject *FileToPython(GSM_File *file) {
    PyObject    *name;
    PyObject    *fullname;
    PyObject    *buffer;
    PyObject    *dt;
    char        *type;
    PyObject    *val;

    name = UnicodeStringToPython(file->Name);
    if (name == NULL) {
        return NULL;
    }

    type = FileTypeToString(file->Type);
    if (type == NULL) {
        Py_DECREF(name);
        return NULL;
    }

    fullname = UnicodeStringToPython(file->ID_FullName);
    if (fullname == NULL) {
        Py_DECREF(name);
        free(type);
        return NULL;
    }

    buffer = PyString_FromStringAndSize((char *)file->Buffer, file->Used);
    if (buffer == NULL) {
        Py_DECREF(name);
        free(type);
        Py_DECREF(fullname);
        return NULL;
    }

    if (file->ModifiedEmpty) {
        file->Modified.Year = 0;
    }
    dt = BuildPythonDateTime(&(file->Modified));
    if (dt == NULL) {
        Py_DECREF(name);
        free(type);
        Py_DECREF(fullname);
        Py_DECREF(buffer);
        return NULL;
    }


    val = Py_BuildValue("{s:i,s:O,s:i,s:i,s:s,s:O,s:O,s:O,s:i,s:i,s:i,s:i}",
            "Used",         file->Used,
            "Name",         name,
            "Folder",       (int)file->Folder,
            "Level",        file->Level,
            "Type",         type,
            "ID_FullName",  fullname,
            "Buffer",       buffer,
            "Modified",     dt,
            "Protected",    (int)file->Protected,
            "ReadOnly",     (int)file->ReadOnly,
            "Hidden",       (int)file->Hidden,
            "System",       (int)file->System);

    Py_DECREF(name);
    free(type);
    Py_DECREF(fullname);
    Py_DECREF(buffer);
    Py_DECREF(dt);
    return val;
}

int FileFromPython(PyObject *dict, GSM_File *file, gboolean check) {
    char            *s;
    GSM_DateTime    nulldt = {0,0,0,0,0,0,0};
    Py_ssize_t      i;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "File is not a dictionary");
        return 0;
    }

    memset(file, 0, sizeof(file));

    if ((file->Used = GetIntFromDict(dict, "Used")) == INT_INVALID) {
        // We don't need Used in all cases, we can get size from buffer
        PyErr_Clear();
    }

    if (!CopyStringFromDict(dict, "Name", 300, file->Name)) {
        if (check) {
            return 0;
        } else {
            PyErr_Clear();
        }
    }

    if ((i = GetIntFromDict(dict, "Folder")) == INT_INVALID) {
        if (check) {
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        file->Folder = i;
    }

    if ((file->Level = GetIntFromDict(dict, "Level")) == INT_INVALID) {
        if (check) {
            return 0;
        } else {
            PyErr_Clear();
        }
    }

    s = GetCharFromDict(dict, "Type");
    if (s == NULL) {
        file->Type = 0;
        if (check) {
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        file->Type = StringToFileType(s);
        if (file->Type == ENUM_INVALID) return 0;
    }

    if (!CopyStringFromDict(dict, "ID_FullName", 800, file->ID_FullName)) {
        if (check) {
            return 0;
        } else {
            PyErr_Clear();
        }
    }

    s = GetDataFromDict(dict, "Buffer", &i);
    if (s == NULL) {
        file->Buffer = NULL;
        file->Used = 0;
        if (check) {
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        if (file->Used == INT_INVALID) {
            file->Used = i;
        } else if (file->Used != (size_t)i) {
            PyErr_Format(PyExc_ValueError, "Used and Buffer size mismatch in File (%li != %" PY_FORMAT_SIZE_T "i)!", (long)file->Used, i);
            return 0;
        }
        file->Buffer = (unsigned char *)malloc(i * sizeof(unsigned char));
        if (file->Buffer == NULL) {
            PyErr_Format(PyExc_MemoryError, "Not enough memory to allocate buffer (wanted %li bytes)", (long)(i * sizeof(unsigned char)));
            return 0;
        }
        memcpy(file->Buffer, s, i);
    }

    file->Modified = GetDateTimeFromDict(dict, "Modified");
    if (file->Modified.Year == -1) {
        file->Modified = nulldt;
        file->ModifiedEmpty = TRUE;
        PyErr_Clear();
    } else {
        file->ModifiedEmpty = FALSE;
    }

    if ((i = GetIntFromDict(dict, "Protected")) == INT_INVALID) {
        if (check) {
            free(file->Buffer);
            file->Buffer = NULL;
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        file->Protected = i;
    }

    if ((i = GetIntFromDict(dict, "ReadOnly")) == INT_INVALID) {
        if (check) {
            free(file->Buffer);
            file->Buffer = NULL;
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        file->ReadOnly = i;
    }

    if ((i = GetIntFromDict(dict, "Hidden")) == INT_INVALID) {
        if (check) {
            free(file->Buffer);
            file->Buffer = NULL;
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        file->Hidden = i;
    }

    if ((i = GetIntFromDict(dict, "System")) == INT_INVALID) {
        if (check) {
            free(file->Buffer);
            file->Buffer = NULL;
            return 0;
        } else {
            PyErr_Clear();
        }
    } else {
        file->System = i;
    }

    return 1;
}
