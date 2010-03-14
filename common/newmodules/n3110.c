/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for accessing functions on the N3110 and similar
  phones.

*/

/* "Turn on" prototypes in n-3110.h */

#define __n_3110_c 

/* System header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
  #include "misc_win32.h"
#endif
 
/* Various header file */
#ifndef VC6
  #include "config.h"
#endif

#include "misc.h"
#include "gsm-api.h"
#include "gsm-common.h"
#include "files/cfgreader.h"
#include "newmodules/n3110.h"
#include "newmodules/n6110.h"
#include "gsm-networks.h"

/* Here we initialise model specific functions. */

GSM_Functions N3110_Functions = {
  N3110_Initialise,
  N3110_DispatchMessage,
  NULL_Terminate,
  NULL_KeepAlive,
  NOTSUPPORTED,
  NOTSUPPORTED,  
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED
};



/* for all these Phones we support N3110 */

GSM_Information N3110_Information = {
  "", /* Supported models in FBUS */
  "",/* Supported models in MBUS */
  "",/* Supported models in FBUS over infrared */
  "",/* Supported models in FBUS over DLR3 */
  "",
  "", /* Supported models in FBUS over Irda sockets */
  "",
  "3110|3810|8110|8110i|8148|8148i", /* FBUS version 1 */
  4,                     /* Max RF Level */
  0,                     /* Min RF Level */
  GRF_Arbitrary,         /* RF level units */
  4,                     /* Max Battery Level */
  0,                     /* Min Battery Level */
  GBU_Arbitrary,         /* Battery level units */
  GDT_None,              /* No date/time support */
  GDT_None,	         /* No alarm support */
  0                      /* Max alarms = 0 */
};

/* Initialise variables and state machine. */
GSM_Error N3110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{
  u8 init[20]={0x02, 0x01, 0x07, 0xa2, 0x88, 0x81, 0x21, 0x55, 0x63, 0xa8,
               0x00, 0x00, 0x07, 0xa3, 0xb8, 0x81, 0x20, 0x15, 0x63, 0x80};

  unsigned char init_char = N6110_SYNC_BYTE;

  int count;
  int InitLength;

  GSM_Error error;
    if (Protocol->Initialise(port_device,initlength,connection,rlp_callback)!=GE_NONE)
  {
    return GE_NOTSUPPORTED;
  }

  InitLength = atoi(initlength);

  if ((strcmp(initlength, "default") == 0) || (InitLength == 0)) {
    InitLength = 250;	/* This is the usual value, lower may work. */
  }

#ifdef DEBUG
  fprintf(stdout,_("Writing init chars...."));
#endif

  /* Initialise link with phone or what have you */
  /* Send init string to phone, this is a bunch of 0x55 characters. Timing is
     empirical. */
  for (count = 0; count < InitLength; count ++) {
    usleep(1000);
    Protocol->WritePhone(1,&init_char);
  }

#ifdef DEBUG
  fprintf(stdout,_("Done\n"));  
#endif

  error=NULL_SendMessageSequence (200, &CurrentGetIMEIError, 20, 0x15, init);
  if (error!=GE_NONE) return error;

  CurrentLinkOK = true;                           

  return (GE_NONE);
}


void N3110_DispatchMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
}
