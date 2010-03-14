/*

  $Id: at-emulator.c,v 1.4 2001/07/03 15:27:03 pkot Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides a virtual modem or "AT" interface to the GSM phone by
  calling code in gsm-api.c. Inspired by and in places copied from the Linux
  kernel AT Emulator IDSN code by Fritz Elfert and others.
  
*/

#define		__data_at_emulator_c


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#ifndef WIN32

  #include <termios.h>

#endif

#include "config.h"
#include "misc.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "data/at-emulator.h"
#include "data/virtmodem.h"
#include "data/datapump.h"

#define MAX_LINE_LENGTH 256

	/* Global variables */
bool ATEM_Initialised = false;	/* Set to true once initialised */
extern bool    CommandMode;
extern int ConnectCount;

char ModelName[80]; /* This seems to be needed to avoid seg-faults */
char PortName[80];


	/* Local variables */
int	PtyRDFD;	/* File descriptor for reading and writing to/from */
int	PtyWRFD;	/* pty interface - only different in debug mode. */ 

u8	ModemRegisters[MAX_MODEM_REGISTERS];
char	CmdBuffer[MAX_CMD_BUFFERS][CMD_BUFFER_LENGTH];
int	CurrentCmdBuffer;
int	CurrentCmdBufferIndex;
bool	VerboseResponse; 	/* Switch betweek numeric (4) and text responses (ERROR) */
char    IncomingCallNo;
int     MessageFormat;          /* Message Format (text or pdu) */

 	/* Current command parser */
void 	(*Parser)(char *);
//void 	(*Parser)(char *) = ATEM_ParseAT; /* Current command parser */

GSM_MemoryType 	SMSType;
int 	SMSNumber;

	/* If initialised in debug mode, stdin/out is used instead
	   of ptys for interface. */
bool	ATEM_Initialise(int read_fd, int write_fd, char *model, char *port)
{
	PtyRDFD = read_fd;
	PtyWRFD = write_fd;

	strncpy(ModelName,model,80);
	strncpy(PortName,port,80);

		/* Initialise command buffer variables */
	CurrentCmdBuffer = 0;
	CurrentCmdBufferIndex = 0;
	
		/* Default to verbose reponses */
	VerboseResponse = true;

		/* Initialise registers */
	ATEM_InitRegisters();
	
		/* Initial parser is AT routine */
	Parser = ATEM_ParseAT;
	
		/* Setup defaults for AT*C interpreter. */
	SMSNumber = 1;
	SMSType = GMT_ME;

		/* Default message format is PDU */
	MessageFormat = PDU_MODE;

	/* Set the call passup so that we get notified of incoming calls */
        GSM->DialData(NULL,-1,&ATEM_CallPassup);

		/* We're ready to roll... */
	ATEM_Initialised = true;
	return (true);
}

	/* Initialise the "registers" used by the virtual modem. */
void	ATEM_InitRegisters(void) 
{

	ModemRegisters[REG_RINGATA] = 0;
	ModemRegisters[REG_RINGCNT] = 2;
	ModemRegisters[REG_ESC] = '+';
	ModemRegisters[REG_CR] = 10;
	ModemRegisters[REG_LF] = 13;
	ModemRegisters[REG_BS] = 8;
	ModemRegisters[S35]=7;
	ModemRegisters[REG_ECHO] = BIT_ECHO;

}


/* This gets called to indicate an incoming call */

void ATEM_CallPassup(char c)
{
	if ((c >= 0) && (c < 9)) {
		ATEM_ModemResult(MR_RING);		
		IncomingCallNo = c;
	}
}


    /* Handler called when characters received from serial port.
       calls state machine code to process it. */

void	ATEM_HandleIncomingData(char *buffer, int length)
{
	int count;
	unsigned char out_buf[3];	

	for (count = 0; count < length ; count++) {
			/* Echo character if appropriate. */
		if (ModemRegisters[REG_ECHO] & BIT_ECHO) {
			out_buf[0] = buffer[count];
			out_buf[1] = 0;
			ATEM_StringOut(out_buf);
		}

			/* If it's a command terminator character, parse what
			   we have so far then go to next buffer. */
		if (buffer[count] == ModemRegisters[REG_CR] ||
		    buffer[count] == ModemRegisters[REG_LF]) {

			CmdBuffer[CurrentCmdBuffer][CurrentCmdBufferIndex] = 0x00;
			Parser(CmdBuffer[CurrentCmdBuffer]);

			CurrentCmdBuffer++;
			if (CurrentCmdBuffer >= MAX_CMD_BUFFERS) {
				CurrentCmdBuffer = 0;
			}
			CurrentCmdBufferIndex = 0;
		} else {
			CmdBuffer[CurrentCmdBuffer][CurrentCmdBufferIndex] = buffer[count];
			CurrentCmdBufferIndex++;
			if (CurrentCmdBufferIndex >= CMD_BUFFER_LENGTH) {
				CurrentCmdBufferIndex = CMD_BUFFER_LENGTH;
			}
		}
	}
}     


	/* Parser for standard AT commands.  cmd_buffer must be null terminated. */
