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
 * Copyright (c) 2010 Michal Cihar <michal@cihar.com>
 */

#include <glib.h>
#define G_UDEV_API_IS_SUBJECT_TO_CHANGE
#include <gudev/gudev.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <gammu.h>		/* For PRINTF_STYLE and locales */
#include "../helper/locales.h"	/* For gettext */

#include "config.h"

#include "udev.h"
#include "main.h"

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
	if (debug)
		vprintf(real_fmt, args);
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
	println(indent, "%-20s %s", _("Name:"), g_udev_device_get_name(device));
	println(indent, "%-20s %s", _("Type:"), g_udev_device_get_devtype(device));
	println(indent, "%-20s %s", _("Subsystem:"), g_udev_device_get_subsystem(device));
	println(indent, "%-20s %s", _("Number:"), g_udev_device_get_number(device));
	println(indent, "%-20s %s", _("Path:"), g_udev_device_get_sysfs_path(device));
	println(indent, "%-20s %s", _("Driver:"), g_udev_device_get_driver(device));
	println(indent, "%-20s %lld", _("Sequential Number:"), (long long int)g_udev_device_get_seqnum(device));
	println(indent, "%-20s %s", _("Device File:"), g_udev_device_get_device_file(device));

	println(indent, " ");
	println(indent, _("Properties:"));

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

static gboolean device_is_acm(GUdevDevice * device)
{
	GUdevDevice *parent;
	parent = g_udev_device_get_parent(device);
	if (parent) {
		/* Serial driver */
		if (g_strcmp0(g_udev_device_get_driver(parent), "cdc_acm") == 0) {
			g_object_unref(parent);
			return TRUE;
		}
		g_object_unref(parent);
	}
	return FALSE;
}

static gboolean device_is_usb_serial(GUdevDevice * device)
{
	GUdevDevice *parent;
	parent = g_udev_device_get_parent(device);
	if (parent) {
		/* Serial driver */
		if (g_strcmp0(g_udev_device_get_subsystem(parent), "usb-serial") == 0) {
			g_object_unref(parent);
			return TRUE;
		}
		g_object_unref(parent);
	}
	return FALSE;
}

static gboolean device_is_serial(GUdevDevice * device)
{
	GUdevDevice *parent;
	parent = g_udev_device_get_parent(device);
	if (parent) {
		/* Serial driver */
		if (g_strcmp0(g_udev_device_get_name(parent), "serial8250") == 0) {
			g_object_unref(parent);
			return TRUE;
		}
		g_object_unref(parent);
	}
	return FALSE;
}

static gboolean device_is_valid(GUdevDevice * device)
{
	if (device_is_serial(device)) {
		return TRUE;
	}
	if (device_is_acm(device)) {
		return TRUE;
	}
	if (device_is_usb_serial(device)) {
		return TRUE;
	}
	return FALSE;
}

static void device_dump_config(GUdevDevice * device)
{
	gchar *device_name, *name;
	device_name = g_strdup_printf("/dev/%s", g_udev_device_get_name(device));

	if (device_is_serial(device)) {
		name = g_strdup_printf(_("Phone on serial port %s"), g_udev_device_get_number(device));
	} else if (device_is_usb_serial(device)) {
		name = g_strdup_printf(_("Phone on USB serial port %s %s"), g_udev_device_get_property(device, "ID_VENDOR"), g_udev_device_get_property(device, "ID_MODEL"));
	} else if (device_is_acm(device)) {
		name = g_strdup_printf("%s %s", g_udev_device_get_property(device, "ID_VENDOR"), g_udev_device_get_property(device, "ID_MODEL"));
	} else {
		name = NULL;
	}
	print_config(device_name, name, "at");
	g_free(device_name);
	g_free(name);
}

void udev_detect(void)
{
	GUdevClient *client;
	const char *subsys[2] = { "tty", NULL };
	GList *list, *iter;

	client = g_udev_client_new(subsys);

	list = g_udev_client_query_by_subsystem(client, subsys[0]);
	for (iter = list; iter; iter = g_list_next(iter)) {
		dump_device_and_parent(G_UDEV_DEVICE(iter->data), 0);
		if (device_is_valid(G_UDEV_DEVICE(iter->data))) {
			device_dump_config(G_UDEV_DEVICE(iter->data));
		}
		g_object_unref(G_UDEV_DEVICE(iter->data));
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
