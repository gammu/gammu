/*
 * $Id: unixirda.h,v 1.1 2001/02/21 19:57:11 chris Exp $
 *
 * G N O K I I
 *
 * A Linux/Unix toolset and driver for Nokia mobile phones.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "linuxirda.h"
#include "misc.h"

int irda_open(void);
int irda_close(int fd);
int irda_write(int __fd, __const __ptr_t __bytes, int __size);
int irda_read(int __fd, __ptr_t __bytes, int __size);
int irda_select(int fd, struct timeval *timeout);
