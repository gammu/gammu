
#ifndef WIN32
#ifndef DJGPP
#ifndef unixserial_h
#define unixserial_h

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

typedef struct {
	int 		hPhone;
	struct termios 	old_settings;
} GSM_Device_SerialData;

#endif
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
