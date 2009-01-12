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

#include "misc.h"
#include <string.h>

char *mystrncpy(char *dest, const char *src, size_t n) {
    strncpy(dest, src, n);
    dest[n] = 0;
    return dest;
}

PRINTF_STYLE(1, 2)
int pyg_error(const char *format, ...) {
	va_list ap;
	int ret;

	printf("python-gammu: ERROR: ");

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

PRINTF_STYLE(1, 2)
int pyg_warning(const char *format, ...) {
	va_list ap;
	int ret;

	printf("python-gammu: WARNING: ");

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

