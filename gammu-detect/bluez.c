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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <gammu.h>		/* For PRINTF_STYLE and locales */
#include "../helper/locales.h"	/* For gettext */

#include "config.h"

#include "bluez.h"
#include "main.h"

/* Generated from http://standards.ieee.org/regauth/oui/oui.txt */

typedef char bluez_prefix[9];

const bluez_prefix at_prefixes[] = {
	/* Sony-Ericsson */
	"00:01:EC",
	"00:0A:D9",
	"00:0E:07",
	"00:0F:DE",
	"00:12:EE",
	"00:15:E0",
	"00:16:20",
	"00:16:B8",
	"00:18:13",
	"00:19:63",
	"00:1A:75",
	"00:1B:59",
	"00:1C:A4",
	"00:1D:28",
	"00:1E:45",
	"00:1E:DC",
	"00:1F:E4",
	"00:21:9E",
	"00:22:98",
	"00:23:45",
	"00:23:F1",
	"00:24:EF",
	"00:25:E7",
	"00:80:37",
	"24:21:AB",
	"30:17:C8",
	"40:2B:A1",
	"58:17:0C",
	"6C:0E:0D",
	"6C:23:B9",
	"90:55:AE",
	"B4:0E:DC",
	"B8:F9:34",
	"C8:35:B8",
	"D0:F0:DB",
	/* Siemens */
	"00:01:E3",
	"00:05:19",
	"00:0B:23",
	"00:0B:A3",
	"00:0D:41",
	"00:0E:8C",
	"00:0F:BB",
	"00:11:06",
	"00:11:33",
	"00:13:A3",
	"00:18:65",
	"00:18:D1",
	"00:19:28",
	"00:1A:E8",
	"00:1B:1B",
	"00:1C:06",
	"00:1F:F8",
	"00:23:41",
	"00:30:05",
	"00:40:43",
	"00:50:07",
	"00:90:40",
	"00:A0:03",
	"00:C0:E4",
	"08:00:06",
	"40:EC:F8",
	"88:4B:39",
	/* Samsung */
	"00:00:F0",
	"00:02:78",
	"00:09:18",
	"00:0D:AE",
	"00:0D:E5",
	"00:12:47",
	"00:12:FB",
	"00:13:77",
	"00:15:99",
	"00:15:B9",
	"00:16:32",
	"00:16:6B",
	"00:16:6C",
	"00:16:DB",
	"00:17:C9",
	"00:17:D5",
	"00:18:AF",
	"00:1A:8A",
	"00:1B:98",
	"00:1C:43",
	"00:1D:25",
	"00:1D:F6",
	"00:1E:7D",
	"00:1E:E1",
	"00:1E:E2",
	"00:1F:CC",
	"00:1F:CD",
	"00:21:19",
	"00:21:4C",
	"00:21:D1",
	"00:21:D2",
	"00:23:39",
	"00:23:3A",
	"00:23:99",
	"00:23:C2",
	"00:23:D6",
	"00:23:D7",
	"00:24:54",
	"00:24:90",
	"00:24:91",
	"00:24:E9",
	"00:25:38",
	"00:25:66",
	"00:25:67",
	"00:26:37",
	"00:26:5D",
	"00:26:5F",
	"00:E0:64",
	"10:1D:C0",
	"34:C3:AC",
	"38:01:97",
	"3C:8B:FE",
	"44:4E:1A",
	"44:F4:59",
	"54:92:BE",
	"60:A1:0A",
	"60:D0:A9",
	"68:EB:AE",
	"78:25:AD",
	"A0:07:98",
	"A0:0B:BA",
	"A8:F2:74",
	"B4:07:F9",
	"BC:47:60",
	"C8:7E:75",
	"D4:88:90",
	"D4:E8:B2",
	"E4:7C:F9",
	"E4:E0:C5",
	"E8:11:32",
	"E8:E5:D6",
	"EC:E0:9B",
	"F4:9F:54",
	"FC:A1:3E",
	/* LG */
	"00:1C:62",
	"00:1E:75",
	"00:1F:6B",
	"00:1F:E3",
	"00:21:FB",
	"00:22:A9",
	"00:24:83",
	"00:25:E5",
	"00:26:E2",
	"00:E0:91",
	"20:21:A5",
	"6C:D6:8A",
	"E8:5B:5B",
	/* BenQ */
	"00:03:9D",
	"00:17:CA",
	"00:1E:21",
	/* Motorola */
	"00:03:E0",
	"00:04:56",
	"00:04:BD",
	"00:08:0E",
	"00:0A:28",
	"00:0B:06",
	"00:0C:E5",
	"00:0E:5C",
	"00:0E:C7",
	"00:0F:9F",
	"00:11:1A",
	"00:11:80",
	"00:11:AE",
	"00:12:25",
	"00:12:8A",
	"00:12:C9",
	"00:13:71",
	"00:14:04",
	"00:14:9A",
	"00:14:E8",
	"00:15:2F",
	"00:15:70",
	"00:15:9A",
	"00:15:A8",
	"00:16:26",
	"00:16:75",
	"00:16:B5",
	"00:17:00",
	"00:17:84",
	"00:17:E2",
	"00:17:EE",
	"00:18:A4",
	"00:18:C0",
	"00:19:2C",
	"00:19:5E",
	"00:19:A6",
	"00:19:C0",
	"00:1A:1B",
	"00:1A:66",
	"00:1A:77",
	"00:1A:AD",
	"00:1A:DB",
	"00:1A:DE",
	"00:1B:52",
	"00:1B:DD",
	"00:1C:11",
	"00:1C:12",
	"00:1C:C1",
	"00:1C:FB",
	"00:1D:6B",
	"00:1D:BE",
	"00:1E:46",
	"00:1E:5A",
	"00:1E:8D",
	"00:1F:7E",
	"00:1F:C4",
	"00:20:40",
	"00:20:75",
	"00:21:1E",
	"00:21:36",
	"00:21:43",
	"00:21:80",
	"00:22:10",
	"00:22:B4",
	"00:23:0B",
	"00:23:68",
	"00:23:74",
	"00:23:75",
	"00:23:95",
	"00:23:A2",
	"00:23:A3",
	"00:23:AF",
	"00:23:ED",
	"00:23:EE",
	"00:24:37",
	"00:24:92",
	"00:24:93",
	"00:24:95",
	"00:24:A0",
	"00:24:A1",
	"00:24:C1",
	"00:25:F1",
	"00:25:F2",
	"00:26:36",
	"00:26:41",
	"00:26:42",
	"00:26:BA",
	"00:50:E3",
	"00:90:9C",
	"00:A0:BF",
	"00:D0:88",
	"00:E0:0C",
	"00:E0:6F",
	"2C:9E:5F",
	"3C:75:4A",
	"40:83:DE",
	"40:FC:89",
	"48:2C:EA",
	"5C:0E:8B",
	"64:ED:57",
	"74:E7:C6",
	"74:F6:12",
	"A4:ED:4E",
	"E4:64:49",
	"E4:83:99",
	"F8:7B:7A",
	/* Alcatel */
	"00:07:72",
	"00:08:9A",
	"00:0E:86",
	"00:0F:62",
	"00:11:3F",
	"00:11:8B",
	"00:15:3F",
	"00:16:4D",
	"00:17:CC",
	"00:19:8F",
	"00:1A:F0",
	"00:1C:8E",
	"00:1D:4C",
	"00:20:32",
	"00:20:60",
	"00:20:DA",
	"00:21:05",
	"00:21:35",
	"00:21:AE",
	"00:23:3E",
	"00:25:BA",
	"00:80:21",
	"00:80:39",
	"00:80:9F",
	"00:A0:81",
	"00:C0:BE",
	"00:D0:95",
	"00:D0:F6",
	"00:E0:B1",
	"00:E0:DA",
	"0C:A4:02",
	"18:42:2F",
	"18:80:F5",
	"24:AF:4A",
	"38:52:1A",
	"48:F8:E1",
	"68:59:7F",
	"6C:BE:E9",
	"7C:20:64",
	"8C:90:D3",
	"90:67:B5",
	"",
	};
