#define _GNU_SOURCE		/* For strcasestr */
#include <string.h>

#include "../helper/locales.h"

#include <gammu.h>
#include <stdlib.h>
#include <signal.h>

#ifdef WIN32
#  include <windows.h>
#  include <process.h>
#  ifdef _MSC_VER
#    include <sys/utime.h>
#  else
#    include <utime.h>
#  endif
#else
#  include <utime.h>
#endif

#include "files.h"
#include "memory.h"
#include "message.h"
#include "common.h"

#include "../helper/formats.h"
#include "../helper/printing.h"
#include "../helper/string.h"

/**
 * Displays status of filesystem (if available).
 */
GSM_Error PrintFileSystemStatus(void)
{
	GSM_FileSystemStatus Status;
	GSM_Error error;

	error = GSM_GetFileSystemStatus(gsm, &Status);
	if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
		return error;
	}

	Print_Error(error);

	printf("\n");

	printf(LISTFORMAT, _("Phone memory"));
	printf(_("%i bytes (free %i bytes, used %i bytes)"),
	       Status.Free + Status.Used, Status.Free, Status.Used);
	printf("\n");

	if (Status.UsedImages != 0 || Status.UsedSounds != 0
	    || Status.UsedThemes != 0) {
		printf(LISTFORMAT, _("Usage details"));
		printf(_("images: %i, sounds: %i, themes: %i"),
		       Status.UsedImages, Status.UsedSounds, Status.UsedThemes);
		printf("\n");
	}

	return error;
}

void GetFileSystemStatus(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	GSM_Init(TRUE);

	error = PrintFileSystemStatus();
	Print_Error(error);

	GSM_Terminate();
}

