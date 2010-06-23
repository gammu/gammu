/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for support for FBUS protocol

*/

/* "Turn on" prototypes in fbus.h */
#define __fbus_c 

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
#include "protocol/fbus.h"
#include "protocol/at.h"
#include "newmodules/newat.h"
#include "newmodules/n6110.h"
#include "misc.h"

GSM_Protocol FBUS_Functions = {
  FBUS_Initialise,
  FBUS_SendMessage,
  FBUS_SendFrame,
  NULL_WritePhone,
  FBUS_Terminate,
  FBUS_RX_StateMachine
};

/* Local variables */
enum FBUS_RX_States RX_State;

u8 MessageDestination, MessageSource;

u16 BufferCount, MultiBufferCount;

u16 MessageLength, MultiMessageLength;

bool RX_Multiple = false;

u8 MessageType,MultiMessageType;

u8 MessageBuffer[FBUS_MAX_RECEIVE_LENGTH * 6],MultiMessageBuffer[FBUS_MAX_RECEIVE_LENGTH * 6];

u8        RequestSequenceNumber = 0x00;

#ifdef DEBUG	
char *N61_PrintDevice(int Device)
{
  switch (Device) {

  case FBUS_DEVICE_PHONE:return _("Phone");
  case FBUS_DEVICE_PC   :return _("PC");
  default               :return _("Unknown");
  }
}
#endif /* DEBUG */

/* N61_RX_DisplayMessage is called when a message we don't know about is
   received so that the user can see what is going back and forth, and perhaps
   shed some more light/explain another message type! */
void N61_RX_DisplayMessage()
{
#ifdef DEBUG
  fprintf(stdout, _("Msg Dest: %s\n"), N61_PrintDevice(MessageDestination));
  fprintf(stdout, _("Msg Source: %s\n"), N61_PrintDevice(MessageSource));
  fprintf(stdout, _("Msg Type: %02x\n"), MessageType);

  hexdump(MessageLength-2,MessageBuffer);
#endif

  AppendLog(MessageBuffer,MessageLength-2,true);
}

/* Prepares the message header and sends it, prepends the message start byte
	   (0x1e) and other values according the value specified when called.
	   Calculates checksum and then sends the lot down the pipe... */
int FBUS_SendFrame(u16 message_length, u8 message_type, u8 *buffer) {

  /* Originally out_buffer[FBUS_MAX_CONTENT_LENGTH + 2],
     but it made problems with MBUS */
  u8 out_buffer[1000];
  
  int count, current=0;
  unsigned char	checksum;

  /* FIXME - we should check for the message length ... */

  /* Now construct the message header. */

  if (CurrentConnectionType==GCT_FBUS)
    out_buffer[current++] = FBUS_FRAME_ID;    /* Start of the frame indicator */
  else
    out_buffer[current++] = FBUS_IR_FRAME_ID; /* Start of the frame indicator */
    
  out_buffer[current++] = FBUS_DEVICE_PHONE; /* Destination */

  out_buffer[current++] = FBUS_DEVICE_PC;    /* Source */

  out_buffer[current++] = message_type; /* Type */

  out_buffer[current++] = 0; /* Length1 */
  out_buffer[current++] = message_length; /* Length2 */

  /* Copy in data if any. */		
  if (message_length != 0) {
    memcpy(out_buffer + current, buffer, message_length);
    current+=message_length;
  }

  /* If the message length is odd we should add pad byte 0x00 */
  if (message_length % 2)
    out_buffer[current++]=0x00;

  /* Now calculate checksums over entire message and append to message. */

  /* Odd bytes */
  checksum = 0;
  for (count = 0; count < current; count+=2)
    checksum ^= out_buffer[count];

  out_buffer[current++] = checksum;

  /* Even bytes */
  checksum = 0;
  for (count = 1; count < current; count+=2)
    checksum ^= out_buffer[count];

  out_buffer[current++] = checksum;
  
#ifdef DEBUG
  NULL_TX_DisplayMessage(current, out_buffer);
#endif /* DEBUG */

  /* Send it out... */
  if (!NULL_WritePhone(current,out_buffer))
    return (false);

  return (true);
}

