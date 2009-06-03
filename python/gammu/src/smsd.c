/*
 * python-gammu - Phone communication libary, SMSD part
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
 */

/* Python-gammu configuration */

/* Python includes */
#include <Python.h>

/* Gammu includes */
#include <gammu.h>
#include <gammu-smsd.h>

/* Locales */
#include <locale.h>

/* For locking */
#ifdef WITH_THREAD
#include "pythread.h"
#endif

/* Convertors between Gammu and Python types */
#include "convertors.h"

/* Error objects */
#include "errors.h"

/* Other useful stuff */
#include "misc.h"

/* Length of buffers used in most of code */
#define BUFFER_LENGTH 255

#ifdef WITH_THREAD

/* Use python locking */

#define BEGIN_PHONE_COMM \
    Py_BEGIN_ALLOW_THREADS \
    PyThread_acquire_lock(self->mutex, 1);

#define END_PHONE_COMM \
    PyThread_release_lock(self->mutex); \
    Py_END_ALLOW_THREADS \
    CheckIncomingEvents(self);

#else

/* No need for locking when no threads */
#define BEGIN_PHONE_COMM
#define END_PHONE_COMM \
    CheckIncomingEvents(self);

#endif

const char program_name[] = "python-gammu";

#define MAX_EVENTS 10

/* ----------------------------------------------------- */

/* Declarations for objects of type SMSD */
typedef struct {
    PyObject_HEAD

	GSM_SMSDConfig *config;
} SMSDObject;

/* ---------------------------------------------------------------- */

static char SMSD_MainLoop__doc__[] =
"MainLoop()\n\n"
"Runs SMS daemon.\n\n"
"@return: None\n"
"@rtype: None\n"
;

static PyObject *
Py_SMSD_MainLoop(SMSDObject *self, PyObject *args, PyObject *kwds)
{
    GSM_Error                   error;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    error = SMSD_MainLoop(self->config, FALSE);
    Py_END_ALLOW_THREADS

    if (!checkError(NULL, error, "SMSD_MainLoop")) return NULL;

    Py_RETURN_NONE;
}

static char SMSD_Shutdown__doc__[] =
"Shutdown()\n\n"
"Flags SMS daemon to stop.\n\n"
"@return: None\n"
"@rtype: None\n"
;

static PyObject *
Py_SMSD_Shutdown(SMSDObject *self, PyObject *args, PyObject *kwds)
{
    GSM_Error                   error;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    error = SMSD_Shutdown(self->config);
    Py_END_ALLOW_THREADS

    if (!checkError(NULL, error, "SMSD_Shutdown")) return NULL;

    Py_RETURN_NONE;
}

static char SMSD_GetStatus__doc__[] =
"GetStatus()\n\n"
"Returns SMSD status.\n\n"
"@return: Dict with status values\n"
"@rtype: dict\n"
;

static PyObject *
Py_SMSD_GetStatus(SMSDObject *self, PyObject *args, PyObject *kwds)
{
    GSM_Error                   error;
	GSM_SMSDStatus status;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    error = SMSD_GetStatus(self->config, &status);
    Py_END_ALLOW_THREADS

    if (!checkError(NULL, error, "SMSD_GetStatus")) return NULL;

    return Py_BuildValue("{s:s,s:s,s:s,s:i,s:i,s:i,s:i,s:i}",
            "Client", status.Client,
            "PhoneID", status.PhoneID,
            "IMEI", status.IMEI,
            "Sent", status.Sent,
            "Received", status.Received,
            "Failed", status.Failed,
            "BatterPercent", status.Charge.BatteryPercent,
            "NetworkSignal", status.Network.SignalPercent);
}

static char SMSD_InjectSMS__doc__[] =
"InjectSMS(Message)\n\n"
"Decodes multi part SMS message.\n\n"
"@param Message: Nessage to inject (can be multipart)\n"
"@type Message: list\n"
"@return: ID of inserted message\n"
"@rtype: string\n"
;

static PyObject *
Py_SMSD_InjectSMS(SMSDObject *self, PyObject *args, PyObject *kwds)
{
    GSM_MultiSMSMessage         smsin;
    static char                 *kwlist[] = {"Message", NULL};
    PyObject                    *value;
    GSM_Error                   error;
    char                        newid[200];

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist,
                &PyList_Type, &(value)))
        return NULL;

    if (!MultiSMSFromPython(value, &smsin)) return NULL;

    Py_BEGIN_ALLOW_THREADS
    error = SMSD_InjectSMS(self->config, &smsin, newid);
    Py_END_ALLOW_THREADS

    if (!checkError(NULL, error, "SMSD_InjectSMS")) return NULL;

    return Py_BuildValue("s", newid);
}

