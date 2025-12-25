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

/**
 * \file s60phone.h
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @addtogroup S60Phone
 * @{
 */

#ifndef __s60phone_h
#define __s60phone_h

#include "../../gsmstate.h"

#define S60_TIMEOUT 60

typedef struct {
	int MajorVersion;
	int MinorVersion;
	char *MessageParts[50];
	int *SMSLocations;
	size_t SMSLocationsSize;
	size_t SMSLocationsPos;
	int *ContactLocations;
	size_t ContactLocationsSize;
	size_t ContactLocationsPos;
	int *CalendarLocations;
	size_t CalendarLocationsSize;
	size_t CalendarLocationsPos;
	int *ToDoLocations;
	size_t ToDoLocationsSize;
	size_t ToDoLocationsPos;
} GSM_Phone_S60Data;

#endif


/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