int FBUS_SendMessage(u16 message_length, u8 message_type, u8 *buffer) {
  
  u8 seqnum;

  u8 frame_buffer[FBUS_MAX_CONTENT_LENGTH + 2];

  u8 nom, lml;  /* number of messages, last message len */
  int i;

  seqnum = 0x40 + RequestSequenceNumber;
  RequestSequenceNumber = (RequestSequenceNumber + 1) & 0x07;
 
  if (message_length > FBUS_MAX_CONTENT_LENGTH) {

    nom = (message_length + FBUS_MAX_CONTENT_LENGTH - 1)
                          / FBUS_MAX_CONTENT_LENGTH;
    lml = message_length - ((nom - 1) * FBUS_MAX_CONTENT_LENGTH);

    for (i = 0; i < nom - 1; i++) {

      memcpy(frame_buffer, buffer + (i * FBUS_MAX_CONTENT_LENGTH),
             FBUS_MAX_CONTENT_LENGTH);
      frame_buffer[FBUS_MAX_CONTENT_LENGTH] = nom - i;
      frame_buffer[FBUS_MAX_CONTENT_LENGTH + 1] = seqnum;

      FBUS_SendFrame(FBUS_MAX_CONTENT_LENGTH + 2, message_type,
                     frame_buffer);

      seqnum = RequestSequenceNumber;
      RequestSequenceNumber = (RequestSequenceNumber + 1) & 0x07;    
    }

    memcpy(frame_buffer, buffer + ((nom - 1) * FBUS_MAX_CONTENT_LENGTH), lml);
    frame_buffer[lml] = 0x01;
    frame_buffer[lml + 1] = seqnum;
    FBUS_SendFrame(lml + 2, message_type, frame_buffer);

  } else {

    memcpy(frame_buffer, buffer, message_length);
    frame_buffer[message_length] = 0x01;
    frame_buffer[message_length + 1] = seqnum;
    FBUS_SendFrame(message_length + 2, message_type, frame_buffer);
  }

  return (true);
}

int FBUS_SendAck(u8 message_type, u8 message_seq) {

  unsigned char request[6];

  request[0] = message_type;
  request[1] = message_seq;

#ifdef DEBUG
  fprintf(stdout, _("[Sending Ack of type %02x, seq: %x]\n"), message_type, message_seq);
#endif /* DEBUG */

  return FBUS_SendFrame(2, FBUS_FRTYPE_ACK, request);
}

/* Applications should call FBUS_Terminate to shut down the FBUS thread and
   close the serial port. */
void FBUS_Terminate(void)
{
  /* Request termination of thread */
  CurrentRequestTerminate = true;

  /* Close serial port. */
  device_close();
}

/* RX_State machine for receive handling.  Called once for each character
   received from the phone/phone. */

