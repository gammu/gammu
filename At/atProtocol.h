/* (c) 2002-2003 by Marcin Wiacek and Michal Cihar */
#ifndef atProtocol_h
#define atProtocol_h
#include "commfun.h"

typedef struct {
	GSM_Protocol_Message 	Msg;
	bool 			wascrlf;
	int			LineStart,LineEnd;
	int			SpecialAnswerLines,SpecialAnswerStart;

	bool			EditMode;	/* wait for modem answer or not */
	bool			FastWrite;
} GSM_Protocol_ATData;

GSM_Error AT_Initialise(int Speed);
GSM_Error AT_Terminate();
GSM_Error AT_SetProtocolData(bool EditMode,bool bFastWrite,DWORD dwFlag);
GSM_Error AT_WriteMessage (unsigned char *buffer,int length, unsigned char type);
GSM_Error AT_StateMachine(unsigned char rx_char);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