static struct PyMethodDef SMSD_methods[] = {
    {"MainLoop",  (PyCFunction)Py_SMSD_MainLoop,  METH_VARARGS|METH_KEYWORDS,   SMSD_MainLoop__doc__},
    {"Shutdown",  (PyCFunction)Py_SMSD_Shutdown,  METH_VARARGS|METH_KEYWORDS,   SMSD_Shutdown__doc__},
    {"GetStatus",  (PyCFunction)Py_SMSD_GetStatus,  METH_VARARGS|METH_KEYWORDS,   SMSD_GetStatus__doc__},
    {"InjectSMS",  (PyCFunction)Py_SMSD_InjectSMS,  METH_VARARGS|METH_KEYWORDS,   SMSD_InjectSMS__doc__},

    {NULL,		NULL, 0, NULL}		/* sentinel */
};

/* ---------- */

#if 0
static PyObject *
SMSD_getattr(SMSDObject *self, char *name)
{
    /* XXXX Add your own getattr code here */
    return  -1;
//        Py_FindMethod(SMSD_methods, (PyObject *)self, name);
}

static int
SMSD_setattr(SMSDObject *self, char *name, PyObject *v)
{
    /* Set attribute 'name' to value 'v'. v==NULL means delete */

    /* XXXX Add your own setattr code here */
    return -1;
}
#endif

#if 0
static int
SMSD_compare(SMSDObject *v, SMSDObject *w)
{
    /* XXXX Compare objects and return -1, 0 or 1 */
}

static PyObject *
SMSD_repr(SMSDObject *self)
{
    PyObject *s;

    /* XXXX Add code here to put self into s */
    return s;
}

static PyObject *
SMSD_str(SMSDObject *self)
{
    PyObject *s;

    /* XXXX Add code here to put self into s */
    return s;
}
#endif

static void
SMSD_dealloc(SMSDObject *self)
{
	SMSD_FreeConfig(self->config);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
SMSD_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SMSDObject *self;

    self = (SMSDObject *)type->tp_alloc(type, 0);

    self->config = SMSD_NewConfig(program_name);
    if (self->config == NULL)
        return NULL;

    return (PyObject *)self;
}

static int
SMSD_init(SMSDObject *self, PyObject *args, PyObject *kwds)
{
    char                *s = NULL;
    static char         *kwlist[] = {"Config", NULL};
    GSM_Error           error;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &s))
        return 0;

    error = SMSD_ReadConfig(s, self->config, TRUE);
    if (!checkError(NULL, error, "SMSD_ReadConfig"))
        return 0;

    return 1;
}

static char SMSDType__doc__[] =
"SMSD(Config)\n\n"
"SMSD object, that is used for communication with phone.\n\n"
// FIXME: following doc should go elsewhere
"param Config: Path to SMSD configuration file\n"
"type Config: string\n"
;

static PyTypeObject SMSDType = {
    PyObject_HEAD_INIT(NULL)
    0,				/*ob_size*/
    "_gammu.SMSD",			/*tp_name*/
    sizeof(SMSDObject),		/*tp_basicsize*/
    0,				/*tp_itemsize*/
    /* methods */
    (destructor)SMSD_dealloc,	/*tp_dealloc*/
    (printfunc)0,		/*tp_print*/
#if 0
    (getattrfunc)SMSD_getattr,	/*tp_getattr*/
    (setattrfunc)SMSD_setattr,	/*tp_setattr*/
#endif
    0,	/*tp_getattr*/
    0,	/*tp_setattr*/
    0,
#if 0
	(cmpfunc)SMSD_compare,		/*tp_compare*/
#endif
    0,
#if 0
	(reprfunc)SMSD_repr,		/*tp_repr*/
#endif
    0,			/*tp_as_number*/
    0,		/*tp_as_sequence*/
    0,		/*tp_as_mapping*/
    (hashfunc)0,		/*tp_hash*/
    (ternaryfunc)0,		/*tp_call*/
    0,
#if 0
	(reprfunc)SMSD_str,		/*tp_str*/
#endif
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    SMSDType__doc__, /* Documentation string */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    SMSD_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)SMSD_init,      /* tp_init */
    0,                         /* tp_alloc */
    SMSD_new,          /* tp_new */
    NULL,                      /* tp_free */
    0,                         /* tp_is_gc */
	0,                         /* tp_bases */
	0,                         /* tp_mro */
	0,                         /* tp_cache */
	0,                         /* tp_subclasses */
	0,                         /* tp_weaklist */
	0,                          /* tp_del */
#if PY_MAJOR_VERSION >= 2 && PY_MINOR_VERSION >= 6
    0,                          /* tp_version_tag */
#endif
};

/* End of code for SMSD objects */
/* -------------------------------------------------------- */

gboolean gammu_smsd_init(PyObject *m) {

    if (PyType_Ready(&SMSDType) < 0)
        return FALSE;
    Py_INCREF(&SMSDType);

    if (PyModule_AddObject(m, "SMSD", (PyObject *)&SMSDType) < 0)
        return FALSE;

    return TRUE;
}
/*
 * vim: expandtab sw=4 ts=4 sts=4:
 */
