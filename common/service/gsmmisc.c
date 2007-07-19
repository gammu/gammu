/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */

#include <gammu-config.h>

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef WIN32
#  include <io.h>
#  include <fcntl.h>
#  include <winsock2.h>
#else
#  include <netdb.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#endif

#include <gammu-keys.h>
#include <gammu-debug.h>

#include "../misc/coding/coding.h"
#include "../gsmcomon.h"
#include "gsmmisc.h"

struct keys_table_position {
	char 	whatchar;
	int 	whatcode;
};

static struct keys_table_position Keys[] = {
	{'m',GSM_KEY_MENU},		{'M',GSM_KEY_MENU},
	{'n',GSM_KEY_NAMES},		{'N',GSM_KEY_NAMES},
	{'p',GSM_KEY_POWER},		{'P',GSM_KEY_POWER},
	{'u',GSM_KEY_UP},		{'U',GSM_KEY_UP},
	{'d',GSM_KEY_DOWN},		{'D',GSM_KEY_DOWN},
	{'+',GSM_KEY_INCREASEVOLUME},	{'-',GSM_KEY_DECREASEVOLUME},
	{'1',GSM_KEY_1},		{'2',GSM_KEY_2},	{'3',GSM_KEY_3},
	{'4',GSM_KEY_4},		{'5',GSM_KEY_5},	{'6',GSM_KEY_6},
	{'7',GSM_KEY_7},		{'8',GSM_KEY_8},	{'9',GSM_KEY_9},
	{'*',GSM_KEY_ASTERISK},		{'0',GSM_KEY_0},	{'#',GSM_KEY_HASH},
	{'g',GSM_KEY_GREEN},		{'G',GSM_KEY_GREEN},
	{'r',GSM_KEY_RED},		{'R',GSM_KEY_RED},
	{'<',GSM_KEY_LEFT},		{'>',GSM_KEY_RIGHT},
	{'[',GSM_KEY_SOFT1},		{']',GSM_KEY_SOFT2},
	{'h',GSM_KEY_HEADSET},		{'H',GSM_KEY_HEADSET},
	{'c',GSM_KEY_CLEAR},		{'C',GSM_KEY_CLEAR},
	{'j',GSM_KEY_JOYSTICK},		{'J',GSM_KEY_JOYSTICK},
	{'f',GSM_KEY_CAMERA},		{'F',GSM_KEY_CAMERA},
	{'o',GSM_KEY_OPERATOR},		{'O',GSM_KEY_OPERATOR},
	{'m',GSM_KEY_MEDIA},		{'M',GSM_KEY_MEDIA},
	{'d',GSM_KEY_DESKTOP},		{'D',GSM_KEY_DESKTOP},
	{'@',GSM_KEY_RETURN},
	{' ',0}
};

GSM_Error MakeKeySequence(char *text, GSM_KeyCode *KeyCode, int *Length)
{
	int 		i,j;
	unsigned char 	key;

	for (i=0;i<(int)(strlen(text));i++) {
		key 	   = text[i];
		KeyCode[i] = GSM_KEY_NONE;
		j 	   = 0;
		while (Keys[j].whatchar!=' ') {
	        	if (Keys[j].whatchar==key) {
				KeyCode[i]=Keys[j].whatcode;
				break;
			}
			j++;
		}
		if (KeyCode[i] == GSM_KEY_NONE) {
			*Length = i;
			return ERR_NOTSUPPORTED;
		}
	}
	*Length = i;
	return ERR_NONE;
}

