/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See AT-6110.c for more details.

*/

#ifndef __AT_h
#define __AT_h

#include "gsm-api.h"

#define AT_MAX_RECEIVE_LENGTH 2000
#define AT_MAX_CONTENT_LENGTH 2000

/* Global variables */
extern GSM_Protocol    AT_Functions;

/* States for receive code. */

GSM_Error AT_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

int AT_SendMessage(u16 message_length, u8 message_type, u8 *buffer);
int AT_SendFrame(u16 message_length, u8 message_type, u8 *buffer);
void AT_Terminate(void);
void AT_RX_StateMachine(unsigned char rx_byte);
bool AT_WritePhone (u16 length, u8 *buffer);

#endif /* __AT_h */