void GetFileSystem(int argc, char *argv[])
{
	GSM_Error error;
	gboolean Start = TRUE, MemoryCard = FALSE;
	GSM_File Files;
	int j;
	long usedphone = 0, usedcard = 0;
	char FolderName[256], IDUTF[200];

	GSM_Init(TRUE);

	while (1) {
		error = GSM_GetNextFileFolder(gsm, &Files, Start);
		if (error == ERR_EMPTY)
			break;
		if (error != ERR_FOLDERPART)
			Print_Error(error);

		if (!Files.Folder) {
			if (GSM_IsPhoneFeatureAvailable
			    (GSM_GetModelInfo(gsm), F_FILES2)) {
				if (DecodeUnicodeString(Files.ID_FullName)[0] ==
				    'a') {
					MemoryCard = TRUE;
					usedcard += Files.Used;
				} else {
					usedphone += Files.Used;
				}
			} else {
				usedphone += Files.Used;
			}
		}

		if (argc <= 2 || !strcasecmp(argv[2], "-flatall") == 0) {
			/* Nokia filesystem 1 */
			if (UnicodeLength(Files.ID_FullName) != 0 &&
			    (DecodeUnicodeString(Files.ID_FullName)[0] == 'C' ||
			     DecodeUnicodeString(Files.ID_FullName)[0] ==
			     'c')) {
				printf("%8s.",
				       DecodeUnicodeString(Files.ID_FullName));
			}
			if (Files.Protected) {
				/* l10n: One char to indicate protected file */
				printf(_("P"));
			} else {
				printf(" ");
			}
			if (Files.ReadOnly) {
				/* l10n: One char to indicate read only file */
				printf(_("R"));
			} else {
				printf(" ");
			}
			if (Files.Hidden) {
				/* l10n: One char to indicate hidden file */
				printf(_("H"));
			} else {
				printf(" ");
			}
			if (Files.System) {
				/* l10n: One char to indicate system file */
				printf(_("S"));
			} else {
				printf(" ");
			}
			if (argc > 2 && strcasecmp(argv[2], "-flat") == 0) {
				if (!Files.Folder) {
					if (strcasecmp(argv[2], "-flatall") ==
					    0) {
						if (!Files.ModifiedEmpty) {
							printf(" %30s",
							       OSDateTime(Files.
									  Modified,
									  FALSE));
						} else
							printf(" %30c", 0x20);
						printf(" %9li", (long)Files.Used);
						printf(" ");
					} else
						printf("|-- ");
				} else {
					if (error == ERR_FOLDERPART) {
						printf("%s ", _("Part of folder"));
					} else {
						printf("%s ", _("Folder"));
					}
				}
			} else {
				if (Files.Level != 1) {
					for (j = 0; j < Files.Level - 2; j++)
						printf(" |   ");
					printf(" |-- ");
				}
				if (Files.Folder) {
					if (error == ERR_FOLDERPART) {
						printf("%s ", _("Part of folder"));
					} else {
						printf("%s ", _("Folder"));
					}
				}
			}
			printf("\"%s\"\n", DecodeUnicodeConsole(Files.Name));
		} else if (argc > 2 && strcasecmp(argv[2], "-flatall") == 0) {
			/* format for a folder ID;Folder;FOLDER_NAME;[FOLDER_PARAMETERS]
			 * format for a file   ID;File;FOLDER_NAME;FILE_NAME;DATESTAMP;FILE_SIZE;[FILE_PARAMETERS]  */
			EncodeUTF8QuotedPrintable(IDUTF, Files.ID_FullName);
			printf("%s;", IDUTF);
			if (!Files.Folder) {
				printf(_("File;"));
				printf("\"%s\";", FolderName);
				printf("\"%s\";",
				       DecodeUnicodeConsole(Files.Name));
				if (!Files.ModifiedEmpty) {
					printf("\"%s\";",
					       OSDateTime(Files.Modified,
							  FALSE));
				} else
					printf("\"%c\";", 0x20);
				printf("%ld;", (long)Files.Used);
			} else {
				if (error == ERR_FOLDERPART) {
					printf("%s;", _("Part of folder"));
				} else {
					printf("%s;", _("Folder"));
				}
				printf("\"%s\";",
				       DecodeUnicodeConsole(Files.Name));
				strcpy(FolderName,
				       DecodeUnicodeConsole(Files.Name));
			}

			if (Files.Protected)
				printf(_("P"));
			if (Files.ReadOnly)
				printf(_("R"));
			if (Files.Hidden)
				printf(_("H"));
			if (Files.System)
				printf(_("S"));
			printf("\n");
		}
		Start = FALSE;
	}

	error = PrintFileSystemStatus();

	if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
		printf("\n");
		printf(_("Used in phone: %li bytes"), usedphone);
		if (MemoryCard)
			printf(_(", used in card: %li bytes"), usedcard);
		printf("\n");
	}

	GSM_Terminate();
}

void SetFileAttrib(int argc, char *argv[])
{
	GSM_File Files;
	GSM_Error error;
	int i;

	Files.ReadOnly = FALSE;
	Files.Protected = FALSE;
	Files.System = FALSE;
	Files.Hidden = FALSE;

	DecodeUTF8QuotedPrintable(Files.ID_FullName, argv[2], strlen(argv[2]));

	for (i = 3; i < argc; i++) {
		if (strcasecmp(argv[i], "-readonly") == 0) {
			Files.ReadOnly = TRUE;
		} else if (strcasecmp(argv[i], "-protected") == 0) {
			Files.Protected = TRUE;
		} else if (strcasecmp(argv[i], "-system") == 0) {
			Files.System = TRUE;
		} else if (strcasecmp(argv[i], "-hidden") == 0) {
			Files.Hidden = TRUE;
		} else {
			fprintf(stderr, _("Unknown attribute (%s)\n"), argv[i]);
		}
	}

	GSM_Init(TRUE);

	error = GSM_SetFileAttributes(gsm, &Files);
	Print_Error(error);

	GSM_Terminate();
}

