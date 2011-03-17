/*
 * python-gammu - Phone communication libary
 * Copyright © 2003 - 2010 Michal Čihař
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

#include "data.h"
#include "convertors.h"

int gammu_create_data(PyObject *d) {
    PyObject *country_list;
    PyObject *network_list;
    PyObject *val;
    int i;

    country_list = PyDict_New();
    if (country_list == NULL)
        return 0;

    for (i = 0; GSM_Countries[i].Code[0] != 0; i++) {
        val = PyString_FromString(GSM_Countries[i].Name);
        PyDict_SetItemString(country_list, GSM_Countries[i].Code, val);
        Py_DECREF(val);
    }

    PyDict_SetItemString(d, "GSMCountries", country_list);
    Py_DECREF(country_list);


    network_list = PyDict_New();
    if (network_list == NULL)
        return 0;

    for (i = 0; GSM_Networks[i].Code[0] != 0; i++) {
        val = PyString_FromString(GSM_Networks[i].Name);
        PyDict_SetItemString(network_list, GSM_Networks[i].Code, val);
        Py_DECREF(val);
    }

    PyDict_SetItemString(d, "GSMNetworks", network_list);
    Py_DECREF(network_list);

    return 1;
}

