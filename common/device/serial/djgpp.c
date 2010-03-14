
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifdef DJGPP

#include "../../gsmcomon.h"
#include "djgpp.h"

/* Close the serial port and restore old settings. */
static GSM_Error serial_close(GSM_StateMachine *s)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;

    return GE_NONE;
}

static GSM_Error serial_open (GSM_StateMachine *s)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;
    
    return GE_NONE;
}

static GSM_Error serial_setparity(GSM_StateMachine *s, bool parity)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;

    return GE_NONE;
}

/* Set the DTR and RTS bit of the serial device. */
static GSM_Error serial_setdtrrts(GSM_StateMachine *s, bool dtr, bool rts)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;

    return GE_NONE;
//    return GE_DEVICEDTRRTSERROR;
}

/* Change the speed of the serial device.
 * RETURNS: Success
 */
static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;
    return GE_NONE;
}

/* Read from serial device. */
static int serial_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;
    return 0;
}

/* Write to serial device. */
static int serial_write(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    GSM_Device_SerialData *d = &s->Device.Data.Serial;

    return 0;
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
