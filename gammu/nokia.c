#define _GNU_SOURCE /* For strcasestr */
#include <string.h>

#include "../helper/locales.h"

#include <gammu.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef GSM_ENABLE_NOKIA_DCT3
#  include "depend/nokia/dct3.h"
#  include "depend/nokia/dct3trac/wmx.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
#  include "depend/nokia/dct4.h"
#endif

#include "nokia.h"
#include "common.h"
#include "files.h"

#include "../helper/formats.h"
#include "../helper/printing.h"
#include "../helper/string.h"

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
void NokiaComposer(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Ringtone 		ringtone;
	gboolean			started;
	int 			i,j;
	GSM_RingNote 		*Note;
	GSM_RingNoteDuration 	Duration;
	GSM_RingNoteDuration 	DefNoteDuration = 32; /* 32 = Duration_1_4 */
	unsigned int		DefNoteScale 	= Scale_880;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);

	if (ringtone.Format != RING_NOTETONE) {
		printf("%s\n", _("It can be RTTL ringtone only used with this option"));
		Terminate(3);
	}

	started = FALSE;
	j	= 0;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) {
					printf(_("Ringtone \"%s\" (tempo = %i Beats Per Minute)"),DecodeUnicodeConsole(ringtone.Name),GSM_RTTLGetTempo(Note->Tempo));
					printf("\n\n");
					started = TRUE;
				}
			}
			if (started) j++;
		}
	}
    	if (j>50) {
		printf_warn(_("length=%i notes, but you will enter only first 50 tones."), j);
	}

	printf("\n\n%s ", _("This ringtone in Nokia Composer in phone should look:"));
	started = FALSE;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) started = TRUE;
			}
			if (started) {
				switch (Note->Duration) {
					case Duration_Full: printf("1"); break;
					case Duration_1_2 : printf("2"); break;
					case Duration_1_4 : printf("4"); break;
					case Duration_1_8 : printf("8"); break;
					case Duration_1_16: printf("16");break;
					case Duration_1_32: printf("32");break;
				}
				if (Note->DurationSpec == DottedNote) printf(".");
				switch (Note->Note) {
					case Note_C  	: printf("c");	break;
					case Note_Cis	: printf("#c");	break;
					case Note_D  	 :printf("d");	break;
					case Note_Dis	: printf("#d");	break;
					case Note_E  	: printf("e");	break;
					case Note_F  	: printf("f");	break;
					case Note_Fis	: printf("#f");	break;
					case Note_G  	: printf("g");	break;
					case Note_Gis	: printf("#g");	break;
					case Note_A  	: printf("a");	break;
					case Note_Ais	: printf("#a");	break;
					case Note_H  	: printf("h");	break;
					case Note_Pause : printf("-");	break;
				}
				if (Note->Note != Note_Pause) printf("%i",Note->Scale - 4);
				printf(" ");
			}
		}
	}

	printf("\n\n%s ", _("To enter it please press:"));
	started = FALSE;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) started = TRUE;
			}
			if (started) {
				switch (Note->Note) {
	      				case Note_C  : case Note_Cis:	printf("1");break;
	      				case Note_D  : case Note_Dis:	printf("2");break;
	      				case Note_E  :			printf("3");break;
	      				case Note_F  : case Note_Fis:	printf("4");break;
	      				case Note_G  : case Note_Gis:	printf("5");break;
	      				case Note_A  : case Note_Ais:	printf("6");break;
	      				case Note_H  :			printf("7");break;
	      				default      :			printf("0");break;
				}
				if (Note->DurationSpec == DottedNote) printf(_("(longer)"));
	    			switch (Note->Note) {
      					case Note_Cis: case Note_Dis:
      					case Note_Fis: case Note_Gis:
      					case Note_Ais:
						printf("#");
						break;
      					default      :
						break;
    				}
				if (Note->Note != Note_Pause) {
					if ((unsigned int)Note->Scale != DefNoteScale) {
						while (DefNoteScale != (unsigned int)Note->Scale) {
							printf("*");
							DefNoteScale++;
							if (DefNoteScale==Scale_7040) DefNoteScale = Scale_880;
						}
					}
				}
				Duration = 0;
				switch (Note->Duration) {
					case Duration_Full : Duration = 128;	break;
					case Duration_1_2  : Duration = 64;	break;
					case Duration_1_4  : Duration = 32;	break;
					case Duration_1_8  : Duration = 16;	break;
					case Duration_1_16 : Duration = 8;	break;
					case Duration_1_32 : Duration = 4;	break;
					default		   : fprintf(stderr, "error\n");break;
				}
				if (Duration > DefNoteDuration) {
		        		while (DefNoteDuration != Duration) {
						printf("9");
			  			DefNoteDuration = DefNoteDuration * 2;
					}
			      	}
				if (Duration < DefNoteDuration) {
		        		while (DefNoteDuration != Duration) {
						printf("8");
			  			DefNoteDuration = DefNoteDuration / 2;
					}
			      	}
				printf(" ");
			}
		}
	}

	printf("\n");
	fflush(stdout);
}

