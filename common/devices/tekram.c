/*
 * $Id: tekram.c,v 1.2 2001/02/21 19:57:03 chris Exp $
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

#include <stdio.h>
#include <string.h>

#ifndef WIN32
  #include <fcntl.h>
  #include <sys/ioctl.h>
  #include <termios.h>
  #include "devices/unixserial.h"
#else
  #include <windows.h>
  #include "devices/winserial.h"
#endif

#include "devices/tekram.h"

int tekram_open(__const char *__file) {

  return (serial_open(__file, O_RDWR | O_NOCTTY | O_NONBLOCK));
}

void tekram_close(int __fd) {

  serial_setdtrrts(__fd, 0, 0);

  serial_close(__fd);
}

void tekram_reset(int __fd) {

  serial_setdtrrts(__fd, 0, 0); usleep(50000);
  serial_setdtrrts(__fd, 1, 0); usleep(1000);
  serial_setdtrrts(__fd, 1, 1); usleep(50);

  serial_changespeed(__fd, 9600);
}

void tekram_changespeed(int __fd, int __speed) {

  unsigned char speedbyte;


  switch (__speed) {

  default:
  case 9600:   speedbyte = TEKRAM_PW | TEKRAM_B9600;   break;
  case 19200:  speedbyte = TEKRAM_PW | TEKRAM_B19200;  break;
  case 38400:  speedbyte = TEKRAM_PW | TEKRAM_B38400;  break;
  case 57600:  speedbyte = TEKRAM_PW | TEKRAM_B57600;  break;
  case 115200: speedbyte = TEKRAM_PW | TEKRAM_B115200; break;

  }


  tekram_reset(__fd);

  serial_setdtrrts(__fd, 1, 0);

  usleep(7);

  serial_write(__fd, &speedbyte, 1);

  usleep(100000);

  serial_setdtrrts(__fd, 1, 1);


  serial_changespeed(__fd, __speed);
}

size_t tekram_read(int __fd, __ptr_t __buf, size_t __nbytes) {

  return (serial_read(__fd, __buf, __nbytes));
}

size_t tekram_write(int __fd, __const __ptr_t __buf, size_t __n) {

  return (serial_write(__fd, __buf, __n));
}

int tekram_select(int fd, struct timeval *timeout) {

  fd_set readfds;

  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);

  return (select(fd + 1, &readfds, NULL, NULL, timeout));

}
