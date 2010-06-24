

#ifndef fbus2_h
#define fbus2_h

#define FBUS2_FRAME_ID       	0x1e
#define FBUS2_IRDA_FRAME_ID    	0x1c
#define FBUS2_DEVICE_PHONE   	0x00 /* Nokia mobile phone */
#define FBUS2_DEVICE_PC      	0x10 /* Our PC */
#define FBUS2_ACK_BYTE	     	0x7f /* Acknowledge of the received frame */

#define FBUS2_MAX_TRANSMIT_LENGTH 120

typedef struct {
	int			MsgSequenceNumber;
	int			MsgRXState;
	int			FramesToGo;
	GSM_Protocol_Message	MultiMsg;
	GSM_Protocol_Message	Msg;
} GSM_Protocol_FBUS2Data;

int FBUS2_WriteDLR3(char *command, int length, int timeout);
GSM_Error FBUS2_Initialise();
GSM_Error FBUS2_Terminate();
GSM_Error FBUS2_StateMachine(unsigned char rx_char);
GSM_Error FBUS2_SendAck( unsigned char 		MsgType,unsigned char 		MsgSequence);
GSM_Error FBUS2_WriteMessage (unsigned char *MsgBuffer,int 	MsgLength, unsigned char 	MsgType);
GSM_Error FBUS2_WriteFrame(unsigned char 	*MsgBuffer,int 			MsgLength,unsigned char 	MsgType);
int FBUS2_ReadAck (bool wait);


#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