void	ATEM_ParseAT(char *cmd_buffer)
{
	char *buf;
	char number[30];

	if (strncasecmp (cmd_buffer, "AT", 2) != 0) {
		ATEM_ModemResult(MR_ERROR);
		return;
	}

	for (buf = &cmd_buffer[2]; *buf;) {
		switch (toupper(*buf)) {

		case 'Z':
			buf++;
			break;
		case 'A':
			buf++;
			/* For now we'll also initialise the datapump + rlp code again */
			DP_Initialise(PtyRDFD, PtyWRFD);
			GSM->DialData(NULL, -1, &DP_CallPassup);
			GSM->AnswerCall(IncomingCallNo);
			CommandMode = false;
			return;
			break;
		case 'D':
			/* Dial Data :-) */
			/* FIXME - should parse this better */
			/* For now we'll also initialise the datapump + rlp code again */
			DP_Initialise(PtyRDFD, PtyWRFD);
			buf++;
			if (toupper(*buf) == 'T') buf++;
			if (*buf == ' ') buf++;
			strncpy(number, buf, 30);
			if (ModemRegisters[S35] == 0) GSM->DialData(number, 1, &DP_CallPassup);
			else GSM->DialData(number, 0, &DP_CallPassup);
			ATEM_StringOut("\n\r");
			CommandMode = false;
			return;
			break;
		case 'H':
			/* Hang Up */
			buf++;
			RLP_SetUserRequest(Disc_Req, true);
			GSM->CancelCall();
			break;
		case 'S':
			/* Change registers - only no. 35 for now */
			buf++;
			if (memcmp(buf, "35=", 3) == 0) {
				buf += 3;
				ModemRegisters[S35] = *buf - '0';
				buf++;
			}
			break;
		  /* E - Turn Echo on/off */
		case 'E':
			buf++;
			switch (ATEM_GetNum(&buf)) {
			case 0:
				ModemRegisters[REG_ECHO] &= ~BIT_ECHO;
				break;
			case 1:
				ModemRegisters[REG_ECHO] |= BIT_ECHO;
				break;
			default:
				ATEM_ModemResult(MR_ERROR);
				return;
			}
			break;
			
		  /* Handle AT* commands (Nokia proprietary I think) */
		case '*':
			buf++;
			if (!strcasecmp(buf, "NOKIATEST")) {
				ATEM_ModemResult(MR_OK); /* FIXME? */
				return;
			} else {
				if (!strcasecmp(buf, "C")) {
					ATEM_ModemResult(MR_OK);
					Parser = ATEM_ParseSMS;
					return;
				}
			}
			break;
			
				/* + is the precursor to another set of commands +CSQ, +FCLASS etc. */
		case '+':
			buf++;
			switch (toupper(*buf)) {
			case 'C':
				buf++;
				/* Returns true if error occured */
				if (ATEM_CommandPlusC(&buf) == true) {
					return;	
				}
				break;
				
			case 'G':
				buf++;
				/* Returns true if error occured */
				if (ATEM_CommandPlusG(&buf) == true) {
					return;	
				}
				break;
				
			default:
				ATEM_ModemResult(MR_ERROR);
				return;
			}
			break;
			
		default: 
			ATEM_ModemResult(MR_ERROR);
			return;
		}
	}
	
	ATEM_ModemResult(MR_OK);
}

static GSM_Error ATEM_ReadSMS(int number, GSM_MemoryType type, GSM_SMSMessage *message)
{
	GSM_Error error;

	message->MemoryType = type;
	message->Location = number;
	error = GSM->GetSMSMessage(message);

	return error;
}

