/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for support for fbusirda protocol

*/

/* "Turn on" prototypes in fbusirda.h */
#define __fbusirda_c 

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
#include "protocol/fbusirda.h"
#include "protocol/fbus.h"
#include "misc.h"

GSM_Protocol FBUSIRDA_Functions = {
  FBUSIRDA_Initialise,
  FBUSIRDA_SendMessage,
  FBUSIRDA_SendFrame,
  FBUSIRDA_WritePhone,
  FBUSIRDA_Terminate,
  FBUSIRDA_RX_StateMachine
};

/* Local variables */
enum FBUS_RX_States RX_State;

u8 MessageDestination, MessageSource;

u16 BufferCount;

u16 MessageLength;

u8 MessageType;

u8 MessageBuffer[FBUS_MAX_RECEIVE_LENGTH * 6];

#ifdef DEBUG
char *FBUSIRDA_PrintDevice(int Device)
{
  switch (Device) {

  case FBUS_DEVICE_PHONE:
    return _("Phone");

  case FBUS_DEVICE_PC:
    return _("PC");

  default:
    return _("Unknown");

  }
}

/* FBUSIRDA_RX_DisplayMessage is called when a message we don't know about is
   received so that the user can see what is going back and forth, and perhaps
   shed some more light/explain another message type! */
void FBUSIRDA_RX_DisplayMessage()
{
  fprintf(stdout, _("Msg Source: %s\n"), FBUSIRDA_PrintDevice(MessageSource));
  fprintf(stdout, _("Msg Dest: %s\n"), FBUSIRDA_PrintDevice(MessageDestination));
  fprintf(stdout, _("Msg Type: %02x\n"), MessageType);

  hexdump( MessageLength, MessageBuffer);
}
#endif /* DEBUG */

/* Prepares the message header and sends it, prepends the message start byte
	   (0x1e) and other values according the value specified when called.
	   Calculates checksum and then sends the lot down the pipe... */
int FBUSIRDA_SendFrame(u16 message_length, u8 message_type, u8 *buffer) {
  u8 out_buffer[FBUSIRDA_MAX_CONTENT_LENGTH + 2];
  
  int current=0;
  int header=0;

  /* FIXME - we should check for the message length ... */

  /* Now construct the message header. */

  out_buffer[current++] = FBUSIRDA_FRAME_ID;    /* Start of the frame indicator */

  out_buffer[current++] = FBUS_DEVICE_PHONE; /* Destination */

  out_buffer[current++] = FBUS_DEVICE_PC;    /* Source */

  out_buffer[current++] = message_type; /* Type */

  out_buffer[current++] = message_length/256; /* Length1 */
  out_buffer[current++] = message_length%256; /* Length2 */

  /* Copy in data if any. */	
	
  if (message_length != 0) {
    memcpy(out_buffer + current, buffer, message_length);
    header = current;
    current+=message_length;
  }

#ifdef DEBUG
  NULL_TX_DisplayMessage( current, out_buffer);
#endif /* DEBUG */

  /* Send it out... */
  return ( FBUSIRDA_WritePhone(current,out_buffer) - header);

}

int FBUSIRDA_SendMessage(u16 message_length, u8 message_type, u8 *buffer) {
  
  u8 frame_buffer[FBUSIRDA_MAX_CONTENT_LENGTH + 2];
  u16 ret;

    memcpy(frame_buffer, buffer, message_length);
    ret = FBUSIRDA_SendFrame(message_length, message_type, frame_buffer);

    if (ret != message_length)
      {
       printf("CAUTION:  FBUSIRDA_SendFrame return written %d bytes of %d !\n",ret, message_length);
       return(false);
      }
    else
      return (true);
}

int FBUSIRDA_SendAck(u8 message_type, u8 message_seq) {
  return(true);
}

/* Applications should call MBUS_Terminate to shut down the MBUS thread and
   close the serial port. */
void FBUSIRDA_Terminate(void)
{  
  /* Request termination of thread */
  CurrentRequestTerminate = true;

  /* Close serial port. */
  device_close();
}

/* RX_State machine for receive handling.  Called once for each character
   received from the phone/phone. */
