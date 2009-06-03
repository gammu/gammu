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

#ifndef __pyg_misc_h__
#define __pyg_misc_h__

/* Python includes */
#include <Python.h>

/* PRINTF_STYLE definition */
#include <gammu-misc.h>

/**
 * Copies at most n chars from src to dest, dest will be \0 terminated.
 */
char *mystrncpy(char *dest, const char *src, size_t n);

PRINTF_STYLE(1, 2)
int pyg_warning(const char *format, ...);

PRINTF_STYLE(1, 2)
int pyg_error(const char *format, ...);

/* Compatiblity with older python releases */
#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_FORMAT_SIZE_T ""
#endif

/**
 * Initialisation of SMSD object.
 */
gboolean gammu_smsd_init(PyObject *m);

#endif