GSM_Error GSM_ReadFile(char *FileName, GSM_File *File)
{
	int 		i = 1000;
	FILE		*file;
	struct stat	fileinfo;

	if (FileName[0] == 0x00) return ERR_UNKNOWN;
	file = fopen(FileName,"rb");
	if (file == NULL) return ERR_CANTOPENFILE;

	free(File->Buffer);
	File->Buffer 	= NULL;
	File->Used 	= 0;
	while (i == 1000) {
		File->Buffer 	= realloc(File->Buffer,File->Used + 1000);
		i 		= fread(File->Buffer+File->Used,1,1000,file);
		File->Used 	= File->Used + i;
	}
	File->Buffer = realloc(File->Buffer,File->Used + 1);
	/* Make it 0 terminated, in case it is needed somewhere (we don't count this to length) */
	File->Buffer[File->Used] = 0;
	fclose(file);

	File->ModifiedEmpty = true;
	if (stat(FileName,&fileinfo) == 0) {
		File->ModifiedEmpty = false;
		dbgprintf("File info read correctly\n");
		//st_mtime is time of last modification of file
		Fill_GSM_DateTime(&File->Modified, fileinfo.st_mtime);
		File->Modified.Year = File->Modified.Year + 1900;
		dbgprintf("FileTime: %02i-%02i-%04i %02i:%02i:%02i\n",
			File->Modified.Day,File->Modified.Month,File->Modified.Year,
			File->Modified.Hour,File->Modified.Minute,File->Modified.Second);
	}

	return ERR_NONE;
}

static void GSM_JADFindLine(GSM_File File, char *Name, char *Value)
{
	unsigned char 	Line[2000];
	int		Pos = 0;

	Value[0] = 0;

	while (1) {
		MyGetLine(File.Buffer, &Pos, Line, File.Used, false);
		if (strlen(Line) == 0) break;
		if (!strncmp(Line,Name,strlen(Name))) {
			Pos = strlen(Name);
			while (Line[Pos] == 0x20) Pos++;
			strcpy(Value,Line+Pos);
			return;
		}
	}
}

GSM_Error GSM_JADFindData(GSM_File File, char *Vendor, char *Name, char *JAR, char *Version, int *Size)
{
	char Size2[200];

	GSM_JADFindLine(File, "MIDlet-Vendor:", Vendor);
	if (Vendor[0] == 0x00) return ERR_FILENOTSUPPORTED;
	dbgprintf("Vendor: \"%s\"\n",Vendor);

	GSM_JADFindLine(File, "MIDlet-Name:", Name);
	if (Name[0] == 0x00) return ERR_FILENOTSUPPORTED;
	dbgprintf("Name: \"%s\"\n",Name);

	GSM_JADFindLine(File, "MIDlet-Jar-URL:", JAR);
	if (JAR[0] == 0x00) return ERR_FILENOTSUPPORTED;
	dbgprintf("JAR file URL: \"%s\"\n",JAR);

	GSM_JADFindLine(File, "MIDlet-Jar-Size:", Size2);
	*Size = -1;
	if (Size2[0] == 0x00) return ERR_FILENOTSUPPORTED;
	dbgprintf("JAR size: \"%s\"\n",Size2);
	(*Size) = atoi(Size2);

	GSM_JADFindLine(File, "MIDlet-Version:", Version);
	dbgprintf("Version: \"%s\"\n",Version);

	return ERR_NONE;
}

void GSM_IdentifyFileFormat(GSM_File *File)
{
	File->Type = GSM_File_Other;
	if (File->Used > 2) {
		if (memcmp(File->Buffer, "BM",2)==0) {
			File->Type = GSM_File_Image_BMP;
		} else if (memcmp(File->Buffer, "GIF",3)==0) {
			File->Type = GSM_File_Image_GIF;
		} else if (File->Buffer[0] == 0x00 && File->Buffer[1] == 0x00) {
			File->Type = GSM_File_Image_WBMP;
		} else if (memcmp(File->Buffer+1, "PNG",3)==0) {
			File->Type = GSM_File_Image_PNG;
		} else if (File->Buffer[0] == 0xFF && File->Buffer[1] == 0xD8) {
			File->Type = GSM_File_Image_JPG;
		} else if (memcmp(File->Buffer, "MThd",4)==0) {
			File->Type = GSM_File_Sound_MIDI;
		} else if (File->Buffer[0] == 0x00 && File->Buffer[1] == 0x02) {
			File->Type = GSM_File_Sound_NRT;
		}
	}
}

void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02d%s%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,
			Date->Timezone == 0 ? "Z" : "",
			13,10);
}

void SaveVCALDate(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02d%C%C",
			Date->Year, Date->Month, Date->Day,13,10);
}