const bluez_prefix nokia_prefixes[] = {
	/* Nokia */
	"00:02:EE",
	"00:0B:E1",
	"00:0E:ED",
	"00:0F:BB",
	"00:10:B3",
	"00:11:9F",
	"00:12:62",
	"00:13:70",
	"00:13:FD",
	"00:14:A7",
	"00:15:2A",
	"00:15:A0",
	"00:15:DE",
	"00:16:4E",
	"00:16:BC",
	"00:17:4B",
	"00:17:B0",
	"00:18:0F",
	"00:18:42",
	"00:18:8D",
	"00:18:C5",
	"00:19:2D",
	"00:19:4F",
	"00:19:79",
	"00:19:B7",
	"00:1A:16",
	"00:1A:89",
	"00:1A:DC",
	"00:1B:33",
	"00:1B:AF",
	"00:1B:EE",
	"00:1C:35",
	"00:1C:9A",
	"00:1C:D4",
	"00:1C:D6",
	"00:1D:3B",
	"00:1D:6E",
	"00:1D:98",
	"00:1D:E9",
	"00:1D:FD",
	"00:1E:3A",
	"00:1E:3B",
	"00:1E:A3",
	"00:1E:A4",
	"00:1F:00",
	"00:1F:01",
	"00:1F:5C",
	"00:1F:5D",
	"00:1F:DE",
	"00:1F:DF",
	"00:21:08",
	"00:21:09",
	"00:21:AA",
	"00:21:AB",
	"00:21:FC",
	"00:21:FE",
	"00:22:65",
	"00:22:66",
	"00:22:FC",
	"00:22:FD",
	"00:23:B4",
	"00:24:03",
	"00:24:04",
	"00:24:7C",
	"00:24:7D",
	"00:25:47",
	"00:25:48",
	"00:25:CF",
	"00:25:D0",
	"00:26:68",
	"00:26:69",
	"00:26:CC",
	"00:40:43",
	"00:A0:8E",
	"00:BD:3A",
	"00:E0:03",
	"0C:DD:EF",
	"18:14:56",
	"18:86:AC",
	"20:D6:07",
	"2C:D2:E7",
	"30:38:55",
	"34:7E:39",
	"3C:F7:2A",
	"5C:57:C8",
	"6C:9B:02",
	"80:50:1B",
	"94:20:53",
	"9C:18:74",
	"9C:4A:7B",
	"A0:4E:04",
	"A8:7B:39",
	"A8:7E:33",
	"C0:38:F9",
	"C8:97:9F",
	"C8:DF:7C",
	"D4:CB:AF",
	"D8:75:33",
	"E0:A6:70",
	"E4:EC:10",
	"EC:9B:5B",
	"FC:E5:57",
	"",
};

