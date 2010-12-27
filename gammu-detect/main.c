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
 * Copyright (c) 2010 Michal Cihar <michal@cihar.com>
 */

#include <glib.h>
#include <glib-object.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <gammu.h>		/* For PRINTF_STYLE and locales */
#include "../helper/locales.h"	/* For gettext */

#include "config.h"
#include "main.h"

#ifdef GUDEV_FOUND
#include "udev.h"
#endif
#ifdef BLUEZ_FOUND
#include "bluez.h"
#endif

gint debug = 0;
#ifdef GUDEV_FOUND
gint no_udev = 0;
#endif
#ifdef BLUEZ_FOUND
gint no_bluez = 0;
#endif

static GOptionEntry entries[] = {
	{"debug", 'd', 0, G_OPTION_ARG_NONE, &debug, N_("Show debugging output for detecting devices."), NULL},
#ifdef GUDEV_FOUND
	{"no-udev", 'u', 0, G_OPTION_ARG_NONE, &no_udev, N_("Disables scanning of udev."), NULL},
#endif
#ifdef BLUEZ_FOUND
	{"no-bluez", 'b', 0, G_OPTION_ARG_NONE, &no_bluez, N_("Disables scanning using Bluez."), NULL},
#endif
	{NULL, 0, 0, G_OPTION_ARG_NONE, NULL, "", NULL}
};

void print_config(const gchar *device, const gchar *name, const gchar *connection)
{
	static gint section = 0;

	if (section == 0) {
		g_print("[gammu]\n");
	} else {
		g_print("[gammu%d]\n", section);
	}

	section++;

	g_print("device = %s\n", device);
	if (name != NULL) {
		g_print("name = %s\n", name);
	}
	if (connection != NULL) {
		g_print("connection = %s\n", connection);
	}
	g_print("\n");
}

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GSM_Error gerror;
	INI_Section *cfg = NULL;
	char *locales_path = NULL;

	GOptionContext *context;

	gerror = GSM_FindGammuRC(&cfg, NULL);
	if (gerror == ERR_NONE) {
		locales_path = INI_GetValue(cfg, "gammu", "gammuloc", FALSE);
	} else {
		locales_path = NULL;
	}
	GSM_InitLocales(locales_path);
#ifdef LIBINTL_LIB_FOUND
	if (locales_path != NULL) {
		bindtextdomain("gammu", locales_path);
	} else {
#if defined(LOCALE_PATH)
		bindtextdomain("gammu", LOCALE_PATH);
#else
		bindtextdomain("gammu", ".");
#endif
	}
	textdomain("gammu");
#endif

	if (cfg != NULL) {
		INI_Free(cfg);
	}

	g_type_init();

	context = g_option_context_new("");
	g_option_context_add_main_entries(context, entries, "gammu");
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_printerr(_("option parsing failed: %s\n"), error->message);
		exit(1);
	}

#ifdef GUDEV_FOUND
	if (!no_udev) {
		udev_detect();
	}
#endif
#ifdef BLUEZ_FOUND
	if (!no_bluez) {
		bluez_detect();
	}
#endif

	return 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