void GetRootFolders(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_File File;
	char IDUTF[200];

	GSM_Init(TRUE);

	File.ID_FullName[0] = 0;
	File.ID_FullName[1] = 0;

	while (1) {
		if (GSM_GetNextRootFolder(gsm, &File) != ERR_NONE)
			break;
		EncodeUTF8QuotedPrintable(IDUTF, File.ID_FullName);
		printf("%s ", IDUTF);
		printf("- %s\n", DecodeUnicodeString(File.Name));
	}

	GSM_Terminate();
}

void GetFolderListing(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	gboolean Start = TRUE;
	GSM_File Files;
	char IDUTF[200];

	GSM_Init(TRUE);

	DecodeUTF8QuotedPrintable(Files.ID_FullName, argv[2], strlen(argv[2]));

	while (1) {
		error = GSM_GetFolderListing(gsm, &Files, Start);
		if (error == ERR_EMPTY)
			break;
		if (error != ERR_FOLDERPART) {
			Print_Error(error);
		} else {
			printf("%s\n\n", _("Part of folder only"));
		}

		/* format for a folder ID;Folder;[FOLDER_PARAMETERS]
		 * format for a file   ID;File;FILE_NAME;DATESTAMP;FILE_SIZE;[FILE_PARAMETERS]  */
		EncodeUTF8QuotedPrintable(IDUTF, Files.ID_FullName);
		printf("%s;", IDUTF);
		if (!Files.Folder) {
			printf(_("File;"));
			printf("\"%s\";", DecodeUnicodeConsole(Files.Name));
			if (!Files.ModifiedEmpty) {
				printf("\"%s\";",
				       OSDateTime(Files.Modified, FALSE));
			} else
				printf("\"%c\";", 0x20);
			printf("%ld;", (long)Files.Used);
		} else {
			printf(_("Folder"));
			printf(";\"%s\";", DecodeUnicodeConsole(Files.Name));
		}

		if (Files.Protected)
			printf(_("P"));
		if (Files.ReadOnly)
			printf(_("R"));
		if (Files.Hidden)
			printf(_("H"));
		if (Files.System)
			printf(_("S"));
		printf("\n");

		Start = FALSE;
	}

	GSM_Terminate();
}

