/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for support for AT protocol

*/

/* "Turn on" prototypes in AT.h */
#define __AT_c 

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
#include "protocol/at.h"

GSM_Protocol AT_Functions = {
  AT_Initialise,
  AT_SendMessage,
  AT_SendFrame,
  AT_WritePhone,
  AT_Terminate,
  AT_RX_StateMachine
};

/* Local variables */
u16 BufferCount;

u16 MessageLength;

u8 MessageBuffer[AT_MAX_RECEIVE_LENGTH * 6];

static bool wascrlf = false;
static char *linestart = MessageBuffer;

/* N61_RX_DisplayMessage is called when a message we don't know about is
   received so that the user can see what is going back and forth, and perhaps
   shed some more light/explain another message type! */

#ifdef DEBUG	
void AT_RX_DisplayMessage()
{
  fprintf(stdout, _("Phone\n"));

  hexdump(MessageLength,MessageBuffer);
}
#endif /* DEBUG */

/* Prepares the message header and sends it, prepends the message start byte
	   (0x1e) and other values according the value specified when called.
	   Calculates checksum and then sends the lot down the pipe... */
int AT_SendFrame(u16 message_length, u8 message_type, u8 *buffer) {

#ifdef DEBUG
  int count;
#endif
  
  /* FIXME - we should check for the message length ... */

#ifdef DEBUG
  fprintf(stdout, _("PC: "));

  for (count = 0; count < message_length-1; count++)
    fprintf(stdout, "%c", buffer[count]);

  fprintf(stdout, "\n");
#endif /* DEBUG */

  /* Send it out... */
  if (!AT_WritePhone(message_length,buffer))
    return (false);

  return (true);
}

int AT_SendMessage(u16 message_length, u8 message_type, u8 *buffer) {
  
  AT_SendFrame(message_length, message_type, buffer);

  return (true);
}

int AT_SendAck(u8 message_type, u8 message_seq) {

  return true;
}

/* Applications should call AT_Terminate to shut down the AT thread and
   close the serial port. */
void AT_Terminate(void)
{
  /* Request termination of thread */
  CurrentRequestTerminate = true;

  /* Close serial port. */
  device_close();
}

/* RX_State machine for receive handling.  Called once for each character
   received from the phone/phone. */

void AT_RX_StateMachine(unsigned char rx_byte) {

  /* FIXME: Check for overflow */
  MessageBuffer[BufferCount] = rx_byte;
  MessageBuffer[BufferCount + 1] = '\0';

//  fprintf(stdout,_("%02x %c\n"),rx_byte,rx_byte);

  if (wascrlf && (rx_byte != 10 && rx_byte != 13)) {
    linestart = MessageBuffer + BufferCount;
    wascrlf = false;
  }

  if ((rx_byte== 10 || rx_byte==13)) wascrlf = true;

  if ((BufferCount > 0) && (10 == MessageBuffer[BufferCount])
       && (13 == MessageBuffer[BufferCount-1])
       && ((0 == strncmp("OK", linestart, 2))
       || (0 == strncmp("ERROR", linestart, 5))
       || (0 == strncmp("+CME ERROR:", linestart, 11))
       || (0 == strncmp("+CMS ERROR:", linestart, 11)))) {

    MessageLength=BufferCount;

#ifdef DEBUG
    AT_RX_DisplayMessage();
#endif

    GSM->DispatchMessage(MessageLength,MessageBuffer, 0x00);

    BufferCount = 0;
    linestart = MessageBuffer;
    wascrlf = false;

  } else BufferCount++;
}
  
/* Called by initialisation code to open comm port in asynchronous mode. */
bool AT_OpenSerial(void)
{
//  device_changespeed(115200);

  device_changespeed(19200);

  return (true);
}

/* Initialise variables and state machine. */
GSM_Error AT_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{

  if (!StartConnection (port_device,false,connection))
    return GE_INTERNALERROR;
      
  CurrentConnectionType = connection;

  if (AT_OpenSerial() != true) return GE_INTERNALERROR;

  BufferCount=0;

  return (GE_NONE);
}

bool AT_WritePhone (u16 length, u8 *buffer) {
  int i;

  for (i=0;i<length;i++) {
    if (device_write(buffer+i,1)!=1) return false;
    /* For some phones like Siemens M20 we need to wait a little
       after writing each char. Possible reason: these phones
       can't receive so fast chars or there is bug here in mygnokii */
    usleep(1);
  }

  return true;
}
