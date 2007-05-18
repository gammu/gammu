/**
 * \file gammu-file.h
 * \author Michal Čihař
 * 
 * File manipulations.
 */
#ifndef __gammu_file_h
#define __gammu_file_h

/**
 * \defgroup File File
 * Files handling.
 */

#include <gammu-types.h>
#include <gammu-datetime.h>

typedef struct {
	int Free;
	int Used;
	int UsedImages;
	int UsedSounds;
	int UsedThemes;
} GSM_FileSystemStatus;

typedef enum {
	GSM_File_Java_JAR = 1,
	GSM_File_Image_JPG,
	GSM_File_Image_BMP,
	GSM_File_Image_GIF,
	GSM_File_Image_PNG,
	GSM_File_Image_WBMP,
	GSM_File_Video_3GP,
	GSM_File_Sound_AMR,
	GSM_File_Sound_NRT,		/**< DCT4 binary format   */
	GSM_File_Sound_MIDI,
#ifdef DEVELOP
	GSM_File_MMS,
#endif
	GSM_File_Other
} GSM_FileType;

typedef struct {
	int Used;			  /**< how many bytes used */
	unsigned char Name[300];	  /**< Name in Unicode	 */
	bool Folder;			  /**< true, when folder 	 */
	int Level;
	GSM_FileType Type;
	unsigned char ID_FullName[800];	  /**< ID in Unicode 	 */
	unsigned char *Buffer;

	GSM_DateTime Modified;
	bool ModifiedEmpty;

	/* File attributes */
	bool Protected;
	bool ReadOnly;
	bool Hidden;
	bool System;
} GSM_File;

GSM_Error GSM_JADFindData(GSM_File File, char *Vendor, char *Name, char *JAR,
			  char *Version, int *Size);

GSM_Error GSM_ReadFile(char *FileName, GSM_File * File);
void GSM_IdentifyFileFormat(GSM_File * File);

/**
 * Gets next filename from filesystem.
 */
GSM_Error GSM_GetNextFileFolder(GSM_StateMachine * s, GSM_File * File,
				bool start);
/**
 * Gets file part from filesystem.
 */
GSM_Error GSM_GetFolderListing(GSM_StateMachine * s, GSM_File * File,
			       bool start);
/**
 * Gets next root folder.
 */
GSM_Error GSM_GetNextRootFolder(GSM_StateMachine * s, GSM_File * File);
/**
 * Sets file system attributes.
 */
GSM_Error GSM_SetFileAttributes(GSM_StateMachine * s, GSM_File * File);
/**
 * Retrieves file part.
 */
GSM_Error GSM_GetFilePart(GSM_StateMachine * s, GSM_File * File, int *Handle,
			  int *Size);
/**
 * Adds file part to filesystem.
 */
GSM_Error GSM_AddFilePart(GSM_StateMachine * s, GSM_File * File, int *Pos,
			  int *Handle);
/**
 * Sends file to phone, it's up to phone to decide what to do with it.
 */
GSM_Error GSM_SendFilePart(GSM_StateMachine * s, GSM_File * File, int *Pos,
			   int *Handle);
/**
 * Acquires filesystem status.
 */
GSM_Error GSM_GetFileSystemStatus(GSM_StateMachine * s,
				  GSM_FileSystemStatus * Status);
/**
 * Deletes file from filessytem.
 */
GSM_Error GSM_DeleteFile(GSM_StateMachine * s, unsigned char *ID);
/**
 * Adds folder to filesystem.
 */
GSM_Error GSM_AddFolder(GSM_StateMachine * s, GSM_File * File);
/**
 * Deletes folder from filesystem.
 */
GSM_Error GSM_DeleteFolder(GSM_StateMachine * s, unsigned char *ID);
#endif
