/* (c) 2002-2004 by Marcin Wiacek */
/* locking device and settings all speeds by Michal Cihar */
/* based on some work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot 
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifndef WIN32
#ifndef DJGPP

#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <termios.h>
#include <errno.h>

#include "../../gsmcomon.h"
#include "ser_unx.h"

#ifndef O_NONBLOCK
#  define O_NONBLOCK  0
#endif

#ifdef __NetBSD__
#  define FNONBLOCK O_NONBLOCK

#  define  B57600   0010001
#  define  B115200  0010002
#  define  B230400  0010003
#  define  B460800  0010004
#  define  B500000  0010005
#  define  B576000  0010006
#  define  B921600  0010007
#  define  B1000000 0010010
#  define  B1152000 0010011
#  define  B1500000 0010012
#  define  B2000000 0010013
#  define  B2500000 0010014
#  define  B3000000 0010015
#  define  B3500000 0010016
#  define  B4000000 0010017
#endif

static GSM_Error serial_close(GSM_StateMachine *s)
{
	GSM_Device_SerialData *d = &s->Device.Data.Serial;

	/* Restores old settings */
	tcsetattr(d->hPhone, TCSANOW, &d->old_settings);

	/* Closes device */
	close(d->hPhone);

	return ERR_NONE;
}

static GSM_Error serial_open (GSM_StateMachine *s)
{
	GSM_Device_SerialData   *d = &s->Device.Data.Serial;
	struct termios	  	t;
	int			i;

	/* O_NONBLOCK MUST is required to avoid waiting for DCD */
	d->hPhone = open(s->CurrentConfig->Device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (d->hPhone < 0) {
		i = errno;
		GSM_OSErrorInfo(s,"open in serial_open");
		if (i ==  2) return ERR_DEVICENOTEXIST;		//no such file or directory
		if (i == 13) return ERR_DEVICENOPERMISSION;	//permission denied
		return ERR_DEVICEOPENERROR;
	}

#ifdef TIOCEXCL
	/* open() calls from other applications shall fail now */
	ioctl(d->hPhone, TIOCEXCL, (char *) 0);
#endif

	if (tcgetattr(d->hPhone, &d->old_settings) == -1) {
		close(d->hPhone);
		GSM_OSErrorInfo(s,"tcgetattr in serial_open");
		return ERR_DEVICEREADERROR;
    	}

    	if (tcflush(d->hPhone, TCIOFLUSH) == -1) {
		serial_close(s);
		GSM_OSErrorInfo(s,"tcflush in serial_open");
		return ERR_DEVICEOPENERROR;
    	}

    	memcpy(&t, &d->old_settings, sizeof(struct termios));

	/* Opening without parity */
    	t.c_iflag       = IGNPAR;
    	t.c_oflag       = 0;
    	/* disconnect line, 8 bits, enable receiver,
     	 * ignore modem lines,lower modem line after disconnect
     	 */
    	t.c_cflag       = B0 | CS8 | CREAD | CLOCAL | HUPCL;
    	/* enable hardware (RTS/CTS) flow control (NON POSIX) 	*/
    	/* t.c_cflag 	|= CRTSCTS;			  	*/
    	t.c_lflag       = 0;
    	t.c_cc[VMIN]    = 1;
    	t.c_cc[VTIME]   = 0;

    	if (tcsetattr(d->hPhone, TCSANOW, &t) == -1) {
		serial_close(s);
		GSM_OSErrorInfo(s,"tcsetattr in serial_open");
		return ERR_DEVICEOPENERROR;
    	}

    	/* Making file descriptor asynchronous. */
    	if (fcntl(d->hPhone, F_SETFL, FASYNC | FNONBLOCK) == -1) {
		serial_close(s);
		GSM_OSErrorInfo(s,"fcntl in serial_open");
		return ERR_DEVICEOPENERROR;
    	}

    	return ERR_NONE;
}

static GSM_Error serial_setparity(GSM_StateMachine *s, bool parity)
{
    	GSM_Device_SerialData   *d = &s->Device.Data.Serial;
    	struct termios	  	t;

    	if (tcgetattr(d->hPhone, &t)) {
		GSM_OSErrorInfo(s,"tcgetattr in serial_setparity");
		return ERR_DEVICEREADERROR;
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
		return ERR_DEVICEPARITYERROR;
    	}

    	return ERR_NONE;
}

static GSM_Error serial_setdtrrts(GSM_StateMachine *s, bool dtr, bool rts)
{
    	GSM_Device_SerialData   *d = &s->Device.Data.Serial;
    	struct termios	  	t;
    	unsigned int	    	flags;

    	if (tcgetattr(d->hPhone, &t)) {
		GSM_OSErrorInfo(s,"tcgetattr in serial_setdtrrts");
		return ERR_DEVICEREADERROR;
    	}

#ifdef CRTSCTS
    	/* Disabling hardware flow control */
    	t.c_cflag &= ~CRTSCTS;
#endif

    	if (tcsetattr(d->hPhone, TCSANOW, &t) == -1) {
		serial_close(s);
		GSM_OSErrorInfo(s,"tcsetattr in serial_setdtrrts");
		return ERR_DEVICEDTRRTSERROR;
    	}

    	flags = TIOCM_DTR;
    	if (dtr) {
		ioctl(d->hPhone, TIOCMBIS, &flags);
	} else {
		ioctl(d->hPhone, TIOCMBIC, &flags);
	}

    	flags = TIOCM_RTS;
    	if (rts) {
		ioctl(d->hPhone, TIOCMBIS, &flags);
	} else {
		ioctl(d->hPhone, TIOCMBIC, &flags);
	}

    	flags = 0;
    	ioctl(d->hPhone, TIOCMGET, &flags);

    	dbgprintf("Serial device:");
    	dbgprintf(" DTR is %s",       flags&TIOCM_DTR?"up":"down");
    	dbgprintf(", RTS is %s",      flags&TIOCM_RTS?"up":"down");
    	dbgprintf(", CAR is %s",      flags&TIOCM_CAR?"up":"down");
    	dbgprintf(", CTS is %s\n",    flags&TIOCM_CTS?"up":"down");
    	if (((flags&TIOCM_DTR)==TIOCM_DTR) != dtr) return ERR_DEVICEDTRRTSERROR;
    	if (((flags&TIOCM_RTS)==TIOCM_RTS) != rts) return ERR_DEVICEDTRRTSERROR;

    	return ERR_NONE;
}

static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
    	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
    	struct termios  	t;
    	int	     		speed2 = B19200;

    	if (tcgetattr(d->hPhone, &t)) {
		GSM_OSErrorInfo(s,"tcgetattr in serial_setspeed");
		return ERR_DEVICEREADERROR;
    	}

    	smprintf(s, "Setting speed to %d\n", speed);

    	switch (speed) {
		case 50:	speed2 = B50;		break;
		case 75:	speed2 = B75;		break;
		case 110:	speed2 = B110;		break;
		case 134:	speed2 = B134;		break;
		case 150:	speed2 = B150;		break;
		case 200:	speed2 = B200;		break;
		case 300:	speed2 = B300;		break;
		case 600:	speed2 = B600;		break;
		case 1200:	speed2 = B1200;		break;
		case 1800:	speed2 = B1800;		break;
		case 2400:	speed2 = B2400;		break;
		case 4800:	speed2 = B4800;		break;
		case 9600:	speed2 = B9600;		break;
		case 19200:	speed2 = B19200;	break;
		case 38400:	speed2 = B38400;	break;
#ifdef B57600
		case 57600:	speed2 = B57600;	break;
		case 115200:	speed2 = B115200;	break;
		case 230400:	speed2 = B230400;	break;
		case 460800:	speed2 = B460800;	break;
#ifdef B500000
		case 500000:	speed2 = B500000;	break;
		case 576000:	speed2 = B576000;	break;
		case 921600:	speed2 = B921600;	break;
		case 1000000:	speed2 = B1000000;	break;
		case 1152000:	speed2 = B1152000;	break;
		case 1500000:	speed2 = B1500000;	break;
		case 2000000:	speed2 = B2000000;	break;
		case 2500000:	speed2 = B2500000;	break;
		case 3000000:	speed2 = B3000000;	break;
		case 3500000:	speed2 = B3500000;	break;
		case 4000000:	speed2 = B4000000;	break;	
#endif
#endif
	}

    	/* This should work on all systems because it is done according to POSIX */
    	cfsetispeed(&t, speed2);
    	cfsetospeed(&t, speed2);

    	if (tcsetattr(d->hPhone, TCSADRAIN, &t) == -1) {
		serial_close(s);
		GSM_OSErrorInfo(s,"tcsetattr in serial_setspeed");
		return ERR_DEVICECHANGESPEEDERROR;
    	}

    	return ERR_NONE;
}

