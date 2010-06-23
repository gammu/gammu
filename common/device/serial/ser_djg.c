
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifdef DJGPP

#include "../../gsmcomon.h"
#include "ser_djg.h"

static GSM_Error serial_close(GSM_StateMachine *s)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

	return ERR_NONE;
}

static GSM_Error serial_open (GSM_StateMachine *s)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;
    
    	return ERR_NONE;
}

static GSM_Error serial_setparity(GSM_StateMachine *s, bool parity)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

    	return ERR_NONE;
}

static GSM_Error serial_setdtrrts(GSM_StateMachine *s, bool dtr, bool rts)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

    	return ERR_NONE;
}

static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

    	return ERR_NONE;
}

static int serial_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

    	return 0;
}

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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
