
#include <string.h>

#include "../misc/coding.h"
#include "gsmcal.h"
#include "gsmmisc.h"

void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02d%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,13,10);
}

void ReadVCALDateTime(char *Buffer, GSM_DateTime *dt)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	memset(dt,0,sizeof(dt));

	strncpy(year, 	Buffer, 	4);
	strncpy(month, 	Buffer+4, 	2);
	strncpy(day, 	Buffer+6, 	2);
	strncpy(hour, 	Buffer+9,	2);
	strncpy(minute, Buffer+11,	2);
	strncpy(second, Buffer+13,	2);

	/* FIXME: Should check ranges... */
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);
	dt->Hour	= atoi(hour);
	dt->Minute	= atoi(minute);
	dt->Second	= atoi(second);
	/* FIXME */
	dt->Timezone	= 0;
}

void SaveVCALText(char *Buffer, int *Length, char *Text, char *Start)
{
	char buffer[1000];

	if (UnicodeLength(Text) != 0) {
		EncodeUTF8(buffer,Text);
		if (UnicodeLength(Text)==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "%s:%s%c%c",Start,DecodeUnicodeString(Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",Start,buffer,13,10);
		}
	}	    
}

bool ReadVCALText(char *Buffer, char *Start, char *Value)
{
	unsigned char buff[200];

	Value[0] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		EncodeUnicode(Value,Buffer+strlen(Start)+1,strlen(Buffer)-(strlen(Start)+1));
		dprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeString2(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		DecodeUTF8(Value,Buffer+strlen(Start)+41,strlen(Buffer)-(strlen(Start)+41));
		dprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeString2(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
//		DecodeUTF8(Value,Buffer+strlen(Start)+41,strlen(Buffer)-(strlen(Start)+41));
//		dprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeString2(Value));
//		return true;
	}
	return false;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
