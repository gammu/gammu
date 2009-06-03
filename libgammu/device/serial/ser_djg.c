/* Some sources from SVAsync (c) 1996, 1997, Samuel Vincent
 * 7337 Carioca Ct, Rohnert Park, Ca 94928
 * "you may freely use it in your programs without paying me anything"
 */
/* Some sources from DZCOMM */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifdef DJGPP

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "ser_djg.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dpmi.h>
#include <pc.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <conio.h>

extern unsigned short __djgpp_ds_alias;
extern void SVAsyncProtISR(void);

static unsigned char SVAsyncStatus=0;

static void lock_interrupt_memory(void);
static void unlock_interrupt_memory(void);

#define Ctrl8259_0 	0x020           /* 8259 port */
#define Ctrl8259_1 	0x021           /* 8259 port (Masks) */
#define BufSize 	32768           /* Buffer Size */

static unsigned char VectorNum;         /* Vector Number */
static unsigned char EnableIRQ;         /* Mask to enable 8259 IRQ */
static unsigned char DisableIRQ;        /* Mask to disable 8259 IRQ */
static _go32_dpmi_seginfo ProtVector;   /* Old Protmode Vector */
static _go32_dpmi_seginfo info;         /* New Protmode Vector */

/* Register Addresses for the UART */
static unsigned short Port;             /* Port Base Address */
unsigned short THR;                     /* Transmitter Holding Register */
unsigned short RDR;                     /* Reciever Data Register */
unsigned short BRDL;                    /* Baud Rate Divisor, Low byte */
unsigned short BRDH;                    /* Baud Rate Divisor, High Byte */
unsigned short IER;                     /* Interupt Enable Register */
unsigned short IIR;                     /* Interupt Identification Register */
unsigned short FCR;                     /* FIFO Control Register */
unsigned short LCR;                     /* Line Control Register */
unsigned short MCR;                     /* Modem Control Register */
unsigned short LSR;                     /* Line Status Register */
unsigned short MSR;                     /* Modem Status Register */
unsigned short SCR;                     /* SCR Register */

/* Data Buffer */
unsigned volatile char 	RecBuffer[BufSize] = { 0 };
unsigned volatile int 	RecHead, RecTail;

/* This uninstalls the ISR and resets the serial port. */
static void SVAsyncStop(void)
{
	if(!SVAsyncStatus) return;
	SVAsyncStatus = 0;

	/***** Mask (disable) 8259 IRQ Interrupt */
	outportb(Ctrl8259_1, (inportb(Ctrl8259_1) | DisableIRQ));

	/***** Disable 8250 interrupt */
	outportb(LCR, (inportb(LCR) & 0x7F));
	outportb(IER, 0);

	/***** Set bit 3 in MCR to 0 */
	outportb(MCR, (inportb(MCR) & 0xF7));

	/***** Interrupts are disabled.  Restore saved interrupt vector. */
	_go32_dpmi_set_protected_mode_interrupt_vector(VectorNum, &ProtVector);
}

/* This will empty the receive buffer */
static void SVAsyncClear(void)
{
	disable();
	RecHead = 0;
	RecTail = 0;
	enable();
}


/* Sets communication parameters
 * Baud = 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 28800, 38400, 57600
 * Control = The value to place in the LCR
 */
void SVAsyncSet(unsigned int Baud, unsigned int Control)
{
	int 		divisor;
	unsigned char 	divlow, divhigh;

	if (!Baud) return;

	divisor = 115200 / Baud;

	disable();

	outportb(LCR, Control | 0x80); 		/* Set Port Toggle to BRDL/BRDH registers */
	divlow = divisor & 0x000000ff;
	divhigh = (divisor >> 8) & 0x000000ff;
	outportb(BRDL, divlow);     		/* Set Baud Rate */
	outportb(BRDH, divhigh);

	outportb(LCR, Control & 0x007F);        /* Set LCR and Port Toggle */

	enable();
}

/* Sets various handshaking lines */
void SVAsyncHand(unsigned int Hand)
{
	outportb(MCR, Hand | 0x08);  /* Keep interrupt enable ON */
}

