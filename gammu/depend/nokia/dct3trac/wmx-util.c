/**
 * Generic utils
 * wumpus 2003 -- www.blacksphere.tk
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "wmx-util.h"

/**
 * Dump raw hex
 */
void dumpraw(const char *str, unsigned char *buffer, size_t length)
{
	size_t x;

	printf("[");
	for(x=0; x<length; x++) {
		if(x != (length-1)) {
			printf("%02x ",buffer[x]);
		} else {
			printf("%02x",buffer[x]);
		}
	}
	printf("]");
}

/**
 * Integer to binary
 */
void binstr(char *dest, unsigned int value, int len)
{
	int x;

	for(x=len-1; x>=0; x--) {
		dest[x] = '0' + (value & 1);
		value >>= 1;
	}
	dest[len] = 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
