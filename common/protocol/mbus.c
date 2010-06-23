/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for support for MBUS protocol

*/

/* "Turn on" prototypes in MBUS.h */
#define __MBUS_c 

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
#include "protocol/mbus.h"
#include "protocol/fbus.h"

GSM_Protocol MBUS_Functions = {
  MBUS_Initialise,
  MBUS_SendMessage,
  MBUS_SendFrame,
  MBUS_WritePhone,
  MBUS_Terminate,
  MBUS_RX_StateMachine
};

/* Local variables */
enum FBUS_RX_States RX_State;

u8 MessageDestination, MessageSource;

u16 BufferCount;

u16 MessageLength;

u8 MessageType;

u8 MessageBuffer[MBUS_MAX_RECEIVE_LENGTH * 6];

static u8 RequestSequenceNumber = 0x00;
  
#ifdef DEBUG	
char *MBUS_PrintDevice(int Device)
{
  switch (Device) {

  case FBUS_DEVICE_PHONE:
    return _("Phone");

  case MBUS_DEVICE_PC1:
    return _("PC");

  case MBUS_DEVICE_PC2:
    return _("PC");

  default:
    return _("Unknown");

  }
}

/* N61_RX_DisplayMessage is called when a message we don't know about is
   received so that the user can see what is going back and forth, and perhaps
   shed some more light/explain another message type! */
void MBUS_RX_DisplayMessage()
{
  fprintf(stdout, _("Msg Dest: %s\n"), MBUS_PrintDevice(MessageDestination));
  fprintf(stdout, _("Msg Source: %s\n"), MBUS_PrintDevice(MessageSource));
  fprintf(stdout, _("Msg Type: %02x\n"), MessageType);

  hexdump(MessageLength,MessageBuffer);
}

#endif /* DEBUG */

/* Prepares the message header and sends it, prepends the message start byte
	   (0x1e) and other values according the value specified when called.
	   Calculates checksum and then sends the lot down the pipe... */
int MBUS_SendFrame(u16 message_length, u8 message_type, u8 *buffer) {
  u8 out_buffer[MBUS_MAX_CONTENT_LENGTH + 2];
  
  int count, current=0;
  unsigned char	checksum;

  /* FIXME - we should check for the message length ... */

  /* Now construct the message header. */

  out_buffer[current++] = MBUS_FRAME_ID;    /* Start of the frame indicator */

  out_buffer[current++] = FBUS_DEVICE_PHONE; /* Destination */

  out_buffer[current++] = MBUS_DEVICE_PC1;    /* Source */

  out_buffer[current++] = message_type; /* Type */

  out_buffer[current++] = (message_length-1)/256; /* Length1 */
  out_buffer[current++] = (message_length-1)%256; /* Length2 */

  /* Copy in data if any. */	
	
  if (message_length != 0) {
    memcpy(out_buffer + current, buffer, message_length);
    current+=message_length;
  }

  /* Now calculate checksum over entire message and append to message. */

  /* All bytes */
  checksum = 0;
  for (count = 0; count < current; count++)
    checksum ^= out_buffer[count];

  out_buffer[current++] = checksum;

#ifdef DEBUG
  NULL_TX_DisplayMessage(current, out_buffer);
#endif /* DEBUG */

  /* Send it out... */
  if (!MBUS_WritePhone(current,out_buffer))
    return (false);

  return (true);
}

int MBUS_SendMessage(u16 message_length, u8 message_type, u8 *buffer) {
  
  u8 frame_buffer[MBUS_MAX_CONTENT_LENGTH + 2];

    RequestSequenceNumber++;

    memcpy(frame_buffer, buffer, message_length);
    frame_buffer[message_length] = RequestSequenceNumber;  
    MBUS_SendFrame(message_length + 1, message_type, frame_buffer);

  return (true);
}

