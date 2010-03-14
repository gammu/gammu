
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

GSM_Error GSM_JavaFindData(GSM_File File, char *Vendor, char *Name, char *JAR)
{
	int 	i;
	bool 	found;

	found = false;
	Vendor[0] = 0;
	for (i=0;i<File.Used - 14;i++) {
		if (!strncmp(File.Buffer+i,"MIDlet-Vendor: ",14)) {
			found = true;
			break;
		}
	}
	if (!found) return GE_UNKNOWN;
	i = i + 15;
	while (File.Buffer[i] != 13 && File.Buffer[i] != 10) {
		Vendor[strlen(Vendor)+1] = 0;
		Vendor[strlen(Vendor)]   = File.Buffer[i];
		i++;
	}

	found = false;
	for (i=0;i<File.Used - 13;i++) {
		if (!strncmp(File.Buffer+i,"MIDlet-Name: ",13)) {
			found = true;
			break;
		}
	}
	if (!found) return GE_UNKNOWN;
	i = i + 13;
	Name[0] = 0;
	while (File.Buffer[i] != 13 && File.Buffer[i] != 10) {
		Name[strlen(Name)+1] = 0;
		Name[strlen(Name)]   = File.Buffer[i];
		i++;
	}

	found = false;
	for (i=0;i<File.Used - 16;i++) {
		if (!strncmp(File.Buffer+i,"MIDlet-Jar-URL: ",16)) {
			found = true;
			break;
		}
	}
	if (!found) return GE_UNKNOWN;
	i = i + 16;
	JAR[0] = 0;
	while (File.Buffer[i] != 13 && File.Buffer[i] != 10) {
		JAR[strlen(JAR)+1] = 0;
		JAR[strlen(JAR)]   = File.Buffer[i];
		i++;
	}

	return GE_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