void FBUSIRDA_RX_StateMachine(unsigned char rx_byte) {

  unsigned char max;

  int i;

  i=0;

  switch (RX_State) {

  case FBUS_RX_Discarding:
    break;
    
  /* Messages from the phone start with an 0x14 (IRDA, Nokia:PhoNet).
     We use this to "synchronise" with the incoming data stream. */

  case FBUS_RX_Sync:

    if (rx_byte == FBUSIRDA_FRAME_ID) {
      BufferCount = 0;	
      RX_State = FBUS_RX_GetDestination;
    }
    
    break;

  case FBUS_RX_GetDestination:

    MessageDestination=rx_byte;
    RX_State = FBUS_RX_GetSource;

 /* When there is a checksum error and things get out of sync we have to manage to resync */
    /* If doing a data call at the time, finding a 0x1e etc is really quite likely in the data stream */
    /* Then all sorts of horrible things happen because the packet length etc is wrong... */
    /* Therefore we test here for a destination of 0x0c and return to the top if it is not */

#ifdef DEBUG
    if (strstr(GSM_Info->IrdaModels,"decode")!=NULL)
     {
       if (rx_byte!=FBUS_DEVICE_PC && rx_byte!=FBUS_DEVICE_PHONE )
        {
          RX_State=FBUS_RX_Sync;
          fprintf(stdout,"decode: expected FBUS_DEVICE_PC/FBUS_DEVICE_PHONE, got %2x\n",rx_byte);
        }
     }//decode Model
    else
     {
#endif
       if (rx_byte!=FBUS_DEVICE_PC)
       {
          RX_State=FBUS_RX_Sync;
#ifdef DEBUG
          fprintf(stdout,"The FBUSIRDA stream is out of sync - expected FBUS_DEVICE_PC, got %2x\n",rx_byte);
#endif
       }
#ifdef DEBUG
     }//not decode Model
#endif

    break;

  case FBUS_RX_GetSource:

    MessageSource=rx_byte;
    RX_State = FBUS_RX_GetType;

#ifdef DEBUG
    if (strstr(GSM_Info->IrdaModels,"decode")!=NULL)
     {
       if (rx_byte!=FBUS_DEVICE_PC && rx_byte!=FBUS_DEVICE_PHONE )
        {
          RX_State=FBUS_RX_Sync;
          fprintf(stdout,"decode: expected FBUS_DEVICE_PC/FBUS_DEVICE_PHONE, got %2x\n",rx_byte);
        }
     }//decode Model
    else
     {
#endif
       if (rx_byte!=FBUS_DEVICE_PHONE)
        {
          RX_State=FBUS_RX_Sync;
#ifdef DEBUG
          fprintf(stdout,"The FBUSIRDA stream is out of sync - expected FBUS_DEVICE_PHONE, got %2x\n",rx_byte);
#endif
        }
#ifdef DEBUG
     }//not decode Model
#endif

    break;

  case FBUS_RX_GetType:

    MessageType=rx_byte;
    RX_State = FBUS_RX_GetLength1;

    break;

  case FBUS_RX_GetLength1:

    MessageLength=0;

    /* MW:Here are problems with conversion. For chars 0-127 it's OK, for
       higher not (probably because rx_byte is char type) - improtant
       for MBUS. So, I make it double and strange - generally it should be
       more simple and make simple convert rx_byte into MessageLength */
    if (rx_byte!=0) {
      max=rx_byte;
      MessageLength=max*256;
    }

    RX_State = FBUS_RX_GetLength2;
    
    break;
    
  case FBUS_RX_GetLength2:

    /* MW:Here are problems with conversion. For chars 0-127 it's OK, for
       higher not (probably because rx_byte is char type) - important
       for MBUS. So, I make it double and strange - generally it should be
       more simple and make simple convert rx_byte into MessageLength */      
#if defined(__svr4__) || defined(__FreeBSD__)
    if (rx_byte!=0) {
      for (i=0;i<rx_byte;i++)
	MessageLength=MessageLength++;
    }
#else
    if (rx_byte!=0) {
      max=rx_byte;
      MessageLength=MessageLength+max;
    }
#endif
    
    RX_State = FBUS_RX_GetMessage;

    break;

  case FBUS_RX_GetMessage:

    MessageBuffer[BufferCount] = rx_byte;
    BufferCount ++;

    if (BufferCount == MessageLength)
       {	
	if (MessageSource == FBUS_DEVICE_PHONE)
	 {
#ifdef DEBUG
	   FBUSIRDA_RX_DisplayMessage();
#endif /* DEBUG */
      	   GSM->DispatchMessage(MessageLength, MessageBuffer, MessageType);
         }
	else
	 {
#ifdef DEBUG
   	   /* for --decodefile */
  	   fprintf(stdout, _("Msg Source: %s\n"), FBUSIRDA_PrintDevice(MessageSource));
  	   fprintf(stdout, _("Msg Dest: %s\n"), FBUSIRDA_PrintDevice(MessageDestination));
  	   fprintf(stdout, _("Msg Type: %02x\n"), MessageType);
           hexdump( MessageLength, MessageBuffer);
#endif /* DEBUG */
	 }
      
      RX_State = FBUS_RX_Sync;
    }
    break;
  }
}

/* Called by initialisation code to open comm port in asynchronous mode. */
bool FBUSIRDA_OpenSerial(void)
{
#ifdef DEBUG
  fprintf(stdout, _("Setting FBUS Irda communication...\n"));
#endif /* DEBUG */

  return (true);

}

/* Initialise variables and state machine. */
GSM_Error FBUSIRDA_Initialise(char *port_device, char *initlength,
                            GSM_ConnectionType connection,
                            void (*rlp_callback)(RLP_F96Frame *frame))
{
  CurrentConnectionType = connection;

  if (!StartConnection(port_device,false,connection)) {
    return GE_INTERNALERROR;
  }
  RX_State=FBUS_RX_Sync;
  
  return (GE_NONE);
}

bool FBUSIRDA_WritePhone (u16 length, u8 *buffer) {

  if (!CurrentDisableKeepAlive)
    usleep(150);

  return (device_write(buffer,length));
}
