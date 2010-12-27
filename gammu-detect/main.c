/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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
 * Copyright (C) 2009 Red Hat, Inc.
 */

#include <glib.h>
#define G_UDEV_API_IS_SUBJECT_TO_CHANGE
#include <gudev/gudev.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include <gammu.h>		/* For PRINTF_STYLE and locales */
#include "../helper/locales.h"	/* For gettext */

gint debug = 0;

static GOptionEntry entries[] = {
	{"debug", 'd', 0, G_OPTION_ARG_NONE, &debug, N_("Show debug output"), NULL},
	{NULL, 0, 0, G_OPTION_ARG_NONE, NULL, "", NULL}
};

static GMainLoop *loop = NULL;

static void signal_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_message("Caught signal %d, shutting down...", signo);
		g_main_loop_quit(loop);
	}
}

static void setup_signals(void)
{
	struct sigaction action;
	sigset_t mask;

	sigemptyset(&mask);
	action.sa_handler = signal_handler;
	action.sa_mask = mask;
	action.sa_flags = 0;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
}

PRINTF_STYLE(2, 3)
static void println(guint indent, const char *fmt, ...)
{
	va_list args;
	char real_fmt[1000] = "; ";
	guint i;

	g_return_if_fail(fmt != NULL);
	g_return_if_fail(indent < sizeof(real_fmt) - 2 - strlen(fmt));

	for (i = 0; i < indent; i++)
		real_fmt[i + 2] = ' ';
	strcpy(&real_fmt[i + 2], fmt);
	real_fmt[i + 2 + strlen(fmt)] = '\n';
	real_fmt[i + 2 + strlen(fmt) + 1] = '\0';

	va_start(args, fmt);
	if (debug) vprintf(real_fmt, args);
	va_end(args);
}

static void dump_device_and_parent(GUdevDevice * device, guint indent)
{
	const gchar *const *list;
	const gchar *const *iter;
	GUdevDevice *parent;
	char propstr[500];
	guint32 namelen = 0, i;

	println(indent, "------------------------------------------------------");
	println(indent, "Name:     %s", g_udev_device_get_name(device));
	println(indent, "Type:     %s", g_udev_device_get_devtype(device));
	println(indent, "Subsys:   %s", g_udev_device_get_subsystem(device));
	println(indent, "Number:   %s", g_udev_device_get_number(device));
	println(indent, "Path:     %s", g_udev_device_get_sysfs_path(device));
	println(indent, "Driver:   %s", g_udev_device_get_driver(device));
	println(indent, "Action:   %s", g_udev_device_get_action(device));
	println(indent, "Seq Num:  %lld", g_udev_device_get_seqnum(device));
	println(indent, "Dev File: %s", g_udev_device_get_device_file(device));

	println(indent, " ");
	println(indent, "Properties:");

	/* Get longest property name length for alignment */
	list = g_udev_device_get_property_keys(device);
	for (iter = list; iter && *iter; iter++) {
		if (strlen(*iter) > namelen)
			namelen = strlen(*iter);
	}
	namelen++;

	for (iter = list; iter && *iter; iter++) {
		strcpy(propstr, *iter);
		strcat(propstr, ":");
		for (i = 0; i < namelen - strlen(*iter); i++)
			strcat(propstr, " ");
		strcat(propstr, g_udev_device_get_property(device, *iter));
		println(indent + 2, "%s", propstr);
	}

	println(indent, " ");

	parent = g_udev_device_get_parent(device);
	if (parent) {
		dump_device_and_parent(parent, indent + 4);
		g_object_unref(parent);
	}
}

int main(int argc, char *argv[])
{
	GUdevClient *client;
	const char *subsys[2] = { "tty", NULL };
	GList *list, *iter;
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

	loop = g_main_loop_new(NULL, FALSE);

	setup_signals();

	client = g_udev_client_new(subsys);

	list = g_udev_client_query_by_subsystem(client, subsys[0]);
	for (iter = list; iter; iter = g_list_next(iter)) {
		dump_device_and_parent(G_UDEV_DEVICE(iter->data), 0);
		g_print("\n");
		g_object_unref(G_UDEV_DEVICE(iter->data));
	}

	return 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