void GetOneFile(GSM_File * File, gboolean newtime, int i)
{
	GSM_Error error;
	FILE *file;
	gboolean start;
	unsigned char buffer[5000];
	struct utimbuf filedate;
	int Handle, Size, p, q, j, old1;
	time_t t_time1, t_time2;
	long diff;

	if (File->Buffer != NULL) {
		free(File->Buffer);
		File->Buffer = NULL;
	}
	File->Used = 0;
	start = TRUE;

	t_time1 = time(NULL);
	old1 = 65536;

	error = ERR_NONE;
	while (error == ERR_NONE) {
		error = GSM_GetFilePart(gsm, File, &Handle, &Size);
		if (error == ERR_NONE || error == ERR_EMPTY
		    || error == ERR_WRONGCRC) {
			if (start) {
				printf(_("Getting \"%s\"\n"),
				       DecodeUnicodeConsole(File->Name));
				start = FALSE;
			}
			if (File->Folder) {
				free(File->Buffer);
				GSM_Terminate();
				printf("%s\n",
				       _ ("Is a folder. Please give only file names."));
				Terminate(3);
			}
			if (Size == 0) {
				fprintf(stderr, "*");
			} else {
				fprintf(stderr, "\r");
				fprintf(stderr, _("%i percent"),
					(int)(File->Used * 100 / Size));
				if (File->Used * 100 / Size >= 2) {
					t_time2 = time(NULL);
					diff = t_time2 - t_time1;
					p = diff * (Size -
						    File->Used) / File->Used;
					if (p != 0) {
						if (p < old1)
							old1 = p;
						q = old1 / 60;
						fprintf(stderr,
							_
							(" (%02i:%02i minutes left)"),
							q, old1 - q * 60);
					} else {
						fprintf(stderr, "%30c", 0x20);
					}
				}
			}
			if (error == ERR_EMPTY)
				break;
			if (error == ERR_WRONGCRC) {
				printf_warn("%s\n",
				    _("File checksum calculated by phone doesn't match with value calculated by Gammu. File is damaged or there is a error in Gammu".));
				break;
			}
		}
		Print_Error(error);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	if (error == ERR_NONE || error == ERR_EMPTY || error == ERR_WRONGCRC) {
		if (File->Used != 0) {
			sprintf(buffer, "%s", DecodeUnicodeConsole(File->Name));
			for (j = strlen(buffer) - 1; j > 0; j--) {
				if (buffer[j] == '\\' || buffer[j] == '/')
					break;
			}
			if (buffer[j] == '\\' || buffer[j] == '/') {
				sprintf(buffer, "%s",
					DecodeUnicodeConsole(File->Name +
							     j * 2 + 2));
			}
			file = fopen(buffer, "wb");
			if (file == NULL) {
				sprintf(buffer, "file%s",
					DecodeUnicodeString(File->ID_FullName));
				file = fopen(buffer, "wb");
			}
			if (file == NULL) {
				sprintf(buffer, "file%i", i);
				file = fopen(buffer, "wb");
			}
			if (file == NULL)
				Print_Error(ERR_CANTOPENFILE);
			printf(_("  Saving to %s\n"), buffer);
			if (fwrite(File->Buffer, 1, File->Used, file) != File->Used) {
				printf_err(_("Error while writing file!\n"));
			}
			free(File->Buffer);
			File->Buffer = NULL;
			fclose(file);
			if (!newtime && !File->ModifiedEmpty) {
				/* access time */
				filedate.actime = Fill_Time_T(File->Modified);
				/* modification time */
				filedate.modtime = Fill_Time_T(File->Modified);
				smprintf(gsm, "Setting date of %s\n", buffer);
				utime(buffer, &filedate);
			}
		}
	}
}

void GetFiles(int argc, char *argv[])
{
	GSM_File File;
	int i;
	gboolean newtime = FALSE;

	File.Buffer = NULL;

	GSM_Init(TRUE);

	for (i = 2; i < argc; i++) {
		if (strcasecmp(argv[i], "-newtime") == 0) {
			newtime = TRUE;
			continue;
		}

		DecodeUTF8QuotedPrintable(File.ID_FullName, argv[i],
					  strlen(argv[i]));
		smprintf(gsm, "grabbing '%s' '%s'\n",
			  DecodeUnicodeString(File.ID_FullName), argv[i]);
		GetOneFile(&File, newtime, i);
	}

	GSM_Terminate();
}

void GetFileFolder(int argc, char *argv[])
{
	GSM_Error error;
	gboolean Start = TRUE;
	GSM_File File;
	int level = 0, allnum = 0, num = 0, filelevel = 0, i = 0;
	gboolean newtime = FALSE, found;
	unsigned char IDUTF[200];

	File.Buffer = NULL;

	GSM_Init(TRUE);

	for (i = 2; i < argc; i++) {
		if (strcasecmp(argv[i], "-newtime") == 0) {
			newtime = TRUE;
			continue;
		}
		allnum++;
	}

	while (allnum != num) {
		error = GSM_GetNextFileFolder(gsm, &File, Start);
		if (error == ERR_EMPTY)
			break;
		Print_Error(error);

		if (level == 0) {
			/* We search for file or folder */
			found = FALSE;
			for (i = 2; i < argc; i++) {
				if (strcasecmp(argv[i], "-newtime") == 0) {
					continue;
				}
				smprintf(gsm, "comparing %s %s\n",
					  DecodeUnicodeString(File.ID_FullName),
					  argv[i]);
				DecodeUTF8QuotedPrintable(IDUTF, argv[i],
							  strlen(argv[i]));
				if (mywstrncasecmp(File.ID_FullName, IDUTF, 0)) {
					smprintf(gsm, "found folder");
					found = TRUE;
					if (File.Folder) {
						level = 1;
						filelevel = File.Level + 1;
						Start = FALSE;
					} else {
						level = 2;
					}
					break;
				}
			}
			if (found && File.Folder)
				continue;
		}
		if (level == 1) {
			/* We have folder */
			smprintf(gsm, "%i %i\n", File.Level, filelevel);
			if (File.Level != filelevel) {
				level = 0;
				num++;
			}
		}

		if (level != 0 && !File.Folder) {
			GetOneFile(&File, newtime, i);
			i++;
		}

		if (level == 2) {
			level = 0;
			num++;
		}

		Start = FALSE;
	}

	free(File.Buffer);
	GSM_Terminate();
}

void AddOneFile(GSM_File * File, const char *text, const gboolean send)
{
	GSM_Error error;
	int Pos, Handle, i, j, old1;
	time_t t_time1, t_time2;
	GSM_DateTime dt;
	long diff;

	GSM_GetCurrentDateTime(&dt);
	t_time1 = Fill_Time_T(dt);
	old1 = 65536;

	smprintf(gsm, "Adding file to filesystem now\n");
	error = ERR_NONE;
	Pos = 0;
	while (error == ERR_NONE) {
		if (send) {
			error = GSM_SendFilePart(gsm, File, &Pos, &Handle);
		} else {
			error = GSM_AddFilePart(gsm, File, &Pos, &Handle);
		}
		if (error != ERR_EMPTY && error != ERR_WRONGCRC)
			Print_Error(error);
		if (File->Used != 0) {
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", text);
			fprintf(stderr, _("%i percent"),
				(int)(Pos * 100 / File->Used));
			if (Pos * 100 / File->Used >= 2) {
				GSM_GetCurrentDateTime(&dt);
				t_time2 = Fill_Time_T(dt);
				diff = t_time2 - t_time1;
				i = diff * (File->Used - Pos) / Pos;
				if (i != 0) {
					if (i < old1)
						old1 = i;
					j = old1 / 60;
					fprintf(stderr,
						_(" (%02i:%02i minutes left)"),
						j, old1 - (j * 60));
				} else {
					fprintf(stderr, "%30c", ' ');
				}
			}
		}
	}
	fprintf(stderr, "\n");
	if (error == ERR_WRONGCRC) {
		printf_warn("%s\n",
		    _("File checksum calculated by phone doesn't match with value calculated by Gammu. File is damaged or there is a error in Gammu".));
	}
}

void AddSendFile(int argc, char *argv[])
{
	GSM_Error error;
	GSM_File File;
	int i, nextlong;
	char IDUTF[200];
	gboolean sendfile = FALSE;
	int optint = 2;

	if (strcasestr(argv[1], "sendfile") != NULL) {
		sendfile = TRUE;
	}

	File.Buffer = NULL;
	if (!sendfile) {
		DecodeUTF8QuotedPrintable(File.ID_FullName, argv[optint],
					  strlen(argv[optint]));
		optint++;
	}
	error = GSM_ReadFile(argv[optint], &File);
	Print_Error(error);
	EncodeUnicode(File.Name, argv[optint], strlen(argv[optint]));
	for (i = strlen(argv[optint]) - 1; i > 0; i--) {
		if (argv[optint][i] == '\\' || argv[optint][i] == '/')
			break;
	}
	if (argv[optint][i] == '\\' || argv[optint][i] == '/') {
		EncodeUnicode(File.Name, argv[optint] + i + 1,
			      strlen(argv[optint]) - i - 1);
	}
	optint++;

	GSM_IdentifyFileFormat(&File);

	File.Protected = FALSE;
	File.ReadOnly = FALSE;
	File.Hidden = FALSE;
	File.System = FALSE;

	if (argc > optint) {
		nextlong = 0;
		for (i = optint; i < argc; i++) {
			switch (nextlong) {
				case 0:
					if (strcasecmp(argv[i], "-type") == 0) {
						nextlong = 1;
						continue;
					}
					if (strcasecmp(argv[i], "-protected") ==
					    0) {
						File.Protected = TRUE;
						continue;
					}
					if (strcasecmp(argv[i], "-readonly") ==
					    0) {
						File.ReadOnly = TRUE;
						continue;
					}
					if (strcasecmp(argv[i], "-hidden") == 0) {
						File.Hidden = TRUE;
						continue;
					}
					if (strcasecmp(argv[i], "-system") == 0) {
						File.System = TRUE;
						continue;
					}
					if (strcasecmp(argv[i], "-newtime") ==
					    0) {
						File.ModifiedEmpty = TRUE;
						continue;
					}
					printf(_("Parameter \"%s\" unknown\n"),
					       argv[i]);
					Terminate(3);
				case 1:
					if (strcasecmp(argv[i], "JAR") == 0) {
						File.Type = GSM_File_Java_JAR;
					} else if (strcasecmp(argv[i], "JPG") ==
						   0) {
						File.Type = GSM_File_Image_JPG;
					} else if (strcasecmp(argv[i], "BMP") ==
						   0) {
						File.Type = GSM_File_Image_BMP;
					} else if (strcasecmp(argv[i], "WBMP")
						   == 0) {
						File.Type = GSM_File_Image_WBMP;
					} else if (strcasecmp(argv[i], "GIF") ==
						   0) {
						File.Type = GSM_File_Image_GIF;
					} else if (strcasecmp(argv[i], "PNG") ==
						   0) {
						File.Type = GSM_File_Image_PNG;
					} else if (strcasecmp(argv[i], "MIDI")
						   == 0) {
						File.Type = GSM_File_Sound_MIDI;
					} else if (strcasecmp(argv[i], "AMR") ==
						   0) {
						File.Type = GSM_File_Sound_AMR;
					} else if (strcasecmp(argv[i], "NRT") ==
						   0) {
						File.Type = GSM_File_Sound_NRT;
					} else if (strcasecmp(argv[i], "3GP") ==
						   0) {
						File.Type = GSM_File_Video_3GP;
					} else {
						printf(_
						       ("What file type (\"%s\") ?\n"),
						       argv[i]);
						Terminate(3);
					}
					nextlong = 0;
					break;
			}
		}
		if (nextlong != 0) {
			printf_err("%s\n", _("Parameter missing!"));
			Terminate(3);
		}
	}

	GSM_Init(TRUE);

	AddOneFile(&File, _("Writing:"), sendfile);
	EncodeUTF8QuotedPrintable(IDUTF, File.ID_FullName);
	printf(_("ID of new file is \"%s\"\n"), IDUTF);

	free(File.Buffer);
	GSM_Terminate();
}

void AddFolder(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	char IDUTF[200];
	GSM_File File;

	DecodeUTF8QuotedPrintable(File.ID_FullName, argv[2], strlen(argv[2]));
	EncodeUnicode(File.Name, argv[3], strlen(argv[3]));
	File.ReadOnly = FALSE;

	GSM_Init(TRUE);

	error = GSM_AddFolder(gsm, &File);
	Print_Error(error);
	EncodeUTF8QuotedPrintable(IDUTF, File.ID_FullName);
	printf(_("ID of new folder is \"%s\"\n"), IDUTF);

	GSM_Terminate();
}

void DeleteFolder(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	unsigned char buffer[500];

	GSM_Init(TRUE);

	DecodeUTF8QuotedPrintable(buffer, argv[2], strlen(argv[2]));

	error = GSM_DeleteFolder(gsm, buffer);
	Print_Error(error);

	GSM_Terminate();
}

void DeleteFiles(int argc, char *argv[])
{
	GSM_Error error;
	int i;
	unsigned char buffer[500];

	GSM_Init(TRUE);

	for (i = 2; i < argc; i++) {
		DecodeUTF8QuotedPrintable(buffer, argv[i], strlen(argv[i]));
		error = GSM_DeleteFile(gsm, buffer);
		Print_Error(error);
	}

	GSM_Terminate();
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
