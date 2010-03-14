
#include <string.h>
#include <stdlib.h>

#include "gsmmisc.h"
#include "../gsmcomon.h"

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
			return GE_NOTSUPPORTED;
		}
	}
	*Length = i;
	return GE_NONE;
}

GSM_Error GSM_ReadFile(char *FileName, GSM_File *File)
{
	int 		i = 1000;
	FILE		*file;

	file = fopen(FileName,"rb");
	if (!file) return(GE_CANTOPENFILE);

	free(File->Buffer);
	File->Buffer 	= NULL;
	File->Used 	= 0;
	while (i == 1000) {
		File->Buffer = realloc(File->Buffer,File->Used + 1000);
		i = fread(File->Buffer+File->Used,1,1000,file);
		File->Used = File->Used + i;
	}
	File->Buffer = realloc(File->Buffer,File->Used);

	fclose(file);
	return GE_NONE;
}

static void GSM_JADFindLine(GSM_File File, char *Name, char *Value)
{
	int 	i;
	bool 	found;

	Value[0] = 0;

	found = false;
	for (i=0;i<File.Used - (int)strlen(Name);i++) {
		if (!strncmp(File.Buffer+i,Name,strlen(Name))) {
			found = true;
			break;
		}
	}
	if (!found) return;
	i = i + strlen(Name);
	while (File.Buffer[i] != 13 && File.Buffer[i] != 10 && i < File.Used) {
		if (strlen(Value) == 0 && File.Buffer[i] == 0x20) {
			i++;
			continue;
		}
		Value[strlen(Value)+1] = 0;
		Value[strlen(Value)]   = File.Buffer[i];
		i++;
	}
}

GSM_Error GSM_JADFindData(GSM_File File, char *Vendor, char *Name, char *JAR, char *Version, int *Size)
{
	char Size2[200];

	GSM_JADFindLine(File, "MIDlet-Vendor:", Vendor);
	if (Vendor[0] == 0x00) return GE_FILENOTSUPPORTED;
	dprintf("Vendor: \"%s\"\n",Vendor);

	GSM_JADFindLine(File, "MIDlet-Name:", Name);
	if (Name[0] == 0x00) return GE_FILENOTSUPPORTED;
	dprintf("Name: \"%s\"\n",Name);

	GSM_JADFindLine(File, "MIDlet-Jar-URL:", JAR);
	if (JAR[0] == 0x00) return GE_FILENOTSUPPORTED;
	dprintf("JAR file URL: \"%s\"\n",JAR);

	GSM_JADFindLine(File, "MIDlet-Jar-Size:", Size2);
	*Size = -1;
	if (Size2[0] == 0x00) return GE_FILENOTSUPPORTED;
	dprintf("JAR size: \"%s\"\n",Size2);
	(*Size) = atoi(Size2);

	GSM_JADFindLine(File, "MIDlet-Version:", Version);
	dprintf("Version: \"%s\"\n",Version);

	return GE_NONE;
}

void GSM_IdentifyFileFormat(GSM_File *File)
{
	File->Type = GSM_File_Other;
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
		File->Type = GSM_File_Ringtone_MIDI;
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