static void ATEM_PrintSMS(char *line, GSM_SMSMessage *message, int mode)
{
	switch (mode) {
	case INTERACT_MODE:
		gsprintf(line, MAX_LINE_LENGTH, _("\n\rDate/time: %d/%d/%d %d:%02d:%02d Sender: %s Msg Center: %s\n\rText: %s\n\r"), message->Time.Day, message->Time.Month, message->Time.Year, message->Time.Hour, message->Time.Minute, message->Time.Second, message->Sender, message->MessageCenter.Number, message->MessageText);
		break;
	case TEXT_MODE:
		if (message->Coding==GSM_Coding_8bit)
			gsprintf(line, MAX_LINE_LENGTH, _("\"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\n\r%s"), (message->Status ? _("REC READ") : _("REC UNREAD")), message->Sender, message->Time.Year, message->Time.Month, message->Time.Day, message->Time.Hour, message->Time.Minute, message->Time.Second, message->Time.Timezone, _("<Not implemented>"));
		else
			gsprintf(line, MAX_LINE_LENGTH, _("\"%s\",\"%s\",,\"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\n\r%s"), (message->Status ? _("REC READ") : _("REC UNREAD")), message->Sender, message->Time.Year, message->Time.Month, message->Time.Day, message->Time.Hour, message->Time.Minute, message->Time.Second, message->Time.Timezone, message->MessageText);
		break;
	case PDU_MODE:
		gsprintf(line, MAX_LINE_LENGTH, _("<Not implemented>"));
		break;
	default:
		gsprintf(line, MAX_LINE_LENGTH, _("<Unknown mode>"));
		break;
	}
}

static void ATEM_EraseSMS(int number, GSM_MemoryType type)
{
	GSM_SMSMessage message;
	message.MemoryType = type;
	message.Location = number;
	if (GSM->DeleteSMSMessage(&message) == GE_NONE) {
		ATEM_ModemResult(MR_OK);
	} else {
		ATEM_ModemResult(MR_ERROR);
	}
}


static void ATEM_HandleSMS()
{
	GSM_SMSMessage	message;
	GSM_Error	error;
	char		buffer[MAX_LINE_LENGTH];

	error = ATEM_ReadSMS(SMSNumber, SMSType, &message);
	switch (error) {
	case GE_NONE:
		ATEM_PrintSMS(buffer, &message, INTERACT_MODE);
		ATEM_StringOut(buffer);
		break;
	default:
		gsprintf(buffer, MAX_LINE_LENGTH, _("\n\rNo message under number %d\n\r"), SMSNumber);
		ATEM_StringOut(buffer);
		break;
	}
	return;
}

	/* Parser for SMS interactive mode */
void	ATEM_ParseSMS(char *buff)
{
	if (!strcasecmp(buff, "HELP")) {
		ATEM_StringOut(_("\n\rThe following commands work...\n\r"));
		ATEM_StringOut("DIR\n\r");
		ATEM_StringOut("EXIT\n\r");
		ATEM_StringOut("HELP\n\r");
		return;
	}

	if (!strcasecmp(buff, "DIR")) {
		SMSNumber = 1;
		ATEM_HandleSMS();
		Parser = ATEM_ParseDIR;
		return;
	}
	if (!strcasecmp(buff, "EXIT")) {
		Parser = ATEM_ParseAT;
		ATEM_ModemResult(MR_OK);
		return;
	} 
	ATEM_ModemResult(MR_ERROR);
}

	/* Parser for DIR sub mode of SMS interactive mode. */
void	ATEM_ParseDIR(char *buff)
{
	switch (toupper(*buff)) {
		case 'P':
			SMSNumber--;
			ATEM_HandleSMS();
			return;
		case 'N':
			SMSNumber++;
			ATEM_HandleSMS();
			return;
		case 'D':
			ATEM_EraseSMS(SMSNumber, SMSType);
			return;
		case 'Q':
			Parser= ATEM_ParseSMS;
			ATEM_ModemResult(MR_OK);
			return;
	}
	ATEM_ModemResult(MR_ERROR);
}
 
	/* Handle AT+C commands, this is a quick hack together at this
	   stage. */
