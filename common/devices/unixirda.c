/*
 * $Id: unixirda.c,v 1.6 2001/08/17 00:18:12 pkot Exp $
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
#include <sys/ioctl.h>

#include "devices/unixirda.h"
#include "devices/linuxirda.h"


#ifndef AF_IRDA
#define AF_IRDA 23
#endif

#define INFO_LEN		22
#define DISCOVERY_TIMEOUT	60.0
#define DISCOVERY_SLEEP		0.4

static char *phone[] = {
        "Nokia 5210",
	"Nokia 6210", "Nokia 6250", "Nokia 6310",
	"Nokia 7110",
	"Nokia 8210", "Nokia 8310", "Nokia 8850"
};

double d_time(void)
{
	double		time;
	struct timeval	tv;
	
	gettimeofday(&tv, NULL);
	
	time = tv.tv_sec + (((double)tv.tv_usec) / 1000000.0);
	
	return time;
}

double d_sleep(double s)
{
	double		time;
	struct timeval	tv1, tv2;
	
	gettimeofday(&tv1, NULL);
	usleep(s * 1000000);
	gettimeofday(&tv2, NULL);
	
	time = tv2.tv_sec - tv1.tv_sec + (((double)(tv2.tv_usec - tv1.tv_usec)) / 1000000.0);
	
	return time;
}

static int irda_discover_device(int fd)
{

	struct irda_device_list	*list;
	struct irda_device_info	*dev;
	unsigned char		*buf;
	int			s, len, i, j;
	int 			daddr = -1;
	double			t1, t2;
	int phones = sizeof(phone) / sizeof(*phone);
	
	len = sizeof(*list) + sizeof(*dev) * 10;	// 10 = max devices in discover
	buf = malloc(len);
	list = (struct irda_device_list *)buf;
	dev = list->dev;
	
	t1 = d_time();
	
	do {
		s = len;
		memset(buf, 0, s);
		
		if (getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &s) == 0) {
			for (i = 0; (i < list->len) && (daddr == -1); i++) {
				for (j = 0; (j < phones) && (daddr == -1); j++) {
					if (strncmp(dev[i].info, phone[j], INFO_LEN) == 0) {
						daddr = dev[i].daddr;
#ifdef DEBUG
						fprintf(stdout,_("%s\t%x\n"), dev[i].info, dev[i].daddr);
#endif
					}
				}
				if (daddr == -1) {
#ifdef DEBUG
					fprintf(stdout,_("unknown: %s\t%x\n"), dev[i].info, dev[i].daddr);
#endif
				}
			}
		}
		
		if (daddr == -1) {
			d_sleep(DISCOVERY_SLEEP);
		}
		
		t2 = d_time();
		
	} while ((t2 - t1 < DISCOVERY_TIMEOUT) && (daddr == -1));
	
	free(buf);
	
	return daddr;
}

int irda_open(void)
{
	struct sockaddr_irda	peer;
	int			fd = -1, daddr;
	int     		pgrp;         
	
	
	fd = socket(AF_IRDA, SOCK_STREAM, 0);	/* Create socket */
	if (fd == -1) {
		perror("socket");
               	exit(1);
       	     }

	/* discover the devices */ 
	daddr = irda_discover_device(fd);
	if (daddr == -1)  {
			printf("irda_discover: no nokia devices found");
                	exit(1);
        	     }

	/* Arrange for the current process to receive
           SIGIO when the state of the socket changes. */
	pgrp = getpid();
	if (fcntl (fd, F_SETOWN, pgrp) < 0)
	perror("F_SETOWN");

	/*  Set the socket state for Asynchronous  */
	if (fcntl (fd, F_SETFL, FASYNC) < 0) {
		perror("fcntl");
                	exit(1);
        	     }

	peer.sir_family = AF_IRDA;
	peer.sir_lsap_sel = LSAP_ANY;
	peer.sir_addr = daddr;
	strcpy(peer.sir_name, "Nokia:PhoNet");
		
	if (connect(fd, (struct sockaddr *)&peer, sizeof(peer))) {	/* Connect to service "Nokia:PhoNet" */
		perror("connect");
		close(fd);
		fd = -1;
		}
	
	return fd;
}

int irda_close(int fd)
{
	shutdown(fd, 0);
	return close(fd);
}

int irda_write(int __fd, __const __ptr_t __bytes, int __size)
{
  int actual,ret;

   actual = 0;
   
   do {
    if ((ret = send(__fd, __bytes, __size - actual, 0)) < 0)
	return(actual);

	actual += ret;
	__bytes += ret;

    } while (actual < __size);

    return (actual);
}

int irda_read(int __fd, __ptr_t __bytes, int __size)
{
	return (recv(__fd, __bytes, __size, 0));
}

int irda_select(int fd, struct timeval *timeout)
{
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	return (select(fd + 1, &readfds, NULL, NULL, timeout));
}
