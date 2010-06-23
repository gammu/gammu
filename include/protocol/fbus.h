/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See fbus-6110.c for more details.

*/

#ifndef __fbus_h
#define __fbus_h

#include "gsm-api.h"

/* This byte is at the beginning of all GSM Frames sent over FBUS to Nokia
   6110 phones. */
#define FBUS_FRAME_ID       0x1e

/* This byte is at the beginning of all GSM Frames sent over infrared to Nokia
   6110 phones. */
#define FBUS_IR_FRAME_ID    0x1c

#define FBUS_DEVICE_PHONE   0x00 /* Nokia mobile phone. */
#define FBUS_DEVICE_PC      0x0c /* Our PC (FBUS). */

#define FBUS_FRTYPE_ACK     0x7f /* Acknowledge of the received frame. */

#define FBUS_MAX_TRANSMIT_LENGTH (256)
#define FBUS_MAX_RECEIVE_LENGTH  (512)
#define FBUS_MAX_CONTENT_LENGTH  (120)

/* Global variables */
extern GSM_Protocol    FBUS_Functions;

/* States for receive code. */

enum FBUS_RX_States {
  FBUS_RX_Sync,
  FBUS_RX_Discarding,
  FBUS_RX_GetDestination,
  FBUS_RX_GetSource,
  FBUS_RX_GetType,
  FBUS_RX_GetLength1,
  FBUS_RX_GetLength2,
  FBUS_RX_GetMessage
};

GSM_Error FBUS_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

int FBUS_SendMessage(u16 message_length, u8 message_type, u8 *buffer);
int FBUS_SendFrame(u16 message_length, u8 message_type, u8 *buffer);
void FBUS_Terminate(void);
void FBUS_RX_StateMachine(unsigned char rx_byte);

#endif /* __fbus_h */