static int serial_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    	GSM_Device_SerialData 		*d = &s->Device.Data.Serial;
    	struct timeval  		timeout2;
    	fd_set	  			readfds;
    	int	     			actual = 0;

    	FD_ZERO(&readfds);
    	FD_SET(d->hPhone, &readfds);

    	timeout2.tv_sec     = 0;
    	timeout2.tv_usec    = 1;

    	if (select(d->hPhone+1, &readfds, NULL, NULL, &timeout2)) {
		actual = read(d->hPhone, buf, nbytes);
		if (actual == -1) GSM_OSErrorInfo(s,"serial_read");
    	}
    	return actual;
}

static int serial_write(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    	GSM_Device_SerialData   *d = &s->Device.Data.Serial;
    	int		     	ret;
    	size_t		  	actual = 0;

    	do {
		ret = write(d->hPhone, (unsigned char *)buf, nbytes - actual);
		if (ret < 0 && errno == EAGAIN) continue;
		if (ret < 0) {
	    		if (actual != nbytes) GSM_OSErrorInfo(s,"serial_write");
	    		return actual;
		}
		actual  += ret;
		buf     += ret;
    		if (s->ConnectionType == GCT_FBUS2PL2303) my_sleep(1);
    	} while (actual < nbytes);
    	return actual;
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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
