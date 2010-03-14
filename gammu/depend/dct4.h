
void DCT4SetPhoneMenus	   (int argc, char *argv[]);
void DCT4tests		   (int argc, char *argv[]);
void DCT4SetVibraLevel	   (int argc, char *argv[]);
void DCT4ResetSecurityCode (int argc, char *argv[]);
void DCT4GetVoiceRecord	   (int argc, char *argv[]);
void DCT4Info		   (int argc, char *argv[]);
void DCT4GetFileSystem	   (int argc, char *argv[]);
void DCT4GetFiles	   (int argc, char *argv[]);

typedef struct {
	bool 		Folder;
	int		Free;
	int		Used;
	unsigned char 	Name[300];
	int		ID;
	int		Level;
	int		Locations[100];
} DCT4_FileFolderInfo;

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