int MBUS_SendAck(u8 message_type, u8 message_seq) {

  unsigned char request[6];

  int count;

  request[0]=MBUS_FRAME_ID;
  request[1]=FBUS_DEVICE_PHONE;
  request[2]=MBUS_DEVICE_PC1;
  request[3]=FBUS_FRTYPE_ACK;
  request[4] = message_seq;
  request[5]=0;

  /* Checksum */
  for (count = 0; count < 5; count++)
    request[5] ^= request[count];

#ifdef DEBUG
  fprintf(stdout, _("[Sending Ack of type %02x, seq: %x]\n"), message_type, message_seq);

  NULL_TX_DisplayMessage(6, request);
#endif /* DEBUG */

  if (!MBUS_WritePhone(6, request)) {
#ifdef DEBUG
    fprintf(stdout,_("Sending ACK failed %i !\n"),count);
#endif
    return (false);
  }

  return(true);
}

/* Applications should call MBUS_Terminate to shut down the MBUS thread and
   close the serial port. */
void MBUS_Terminate(void)
{  
  /* Request termination of thread */
  CurrentRequestTerminate = true;

  /* RTS low */
  device_setdtrrts(0, 0);

  /* Close serial port. */
  device_close();
}

/* RX_State machine for receive handling.  Called once for each character
   received from the phone/phone. */
