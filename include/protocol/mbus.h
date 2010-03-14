/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See mbus-6110.c for more details.

*/

#ifndef __mbus_h
#define __mbus_h

#include "gsm-api.h"

/* This byte is at the beginning of all GSM Frames sent over MBUS to Nokia
   6110 phones. */
#define MBUS_FRAME_ID       0x1f

/* Our PC (MBUS). */
#define MBUS_DEVICE_PC1     0x10
#define MBUS_DEVICE_PC2     0x1d

#define MBUS_MAX_TRANSMIT_LENGTH (1000)
#define MBUS_MAX_RECEIVE_LENGTH  (1000)
#define MBUS_MAX_CONTENT_LENGTH  (1000)

/* Global variables */
extern GSM_Protocol    MBUS_Functions;

GSM_Error MBUS_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

int MBUS_SendMessage(u16 message_length, u8 message_type, u8 *buffer);
int MBUS_SendFrame(u16 message_length, u8 message_type, u8 *buffer);
void MBUS_Terminate(void);
bool MBUS_WritePhone (u16 length, u8 *buffer);
void MBUS_RX_StateMachine(unsigned char rx_byte);

#endif /* __fbus_h */