bool ReadVCALDateTime(const char *Buffer, GSM_DateTime *dt)
{
	time_t time_t;
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	memset(dt,0,sizeof(GSM_DateTime));

	strncpy(year, 	Buffer, 	4);
	strncpy(month, 	Buffer+4, 	2);
	strncpy(day, 	Buffer+6, 	2);
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);

	if (Buffer[8] == 'T') {
		strncpy(hour, 	Buffer+9,	2);
		strncpy(minute, Buffer+11,	2);
		strncpy(second, Buffer+13,	2);
		dt->Hour	= atoi(hour);
		dt->Minute	= atoi(minute);
		dt->Second	= atoi(second);

		/**
		 * @todo Handle properly timezone information
		 */
		if (Buffer[15] == 'Z') dt->Timezone = 0; /* Z = ZULU = GMT */
	}

	if (!CheckTime(dt)) {
		dbgprintf("incorrect date %d-%d-%d %d:%d:%d\n",dt->Day,dt->Month,dt->Year,dt->Hour,dt->Minute,dt->Second);
		return false;
	}
	if (dt->Year!=0) {
		if (!CheckDate(dt)) {
			dbgprintf("incorrect date %d-%d-%d %d:%d:%d\n",dt->Day,dt->Month,dt->Year,dt->Hour,dt->Minute,dt->Second);
			return false;
		}
	}

	if (dt->Timezone != 0) {
		time_t = Fill_Time_T(*dt) + dt->Timezone*3600;
		Fill_GSM_DateTime(dt, time_t);
		dt->Year +=1900;
	}

	return true;
}

bool ReadVCALInt(char *Buffer, char *Start, int *Value)
{
	unsigned char buff[200];

	*Value = 0;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		int lstart = strlen(Start);
		int lvalue = strlen(Buffer)-(lstart+1);
		strncpy(buff,Buffer+lstart+1,lvalue);
		strncpy(buff+lvalue,"\0",1);
		if (sscanf(buff,"%i",Value)) {
			dbgprintf("ReadVCalInt is \"%i\"\n",*Value);
			return true;
		}
	}
	return false;
}


bool ReadVCALDate(char *Buffer, char *Start, GSM_DateTime *Date, bool *is_date_only)
{
	unsigned char buff[200];
	int lstart, lvalue;

	strcpy(buff,Start);
	strcat(buff,":");
	*is_date_only = false;
	if (strncmp(Buffer,buff,strlen(buff))) {
		strcpy(buff,Start);
		strcat(buff,";VALUE=DATE:");
		if (strncmp(Buffer,buff,strlen(buff)))
			return false;
		*is_date_only = true;
	}

	lstart = strlen(buff);
	lvalue = strlen(Buffer)-lstart;
	strncpy(buff,Buffer+lstart,lvalue);
	strncpy(buff+lvalue,"\0",1);
	if (ReadVCALDateTime(buff,Date)) {
		dbgprintf("ReadVCALDateTime is \"%04d.%02d.%02d %02d:%02d:%02d\"\n",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second);
		return true;
	}
	return false;
}


void SaveVCALText(char *Buffer, int *Length, unsigned char *Text, char *Start, bool UTF8)
{
	char buffer[1000];

	if (UnicodeLength(Text) != 0) {
		if (UTF8) { 
			EncodeUTF8(buffer, Text);
			*Length += sprintf(Buffer+(*Length), "%s:%s%c%c", Start, DecodeUnicodeString(Text), 13, 10);
		} else {
			EncodeUTF8QuotedPrintable(buffer,Text);
			if (UnicodeLength(Text)==strlen(buffer)) {
				*Length+=sprintf(Buffer+(*Length), "%s:%s%c%c",Start,DecodeUnicodeString(Text),13,10);
			} else {
				*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",Start,buffer,13,10);
			}
		}
	}
}