void NokiaSecurityCode(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3GetSecurityCode(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
/* 	DCT4ResetSecurityCode(argc, argv); */
	DCT4GetSecurityCode(argc,argv);
#endif

	GSM_Terminate();
}

void NokiaSetPhoneMenus(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3SetPhoneMenus (argc, argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4SetPhoneMenus (argc, argv);
#endif

	GSM_Terminate();
}

void NokiaSelfTests(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3SelfTests(argc, argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4SelfTests(argc, argv);
#endif

	GSM_Terminate();
}

typedef struct _PlayListEntry PlayListEntry;

struct _PlayListEntry {
	unsigned char		*Name;
	unsigned char		*NameUP;
	PlayListEntry		*Next;
};

void NokiaAddPlayLists2(unsigned char *ID,unsigned char *Name,unsigned char *IDFolder)
{
	GSM_Error error;
	gboolean 			Start = TRUE, Available = FALSE;
	GSM_File	 	Files,Files2,Files3;
	int 			j,NamesPos2=0;
	size_t i, NamesPos = 0;
	unsigned char		Buffer[20],Buffer2[500];
	unsigned char		*Names,*Names2,*Pointer;
	PlayListEntry		*First,*Entry=NULL,*Prev;

	First = NULL; Names=NULL; Names2=NULL;

	CopyUnicodeString(Files.ID_FullName,ID);

	printf(_("Checking %s\n"),DecodeUnicodeString(Name));
	/* looking into folder content (searching for mp3 and similiar) */
	while (1) {
		error = GSM_GetFolderListing(gsm,&Files,Start);
		if (error == ERR_FOLDERPART) {
			printf("%s\n", _("  Only part handled!"));
			break;
		}
		if (error == ERR_EMPTY) break;
		if (error == ERR_FILENOTEXIST) return;
	    	Print_Error(error);

		if (!Files.Folder) {
			if (strcasestr(DecodeUnicodeConsole(Files.Name),".mp3")!=NULL ||
			    strcasestr(DecodeUnicodeConsole(Files.Name),".aac")!=NULL) {
				if (First==NULL) {
					First = malloc(sizeof(PlayListEntry));
					Entry = First;
				} else {
					Entry->Next = malloc(sizeof(PlayListEntry));
					Entry = Entry->Next;
				}
				Entry->Next = NULL;
				Entry->Name = malloc(strlen(DecodeUnicodeString(Files.ID_FullName))+1);
				sprintf(Entry->Name,"%s",DecodeUnicodeString(Files.ID_FullName));
				/* converting Gammu drives to phone drives */
				if (Entry->Name[0]=='a' || Entry->Name[0]=='A') {
					Entry->Name[0]='b';
				} else if (Entry->Name[0]=='d' || Entry->Name[0]=='D') {
					Entry->Name[0]='a';
				}

				Entry->NameUP = malloc(strlen(DecodeUnicodeString(Files.ID_FullName))+1);
				for (i = 0; i < strlen(DecodeUnicodeString(Files.ID_FullName)) + 1; i++) {
					Entry->NameUP[i] = tolower(Entry->Name[i]);
				}
			}
		} else {
			Names = (unsigned char *)realloc(Names,NamesPos+UnicodeLength(Files.ID_FullName)*2+2);
			CopyUnicodeString(Names+NamesPos,Files.ID_FullName);
			NamesPos+=UnicodeLength(Files.ID_FullName)*2+2;

			Names2 = (unsigned char *)realloc(Names2,NamesPos2+UnicodeLength(Files.Name)*2+2);
			CopyUnicodeString(Names2+NamesPos2,Files.Name);
			NamesPos2+=UnicodeLength(Files.Name)*2+2;
		}

		Start = FALSE;
	}
	if (First!=NULL) {
		/* sorting songs names */
		Entry=First;
		while (Entry->Next!=NULL) {
			if (strcmp(Entry->NameUP,Entry->Next->NameUP)>0) {
				Pointer=Entry->Next->Name;
				Entry->Next->Name = Entry->Name;
				Entry->Name = Pointer;

				Pointer=Entry->Next->NameUP;
				Entry->Next->NameUP = Entry->NameUP;
				Entry->NameUP = Pointer;

				Entry=First;
				continue;
			}
			Entry=Entry->Next;
		}
		/* we checking, if file already exist.if yes, we look for another... */
		i 		= 0;
		Files3.Buffer 	= NULL;
		while (1) {
			CopyUnicodeString(Files3.ID_FullName,IDFolder);
	        	CopyUnicodeString(Buffer2,Name);
			if (i!=0) {
				sprintf(Buffer, "%ld", (long)i);
		        	EncodeUnicode(Buffer2+UnicodeLength(Buffer2)*2,Buffer,strlen(Buffer));
			}
	        	EncodeUnicode(Buffer2+UnicodeLength(Buffer2)*2,".m3u",4);

			Start = TRUE;
			Available = FALSE;
			while (1) {
				error = GSM_GetFolderListing(gsm,&Files3,Start);
				if (error == ERR_FOLDERPART) {
					printf("%s\n", _("  Problem with adding playlist"));
					break;
				}
				if (error == ERR_EMPTY) break;
			    	Print_Error(error);

				if (!Files3.Folder) {
					if (mywstrncasecmp(Buffer2,Files3.Name,-1)) {
						Available = TRUE;
						break;
					}
				}
				Start = FALSE;
			}
			if (!Available) break;
			i++;
		}

		/* preparing new playlist file date */
		Files2.System	 = FALSE;
		Files2.Folder 	 = FALSE;
		Files2.ReadOnly	 = FALSE;
		Files2.Hidden	 = FALSE;
		Files2.Protected = FALSE;
		Files2.ModifiedEmpty = FALSE;
		GSM_GetCurrentDateTime (&Files2.Modified);
		CopyUnicodeString(Files2.ID_FullName,IDFolder);
	        CopyUnicodeString(Files2.Name,Buffer2);

		/* preparing new playlist file content */
		Files2.Buffer=NULL;
		Files2.Buffer = (unsigned char *)realloc(Files2.Buffer,10);
		sprintf(Files2.Buffer,"#EXTM3U%c%c",13,10);
		Files2.Used = 9;
		Entry=First;
		while (Entry!=NULL) {
			Files2.Buffer = (unsigned char *)realloc(Files2.Buffer,Files2.Used+strlen(Entry->Name)+2+1);
			sprintf(Files2.Buffer+Files2.Used,"%s%c%c",Entry->Name,13,10);
			Files2.Used+=strlen(Entry->Name)+2;
			Entry=Entry->Next;
		}
		Files2.Used	 -= 2;
		for (i=0;i<Files2.Used;i++) {
			if (Files2.Buffer[i]=='/') Files2.Buffer[i]='\\';
		}

		/* adding new playlist file */
		sprintf(Buffer2, _("Writing file %s:"), DecodeUnicodeString(Files2.Name));
		AddOneFile(&Files2, Buffer2, FALSE);

		/* cleaning buffers */
		free(Files2.Buffer);
		Files2.Buffer=NULL;
		while (Entry!=NULL) {
			Entry=First;
			Prev=NULL;
			while (Entry->Next!=NULL) {
				Prev=Entry;
				Entry=Entry->Next;
			}
			free(Entry->Name);
			free(Entry->NameUP);
			free(Entry);
			Entry=NULL;
			if (Prev!=NULL) Prev->Next = NULL;
		}
	}

	/* going into subfolders */
	if (NamesPos != 0) {
		i = 0; j = 0;
		while (i != NamesPos) {
			NokiaAddPlayLists2(Names+i,Names2+j,IDFolder);
			i+=UnicodeLength(Names+i)*2+2;
			j+=UnicodeLength(Names2+j)*2+2;
		}
	}
	free(Names);
	free(Names2);
}

void NokiaAddPlayLists(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	gboolean 			Start = TRUE;
	GSM_File	 	Files;
	unsigned char		buffer[20],buffer2[20],IDFolder[100];

	GSM_Init(TRUE);

	/* delete old playlists */
	EncodeUnicode(IDFolder,"d:\\predefplaylist",17);
	CopyUnicodeString(Files.ID_FullName,IDFolder);
	error = GSM_GetFolderListing(gsm,&Files,Start);
	if (error == ERR_FILENOTEXIST) {
		EncodeUnicode(IDFolder,"d:\\predefgallery\\predefplaylist",17+14);
		CopyUnicodeString(Files.ID_FullName,IDFolder);
		error = GSM_GetFolderListing(gsm,&Files,Start);
	} else if (error != ERR_EMPTY) {
	    	Print_Error(error);
	}
	if (error == ERR_FILENOTEXIST) {
		printf("%s\n", _("Your phone model is not supported. Please report"));
		GSM_Terminate();
		Terminate(3);
	} else if (error != ERR_EMPTY) {
	    	Print_Error(error);
	}
	while (1) {
		if (!Files.Folder) {
			if (strstr(DecodeUnicodeConsole(Files.Name),".m3u")!=NULL) {
				error = GSM_DeleteFile(gsm,Files.ID_FullName);
			    	Print_Error(error);
			}
		}
		Start = FALSE;
		error = GSM_GetFolderListing(gsm,&Files,Start);
		if (error == ERR_FOLDERPART) {
			printf("%s\n", _("Problem with deleting playlist"));
			break;
		}
		if (error == ERR_EMPTY) break;
	    	Print_Error(error);
	}

	/* go over phone memory and add new one playlists */
	EncodeUnicode(buffer,"d:",2);
	EncodeUnicode(buffer2,"root",4);
	NokiaAddPlayLists2(buffer,buffer2,IDFolder);
	/* go over memory card and add new one playlists */
	EncodeUnicode(buffer,"a:",2);
	EncodeUnicode(buffer2,"root",4);
	NokiaAddPlayLists2(buffer,buffer2,IDFolder);

	GSM_Terminate();
}

struct NokiaFolderInfo {
	const char	*model;
	const char 	*parameter;
	const char	*folder;
	const char 	*level;
};

struct NokiaFolderInfo Folder[] = {
	/* Language indepedent in DCT4 in filesystem 1 */
	{"",	 "Application",	   "applications",	"3"},
	{"",	 "Game",	   "games",		"3"},
	/* Language indepedent in DCT4/TIKU/BB5 in filesystem 2 */
	{"", 	 "Gallery",	   "d:/predefgallery/predefgraphics",			""},
	{"", 	 "Gallery2",	   "d:/predefgallery/predefgraphics/predefcliparts",	""},
	{"", 	 "Camera",	   "d:/predefgallery/predefphotos",			""},
	{"", 	 "Tones",	   "d:/predefgallery/predeftones",			""},
	{"", 	 "Tones2",	   "d:/predefgallery/predefmusic",			""},
	{"", 	 "Records",	   "d:/predefgallery/predefrecordings",			""},
	{"", 	 "Video",	   "d:/predefgallery/predefvideos",			""},
	{"", 	 "Playlist",	   "d:/predefplaylist",					""},
	{"", 	 "MemoryCard",	   "a:",						""},
	    /* now values first seen in S40 3.0 */
	{"",	 "Application",	   "d:/predefjava/predefcollections",			""},
	{"",	 "Game",	   "d:/predefjava/predefgames",				""},

	/* Language depedent in DCT4 filesystem 1 */
	{"",	 "Gallery",	   "Clip-arts",					"3"},
	{"",	 "Gallery",	   "004F006200720061007A006B0069",		"3"},/* obrazki PL 6220 */
	{"",	 "Gallery",	   "Pictures",					"2"},/* 3510 */
	{"",	 "Gallery2",	   "Graphics",					"3"},
	{"",	 "Gallery2",	   "00470072006100660069006B0061",		"3"},/* grafika PL 6220 */
	{"",	 "Camera",	   "Images",					"3"},
	{"",	 "Camera",	   "005A0064006A0119006300690061",		"3"},/* zdjecia PL 6220 */
	{"",	 "Tones",	   "Tones",					"3"},
	{"",	 "Tones",	   "0044017A007700690119006B0069",		"3"},/* dzwieki pl 6220 */
	{"",	 "Records",	   "Recordings",				"3"},
	{"",	 "Records",	   "004E0061006700720061006E00690061",		"3"},/* nagrania pl 6220 */
	{"",	 "Video",	   "Video clips",				"3"},
	{"",	 "Video",	   "0057006900640065006F006B006C006900700079",	"3"},/* wideoklipy pl 6220 */

	/* Language indepedent in OBEX */
	{"obex", "MMSUnreadInbox", "predefMessages\\predefINBOX", 		""},
	{"obex", "MMSReadInbox",   "predefMessages\\predefINBOX", 		""},
	{"obex", "MMSOutbox",	   "predefMessages\\predefOUTBOX", 		""},
	{"obex", "MMSSent",	   "predefMessages\\predefSENT", 		""},
	{"obex", "MMSDrafts",	   "predefMessages\\predefDRAFTS", 		""},
/* 	{"obex", "Application,	   "predefjava\\predefapplications", 		""}, */
/* 	{"obex", "Game",	   "predefjava\\predefgames", 			""}, */
	{"obex", "Gallery",	   "predefgallery\\predefgraphics", 		""},
	{"obex", "Tones",	   "predefgallery\\predeftones", 		""},

	/* End of list */
	{"",	 "",		   "",		""}
};

void NokiaAddFile(int argc, char *argv[])
{
	GSM_Error error;
	GSM_File		File, Files, File2;
	FILE			*file;
	unsigned char 		buffer[10000],JAR[500],Vendor[500],Name[500],Version[500],FileID[400];
	gboolean 			Start = TRUE, Found = FALSE, wasclr;
	gboolean			ModEmpty = FALSE, Overwrite = FALSE, OverwriteAll = FALSE;
	size_t i = 0, Pos;
	int Size, Size2;
	int nextlong, j;

	while (Folder[i].parameter[0] != 0) {
		if (strcasecmp(argv[2],Folder[i].parameter) == 0) {
			Found = TRUE;
			break;
		}
		i++;
	}
	if (!Found) {
		printf(_("What folder type (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	if (strcasecmp(argv[2],"Application") == 0 || strcasecmp(argv[2],"Game") == 0) {
		sprintf(buffer,"%s.jad",argv[3]);
		file = fopen(buffer,"rb");
		if (file == NULL) Print_Error(ERR_CANTOPENFILE);
		fclose(file);
		sprintf(buffer,"%s.jar",argv[3]);
		file = fopen(buffer,"rb");
		if (file == NULL) Print_Error(ERR_CANTOPENFILE);
		fclose(file);
	} else {
		file = fopen(argv[3],"rb");
		if (file == NULL) Print_Error(ERR_CANTOPENFILE);
		fclose(file);
	}

	GSM_Init(TRUE);

	Found = FALSE;
    	if (GSM_GetUsedConnection(gsm) == GCT_IRDAOBEX || GSM_GetUsedConnection(gsm) == GCT_BLUEOBEX) {
		i = 0;
		while (Folder[i].parameter[0] != 0) {
			if (!strcmp("obex",Folder[i].model) &&
			     strcasecmp(argv[2],Folder[i].parameter) == 0) {
				EncodeUnicode(Files.ID_FullName,Folder[i].folder,strlen(Folder[i].folder));
				Found = TRUE;
				break;
			}
			i++;
		}
		if (!Found) {
			printf("%s\n", _("Folder not found. Probably function not supported!"));
			GSM_Terminate();
			Terminate(3);
		}
	} else if (GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(gsm), F_FILES2)) {
		i = 0;
		while (Folder[i].parameter[0] != 0) {
			if ((Folder[i].folder[0] == 'a' || Folder[i].folder[0] == 'd') &&
			    Folder[i].level[0] == 0x00 &&
			    strcasecmp(argv[2],Folder[i].parameter) == 0) {
				if (strstr(Folder[i].folder,"d:/predefjava/")!= NULL &&
				    !GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(gsm), F_SERIES40_30)) {
					i++;
					continue;
				}
				EncodeUnicode(Files.ID_FullName,Folder[i].folder,strlen(Folder[i].folder));
				Found = TRUE;
				break;
			}
			i++;
		}
	}
	if (!Found) {
		fprintf(stderr, _("Searching for phone folder: "));
		while (1) {
			error = GSM_GetNextFileFolder(gsm,&Files,Start);
			if (error == ERR_EMPTY) break;
		    	Print_Error(error);

			if (Files.Folder) {
				smprintf(gsm, "folder %s level %i\n",DecodeUnicodeConsole(Files.Name),Files.Level);
				Found 	= FALSE;
				i 	= 0;
				while (Folder[i].parameter[0] != 0) {
					EncodeUnicode(buffer,Folder[i].folder,strlen(Folder[i].folder));
					smprintf(gsm, "comparing \"%s\" \"%s\" \"%s\"\n",GSM_GetModelInfo(gsm)->model,DecodeUnicodeString(Files.ID_FullName),Folder[i].level);
					if (strcasecmp(argv[2],Folder[i].parameter) == 0  &&
					    mywstrncasecmp(Files.Name,buffer,0) &&
					    Files.Level == atoi(Folder[i].level)) {
						smprintf(gsm, "found 1\n");
						Found = TRUE;
						break;
					}
					if (strcasecmp(argv[2],Folder[i].parameter) == 0 &&
					    !strcmp(DecodeUnicodeString(Files.ID_FullName),Folder[i].folder) &&
					    Folder[i].level[0] == 0x00) {
						Found = TRUE;
						smprintf(gsm, "found 2\n");
						break;
					}
					if (Folder[i].folder[0]>='0'&&Folder[i].folder[0] <='9') {
						DecodeHexUnicode (buffer, Folder[i].folder,strlen(Folder[i].folder));
						smprintf(gsm, "comparing \"%s\"",DecodeUnicodeString(buffer));
						smprintf(gsm, "and \"%s\"\n",DecodeUnicodeString(Files.Name));
						if (strcasecmp(argv[2],Folder[i].parameter) == 0  &&
						    mywstrncasecmp(Files.Name,buffer,0) &&
						    Files.Level == atoi(Folder[i].level)) {
							Found = TRUE;
							smprintf(gsm, "found 3\n");
							break;
						}
					}
					i++;
				}
				if (Found) break;
			}
			fprintf(stderr, "*");

			Start = FALSE;
		}
		fprintf(stderr, "\n");
		fflush(stderr);
	}
	if (!Found) {
		printf("%s\n", _("Folder not found. Probably function not supported!"));
		GSM_Terminate();
		Terminate(3);
	}
	File.Buffer 	= NULL;
	File.Protected  = FALSE;
	File.ReadOnly   = FALSE;
	File.Hidden	= FALSE;
	File.System	= FALSE;

	if (strcasecmp(argv[2],"Application") == 0 || strcasecmp(argv[2],"Game") == 0) {
		/* reading jar file */
		sprintf(buffer,"%s.jar",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);
		Size2 = File.Used;

		/* reading jad file */
		sprintf(buffer,"%s.jad",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);

		/* Getting values from JAD file */
		error = GSM_JADFindData(File, Vendor, Name, JAR, Version, &Size);
		if (error == ERR_FILENOTSUPPORTED) {
			if (Vendor[0] == 0x00) {
				fprintf(stderr, "%s\n", _("No vendor info in JAD file"));
				GSM_Terminate();
				return;
			}
			if (Name[0] == 0x00) {
				fprintf(stderr, "%s\n", _("No name info in JAD file"));
				GSM_Terminate();
				return;
			}
			if (JAR[0] == 0x00) {
				fprintf(stderr, "%s\n", _("No JAR URL info in JAD file"));
				GSM_Terminate();
				return;
			}
		}
		if (Size != Size2) {
			printf_info("%s\n", _("Declared JAR file size is different than real. Fixed by Gammu."));
			for (i=0;i<File.Used;i++) {
				if (strncasecmp(File.Buffer+i,"MIDlet-Jar-Size: ",17) == 0) {
					break;
				}
			}
			Pos = i;
			while (TRUE) {
				if (Pos ==0 || File.Buffer[Pos] == 13 || File.Buffer[Pos] == 10) break;
				Pos--;
			}
			i+= 15;
			while (TRUE) {
				if (i == File.Used || File.Buffer[i] == 13 || File.Buffer[i] == 10) break;
				i++;
			}
			while (i != File.Used) {
				File.Buffer[Pos] = File.Buffer[i];
				i++;
				Pos++;
			}
			File.Used = File.Used - (i - Pos);
			File.Buffer = realloc(File.Buffer,File.Used);
		} else if (Size == -1) {
			printf_info("%s\n", _("No JAR size info in JAD file. Added by Gammu."));
		}
		if (Size != Size2) {
			sprintf(buffer,"\nMIDlet-Jar-Size: %i",Size2);
			File.Buffer = realloc(File.Buffer,File.Used + strlen(buffer));
			memcpy(File.Buffer+File.Used,buffer,strlen(buffer));
			File.Used += strlen(buffer);
			Size = Size2;
		}
  		fprintf(stderr, _("Adding \"%s\""),Name);
		if (Version[0] != 0x00) fprintf(stderr, _(" version %s"),Version);
		fprintf(stderr, _(" created by %s\n"),Vendor);

		/* Bostjan Muller 3200 RH-30 3.08 */
		if (strstr(JAR,"http://") != NULL) {
			i = strlen(JAR)-1;
			while (JAR[i] != '/') i--;
			strcpy(buffer,JAR+i+1);
			strcpy(JAR,buffer);
			smprintf(gsm, "New file name is \"%s\"\n",JAR);
		}

		/* Changing all #13 or #10 to #13#10 in JAD */
		Pos    = 0;
		wasclr = FALSE;
		for (i=0;i<File.Used;i++) {
			switch (File.Buffer[i]) {
			case 0x0D:
			case 0x0A:
				if (!wasclr) {
					buffer[Pos++] = 0x0D;
					buffer[Pos++] = 0x0A;
					wasclr = TRUE;
				} else wasclr = FALSE;
				break;
			default:
				buffer[Pos++] 	= File.Buffer[i];
				wasclr		= FALSE;
			}
		}
		File.Buffer = realloc(File.Buffer, Pos);
		File.Used   = Pos;
		memcpy(File.Buffer,buffer,Pos);

		if (argc > 4) {
			for (j = 4; j < argc; j++) {
				if (strcasecmp(argv[j],"-overwrite") == 0) Overwrite = TRUE;
				if (strcasecmp(argv[j],"-overwriteall") == 0) {
					Overwrite = TRUE;
					OverwriteAll = TRUE;
				}
			}
		}

		/* adding folder */
		if (strstr(DecodeUnicodeString(Files.ID_FullName),"d:/predefjava/")== NULL) {
			strcpy(buffer,Vendor);
			strcat(buffer,Name);
			EncodeUnicode(File.Name,buffer,strlen(buffer));
			CopyUnicodeString(File.ID_FullName,Files.ID_FullName);
			error = GSM_AddFolder(gsm,&File);
			if (Overwrite && (error == ERR_FILEALREADYEXIST)) {
				printf_info("%s\n", _("Application already exists. Deleting by Gammu."));

				Start = TRUE;
				CopyUnicodeString(File2.ID_FullName,Files.ID_FullName);
				while (1) {
					error = GSM_GetFolderListing(gsm,&File2,Start);
					if (error == ERR_EMPTY) break;
					Print_Error(error);

					if (File2.Folder && !strcmp(DecodeUnicodeString(File2.Name),buffer)) {
						break;
					}

					Start = FALSE;
				}

				Start = TRUE;
				CopyUnicodeString(File.ID_FullName,File2.ID_FullName);
				while (1) {
					error = GSM_GetFolderListing(gsm,&File2,Start);
					if (error == ERR_EMPTY) break;
					Print_Error(error);

					strcpy(buffer,DecodeUnicodeString(File2.Name));

					i = strlen(buffer);
					if (i < 4) continue;

					i -= 4;

					if (OverwriteAll ||
					    strcmp(buffer + i,".jad") == 0 ||
					    strcmp(buffer + i,".jar") == 0) {
						fprintf(stderr, _("  Deleting %s\n"),buffer);
					error = GSM_DeleteFile(gsm,File2.ID_FullName);
					Print_Error(error);

						CopyUnicodeString(File2.ID_FullName,File.ID_FullName);
						Start = TRUE;
					} else {
						Start = FALSE;
					}
				}
			} else {
			    	Print_Error(error);
			}
			CopyUnicodeString(FileID,File.ID_FullName);
		} else {
			if (Overwrite) {
				Start = TRUE;
				CopyUnicodeString(File2.ID_FullName,Files.ID_FullName);

				printf_info("%s\n", _("Application already exists. Deleting by Gammu."));

				while (TRUE) {
					error = GSM_GetFolderListing(gsm,&File2,Start);
					if (error == ERR_EMPTY) break;
					Print_Error(error);

					strcpy(buffer,DecodeUnicodeString(File2.Name));

					i = strlen(Name);
					if (strncmp(buffer,Name,i) == 0 &&
					    (strcmp(buffer + i,".jad") == 0 ||
					     strcmp(buffer + i,".jar") == 0 ||
					     (strncmp(buffer + i,"\177_m_",4) == 0 && OverwriteAll))) {
						fprintf(stderr, _("  Deleting %s\n"),buffer);
						error = GSM_DeleteFile(gsm,File2.ID_FullName);
						Print_Error(error);

						CopyUnicodeString(File2.ID_FullName,Files.ID_FullName);
						Start = TRUE;
					} else {
						Start = FALSE;
					}
				}
			}

			CopyUnicodeString(FileID,Files.ID_FullName);
			CopyUnicodeString(File.ID_FullName,Files.ID_FullName);
		}

		/* adding jad file */
		strcpy(buffer,JAR);
		buffer[strlen(buffer) - 1] = 'd';
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		File.Type 	   = GSM_File_Other;
		File.ModifiedEmpty = TRUE;
		smprintf(gsm, "file id is \"%s\"\n",DecodeUnicodeString(File.ID_FullName));
		AddOneFile(&File, _("Writing JAD file:"), FALSE);

		if (argc > 4) {
			for (j = 4; j < argc; j++) {
				if (strcasecmp(argv[j],"-readonly") == 0) File.ReadOnly = TRUE;
			}
		}

		/* reading jar file */
		sprintf(buffer,"%s.jar",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);

		/* adding jar file */
		CopyUnicodeString(File.ID_FullName,FileID);
		strcpy(buffer,JAR);
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		File.Type 	   = GSM_File_Java_JAR;
		File.ModifiedEmpty = TRUE;
		AddOneFile(&File, _("Writing JAR file:"), FALSE);
		free(File.Buffer);
		GSM_Terminate();
		return;
	}

	if (strcasecmp(argv[2],"Gallery" 	 ) == 0 ||
	    strcasecmp(argv[2],"Gallery2"	 ) == 0 ||
	    strcasecmp(argv[2],"Camera"  	 ) == 0 ||
	    strcasecmp(argv[2],"Tones"   	 ) == 0 ||
	    strcasecmp(argv[2],"Tones2"  	 ) == 0 ||
	    strcasecmp(argv[2],"Records" 	 ) == 0 ||
	    strcasecmp(argv[2],"Video"   	 ) == 0 ||
	    strcasecmp(argv[2],"Playlist"	 ) == 0 ||
	    strcasecmp(argv[2],"MemoryCard"   ) == 0) {
		strcpy(buffer,argv[3]);
		if (argc > 4) {
			nextlong = 0;
			for (j = 4; j < argc; j++) {
				switch(nextlong) {
				case 0:
					if (strcasecmp(argv[j],"-name") == 0) {
						nextlong = 1;
						continue;
					}
					if (strcasecmp(argv[j],"-protected") == 0) {
						File.Protected = TRUE;
						continue;
					}
					if (strcasecmp(argv[j],"-readonly") == 0) {
						File.ReadOnly = TRUE;
						continue;
					}
					if (strcasecmp(argv[j],"-hidden") == 0) {
						File.Hidden = TRUE;
						continue;
					}
					if (strcasecmp(argv[j],"-system") == 0) {
						File.System = TRUE;
						continue;
					}
					if (strcasecmp(argv[j],"-newtime") == 0) {
						ModEmpty = TRUE;
						continue;
					}
					printf(_("Parameter \"%s\" unknown\n"),argv[j]);
					Terminate(3);
				case 1:
					strcpy(buffer,argv[j]);
					nextlong = 0;
					break;
				}
			}
			if (nextlong!=0) {
				printf_err("%s\n", _("Parameter missing!"));
				Terminate(3);
			}
		}
	}

	error = GSM_ReadFile(argv[3], &File);
	Print_Error(error);
	if (ModEmpty) File.ModifiedEmpty = TRUE;

	CopyUnicodeString(File.ID_FullName,Files.ID_FullName);
	EncodeUnicode(File.Name,buffer,strlen(buffer));
	for (i=strlen(buffer)-1;i>0;i--) {
		if (buffer[i] == '\\' || buffer[i] == '/') break;
	}
	if (buffer[i] == '\\' || buffer[i] == '/') {
		EncodeUnicode(File.Name,buffer+i+1,strlen(buffer)-i-1);
	}

	GSM_IdentifyFileFormat(&File);

	AddOneFile(&File, _("Writing file:"), FALSE);
	free(File.Buffer);
	GSM_Terminate();
}

void NokiaGetADC(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3GetADC(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4GetADC(argc, argv);
#endif

	GSM_Terminate();
}

void NokiaDisplayTest(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3DisplayTest(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4DisplayTest(argc, argv);
#endif

	GSM_Terminate();
}

void NokiaGetT9(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3GetT9(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4GetT9(argc, argv);
#endif

	GSM_Terminate();
}

void NokiaVibraTest(int argc, char *argv[])
{
	GSM_Init(TRUE);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3VibraTest(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4VibraTest(argc, argv);
#endif

	GSM_Terminate();
}
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

