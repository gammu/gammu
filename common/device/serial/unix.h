
#ifndef WIN32
#ifndef DJGPP
#ifndef unixserial_h
#define unixserial_h

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

typedef struct {
    int 		hPhone;
    struct termios 	backup_termios; /* Structure to backup the setting of the terminal. */
} GSM_Device_SerialData;

#endif
#endif
#endif
