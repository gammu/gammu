/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.

*/

#ifndef __fbus3110_h
#define __fbus3110_h

#include "gsm-api.h"

#define FBUS3110_FRAME_TO_PHONE 0x01     /* Command from PC to phone */
#define FBUS3110_DATAFRAME_TO_PHONE 0x02 /* Data call frame from PC to phone */
#define FBUS3110_FRAME_TO_PC 0x03        /* Command from phone to PC */
#define FBUS3110_DATAFRAME_TO_PC 0x04    /* Data call frame from phone to PC */

#define FBUS3110_MAX_TRANSMIT_LENGTH (256)
#define FBUS3110_MAX_RECEIVE_LENGTH  (512)
#define FBUS3110_MAX_CONTENT_LENGTH  (120)

/* Global variables */
extern GSM_Protocol    FBUS3110_Functions;

GSM_Error FBUS3110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

int FBUS3110_SendMessage(u16 message_length, u8 message_type, u8 *buffer);
int FBUS3110_SendFrame(u16 message_length, u8 message_type, u8 *buffer);
void FBUS3110_Terminate(void);
void FBUS3110_RX_StateMachine(unsigned char rx_byte);

#endif /* __FBUS3110_h */
