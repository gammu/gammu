/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.

*/

#ifndef __n_3110_h
#define __n_3110_h

#include "gsm-api.h"

/* Global variables */
extern bool            N3110_LinkOK;
extern GSM_Functions   N3110_Functions;
extern GSM_Information N3110_Information;

/* Prototypes for the functions designed to be used externally. */
GSM_Error N3110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

void      N3110_DispatchMessage(u16 MessageLength, u8 *MessageBuffer,
                              u8 MessageType);

#endif /* __n_3110_h */
