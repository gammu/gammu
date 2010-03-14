
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifndef WIN32
#ifndef DJGPP

#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <termios.h>

#include "../../gsmcomon.h"
#include "unix.h"

/* Close the serial port and restore old settings. */
static GSM_Error serial_close(GSM_StateMachine *s)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;

    /*FIXME: error checking */
    tcsetattr(d->hPhone, TCSANOW, &d->backup_termios);
    close(d->hPhone); 
    
    return GE_NONE;
}

#ifndef O_NONBLOCK
#  define O_NONBLOCK  0
#endif

static GSM_Error serial_open (GSM_StateMachine *s)
{
    GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
    struct termios 	  	t;
    
    /* O_NONBLOCK MUST be used here as the CLOCAL may be currently off
     * and if DCD is down the "open" syscall would be stuck wating for DCD.
     */
    d->hPhone = open(s->Config.Device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (d->hPhone < 0) {
	GSM_OSErrorInfo(s,"open in serial_open");
	return GE_DEVICEOPENERROR;
    }

    if (tcgetattr(d->hPhone, &d->backup_termios) == -1) {
        /* Don't call serial_close since backup_termios is not valid */
	close(d->hPhone);
	GSM_OSErrorInfo(s,"tcgetattr in serial_open");
        return GE_DEVICEREADERROR;
    }

    if (tcflush(d->hPhone, TCIFLUSH) == -1) {
        serial_close(s);
	GSM_OSErrorInfo(s,"tcflush in serial_open");
	return GE_DEVICEOPENERROR;    
    }

    /* Initialise the port settings */
    memcpy(&t, &d->backup_termios, sizeof(struct termios));

    /* Set port settings for canonical input processing */
    /* without parity */
    t.c_iflag 		= IGNPAR;
    t.c_oflag 		= 0;
    /* disconnect line, 8 bits, enable receiver,
     * ignore modem lines,lower modem line after disconnect
     */
    t.c_cflag 		= B0 | CS8 | CREAD | CLOCAL | HUPCL;
    /* enable hardware (RTS/CTS) flow control (NON POSIX) */
    /* t.c_cflag 	|= CRTSCTS;			  */
    t.c_lflag 		= 0;
    t.c_cc[VMIN] 	= 1;
    t.c_cc[VTIME] 	= 0;

    if (tcsetattr(d->hPhone, TCSANOW, &t) == -1) {
        serial_close(s);
	GSM_OSErrorInfo(s,"tcsetattr in serial_open");
	return GE_DEVICEOPENERROR;    
    }

    /* Make filedescriptor asynchronous. */
    if (fcntl(d->hPhone, F_SETFL, FASYNC | FNONBLOCK) == -1) {
	serial_close(s);
	GSM_OSErrorInfo(s,"fcntl in serial_open");
	return GE_DEVICEOPENERROR;    
    }

    return GE_NONE;
}

static GSM_Error serial_setparity(GSM_StateMachine *s, bool parity)
{
    GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
    struct termios 	  	t;
    
    if (tcgetattr(d->hPhone, &t)) {
	GSM_OSErrorInfo(s,"tcgetattr in serial_setparity");
	return GE_DEVICEREADERROR;
    }

    if (parity) {
	t.c_cflag |= (PARENB | PARODD);
	t.c_iflag = 0;
    } else {
	t.c_iflag = IGNPAR;
    }

    if (tcsetattr(d->hPhone, TCSANOW, &t) == -1){
        serial_close(s);
	GSM_OSErrorInfo(s,"tcsetattr in serial_setparity");
        return GE_DEVICEPARITYERROR;
    }
    
    return GE_NONE;
}

/* Set the DTR and RTS bit of the serial device. */
static GSM_Error serial_setdtrrts(GSM_StateMachine *s, bool dtr, bool rts)
{
    GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
    struct termios 	  	t;
    unsigned int 		flags;

    if (tcgetattr(d->hPhone, &t)) {
	GSM_OSErrorInfo(s,"tcgetattr in serial_setdtrrts");
	return GE_DEVICEREADERROR;
    }

    t.c_cflag &= ~CRTSCTS;	/* Disable hardware flow control */

    if (tcsetattr(d->hPhone, TCSANOW, &t) == -1) {
        serial_close(s);
	GSM_OSErrorInfo(s,"tcsetattr in serial_setdtrrts");
	return GE_DEVICEDTRRTSERROR;    
    }
    
    flags = TIOCM_DTR;
    if (dtr) ioctl(d->hPhone, TIOCMBIS, &flags);
        else ioctl(d->hPhone, TIOCMBIC, &flags);

    flags = TIOCM_RTS;
    if (rts) ioctl(d->hPhone, TIOCMBIS, &flags);
    	else ioctl(d->hPhone, TIOCMBIC, &flags);

    flags = 0;
    ioctl(d->hPhone, TIOCMGET, &flags);

    dprintf("Serial device:");
    dprintf(" DTR is %s", 	flags&TIOCM_DTR?"up":"down");
    dprintf(", RTS is %s", 	flags&TIOCM_RTS?"up":"down");
    dprintf(", CAR is %s", 	flags&TIOCM_CAR?"up":"down");
    dprintf(", CTS is %s\n", 	flags&TIOCM_CTS?"up":"down");
    if (((flags&TIOCM_DTR)==TIOCM_DTR) != dtr) return GE_DEVICEDTRRTSERROR;
    if (((flags&TIOCM_RTS)==TIOCM_RTS) != rts) return GE_DEVICEDTRRTSERROR;

    return GE_NONE;
}

/* Change the speed of the serial device.
 * RETURNS: Success
 */
static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;
    struct termios 	  t;
    int 		  speed2=B9600;

    if (tcgetattr(d->hPhone, &t)) {
	GSM_OSErrorInfo(s,"tcgetattr in serial_setspeed");
	return GE_DEVICEREADERROR;
    }

    switch (speed) {
	case 9600:   speed2 = B9600;   break;
	case 19200:  speed2 = B19200;  break;
	case 38400:  speed2 = B38400;  break;
	case 57600:  speed2 = B57600;  break;
	case 115200: speed2 = B115200; break;
    }

    t.c_cflag |= speed2;

    /* Required in FreeBSD */
    t.c_ispeed = speed2;
    t.c_ospeed = speed2;

    if (tcsetattr(d->hPhone, TCSADRAIN, &t) == -1) {
        serial_close(s);
	GSM_OSErrorInfo(s,"tcsetattr in serial_setspeed");
	return GE_DEVICECHANGESPEEDERROR;    
    }

    return GE_NONE;
}

/* Read from serial device. */
static int serial_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;
    struct timeval 	  timeout2;
    fd_set		  readfds;
    int			  actual = 0;

    FD_ZERO(&readfds);
    FD_SET(d->hPhone, &readfds);

    timeout2.tv_sec	= 0;
    timeout2.tv_usec	= 1;

    if (select(d->hPhone+1, &readfds, NULL, NULL, &timeout2)) {
	actual = read(d->hPhone, buf, nbytes);
	if (actual == -1) GSM_OSErrorInfo(s,"serial_read");
    }
    return actual;
}

/* Write to serial device. */
static int serial_write(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
    int				ret;
    size_t			actual = 0;
	    
    do {
	if ((ret = write(d->hPhone, buf, nbytes - actual)) < 0) {
	    if (actual!=nbytes) GSM_OSErrorInfo(s,"serial_write");
	    return(actual);
	}
	actual 	+= ret;
	buf 	+= ret;
    } while (actual < nbytes);
    return (actual);
}

GSM_Device_Functions SerialDevice = {
	serial_open,
	serial_close,
	serial_setparity,
	serial_setdtrrts,
	serial_setspeed,
	serial_read,
	serial_write
};

#endif
#endif
#endif
