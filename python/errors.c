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

#include "pyg-config.h"
#include "errors.h"
#include "convertors.h"

PyObject *GammuError;

PyObject **gammu_error_map;

int checkError(GSM_StateMachine *s, GSM_Error error, const char *where) {
    PyObject    *val;
    PyObject    *text;
    PyObject    *err = GammuError;
    const char  *msg;

    if (error == ERR_NONE) return 1;

    if (error < ERR_LAST_VALUE && error >= ERR_NONE)
        err = gammu_error_map[error];

    msg = GSM_ErrorString(error);

    text = LocaleStringToPython(msg);
    if (text == NULL) {
        return 0;
    }

    val = Py_BuildValue("{s:O,s:s,s:i}",
            "Text", text,
            "Where", where,
            "Code", error);
    Py_DECREF(text);

    if (val == NULL) {
        PyErr_Format(err, "GSM Error %d (%s) in %s", error, msg, where);
    } else {
        PyErr_SetObject(err, val);
        Py_DECREF(val);
    }

    return 0;
}

int gammu_create_errors(PyObject *d) {
    PyObject *help_text;
    PyObject *error_dict;
    PyObject *error_list;
    PyObject *errornumber_list;
    PyObject *id;
    GSM_Error error;
    char errname[100];

    /* Prepare error code -> python objects mapping */
    gammu_error_map = (PyObject **)malloc(sizeof(PyObject *) * ((int)ERR_LAST_VALUE + 1));
    if (gammu_error_map == NULL)
        return 0;

    /* Prepare dict for error codes */
    error_list = PyDict_New();
    if (error_list == NULL)
        return 0;
    errornumber_list = PyDict_New();
    if (errornumber_list == NULL)
        return 0;

    /* Help text */
    help_text = PyString_FromString("Generic class as parent for all gammu exceptions. This is never raised directly.");
    if (help_text == NULL)
        return 0;
    error_dict = PyDict_New();
    if (error_dict == NULL)
        return 0;
    PyDict_SetItemString(error_dict, "__doc__", help_text);
    Py_DECREF(help_text);

    /* Create exception */
    GammuError = PyErr_NewException("gammu.GSMError", NULL, error_dict);
    Py_DECREF(error_dict);
    if (GammuError == NULL)
        return 0;
    PyDict_SetItemString(d, "GSMError", GammuError);
    Py_DECREF(GammuError);

    /* Create exception objects and mappings */
	for (error = ERR_NONE; error < ERR_LAST_VALUE; error++) {

        /* Do we know name of this exception? */
        if (GSM_ErrorName(error) == NULL) {
            pyg_error("failed to convert error code %d to string!\n", error);
            continue;
        }

        /* Help text */
        help_text = PyString_FromFormat("Exception corresponding to gammu error ERR_%s.\n"
                "Verbose error description: %s",
                GSM_ErrorName(error),
                GSM_ErrorString(error));
        if (help_text == NULL)
            return 0;

        /* Exception dictionary (contains only help for now) */
        error_dict = PyDict_New();
        if (error_dict == NULL)
            return 0;
        PyDict_SetItemString(error_dict, "__doc__", help_text);
        Py_DECREF(help_text);

        /* Create exception */
        strcpy(errname, "gammu.ERR_");
        strcat(errname, GSM_ErrorName(error));
        gammu_error_map[error] = PyErr_NewException(errname, GammuError, error_dict);
        Py_DECREF(error_dict);
        if (gammu_error_map[error] == NULL)
            return 0;

        /* Add it to module dictionary */
        strcpy(errname, "ERR_");
        strcat(errname, GSM_ErrorName(error));
        PyDict_SetItemString(d, errname, gammu_error_map[error]);
        Py_DECREF(gammu_error_map[error]);

        /* Store ids in dictionary */
        id = PyInt_FromLong(error);
        if (id == NULL)
            return 0;
        PyDict_SetItemString(error_list, errname, id);
        PyDict_SetItem(errornumber_list, id, PyString_FromString(errname));
        Py_DECREF(id);
    }

    /* Add ID dictionaries to module */
    PyDict_SetItemString(d, "Errors", error_list);
    Py_DECREF(error_list);

    PyDict_SetItemString(d, "ErrorNumbers", errornumber_list);
    Py_DECREF(errornumber_list);

    return 1;
}