bool	ATEM_CommandPlusC(char **buf)
{
	float		rflevel;
	GSM_RFUnits	rfunits = GRF_CSQ;
	char		buffer[MAX_LINE_LENGTH], buffer2[MAX_LINE_LENGTH];
	int		status, index;
	GSM_Error	error;
	GSM_SMSMessage	message;

	if (strncasecmp(*buf, "SQ", 2) == 0) {
		buf[0] += 2;

	    	if (GSM->GetRFLevel(&rfunits, &rflevel) == GE_NONE) {
			gsprintf(buffer, MAX_LINE_LENGTH, "\n\r+CSQ: %0.0f, 99", rflevel);
			ATEM_StringOut(buffer);
			return (false);
		} else {
			return (true);
		}
	}
		/* AT+CGMI is Manufacturer information for the ME (phone) so
		   it should be Nokia rather than gnokii... */
	if (strncasecmp(*buf, "GMI", 3) == 0) {
		buf[0] += 3;
		ATEM_StringOut(_("\n\rNokia Mobile Phones"));
		return (false);
	}

		/* AT+CGSN is IMEI */
	if (strncasecmp(*buf, "GSN", 3) == 0) {
		buf[0] += 3;
		if (GSM->GetIMEI(buffer2) == GE_NONE) {
			gsprintf(buffer, MAX_LINE_LENGTH, "\n\r%s", buffer2);
			ATEM_StringOut(buffer);
			return (false);
		} else {
			return (true);
		}
	}

		/* AT+CGMR is Revision (hardware) */
	if (strncasecmp(*buf, "GMR", 3) == 0) {
		buf[0] += 3;
		if (GSM->GetRevision(buffer2) == GE_NONE) {
			gsprintf(buffer, MAX_LINE_LENGTH, "\n\r%s", buffer2);
			ATEM_StringOut(buffer);
			return (false);
		} else {
			return (true);
		}
	}

		/* AT+CGMM is Model code  */
	if (strncasecmp(*buf, "GMM", 3) == 0) {
		buf[0] += 3;
		if (GSM->GetModel(buffer2) == GE_NONE) {
			gsprintf(buffer, MAX_LINE_LENGTH, "\n\r%s", buffer2);
			ATEM_StringOut(buffer);
			return (false);
		} else {
			return (true);
		}
	}

		/* AT+CMGF is mode selection for message format  */
	if (strncasecmp(*buf, "MGF", 3) == 0) {
		buf[0] += 3;
		switch (**buf) {
		case '=':
			buf[0]++;
			switch (**buf) {
			case '0':
				buf[0]++;
				MessageFormat = PDU_MODE;
				break;
			case '1':
				buf[0]++;
				MessageFormat = TEXT_MODE;
				break;
			default:
				return (true);
			}
			break;
		case '?':
			buf[0]++;
			gsprintf(buffer, MAX_LINE_LENGTH, "\n\r+CMGF: %d", MessageFormat);
			ATEM_StringOut(buffer);
			break;
		default:
			return (true);
		}
		return (false);
	}

		/* AT+CMGR is reading a message */
	if (strncasecmp(*buf, "MGR", 3) == 0) {
		buf[0] += 3;
		switch (**buf) {
		case '=':
			buf[0]++;
			sscanf(*buf, "%d", &index);
			buf[0] += strlen(*buf);

			error = ATEM_ReadSMS(index, SMSType, &message);
			switch (error) {
			case GE_NONE:
				ATEM_PrintSMS(buffer2, &message, MessageFormat);
				gsprintf(buffer, MAX_LINE_LENGTH, "\n\r+CMGR: %s", buffer2);
				ATEM_StringOut(buffer);
				break;
			default:
				gsprintf(buffer, MAX_LINE_LENGTH, "\n\r+CMS ERROR: %d\n\r", error);
				ATEM_StringOut(buffer);
				return (true);
			}
			break;
		default:
			return (true);
		}
		return (false);
	}

		/* AT+CMGL is listing messages */
	if (strncasecmp(*buf, "MGL", 3) == 0) {
		buf[0]+=3;
		status = -1;

		switch (**buf) {
		case 0:
		case '=':
			buf[0]++;
			/* process <stat> parameter */
			if (*(*buf-1) == 0 || /* i.e. no parameter given */
				strcasecmp(*buf, "1") == 0 ||
				strcasecmp(*buf, "3") == 0 ||
				strcasecmp(*buf, "\"REC READ\"") == 0 ||
				strcasecmp(*buf, "\"STO SENT\"") == 0) {
				status = GSS_SENTREAD;
			} else if (strcasecmp(*buf, "0") == 0 ||
				strcasecmp(*buf, "2") == 0 ||
				strcasecmp(*buf, "\"REC UNREAD\"") == 0 ||
				strcasecmp(*buf, "\"STO UNSENT\"") == 0) {
				status = GSS_NOTSENTREAD;
			} else if (strcasecmp(*buf, "4") == 0 ||
				strcasecmp(*buf, "\"ALL\"") == 0) {
				status = 4; /* ALL */
			} else {
				return true;
			}
			buf[0] += strlen(*buf);

			/* check all message storages */
			for (index = 1; index <= 20; index++) {
				error = ATEM_ReadSMS(index, SMSType, &message);
				switch (error) {
				case GE_NONE:
					/* print messsage if it has the required status */
					if (message.Status == status || status == 4 /* ALL */) {
						ATEM_PrintSMS(buffer2, &message, MessageFormat);
						gsprintf(buffer, MAX_LINE_LENGTH, "\n\r+CMGL: %d,%s", index, buffer2);
						ATEM_StringOut(buffer);
					}
					break;
				case GE_EMPTYSMSLOCATION:
					/* don't care if this storage is empty */
					break;
				default:
					/* print other error codes and quit */
					gsprintf(buffer, MAX_LINE_LENGTH, "\n\r+CMS ERROR: %d\n\r", error);
					ATEM_StringOut(buffer);
					return (true);
				}
			}
			break;
		default:
			return (true);
		}
		return (false);
	}

	return (true);
}

	/* AT+G commands.  Some of these responses are a bit tongue in cheek... */
