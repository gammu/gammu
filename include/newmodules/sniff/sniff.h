/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See fbus-sniff.c for more details.

*/

#ifndef __n_sniff_h
#define __n_sniff_h

#include "gsm-api.h"

/* Global variables */
extern bool            Nsniff_LinkOK;
extern GSM_Functions   Nsniff_Functions;
extern GSM_Information Nsniff_Information;

/* Prototypes for the functions designed to be used externally. */
GSM_Error Nsniff_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

void      Nsniff_DispatchMessage(u16 MessageLength, u8 *MessageBuffer,
                              u8 MessageType);

#endif /* __n_sniff_h */
