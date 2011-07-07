#ifndef _FileSystem_
#define _FileSystem_

#define  MOBILE_MAX_PATH   1000
static unsigned char szFileString[MOBILE_MAX_PATH];
static unsigned char *szFileFolderPath;

static int AllFilesNum;
static int NewFilesListIndex;
static int FilesList[1000];
static int Potolevel;

// Structure used for saving date and time 
typedef struct {
	int					Timezone;	//The difference between local time and GMT in hours
	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;
	unsigned int 		Day;	//January = 1, February = 2, etc.
	unsigned int 		Month;
	unsigned int		Year;	//Complete year number. Not 03, but 2003
} anw_DateTime;


typedef struct {
	int		Used;		/* how many bytes used 	*/
	unsigned char 	Name[300];	/* Name			*/
	int				index;
	anw_DateTime	Modified;
} AnwMobile_FileInfo;

struct FolderInfo {
	TCHAR	*model;
	TCHAR 	*parameter;
	TCHAR	*folder;
	TCHAR 	*Location;
};


typedef struct
{
	int				Location;
	unsigned char	FolderName[30];
}FileSystemFolder;

typedef struct
{
	int					Number;
	FileSystemFolder	Folder[10];
}MultimediaFolder;

// add new model step , mingfa
// You can enable debug log and run ringtoneManager to get the followig value of Mobile 
static struct FolderInfo FileFolder[] = {
	/* Language indepedent in DCT4 */
	{_T(""),	 _T("MMSUnreadInbox"),	_T("INBOX"),	_T("3")},
	{_T(""),	 _T("MMSReadInbox"),	_T("INBOX"),	_T("13")},
	{_T(""),	 _T("MMSOutbox"),		_T("OUTBOX"),	_T("14")},
	{_T(""),	 _T("MMSSent"),			_T("SENT"),		_T("15")},
	{_T(""),	 _T("MMSDrafts"),		_T("DRAFTS"),	_T("16")},
	{_T(""),	 _T("Application"),		_T("applications"),	_T("3")},
	{_T(""),	 _T("Game"),		_T("games"),	_T("3")},
	/* Language depedent in DCT4 */
	{_T(""),	 _T("Gallery"),		_T("Pictures"),	_T("2")}, /* 3510  */
	{_T(""),	 _T("Gallery"),		_T("Graphics"),	_T("3")}, /* 3510i */
	{_T(""),	 _T("Gallery"),		_T("Images"),	_T("3")}, /* 6610  */
	{_T("3510"), _T("Gallery"),		_T(""),		_T("8")},
	{_T("3510i"),_T("Gallery"),		_T(""),		_T("3")},
	{_T("3220"),_T("Gallery"),		_T(""),		_T("5")},
	{_T("5100"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("5140"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("6220"), _T("Gallery"),		_T(""),		_T("5")},
	{_T("6100"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("6800"), _T("Gallery"),		_T(""),		_T("3")},//add new model step , v1.0.2.4
	{_T("6810"), _T("Gallery"),		_T(""),		_T("4")},//Compare; add new model step , mingfa 
	{_T("6820"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("7200"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("7210"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("7250"), _T("Gallery"),		_T(""),		_T("3")},
	{_T("7250i"), _T("Gallery"),		_T(""),		_T("3")},
	{_T(""),	 _T("Tones"),		_T("Tones"),	_T("3")},
	{_T("3510i"),_T("Tones"),		_T(""),		_T("4")},
	{_T("5100"), _T("Tones"),		_T(""),		_T("4")},
	{_T("5140"), _T("Tones"),		_T(""),		_T("7")},
	{_T("6220"), _T("Tones"),		_T(""),		_T("6")},
	{_T("6100"), _T("Tones"),		_T(""),		_T("4")},
	{_T("7210"), _T("Tones"),		_T(""),		_T("4")},
	{_T("6610"), _T("Tones"),		_T(""),		_T("4")},
	{_T("6610i"), _T("Tones"),		_T(""),		_T("5")},
	{_T("6800"), _T("Tones"),		_T(""),		_T("4")},// add new model step , v1.0.2.4
	{_T("6810"), _T("Tones"),		_T(""),		_T("5")},// add new model step , mingfa
	{_T("6820"), _T("Tones"),		_T(""),		_T("6")},
	{_T("7200"), _T("Tones"),		_T(""),		_T("8")},
	{_T("7250"), _T("Tones"),		_T(""),		_T("5")},
	{_T("7250i"), _T("Tones"),		_T(""),		_T("5")},
	{_T(""),	 _T("MMS"),			_T("MMS"),		_T("16")},
	/* Language indepedent in OBEX */
	{_T("obex"), _T("MMSUnreadInbox"),	_T(""),		_T("predefMessages\\predefINBOX")	},
	{_T("obex"), _T("MMSReadInbox"),	_T(""),		_T("predefMessages\\predefINBOX")	},
	{_T("obex"), _T("MMSOutbox"),		_T(""),		_T("predefMessages\\predefOUTBOX")	},
	{_T("obex"), _T("MMSSent"),		_T(""),		_T("predefMessages\\predefSENT")	}, 	
	{_T("obex"), _T("MMSDrafts"),		_T(""),		_T("predefMessages\\predefDRAFTS")	},
//	{_T("obex"), "Application,		_T(""),		"predefjava\\predefapplications"},
//	{_T("obex"), "Game",		_T(""),		"predefjava\\predefgames"	},
	{_T("obex"), _T("Gallery"),		_T(""),		_T("predefgallery\\predefgraphics")	},
	{_T("obex"), _T("Tones"),		_T(""),		_T("predefgallery\\predeftones")	},

	/* End of list */
	{_T(""),	 _T(""),			_T(""),		_T("")}
};

// Functions
int WINAPI GetWorkFolder(void);
void WINAPI SetWorkFolder(int folder);
int WINAPI GetMultimediaFolder(MultimediaFolder * mFolder);
int WINAPI GetRootFolder(MultimediaFolder * mFolder);
int WINAPI GetFolder(int Folder , MultimediaFolder * mFolder);

typedef int (WINAPI* GetWorkFolderProc)(void);
typedef void (WINAPI* SetWorkFolderProc)(int folder);
typedef int (WINAPI* GetMultimediaFolderProc)(MultimediaFolder * mFolder);
typedef int (WINAPI* GetRootFolderProc)(MultimediaFolder * mFolder);
typedef int (WINAPI* GetFolderProc)(int Folder , MultimediaFolder * mFolder);

extern GetWorkFolderProc				GetWorkFolderfn;
extern SetWorkFolderProc				SetWorkFolderfn;
extern GetMultimediaFolderProc			GetMultimediaFolderfn;
extern GetRootFolderProc			    GetRootFolderfn;
extern GetFolderProc					GetFolderfn;

#endif