static void lock_interrupt_memory(void)
{
	int 		errval;
	__dpmi_meminfo 	info;
	unsigned long 	address;

	__dpmi_get_segment_base_address(_my_ds(), &address);

	info.address = (int) address + (int) &RDR;
	info.size = sizeof(RDR);
	errval = __dpmi_lock_linear_region(&info);
	if(errval == -1) printf("Error in locking memory\n!");

	info.address = (int) address + (int) &LSR;
	info.size = sizeof(LSR);
	errval = __dpmi_lock_linear_region(&info);
	if(errval == -1) printf("Error in locking memory\n!");

	info.address = (int) address + (int) &RecHead;
	info.size = sizeof(RecHead);
	errval = __dpmi_lock_linear_region(&info);
	if(errval == -1) printf("Error in locking memory\n!");

	info.address = (int) address + (int) &RecBuffer;
	info.size = sizeof(RecBuffer);
	errval = __dpmi_lock_linear_region(&info);
	if(errval == -1) printf("Error in locking memory\n!");

	info.address = (int) address + (int) RecBuffer;
	info.size = BufSize;
	errval = __dpmi_lock_linear_region(&info);
	if(errval == -1) printf("Error in locking memory\n!");

	__dpmi_get_segment_base_address(_my_cs(), &address);

	info.address = (int) address + (int) SVAsyncProtISR;
	info.size = 4096; /* 4096 bytes is probably overkill. */
	errval = __dpmi_lock_linear_region(&info);
	if(errval == -1) printf("Error in locking memory\n!");
}

static void unlock_interrupt_memory(void)
{
	__dpmi_meminfo 	info;
	unsigned long 	address;

	__dpmi_get_segment_base_address(_my_ds(), &address);
	info.address = (int) address + (int) &RDR;
	info.size = sizeof(RDR);
	__dpmi_unlock_linear_region(&info);
	info.address = (int) address + (int) &LSR;
	info.size = sizeof(LSR);
	__dpmi_unlock_linear_region(&info);
	info.address = (int) address + (int) &RecHead;
	info.size = sizeof(RecHead);
	__dpmi_unlock_linear_region(&info);
	info.address = (int) address + (int) &RecBuffer;
	info.size = sizeof(RecBuffer);
	__dpmi_unlock_linear_region(&info);
	info.address = (int) address + (int) RecBuffer;
	info.size = BufSize;
	__dpmi_unlock_linear_region(&info);

	__dpmi_get_segment_base_address(_my_cs(), &address);

	info.address = (int) address + (int) SVAsyncProtISR;
	info.size = 4096; /* probably overkill */
	__dpmi_unlock_linear_region(&info);
}

static GSM_Error serial_close(GSM_StateMachine *s)
{
	SVAsyncStop();

	return ERR_NONE;
}