bool	ATEM_CommandPlusG(char **buf)
{
	char		buffer[MAX_LINE_LENGTH];

		/* AT+GMI is Manufacturer information for the TA (Terminal Adaptor) */
	if (strncasecmp(*buf, "MI", 3) == 0) {
		buf[0] += 2;

		ATEM_StringOut(_("\n\rHugh Blemings, Pavel Janík ml. and others..."));
		return (false);
	}

		/* AT+GMR is Revision information for the TA (Terminal Adaptor) */
	if (strncasecmp(*buf, "MR", 3) == 0) {
		buf[0] += 2;
		gsprintf(buffer, MAX_LINE_LENGTH, "\n\r%s %s %s", VERSION, __TIME__, __DATE__);

		ATEM_StringOut(buffer);
		return (false);
	}

		/* AT+GMM is Model information for the TA (Terminal Adaptor) */
	if (strncasecmp(*buf, "MM", 3) == 0) {
		buf[0] += 2;

		gsprintf(buffer, MAX_LINE_LENGTH, _("\n\rgnokii configured for %s on %s"), ModelName, PortName);
		ATEM_StringOut(buffer);
		return (false);
	}

		/* AT+GSN is Serial number for the TA (Terminal Adaptor) */
	if (strncasecmp(*buf, "SN", 3) == 0) {
		buf[0] += 2;

		gsprintf(buffer, MAX_LINE_LENGTH, _("\n\rnone built in, choose your own"));
		ATEM_StringOut(buffer);
		return (false);
	}

	return (true);
}

	/* Send a result string back.  There is much work to do here, see
	   the code in the isdn driver for an idea of where it's heading... */
void	ATEM_ModemResult(int code) 
{
	char	buffer[16];

	if (VerboseResponse == false) {
		sprintf(buffer, "\n\r%d\n\r", code);
		ATEM_StringOut(buffer);
	} else {
		switch (code) {
			case MR_OK:	
					ATEM_StringOut("\n\rOK\n\r");
					break;

			case MR_ERROR:
					ATEM_StringOut("\n\rERROR\n\r");
					break;

			case MR_CARRIER:
					ATEM_StringOut("\n\rCARRIER\n\r");
					break;

			case MR_CONNECT:
					ATEM_StringOut("\n\rCONNECT\n\r");
					break;

			case MR_NOCARRIER:
					ATEM_StringOut("\n\rNO CARRIER\n\r");
					break;
		        case MR_RING:
					ATEM_StringOut("RING\n\r");
					break;
			default:
					ATEM_StringOut(_("\n\rUnknown Result Code!\n\r"));
					break;
		}
	}

}


	/* Get integer from char-pointer, set pointer to end of number
	   stolen basically verbatim from ISDN code.  */
int ATEM_GetNum(char **p)
{
	int v = -1;

	while (*p[0] >= '0' && *p[0] <= '9') {
		v = ((v < 0) ? 0 : (v * 10)) + (int) ((*p[0]++) - '0');
	}

	return v;
}

	/* Write string to virtual modem port, either pty or
	   STDOUT as appropriate.  This function is only used during
	   command mode - data pump is used when connected.  */
void	ATEM_StringOut(char *buffer)
{
	int		count = 0;
	char	out_char;

	while (count < strlen(buffer)) {

			/* Translate CR/LF/BS as appropriate */
		switch (buffer[count]) {
			case '\r':
				out_char = ModemRegisters[REG_CR];
				break;
			case '\n':
				out_char = ModemRegisters[REG_LF];
				break;
			case '\b':
				out_char = ModemRegisters[REG_BS];
				break;
			default:
				out_char = buffer[count];
				break;
		}

		write(PtyWRFD, &out_char, 1);
		count ++;
	}

}
