/* Some sources from SVAsync (c) 1996, 1997, Samuel Vincent
 * 7337 Carioca Ct, Rohnert Park, Ca 94928
 * "you may freely use it in your programs without paying me anything"
 */

#ifdef DJGPP
#ifndef djgppserial_h
#define djgppserial_h

typedef struct {
	int 		hPhone;
	int 		Speed;
	unsigned int 	Control;
	bool		Parity;
} GSM_Device_SerialData;

/* Defines for Com Port Paramaters, the second paramater to SVAsyncSet() */
#define BITS_8          0x03
#define BITS_7          0x02
#define STOP_1          0x00
#define STOP_2          0x04
#define EVEN_PARITY     0x18
#define ODD_PARITY      0x08
#define NO_PARITY       0x00

/* Defines for SVAsyncHand() */
#define DTR             0x01
#define RTS             0x02
#define USER            0x04
#define LOOPBACK        0x10

/* Defines for SVAsyncStat() */
#define D_CTS   	0x0100
#define D_DSR   	0x0200
#define D_RI            0x0400
#define D_DCD   	0x0800
#define CTS             0x1000
#define DSR             0x2000
#define RI              0x4000
#define DCD             0x8000
#define PARITY  	0x0004
#define THREMPTY        0x0020
#define BREAKDET        0x1000

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
