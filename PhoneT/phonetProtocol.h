/* (c) 2002-2003 by Marcin Wiacek */
/* Based on some work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */
#ifndef PHONETPROTOCOL_h
#define PHONETPROTOCOL_h
#include "commfun.h"

#define PHONET_FRAME_ID      	0x14
#define PHONET_BLUE_FRAME_ID	0x19
#define PHONET_DEVICE_PHONE   	0x00 /* Nokia mobile phone */
#define PHONET_DEVICE_PC      	0x0c /* Our PC */
#define PHONET_BLUE_DEVICE_PC   0x10 /* Our PC */

typedef struct {
	int			MsgRXState;
	GSM_Protocol_Message	Msg;
} GSM_Protocol_PHONETData;

GSM_Error PHONET_Terminate();
GSM_Error PHONET_Initialise();
GSM_Error PHONET_StateMachine(unsigned char rx_char);
GSM_Error PHONET_WriteMessage (unsigned char *MsgBuffer,int MsgLength,unsigned char	MsgType);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