static GSM_Error serial_open (GSM_StateMachine *s)
{
    	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
	unsigned char 		temp;
	int			i;

	/**** Set various things according to com port number */
	if (strcasecmp(s->CurrentConfig->Device,"com1:") == 0) {
		Port 		= 0x03F8;
		VectorNum 	= 0x0C;
		EnableIRQ 	= 0xEF;
		DisableIRQ 	= 0x10;
	} else if (strcasecmp(s->CurrentConfig->Device,"com2:") == 0) {
		Port 		= 0x02F8;
		VectorNum 	= 0x0B;
		EnableIRQ 	= 0xF7;
		DisableIRQ 	= 0x08;
	} else if (strcasecmp(s->CurrentConfig->Device,"com3:") == 0) {
		Port 		= 0x03E8;
		VectorNum 	= 0x0C;
		EnableIRQ 	= 0xEF;
		DisableIRQ 	= 0x10;
	} else if (strcasecmp(s->CurrentConfig->Device,"com4:") == 0) {
		Port 		= 0x02E8;
		VectorNum 	= 0x0B;
		EnableIRQ 	= 0xF7;
		DisableIRQ 	= 0x08;
	} else return ERR_NOTSUPPORTED;

	/**** Compute Register locations */
	THR 	= Port;
	RDR 	= Port;
	BRDL 	= Port;
	BRDH 	= 1 + Port;
	IER 	= 1 + Port;
	IIR 	= 2 + Port;
	FCR 	= 2 + Port;
	LCR 	= 3 + Port;
	MCR 	= 4 + Port;
	LSR 	= 5 + Port;
	MSR 	= 6 + Port;
	SCR 	= 7 + Port;

	/***** Initalize Buffer */
	SVAsyncClear();

	lock_interrupt_memory();
	atexit(unlock_interrupt_memory);
	/***** Set bit 3 in MCR to 0 */
	outportb(MCR, (inportb(MCR) & 0xF7));

	/*** Save and reassign interrupt vectors */

	_go32_dpmi_get_protected_mode_interrupt_vector(VectorNum, &ProtVector);

	info.pm_offset = (int) SVAsyncProtISR;
	info.pm_selector = _my_cs();
	_go32_dpmi_set_protected_mode_interrupt_vector(VectorNum, &info);

	atexit(SVAsyncStop);

	/***** Enable 8259 interrupt (IRQ) line for this async adapter */
	outportb(Ctrl8259_1, (inportb(Ctrl8259_1) & EnableIRQ));

	/***** Enable 8250 Interrupt-on-data-ready */
	outportb(LCR, (inportb(LCR) & 0x7F));

	outportb(IER, 0);
	if (inportb(IER)) {
		SVAsyncStatus = 0;
		return ERR_UNKNOWN;
	}
	outportb(IER, 0x01);

	/***** Clear 8250 Status and data registers */
	do {
		temp=inportb(RDR);
		temp=inportb(LSR);
		temp=inportb(MSR);
		temp=inportb(IIR);
	} while(!(temp & 1));

	/***** Set Bit 3 of MCR -- Enable interupts */
	outportb(MCR, (inportb(MCR) | 0x08));

	SVAsyncStatus = 1;
	/***** Clear Buffer Just in case */
	SVAsyncClear();

   	/* Code based on stuff from SVAsync lib.
    	 * Clear UART Status and data registers
    	 * setting up FIFO if possible
    	 */
   	outportb(SCR, 0x55);
   	if (inportb(SCR) == 0x55) {
      		/* On the off chance that SCR is actually hardwired to 0x55,
	         * do the same check with a different value.
	         */
      		outportb(SCR, 0xAA);
      		if (inportb(SCR) == 0xAA) {
	 		/* The chip is better than an 8250 - it has a scratch pad */
	 		outportb(SCR, i); /* Set SCR back to what it was before */
	 		inportb(SCR);     /* Give slow motherboards a chance    */

	 		/* Is there a FIFO ? - go through twice for slow motherboards */
	 		outportb(FCR, 0x01);
	 		i = inportb(FCR);
	 		outportb(FCR, 0x01);
	 		i = inportb(FCR);

	 		/* Some old stuff relies on this (no idea why) */
	 		outportb(FCR, 0x00);
	 		inportb(FCR); /* Give slow motherboards a chance */

	 		if ((i&0x80) == 0) {
				smprintf(s,"UART 16450 or UART 8250 with scratch pad\n");
	 		} else if ((i&0x40) == 0) {
				smprintf(s,"UART 16550 - broken FIFO\n");
	 		} else {
		    		/* It's a 16450A series : try and start the FIFO.
		     		* It appears that some chips need a two call protocol, but
		     		* those that don't seem to work even if you do start it
		     		* twice. The first call is simply to start it, the second
		     		* starts it and sets an 8 byte FIFO trigger level.
		     		*/
		    		outportb(FCR, 0x01);
		    		inportb(FCR); /* Give slow motherboards a chance */
		    		outportb(FCR, 0x87);
		    		inportb(FCR); /* Give slow motherboards a chance */

		    		/* Check that the FIFO initialised */
		    		if ((inportb(IIR) & 0xc0) != 0xc0) {
			       		/*
					 * It didn't so we assume it isn't there but disable it to
					 * be on the safe side.
					 */
			       		outportb(IIR, 0xfe);
			       		inportb(IIR); /* Give slow motherboards a chance */
					smprintf(s,"UART 16450A - FIFO disabled\n");
				} else {
					smprintf(s,"UART 16450A - FIFO enabled\n");
				}
			}
		} else {
			smprintf(s,"UART 8250\n");
		}
	}

	d->Control 	= BITS_8 | STOP_1;
	d->Parity 	= FALSE;
	d->Speed 	= 9600;
	SVAsyncSet(d->Speed,d->Control | NO_PARITY);

	return ERR_NONE;
}

static GSM_Error serial_setparity(GSM_StateMachine *s, gboolean parity)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

	d->Parity = parity;

	if (parity) {
		SVAsyncSet(d->Speed, d->Control | ODD_PARITY);
	} else {
		SVAsyncSet(d->Speed, d->Control | NO_PARITY);
	}

	return ERR_NONE;
}

static GSM_Error serial_setdtrrts(GSM_StateMachine *s, gboolean dtr, gboolean rts)
{
	if (dtr && rts) {
		SVAsyncHand(DTR | RTS);
	} else if (dtr) {
		SVAsyncHand(DTR);
	} else if (rts) {
		SVAsyncHand(RTS);
	} else {
		SVAsyncHand(0);
	}

	return ERR_NONE;
}

static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
    	GSM_Device_SerialData *d = &s->Device.Data.Serial;

	d->Speed = speed;

	if (d->Parity) {
		SVAsyncSet(d->Speed, d->Control | ODD_PARITY);
	} else {
		SVAsyncSet(d->Speed, d->Control | NO_PARITY);
	}

	return ERR_NONE;
}

static int serial_read(GSM_StateMachine *s, char *buf, size_t nbytes)
{
	if(RecTail == RecHead) return 0;

	disable();
        buf[0] = RecBuffer[RecTail++];
	if(RecTail >= BufSize) RecTail = 0;
	enable();

	return 1;
}

static int serial_write(GSM_StateMachine *s, char *buf, size_t nbytes)
{
	int i;

	for (i=0;i<nbytes;i++) {
		while(~inportb(LSR) & 0x20);
		outportb(THR, buf[i]);
	}

    	return i;
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
