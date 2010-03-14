
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "../gsmcomon.h"
#include "../misc/coding.h"
#include "gsmmms.h"

void GSM_EncodeMMSIndicatorSMSText(char *Buffer, int *Length, GSM_MMSIndicator Indicator)
{
	unsigned char 	buffer[200];
	int		i;

	strcpy(Buffer+(*Length),"\xE6\x06\""); (*Length)=(*Length)+3;
	strcpy(Buffer+(*Length),"application/vnd.wap.mms-message");	(*Length)=(*Length)+31;
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\xAF\x84\x8C\x82\x98"); (*Length)=(*Length)+5;
	i = strlen(Indicator.Address);
	while (Indicator.Address[i] != '/' && i!=0) i--;
	strcpy(Buffer+(*Length),Indicator.Address+i+1); (*Length)=(*Length)+strlen(Indicator.Address+i+1);
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x8D\x90\x89"); (*Length)=(*Length)+3;
	sprintf(buffer,"%s/TYPE=PLMN",Indicator.Sender);
	Buffer[(*Length)++] = strlen(buffer);
	Buffer[(*Length)++] = 0x80;
	strcpy(Buffer+(*Length),buffer); (*Length)=(*Length)+strlen(buffer);
	Buffer[(*Length)++] = 0x00;

	Buffer[(*Length)++] = 0x96;
	strcpy(Buffer+(*Length),Indicator.Title); (*Length)=(*Length)+strlen(Indicator.Title);
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x8A\x80\x8E\x02\x47\xBB\x88\x05\x81\x03\x02\xA3"); (*Length)=(*Length)+9+3;
	Buffer[(*Length)++] = 0x00;

	Buffer[(*Length)++] = 0x83;
	strcpy(Buffer+(*Length),Indicator.Address); (*Length)=(*Length)+strlen(Indicator.Address);
	Buffer[(*Length)++] = 0x00;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