unsigned char *VCALGetTextPart(unsigned char *Buff, int *pos)
{
	static unsigned char	tmp[1000];
	unsigned char		*start;

	start = Buff + *pos;
	while (Buff[*pos] != 0 || Buff[*pos + 1] != 0) {
		if (Buff[*pos] == 0 && Buff[*pos + 1] == ';') {
			Buff[*pos + 1] = 0;
			CopyUnicodeString(tmp, start);
			Buff[*pos + 1] = ';';
			*pos += 2;
			return tmp;
		}
		*pos += 2;
	}
	if (start == Buff || (start[0] == 0 && start[1] == 0)) return NULL;
	CopyUnicodeString(tmp, start);
	return tmp;
}

/**
 * Reads text for vCalendar/vCard formatted line.
 *
 * \todo We should try to detect utf-8 and automatically use it.
 *
 * \param Buffer Buffer with input data.
 * \param Start Start expression of line.
 * \param Value Storage for value.
 * \param UTF8 Whether string should be treated as utf-8.
 */
bool ReadVCALText(char *Buffer, char *Start, unsigned char *Value, bool UTF8)
{
	unsigned char buff[200];

	Value[0] = 0x00;
	Value[1] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		if (UTF8) {
			DecodeUTF8(Value, Buffer + strlen(Start) + 1, strlen(Buffer) - (strlen(Start) + 1));
		} else {
			DecodeISO88591(Value, Buffer + strlen(Start) + 1, strlen(Buffer) - (strlen(Start) + 1));
		}
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		DecodeISO88591QuotedPrintable(Value,Buffer+strlen(Start)+27,strlen(Buffer)-(strlen(Start)+27));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41,strlen(Buffer)-(strlen(Start)+41));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		DecodeUTF8(Value,Buffer+strlen(Start)+15,strlen(Buffer)-(strlen(Start)+15));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		DecodeUTF7(Value,Buffer+strlen(Start)+15,strlen(Buffer)-(strlen(Start)+15));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	return false;
}

bool GSM_ReadHTTPFile(unsigned char *server, unsigned char *filename, GSM_File *file)
{
	int			s, len;
	struct sockaddr_in 	address;
	struct hostent 		*address2;
	unsigned char 		buff[200];
#ifdef WIN32
	WSADATA			wsaData;

	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) return false;
#endif

	s = socket(AF_INET,SOCK_STREAM,0);
#ifdef WIN32
	if (s==INVALID_SOCKET) return false;
#else
	if (s==-1) return false;
#endif

	address2 = gethostbyname(server);
	if (address2 == NULL) return false;

	memset((char *) &address, 0, sizeof(address));
	address.sin_family 	= AF_INET;
	address.sin_port 	= htons(80);
	address.sin_addr.s_addr = *(u_long *) *(address2->h_addr_list);

	if (connect(s,(struct sockaddr *)&address,sizeof(address))<0) return false;

	sprintf(buff,"GET /%s HTTP/1.1\x0d\x0aHost: %s\x0d\x0aUser-Agent: Gammu/%s\x0d\x0a\x0d\x0a", filename, server, VERSION);
	if (send(s,buff,strlen(buff),0)<0) return false;

	free(file->Buffer);
	file->Buffer 	= NULL;
	file->Used 	= 0;

#ifdef WIN32
	while ((len=recv(s,buff,200,0))>0) {
#else
	while ((len=read(s,buff,200))>0) {
#endif
		file->Buffer = realloc(file->Buffer,file->Used + len);
		memcpy(file->Buffer+file->Used,buff,len);
		file->Used += len;
	}
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif

	if (file->Buffer == NULL) return false;
	if (strstr(file->Buffer,"HTTP/1.1 200 OK")==NULL) {
		free(file->Buffer);
		file->Buffer = NULL;
		file->Used   = 0;
		return false;
	}

	return true;
}

void GSM_ClearBatteryCharge(GSM_BatteryCharge *bat)
{
    bat->BatteryPercent = -1;
    bat->ChargeState = 0;
    bat->BatteryType = 0;
    bat->BatteryVoltage = -1;
    bat->ChargeVoltage = -1;
    bat->ChargeCurrent = -1;
    bat->PhoneCurrent = -1;
    bat->BatteryTemperature = -1;
    bat->PhoneTemperature = -1;
    bat->BatteryCapacity = -1;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
