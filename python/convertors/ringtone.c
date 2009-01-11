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

/* Ringtones related conversions */

#include "pyg-config.h"
#include "convertors.h"
#include "misc.h"

char *RingNoteDurationToString(GSM_RingNoteDuration type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case Duration_Full:
            s = strdup("Full");
            break;
        case Duration_1_2:
            s = strdup("1_2");
            break;
        case Duration_1_4:
            s = strdup("1_4");
            break;
        case Duration_1_8:
            s = strdup("1_8");
            break;
        case Duration_1_16:
            s = strdup("1_16");
            break;
        case Duration_1_32:
            s = strdup("1_32");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteDuration from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_RingNoteDuration StringToRingNoteDuration(char *s){
    if (strcmp("Full", s) == 0) return Duration_Full;
    else if (strcmp("1_2", s) == 0) return Duration_1_2;
    else if (strcmp("1_4", s) == 0) return Duration_1_4;
    else if (strcmp("1_8", s) == 0) return Duration_1_8;
    else if (strcmp("1_16", s) == 0) return Duration_1_16;
    else if (strcmp("1_32", s) == 0) return Duration_1_32;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteDuration '%s'", s);
    return ENUM_INVALID;
}

char *RingNoteDurationSpecToString(GSM_RingNoteDurationSpec type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case NoSpecialDuration:
            s = strdup("NoSpecialDuration");
            break;
        case DottedNote:
            s = strdup("DottedNote");
            break;
        case DoubleDottedNote:
            s = strdup("DoubleDottedNote");
            break;
        case Length_2_3:
            s = strdup("Length_2_3");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteDurationSpec from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_RingNoteDurationSpec StringToRingNoteDurationSpec(char *s){
    if (strcmp("NoSpecialDuration", s) == 0) return NoSpecialDuration;
    else if (strcmp("DottedNote", s) == 0) return DottedNote;
    else if (strcmp("DoubleDottedNote", s) == 0) return DoubleDottedNote;
    else if (strcmp("Length_2_3", s) == 0) return Length_2_3;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteDurationSpec '%s'", s);
    return ENUM_INVALID;
}

char *RingNoteNoteToString(GSM_RingNoteNote type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case Note_Pause:
            s = strdup("Pause");
            break;
        case Note_C:
            s = strdup("C");
            break;
        case Note_Cis:
            s = strdup("Cis");
            break;
        case Note_D:
            s = strdup("D");
            break;
        case Note_Dis:
            s = strdup("Dis");
            break;
        case Note_E:
            s = strdup("E");
            break;
        case Note_F:
            s = strdup("F");
            break;
        case Note_Fis:
            s = strdup("Fis");
            break;
        case Note_G:
            s = strdup("G");
            break;
        case Note_Gis:
            s = strdup("Gis");
            break;
        case Note_A:
            s = strdup("A");
            break;
        case Note_Ais:
            s = strdup("Ais");
            break;
        case Note_H:
            s = strdup("H");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteNote from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_RingNoteNote StringToRingNoteNote(char *s){
    if (strcmp("Pause", s) == 0) return Note_Pause;
    else if (strcmp("C", s) == 0) return Note_C;
    else if (strcmp("Cis", s) == 0) return Note_Cis;
    else if (strcmp("D", s) == 0) return Note_D;
    else if (strcmp("Dis", s) == 0) return Note_Dis;
    else if (strcmp("E", s) == 0) return Note_E;
    else if (strcmp("F", s) == 0) return Note_F;
    else if (strcmp("Fis", s) == 0) return Note_Fis;
    else if (strcmp("G", s) == 0) return Note_G;
    else if (strcmp("Gis", s) == 0) return Note_Gis;
    else if (strcmp("A", s) == 0) return Note_A;
    else if (strcmp("Ais", s) == 0) return Note_Ais;
    else if (strcmp("H", s) == 0) return Note_H;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteNote '%s'", s);
    return ENUM_INVALID;
}

char *RingNoteStyleToString(GSM_RingNoteStyle type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case NaturalStyle:
            s = strdup("Natural");
            break;
        case ContinuousStyle:
            s = strdup("Continuous");
            break;
        case StaccatoStyle:
            s = strdup("Staccato");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteStyle from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_RingNoteStyle StringToRingNoteStyle(char *s){
    if (strcmp("Natural", s) == 0) return NaturalStyle;
    else if (strcmp("Continuous", s) == 0) return ContinuousStyle;
    else if (strcmp("Staccato", s) == 0) return StaccatoStyle;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteStyle '%s'", s);
    return ENUM_INVALID;
}

char *RingCommandTypeToString(GSM_RingCommandType type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case RING_Note:
            s = strdup("Note");
            break;
        case RING_EnableVibra:
            s = strdup("EnableVibra");
            break;
        case RING_DisableVibra:
            s = strdup("DisableVibra");
            break;
        case RING_EnableLight:
            s = strdup("EnableLight");
            break;
        case RING_DisableLight:
            s = strdup("DisableLight");
            break;
        case RING_EnableLED:
            s = strdup("EnableLED");
            break;
        case RING_DisableLED:
            s = strdup("DisableLED");
            break;
        case RING_Repeat:
            s = strdup("Repeat");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingCommandType from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_RingCommandType StringToRingCommandType(char *s) {
    if (strcmp("Note", s) == 0) return RING_Note;
    else if (strcmp("EnableVibra", s) == 0) return RING_EnableVibra;
    else if (strcmp("DisableVibra", s) == 0) return RING_DisableVibra;
    else if (strcmp("EnableLight", s) == 0) return RING_EnableLight;
    else if (strcmp("DisableLight", s) == 0) return RING_DisableLight;
    else if (strcmp("EnableLED", s) == 0) return RING_EnableLED;
    else if (strcmp("DisableLED", s) == 0) return RING_DisableLED;
    else if (strcmp("Repeat", s) == 0) return RING_Repeat;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingCommandType '%s'", s);
    return 0;
}

GSM_RingNoteScale IntToRingNoteScale(int v) {
    if (v == 55) return Scale_55;
    else if (v == 110) return Scale_110;
    else if (v == 220) return Scale_220;
    else if (v == 440) return Scale_440;
    else if (v == 880) return Scale_880;
    else if (v == 1760) return Scale_1760;
    else if (v == 3520) return Scale_3520;
    else if (v == 7040) return Scale_7040;
    else if (v == 14080) return Scale_14080;

    PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteScale %d", v);
    return 0;
}

int RingNoteScaleToInt(GSM_RingNoteScale type) {
    int v = -1;

    switch (type) {
        case Scale_55:
            v = 55;
            break;
        case Scale_110:
            v = 110;
            break;
        case Scale_220:
            v = 220;
            break;
        case Scale_440:
            v = 440;
            break;
        case Scale_880:
            v = 880;
            break;
        case Scale_1760:
            v = 1760;
            break;
        case Scale_3520:
            v = 3520;
            break;
        case Scale_7040:
            v = 7040;
            break;
        case Scale_14080:
            v = 14080;
            break;
    }

    if (v == -1) {
        PyErr_Format(PyExc_ValueError, "Bad value for GSM_RingNoteScale from Gammu: '%d'", type);
        return -1;
    }

    return v;
}

PyObject *RingCommadToPython(GSM_RingCommand *cmd) {
    PyObject        *result;
    char            *type, *style, *note, *durspec, *duration;
    int             scale;

    type = RingCommandTypeToString(cmd->Type);
    if (type == NULL) return NULL;

    scale = RingNoteScaleToInt(cmd->Note.Scale);
    if (scale == -1) {
        free(type);
        return NULL;
    }

    style = RingNoteStyleToString(cmd->Note.Style);
    if (style == NULL) {
        free(type);
        return NULL;
    }

    note = RingNoteNoteToString(cmd->Note.Note);
    if (note == NULL) {
        free(style);
        free(type);
        return NULL;
    }

    durspec = RingNoteDurationSpecToString(cmd->Note.DurationSpec);
    if (durspec == NULL) {
        free(style);
        free(type);
        free(note);
        return NULL;
    }

    duration = RingNoteDurationToString(cmd->Note.Duration);
    if (duration == NULL) {
        free(durspec);
        free(style);
        free(type);
        free(note);
        return NULL;
    }

    result = Py_BuildValue("{s:i,s:s,s:i,s:i,s:s,s:s,s:s,s:s}",
            "Value",            (int)cmd->Value,
            "Type",             type,
            "Tempo",            cmd->Note.Tempo,
            "Scale",            scale,
            "Style",            style,
            "Note",             note,
            "DurationSpec",     durspec,
            "Duration",         duration
            );

    free(duration);
    free(durspec);
    free(style);
    free(type);
    free(note);

    return result;
}

int RingCommadFromPython(PyObject *dict, GSM_RingCommand *cmd) {
    int             i;
    char            *s;

    i = GetIntFromDict(dict, "Value");
    if (i == INT_INVALID) return 0;
    cmd->Value = i;

    s = GetCharFromDict(dict, "Type");
    if (s == NULL) return 0;
    cmd->Type = StringToRingCommandType(s);
    if (cmd->Type == 0) return 0;

    i = GetIntFromDict(dict, "Tempo");
    if (i == INT_INVALID) return 0;
    cmd->Note.Tempo = i;

    i = GetIntFromDict(dict, "Scale");
    if (i == INT_INVALID) return 0;
    cmd->Note.Scale = IntToRingNoteScale(i);
    if (cmd->Note.Scale == 0) return 0;

    s = GetCharFromDict(dict, "Style");
    if (s == NULL) return 0;
    cmd->Note.Style = StringToRingNoteStyle(s);
    if (cmd->Note.Style == ENUM_INVALID) return 0;

    s = GetCharFromDict(dict, "Note");
    if (s == NULL) return 0;
    cmd->Note.Note = StringToRingNoteNote(s);
    if (cmd->Note.Note == ENUM_INVALID) return 0;

    s = GetCharFromDict(dict, "DurationSpec");
    if (s == NULL) return 0;
    cmd->Note.DurationSpec = StringToRingNoteDurationSpec(s);
    if (cmd->Note.DurationSpec == ENUM_INVALID) return 0;

    s = GetCharFromDict(dict, "Duration");
    if (s == NULL) return 0;
    cmd->Note.Duration = StringToRingNoteDuration(s);
    if (cmd->Note.Duration == ENUM_INVALID) return 0;

    return 1;
}

PyObject *RingtoneToPython(GSM_Ringtone *inring) {
    GSM_Ringtone    ring;
    PyObject        *name;
    PyObject        *v;
    PyObject        *f;
    PyObject        *result;
    int             i;

    if (inring->Format != RING_NOTETONE) {
        if (GSM_RingtoneConvert(&ring, inring, RING_NOTETONE) != ERR_NONE) {
            pyg_warning("Ringtone can not be converted to RING_NOTETONE, ignoring!\n");
            Py_RETURN_NONE;
        }
    } else {
        ring = *inring;
    }

    v = PyList_New(0);
    if (v == NULL) return NULL;

    for (i = 0; i < ring.NoteTone.NrCommands; i++) {
        f = RingCommadToPython(&(ring.NoteTone.Commands[i]));
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

    name = UnicodeStringToPython(ring.Name);
    if (name == NULL) {
        Py_DECREF(v);
        return NULL;
    }

    result = Py_BuildValue("{s:i,s:O,s:O}",
            "AllNotesScale",        (int)ring.NoteTone.AllNotesScale,
            "Name",                 name,
            "Notes",                v
            );
    Py_DECREF(v);
    Py_DECREF(name);

    return result;
}

int RingtoneFromPython(PyObject *dict, GSM_Ringtone *ring) {
    Py_ssize_t  i;
    Py_ssize_t  len;
    PyObject    *list;
    PyObject    *item;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "Ringtone is not a dictionary");
        return 0;
    }

    memset(ring, 0, sizeof(GSM_Ringtone));

    /* FIXME: change constant to some define */
    if (!CopyStringFromDict(dict, "Name", 19, ring->Name)) {
        return 0;
    }

    list = PyDict_GetItemString(dict, "Notes");

    if (!PyList_Check(list)) {
        PyErr_Format(PyExc_ValueError, "Notes are not a list");
        return 0;
    }

    len = PyList_Size(list);

    if (len > GSM_MAX_RINGTONE_NOTES) {
        pyg_warning("Truncating Notes entries to %d entries! (from %" PY_FORMAT_SIZE_T "d))\n", GSM_MAX_RINGTONE_NOTES, len);
        len = GSM_MAX_RINGTONE_NOTES;
    }
    ring->NoteTone.NrCommands = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(list, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %" PY_FORMAT_SIZE_T "d in Notes is not a dictionary", i);
            return 0;
        }
        if (!RingCommadFromPython(item, &(ring->NoteTone.Commands[i]))) return 0;
    }

    return 1;
}
