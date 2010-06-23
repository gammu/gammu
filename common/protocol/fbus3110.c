/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for support for FBUS3110 protocol

*/

/* "Turn on" prototypes in FBUS3110.h */
#define __fbus3110_c 

/* System header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#ifdef WIN32
  #include <windows.h>
  #include "misc_win32.h"
#else
  #include <ctype.h>
#endif

/* Various header file */
#include "devices/device.h"
#include "gsm-api.h"
#include "protocol/fbus3110.h"
#include "protocol/fbus.h"
#include "newmodules/n6110.h"
#include "misc.h"

GSM_Protocol FBUS3110_Functions = {
  FBUS3110_Initialise,
  FBUS3110_SendMessage,
  FBUS3110_SendFrame,
  NULL_WritePhone,
  FBUS3110_Terminate,
  FBUS3110_RX_StateMachine
};

/* Local variables */
enum FBUS_RX_States RX_State;

u8 MessageDestination, MessageSource;

u16 BufferCount;

u16 MessageLength;

u8 MessageType;

u8 MessageBuffer[FBUS3110_MAX_RECEIVE_LENGTH * 6];

u8 RequestSequenceNumber3110=0x10;

#ifdef DEBUG	
char *N31_PrintDevice(int Device)
{
  switch (Device) {

//  case FBUS3110_DEVICE_PHONE:return _("Phone");
//  case FBUS3110_DEVICE_PC   :return _("PC");
  default               :return _("Unknown");
  }
}
#endif /* DEBUG */

/* N61_RX_DisplayMessage is called when a message we don't know about is
   received so that the user can see what is going back and forth, and perhaps
   shed some more light/explain another message type! */
void N31_RX_DisplayMessage()
{
#ifdef DEBUG
  fprintf(stdout, _("Msg Dest: %s\n"), N31_PrintDevice(MessageDestination));
  fprintf(stdout, _("Msg Source: %s\n"), N31_PrintDevice(MessageSource));
  fprintf(stdout, _("Msg Type: %02x\n"), MessageType);

  hexdump(MessageLength-2,MessageBuffer);
#endif

  AppendLog(MessageBuffer,MessageLength-2,true);
}

/* Prepares the message header and sends it, prepends the message start byte
	   (0x1e) and other values according the value specified when called.
	   Calculates checksum and then sends the lot down the pipe... */
int FBUS3110_SendFrame(u16 message_length, u8 message_type, u8 *buffer) {

  /* Originally out_buffer[FBUS3110_MAX_CONTENT_LENGTH + 2],
     but it made problems with MBUS */
  u8 out_buffer[1000];
  
  int count, current=0;
  unsigned char	checksum;

  /* Now construct the message header. */
  out_buffer[current++] = FBUS3110_FRAME_TO_PHONE;   /* Start of frame */
  out_buffer[current++] = message_length + 2;        /* Length */
  out_buffer[current++] = message_type;              /* Type */
  out_buffer[current++] = RequestSequenceNumber3110; /* Sequence number */

  /* Copy in data if any. */
  if (message_length != 0) {
    memcpy(out_buffer + current, buffer, message_length);
    current += message_length;
  }

  /* Now calculate checksum over entire message and append to message. */
  checksum = 0;
  for (count = 0; count < current; count++) checksum ^= out_buffer[count];
  out_buffer[current++] = checksum;

#ifdef DEBUG
  NULL_TX_DisplayMessage(current, out_buffer);
#endif /* DEBUG */

  /* Send it out... */
  if (!NULL_WritePhone(current,out_buffer)) return (false);

  return (true);
}

/* Any command we originate must have a unique SequenceNumber.
   Observation to date suggests that these values startx at 0x10
   and cycle up to 0x17 before repeating again.  Perhaps more
   accurately, the numbers cycle 0,1,2,3..7 with bit 4 of the byte
   premanently set. */

void FBUS3110_UpdateSequenceNumber(void)
{
  RequestSequenceNumber3110++;

  if (RequestSequenceNumber3110 > 0x17 || RequestSequenceNumber3110 < 0x10)
    RequestSequenceNumber3110 = 0x10;
}

int FBUS3110_SendMessage(u16 message_length, u8 message_type, u8 *buffer) {

  FBUS3110_UpdateSequenceNumber();

  return FBUS3110_SendFrame(message_length, message_type, buffer);
}

int FBUS3110_SendAck(u8 message_type, u8 message_seq) {

  return 0;
}

/* Applications should call FBUS3110_Terminate to shut down the FBUS3110 thread and
   close the serial port. */
void FBUS3110_Terminate(void)
{
  /* Request termination of thread */
  CurrentRequestTerminate = true;

  /* Close serial port. */
  device_close();
}

/* RX_State machine for receive handling.  Called once for each character
   received from the phone/phone. */

void FBUS3110_RX_StateMachine(unsigned char rx_byte) {

  static int checksum;
  
  /* XOR the byte with the current checksum */
  checksum ^= rx_byte;

  switch (RX_State) {
	
  case FBUS_RX_Discarding:
    if (rx_byte != 0x55) break;

  case FBUS_RX_Sync:

    if (rx_byte == 0x04 || rx_byte == 0x03) {
      MessageDestination=rx_byte;
      RX_State = FBUS_RX_GetLength1;
    }
    break;

  case FBUS_RX_GetLength1:

    MessageLength = rx_byte;
    RX_State = FBUS_RX_GetType;
    
    break;

  case FBUS_RX_GetType:

    MessageType=rx_byte;
    RX_State = FBUS_RX_GetMessage;

    break;

  case FBUS_RX_GetMessage:

    MessageBuffer[BufferCount] = rx_byte;
    BufferCount ++;
    
    if (BufferCount>FBUS3110_MAX_RECEIVE_LENGTH) {
#ifdef DEBUG
      fprintf(stdout, "FB31: Message buffer overun - resetting\n");
#endif
      AppendLogText("OVERUN\n",false);
      RX_State = FBUS_RX_Sync;
      break;
    }

    /* If this is the last byte, it's the checksum. */
    if (BufferCount == MessageLength) {

      RX_State = FBUS_RX_Sync;    
    }
    break;
  default:
    break;
  }
}
  
/* Called by initialisation code to open comm port in asynchronous mode. */
bool FBUS3110_OpenSerial(void)
{
  switch (CurrentConnectionType) {
     case GCT_FBUS3110:

#ifdef DEBUG
       fprintf(stdout, _("Setting cable for FBUS3110 communication...\n"));
#endif /* DEBUG */

       device_changespeed(115200);   
       break;
     default:
#ifdef DEBUG
       fprintf(stdout,_("Wrong connection type for FBUS3110 module. Inform marcin-wiacek@topnet.pl about it\n"));
#endif
       break;
  }

  return (true);
}

/* Initialise variables and state machine. */
GSM_Error FBUS3110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{
  if (!StartConnection (port_device,false,connection))
    return GE_INTERNALERROR;
      
  CurrentConnectionType = connection;

  if (FBUS3110_OpenSerial() != true) return GE_INTERNALERROR;

  return (GE_NONE);
}
