/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef WIN32
#  include <io.h>
#  include <fcntl.h>
#else
#  include <netdb.h>
#  include <netinet/in.h>
#endif

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
	File->Buffer = realloc(File->Buffer,File->Used);
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
		MyGetLine(File.Buffer, &Pos, Line, File.Used);
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
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02d%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,13,10);
}

bool ReadVCALDateTime(char *Buffer, GSM_DateTime *dt)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	memset(dt,0,sizeof(dt));

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
	}

	/* FIXME */
	dt->Timezone	= 0;

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
	return true;
}

void SaveVCALText(char *Buffer, int *Length, char *Text, char *Start)
{
	char buffer[1000];

	if (UnicodeLength(Text) != 0) {
		EncodeUTF8QuotedPrintable(buffer,Text);
		if (UnicodeLength(Text)==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "%s:%s%c%c",Start,DecodeUnicodeString(Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",Start,buffer,13,10);
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

bool ReadVCALText(char *Buffer, char *Start, char *Value)
{
	unsigned char buff[200];

	Value[0] = 0x00;
	Value[1] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		EncodeUnicode(Value,Buffer+strlen(Start)+1,strlen(Buffer)-(strlen(Start)+1));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	/* SE T68i */
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+27,strlen(Buffer)-(strlen(Start)+27));
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

	sprintf(buff,"GET http://%s/%s HTTP/1.0\n\n",server,filename);
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

	close(s);
	
	if (file->Buffer == NULL) return false;
	if (strstr(file->Buffer,"HTTP/1.1 200 OK")==NULL) {
		free(file->Buffer);
		file->Buffer = NULL;
		file->Used   = 0;
		return false;
	}

	return true;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
