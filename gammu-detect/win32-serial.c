/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (c) 2011 Michal Cihar <michal@cihar.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <glib.h>

#include <gammu.h>		/* For PRINTF_STYLE and locales */
#include "config.h"

#include "win32-serial.h"
#include "main.h"
#include "../libgammu/misc/string.c"
#include "../helper/locales.h"	/* For gettext */

void win32_serial_detect(void)
{
    DWORD chars;
    char buffer[65535];
    gchar *name;
    DWORD i;

    chars = QueryDosDevice(NULL, buffer, sizeof(buffer));

    if (chars) {
        for (i = 0; buffer[i] != 0 && i < chars; i += strlen(buffer + i) + 1) {
            if (strncasecmp(buffer + i, "com", 3) == 0) {
                name = g_strdup_printf(_("Phone on serial port %s"), buffer + i);
                print_config(buffer + i, name, "at");
                g_free(name);
            }
        }
    }

}