void MBUS_RX_StateMachine(unsigned char rx_byte) {

  static struct timeval time_now, time_last, time_diff;
  
  unsigned char max;

  static int checksum[2];

#if defined(__svr4__) || defined(__FreeBSD__) || defined(DEBUG)
  int i=0;
#endif

#ifdef DEBUG
  if (strcmp(GSM_Info->MBUSModels, "mbussniff"))
  {
#endif

  checksum[0]=checksum[1];
  checksum[1] ^= rx_byte;

  switch (RX_State) {
	
  /* Messages from the phone start with an 0x1f (MBUS).
     We use this to "synchronise" with the incoming data stream. However,
     if we see something else, we assume we have lost sync and we require
     a gap of at least 5ms before we start looking again. This is because
     the data part of the frame could contain a byte which looks like the
     sync byte */
  case FBUS_RX_Discarding:
#ifndef VC6
    gettimeofday(&time_now, NULL);
    timersub(&time_now, &time_last, &time_diff);
    if (time_diff.tv_sec == 0 && time_diff.tv_usec < 5000) {
      time_last = time_now;  /* no gap seen, continue discarding */
      break;
    }
    /* else fall through to... */
#endif

  case FBUS_RX_Sync:

    if (rx_byte == MBUS_FRAME_ID) {

      BufferCount = 0;
	
      RX_State = FBUS_RX_GetDestination;
	
      /* Initialize checksum. */
      checksum[1] = MBUS_FRAME_ID;
    } else {
      /* Lost frame sync */
      RX_State = FBUS_RX_Discarding;
#ifndef VC6
      gettimeofday(&time_last, NULL);
#endif
    }
    
    break;

  case FBUS_RX_GetDestination:

    MessageDestination=rx_byte;
    RX_State = FBUS_RX_GetSource;

    /* When there is a checksum error and things get out of sync we have to manage to resync */
    /* If doing a data call at the time, finding a 0x1e etc is really quite likely in the data stream */
    /* Then all sorts of horrible things happen because the packet length etc is wrong... */
    /* Therefore we test here for a destination of 0x0c and return to the top if it is not */
    /* The same testing for MBUS. Only one change: MBUS returns, what we send.
       So, the byte can be 0x10 (destination MBUS) or 0x00 (phone) */
    if (rx_byte!=MBUS_DEVICE_PC1 && rx_byte!=MBUS_DEVICE_PC2 && rx_byte!=FBUS_DEVICE_PHONE) {
      RX_State=FBUS_RX_Sync;
#ifdef DEBUG
      fprintf(stdout,"The mbus stream is out of sync - expected 0x10 or 0x00, got %2x\n",rx_byte);
#endif
    }

    break;

  case FBUS_RX_GetSource:

    MessageSource=rx_byte;
    RX_State = FBUS_RX_GetType;

    /* Source should be 0x00 or 0x10 */
    if (rx_byte!=FBUS_DEVICE_PHONE && rx_byte!=MBUS_DEVICE_PC1 && rx_byte!=MBUS_DEVICE_PC2)  {
      RX_State=FBUS_RX_Sync;
#ifdef DEBUG
      fprintf(stdout,"The mbus stream is out of sync - expected 0x00 or 0x10, got %2x\n",rx_byte);
#endif
    }    
    
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
       higher not (probably because rx_byte is char type) - improtant
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

    /* In MBUS ACK ends here */
    if (MessageType==FBUS_FRTYPE_ACK)
    {
#ifdef DEBUG
      fprintf(stdout, _("[Received Ack from phone]\n"));
#endif /* DEBUG */
      RX_State = FBUS_RX_Sync;
    }
    
    break;
    
  case FBUS_RX_GetMessage:

    MessageBuffer[BufferCount] = rx_byte;
    BufferCount ++;

    /* If this is the last byte, it's the checksum. */
    if (BufferCount == MessageLength+2) {
	
      /* Is the checksum correct? */
      if (checksum[0] == rx_byte) {

        /* We do not want to send ACK of ACKs and ACK of RLP frames. */
        if (MessageType != FBUS_FRTYPE_ACK && MessageType != 0xf1
	    && (MessageDestination==MBUS_DEVICE_PC1 || MessageDestination==MBUS_DEVICE_PC2)) {
	  MBUS_SendAck(MessageType, MessageBuffer[BufferCount-2]);
        }

        /* We don't write info about messages sent to phone */
        if (MessageDestination!=FBUS_DEVICE_PHONE) {
#ifdef DEBUG
          /* Do not debug Ack and RLP frames to detail. */
          if (MessageType != FBUS_FRTYPE_ACK && MessageType != 0xf1)
              MBUS_RX_DisplayMessage();
#endif /* DEBUG */

          GSM->DispatchMessage(MessageLength, MessageBuffer, MessageType);

        } else {
#ifdef DEBUG
          if (strstr(GSM_Info->MBUSModels, "sniff") != NULL) {
             fprintf(stdout, _("PC: "));

             fprintf(stdout, "%02x:", MBUS_FRAME_ID);
             fprintf(stdout, "%02x:", MBUS_DEVICE_PC1);
             fprintf(stdout, "%02x:", FBUS_DEVICE_PHONE);
             fprintf(stdout, "%02x:", MessageType);
             fprintf(stdout, "%02x:", MessageLength/256);
             fprintf(stdout, "%02x:", MessageLength%256);

//             NULL_TX_DisplayMessage(current, out_buffer);

             for (i = 0; i < BufferCount; i++)
               fprintf(stdout, "%02x:", MessageBuffer[i]);

             fprintf(stdout, "\n");
	  }
#endif /* DEBUG */
        }
      } else {
#ifdef DEBUG
	fprintf(stdout, _("Bad checksum %02x (should be %02x), msg len=%i !\n"),rx_byte,checksum[0],MessageLength);
#endif /* DEBUG */
      }
      RX_State = FBUS_RX_Sync;
    }
    break;
  }

#ifdef DEBUG

  } else {
    if (isprint(rx_byte))
      fprintf(stdout, "[%02x%c]", rx_byte, rx_byte);
    else
      fprintf(stdout, "[%02x ]", rx_byte);

  }

#endif
}

/* Called by initialisation code to open comm port in asynchronous mode. */
bool MBUS_OpenSerial(void)
{
#ifdef DEBUG
  fprintf(stdout, _("Setting MBUS communication...\n"));
#endif /* DEBUG */

  device_changespeed(9600);

  usleep(100);

  if (strstr(GSM_Info->MBUSModels, "sniff") == NULL) {

    /* leave RTS high, DTR low for duration of session. */
    device_setdtrrts(0, 1);
  
    usleep(100);
  }

  return (true);

}

/* Initialise variables and state machine. */
GSM_Error MBUS_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{
  if (!StartConnection (port_device,true,connection))
    return GE_INTERNALERROR;
    
  CurrentConnectionType = connection;

  if (MBUS_OpenSerial() != true) {
    /* Fail so sit here till calling code works out there is a problem. */
//    while (!CurrentRequestTerminate)
//      usleep (100000);
	  
    return GE_INTERNALERROR;
  }

  return (GE_NONE);
}

bool MBUS_WritePhone (u16 length, u8 *buffer) {

  if (!CurrentDisableKeepAlive)
    usleep(150);

  if (device_write(buffer,length)!=length)
    return false;
  else
    return true;
}