void FBUS_RX_StateMachine(unsigned char rx_byte) {

  static struct timeval time_now, time_last, time_diff;
  
  static int checksum[2];
  
  int i=0;

#ifdef DEBUG
  /* For model sniff only display received bytes */
  if (strcmp(GSM_Info->FBUSModels, "sniff"))
  {
#endif

  /* XOR the byte with the current checksum */
  checksum[BufferCount&1] ^= rx_byte;

  switch (RX_State) {
	
    /* Messages from the phone start with an 0x1e (FBUS) or 0x1c (IR) or 0x1f (MBUS).
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

      if ((CurrentConnectionType==GCT_FBUS && rx_byte == FBUS_FRAME_ID) ||
          ((CurrentConnectionType==GCT_Infrared ||
	   CurrentConnectionType==GCT_Tekram) && rx_byte == FBUS_IR_FRAME_ID)) {

        BufferCount = 0;

	RX_State = FBUS_RX_GetDestination;
	
	/* Initialize checksums. */
	checksum[0] = rx_byte;
	checksum[1] = 0;
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
    if (rx_byte!=FBUS_DEVICE_PC && strstr(GSM_Info->FBUSModels, "sniff")==NULL) {
      RX_State=FBUS_RX_Sync;
#ifdef DEBUG
      fprintf(stdout,"The fbus stream is out of sync - expected 0x0c, got %2x\n",rx_byte);
#endif
      AppendLogText("SYNC\n",false);
    }

    break;

  case FBUS_RX_GetSource:

    MessageSource=rx_byte;
    RX_State = FBUS_RX_GetType;

    /* Source should be 0x00 */
    if (rx_byte!=FBUS_DEVICE_PHONE && strstr(GSM_Info->FBUSModels, "sniff")==NULL)  {
      RX_State=FBUS_RX_Sync;
#ifdef DEBUG
      fprintf(stdout,"The fbus stream is out of sync - expected 0x00, got %2x\n",rx_byte);
#endif
      AppendLogText("SYNC\n",false);
    }
    
    break;

  case FBUS_RX_GetType:

    MessageType=rx_byte;

    RX_State = FBUS_RX_GetLength1;

    break;

  case FBUS_RX_GetLength1:

    MessageLength = 0;

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
    MessageLength = rx_byte;
#endif
    
    RX_State = FBUS_RX_GetMessage;
    
    break;
    
  case FBUS_RX_GetMessage:

    MessageBuffer[BufferCount] = rx_byte;
    BufferCount ++;
    
    if (BufferCount>FBUS_MAX_RECEIVE_LENGTH*6) {
#ifdef DEBUG
      fprintf(stdout, "FB61: Message buffer overun - resetting\n");
#endif
      AppendLogText("OVERUN\n",false);
      RX_Multiple=false;
      RX_State = FBUS_RX_Sync;
      break;
    }

    /* If this is the last byte, it's the checksum. */
    if (BufferCount == MessageLength+(MessageLength%2)+2) {

      /* Is the checksum correct? */
      if (checksum[0] == checksum[1]) {
	  
        if (RX_Multiple) {

	  if (MessageType==MultiMessageType) {

            if (MessageLength+MultiMessageLength>FBUS_MAX_RECEIVE_LENGTH*6) {
#ifdef DEBUG
              fprintf(stdout, "FB61: Message buffer overun - resetting\n");
#endif
              AppendLogText("OVERUN\n",false);
              RX_Multiple=false;
              RX_State = FBUS_RX_Sync;
              break;
            }
	
	    /* We copy next part of multiframe message into special buffer */
	    for (i=0;i<MessageLength;i++) {
	      MultiMessageBuffer[i+MultiMessageLength]=MessageBuffer[i];
	    }
	    MultiMessageLength=MultiMessageLength+MessageLength-2;

            FBUS_SendAck(MessageType, MessageBuffer[MessageLength-1] & 0x0f);		
		
            if ((MessageLength > 1) && (MessageBuffer[MessageLength-2] != 0x01))
            {
            } else {
              for (i=0;i<MultiMessageLength+2;i++) {
		MessageBuffer[i]=MultiMessageBuffer[i];
	      }
	      MessageLength=MultiMessageLength+2;
	      RX_Multiple=false;

              /* Do not debug Ack and RLP frames to detail. */
              if (MessageType != FBUS_FRTYPE_ACK && MessageType != 0xf1)
                N61_RX_DisplayMessage();

              GSM->DispatchMessage(MessageLength-2, MessageBuffer, MessageType);
            }

	  } else {

            /* We do not want to send ACK of ACKs and ACK of RLP frames. */
            if (MessageType != FBUS_FRTYPE_ACK && MessageType != 0xf1) {
              FBUS_SendAck(MessageType, MessageBuffer[MessageLength-1] & 0x0f);

              if ((MessageLength > 1) && (MessageBuffer[MessageLength-2] != 0x01))
              {
#ifdef DEBUG
                fprintf(stdout,_("Multiframe message in multiframe message !\n"));
                fprintf(stdout,_("Please report it !\n"));
#endif
                RX_State = FBUS_RX_Sync;
              }
            }
	  }
        } else {

          /* We do not want to send ACK of ACKs and ACK of RLP frames. */
          if (MessageType != FBUS_FRTYPE_ACK && MessageType != 0xf1) {
            FBUS_SendAck(MessageType, MessageBuffer[MessageLength-1] & 0x0f);

            if ((MessageLength > 1) && (MessageBuffer[MessageLength-2] != 0x01))
            {
              /* We copy previous part of multiframe message into special buffer */
              RX_Multiple = true;
              for (i=0;i<MessageLength-2;i++) {
	        MultiMessageBuffer[i]=MessageBuffer[i];
	      }
	      MultiMessageLength=MessageLength-2;
	      MultiMessageType=MessageType;
	    }
	  }

          if (!RX_Multiple && MessageDestination!=FBUS_DEVICE_PHONE ) {
            /* Do not debug Ack and RLP frames to detail. */
            if (MessageType != FBUS_FRTYPE_ACK && MessageType != 0xf1)
              N61_RX_DisplayMessage();

            GSM->DispatchMessage(MessageLength-2, MessageBuffer, MessageType);
          }

#ifdef DEBUG
          /* When make debug and message is to phone display it */
	  if (MessageDestination==FBUS_DEVICE_PHONE) {
            for (i=MessageLength;i>=0;i--)
              MessageBuffer[i+6]=MessageBuffer[i];
            MessageBuffer[0]=FBUS_FRAME_ID;
            MessageBuffer[1]=FBUS_DEVICE_PHONE;
            MessageBuffer[2]=FBUS_DEVICE_PC;
            MessageBuffer[3]=MessageType;
            MessageBuffer[4]=0;
            MessageBuffer[5]=MessageLength;
            MessageLength=MessageLength+8;
            if (MessageLength % 2) MessageLength++;
	    NULL_TX_DisplayMessage(MessageLength, MessageBuffer);
          }
#endif	  
	}
      } else {
#ifdef DEBUG
          fprintf(stdout, _("Bad checksum %02x (should be %02x), msg len=%i !\n"),checksum[0],checksum[1],MessageLength);
#endif /* DEBUG */
        AppendLogText("CHECKSUM\n",false);

	/* Just to be sure! */
	RX_Multiple=false;
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
bool FBUS_OpenSerial(void)
{
  /* Uncomment, if want to test first method for DLR3 */
  unsigned char req[3]   = {"AT\r"};  
  unsigned char req2[5]  = {"AT&F\r"};  
  unsigned char req3[13] = {"AT*NOKIAFBUS\r"};  

  switch (CurrentConnectionType) {
     case GCT_FBUS:

#ifdef DEBUG
       fprintf(stdout, _("Setting cable for FBUS communication...\n"));
#endif /* DEBUG */

       device_changespeed(115200);
   
       /* Colin wrote:
       The data suite cable has some electronics built into the connector. This of
       course needs a power supply of some sorts to operate properly.

       In this case power is drawn off the handshaking lines of the PC. DTR has to
       be set and RTS have to be cleared, thus if you use a terminal program (that
       does not set the handshaking lines to these conditions) you will get weird
       results. It will not set them like this since if Request To Send (RTS) is
       not set the other party will not send any data (in hardware handshaking)
       and if DTS is not set (handshaking = none) the cable will not receive
       power. */
       /* clearing the RTS bit and setting the DTR bit*/
       device_setdtrrts(1, 0);

       break;
     case GCT_DLR3:

#ifdef DEBUG
       fprintf(stdout, _("Setting DLR3 cable for FBUS communication...\n"));
#endif /* DEBUG */

       /* There are 2 ways to init DLR in FBUS: Here is first described by
          Reuben Harris [reuben.harris@snowvalley.com] and used in Logo Manager,
            1. Firstly set the connection baud to 19200, DTR off, RTS off,
	       Parity on, one stop bit, 
            2. Send "AT\r\n". The response should be "AT\r\n\r\nOK\r\n".
            3. Send "AT&F\r\n". The response should be "AT&F\r\n\r\nOK\r\n".
            4. Send "AT*NOKIAFBUS\r\n". The response should be
               "AT*NOKIAFBUS\r\n\r\nOK\r\n".
            5. Set speed to 115200 
          This seems to be compatible with more phones*/

       device_changespeed(19200);

       /*leave RTS low, DTR low for duration of session.*/
       device_setdtrrts(0, 0);
                            
       Protocol->WritePhone (3,req );usleep(300);
       Protocol->WritePhone (5,req2);usleep(300);
       Protocol->WritePhone(13,req3);usleep(300);

       device_changespeed(115200);

//     /*  Second method for DLR3:
//         Used by some 7110 soft, but not compatible with some other
//         phones supporting DLR3 - 7160, NCP2.0*/
//     device_changespeed(115200);
//     /*leave RTS low, DTR low for duration of session.*/
//     device_setdtrrts(0, 0);      
//     usleep(100000);

       CurrentConnectionType=GCT_FBUS;

       break;
     case GCT_Infrared:
       /* It's more complicated and not done here */
       break;

     case GCT_Tekram:
       /* It's more complicated and not done here */
       break;

     default:
#ifdef DEBUG
       fprintf(stdout,_("Wrong connection type for fbus module. Inform marcin-wiacek@topnet.pl about it\n"));
#endif
       break;
  }

  return (true);
}

/* Initialise variables and state machine. */
GSM_Error FBUS_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{

  if (!StartConnection (port_device,false,connection))
    return GE_INTERNALERROR;
      
  CurrentConnectionType = connection;

  if (FBUS_OpenSerial() != true) return GE_INTERNALERROR;

  return (GE_NONE);
}