int find_list(const char *match, const bluez_prefix *list)
{
	int pos;

	for (pos = 0; list[pos][0] != 0; pos++) {
		if (strncasecmp(list[pos], match, 8) == 0) {
			return 1;
		}
	}
	return 0;
}

void bluez_detect(void)
{
	inquiry_info *ii = NULL;
	int max_rsp, num_rsp;
	int dev_id, sock, len, flags;
	int i;
	char addr[19] = { 0 };
	char name[248] = { 0 };

	dev_id = hci_get_route(NULL);
	if (dev_id < 0) {
		perror("Error finding Bluetooth adapter");
		return;
	}
	sock = hci_open_dev(dev_id);
	if (sock < 0) {
		perror("Error opening Bluetooth socket");
		return;
	}

	len = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	ii = (inquiry_info *) malloc(max_rsp * sizeof(inquiry_info));

	num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	if (num_rsp < 0)
		perror("hci_inquiry");

	for (i = 0; i < num_rsp; i++) {
		ba2str(&(ii + i)->bdaddr, addr);
		memset(name, 0, sizeof(name));
		if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(name), name, 0) < 0)
			strcpy(name, "[unknown]");
		if (debug)
			printf("; %s  %s\n", addr, name);
		if (find_list(addr, at_prefixes)) {
			print_config(addr, name, "blueat");
		}
		if (find_list(addr, nokia_prefixes)) {
			print_config(addr, name, "bluephonet");
		}
	}

	free(ii);
	close(sock);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
