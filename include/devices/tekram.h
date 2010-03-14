/*
 * $Id: tekram.h,v 1.1 2001/02/21 19:57:11 chris Exp $
 *
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

#ifndef __devices_tekram_h
#define __devices_tekram_h

#ifdef WIN32
  #include <stddef.h>
  /* FIXME: this should be solved in config.h in 0.4.0 */
  #define __const const
  typedef void * __ptr_t;
#else
  #include <unistd.h>
#endif	/* WIN32 */

#include "misc.h"


#define TEKRAM_B115200 0x00
#define TEKRAM_B57600  0x01
#define TEKRAM_B38400  0x02
#define TEKRAM_B19200  0x03
#define TEKRAM_B9600   0x04

#define TEKRAM_PW      0x10 /* Pulse select bit */


int tekram_open(__const char *__file);
void tekram_close(int __fd);

void tekram_setdtrrts(int __fd, int __dtr, int __rts);
void tekram_changespeed(int __fd, int __speed);

size_t tekram_read(int __fd, __ptr_t __buf, size_t __nbytes);
size_t tekram_write(int __fd, __const __ptr_t __buf, size_t __n);

int tekram_select(int fd, struct timeval *timeout);

#endif  /* __devices_tekram_h */




