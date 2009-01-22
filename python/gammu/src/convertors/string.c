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

/* Unicode strings conversion between Gammu and Python */

#include "convertors.h"

unsigned char *StringPythonToGammu(PyObject *o) {
    PyObject        *u;
    Py_UNICODE      *ps;
    unsigned char   *gs;

    u = PyObject_Unicode(o);
    if (u == NULL) {
        PyErr_Format(PyExc_ValueError, "Value can not be converted to unicode object");
        return NULL;
    }

    ps = PyUnicode_AsUnicode(u);
    if (ps == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get unicode value");
        return NULL;
    }
    gs = strPythonToGammu(ps, PyUnicode_GetSize(u));
    Py_DECREF(u);
    return gs;
}

unsigned char *strPythonToGammu(const Py_UNICODE *src, const size_t len) {
    unsigned char *dest;
    size_t i;

    /* Allocate memory */
    dest = malloc((len + 1) * 2 * sizeof(char));
    if (dest == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    /* Convert and copy string. */
    for (i = 0; i < len; i++) {
        dest[(i * 2)]  	    = (src[i] >> 8) & 0xff;
        dest[(i * 2) + 1]   = src[i] & 0xff;
    }

	/* Zero terminate string. */
	dest[(len * 2)]     = 0;
	dest[(len * 2) + 1] = 0;

    return dest;
}

Py_UNICODE *strGammuToPython(const unsigned char *src) {
    int len = 0;

    /* Get string length */
    while (src[len*2] != 0 || src[(len*2)+1] != 0 ) len++;

    return strGammuToPythonL(src, len);
}

Py_UNICODE *strGammuToPythonL(const unsigned char *src, const int len) {
    Py_UNICODE *dest;
    int i;

    /* Allocate memory */
    dest = malloc((len + 1) * sizeof(Py_UNICODE));
    if (dest == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    /* Convert string including zero at the end. */
    for (i = 0; i <= len; i++) {
        dest[i] = (src[2*i] << 8) + src[(2*i) + 1];
    }

    return dest;
}

PyObject *UnicodeStringToPython(const unsigned char *src) {
    Py_ssize_t  len;

    len = UnicodeLength(src);
    return UnicodeStringToPythonL(src, len);
}

PyObject *UnicodeStringToPythonL(const unsigned char *src, const Py_ssize_t len) {
    Py_UNICODE  *val;
    PyObject    *res;

    val = strGammuToPythonL(src, len);
    if (val == NULL) return NULL;
    res = PyUnicode_FromUnicode(val, len);
    free(val);
    return res;
}

PyObject *LocaleStringToPython(const char *src) {
    unsigned char   *w;
    size_t          len;
    PyObject        *ret;

    /* Length of input */
    len = strlen(src);

    /* Allocate it */
    w = malloc(2 * (len + 5));
    if (w == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    EncodeUnicode(w, src, len);

    ret = UnicodeStringToPython(w);
    free(w);
    return ret;
}
