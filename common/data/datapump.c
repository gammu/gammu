/*

  $Id: datapump.c,v 1.3 2001/02/21 19:57:00 chris Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides routines to handle processing of data when connected in
  fax or data mode. Converts data from/to GSM phone to virtual modem
  interface.

*/

#define		__data_datapump_c


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <unistd.h>


#include "misc.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "data/at-emulator.h"
#include "data/virtmodem.h"
#include "data/datapump.h"
#include "data/rlp-common.h"

/* Global variables */
extern bool CommandMode;

/* Local variables */
int		PtyRDFD;	/* File descriptor for reading and writing to/from */
int		PtyWRFD;	/* pty interface - only different in debug mode. */ 
struct pollfd ufds;
u8 pluscount;
bool connected;

bool DP_Initialise(int read_fd, int write_fd)
{
	PtyRDFD = read_fd;
	PtyWRFD = write_fd;
	ufds.fd=PtyRDFD;
	ufds.events=POLLIN;
	RLP_Initialise(GSM->SendRLPFrame, DP_CallBack);
	RLP_SetUserRequest(Attach_Req,true);
	pluscount=0;
	connected=false;
	return true;
}


int DP_CallBack(RLP_UserInds ind, u8 *buffer, int length)
{
	int temp;

	switch(ind) {
	case Data:
		if (CommandMode==false) write(PtyWRFD, buffer, length);
		break;
	case Conn_Ind:
		if (CommandMode==false) ATEM_ModemResult(MR_CARRIER);
		RLP_SetUserRequest(Conn_Req,true);
		break;
	case StatusChange:
		if (buffer[0]==0) {
			connected=true;
			if (CommandMode==false) ATEM_ModemResult(MR_CONNECT);
		}
		break;
	case Disc_Ind:
		if (CommandMode==false) ATEM_ModemResult(MR_NOCARRIER);
		connected=false;
		/* Set the call passup back to the at emulator */
		GSM->DialData(NULL,-1,&ATEM_CallPassup);
		CommandMode=true;
		break;
	case Reset_Ind:
		RLP_SetUserRequest(Reset_Resp,true);
		break;
	case GetData:
		if (poll(&ufds,1,0)) {

			/* Check if the program has closed */
			/* Return to command mode */
			/* Note that the call will still be in progress, */
			/* as with a normal modem (I think) */

			if (ufds.revents!=POLLIN) { 
				CommandMode=true;
				/* Set the call passup back to the at emulator */
				GSM->DialData(NULL,-1,&ATEM_CallPassup);
				return 0;
			}

			temp = read(PtyRDFD, buffer, length);

			if (temp<0) return 0; /* FIXME - what do we do now? */

			/* This will only check +++ and the beginning of a read */
			/* But there should be a pause before it anyway */
      
			if (buffer[0]=='+') {
				pluscount++;
				if (temp>1) {
					if (buffer[1]=='+') pluscount++;
					else pluscount=0;
					if (temp>2) {
						if (buffer[2]=='+') pluscount++;
						else pluscount=0;
						if (temp>3) pluscount=0;
					}
				}
			} else pluscount=0;
      
			if (pluscount==3) {
				CommandMode=true;
				/* Set the call passup back to the at emulator */
				GSM->DialData(NULL,-1,&ATEM_CallPassup);
				ATEM_ModemResult(MR_OK);
				break;
			}
      
			return temp;
		}
		return 0;
		break;

	default:

	}
	return 0;
}

void DP_CallPassup(char c)
{
	switch (c) {
	case 'D':
		if (CommandMode==false) ATEM_ModemResult(MR_CARRIER);
		RLP_SetUserRequest(Conn_Req,true);
		connected=true;
		break;
	case ' ':
		CommandMode=true;
		/* Set the call passup back to the at emulator */
		GSM->DialData(NULL,-1,&ATEM_CallPassup);
		ATEM_ModemResult(MR_NOCARRIER);
		RLP_SetUserRequest(Disc_Req, true);
		connected=false;
		break;
	default:
		break;
	}
}
