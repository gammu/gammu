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
#include <gammu-limits.h>

/**
 * Status of filesystem.
 *
 * \ingroup File
 */
typedef struct {
	int Free;
	int Used;
	int UsedImages;
	int UsedSounds;
	int UsedThemes;
} GSM_FileSystemStatus;

/**
 * File type identifier.
 *
 * \ingroup File
 */
typedef enum {
	GSM_File_Other = 1,
	GSM_File_Java_JAR,
	GSM_File_Image_JPG,
	GSM_File_Image_BMP,
	GSM_File_Image_GIF,
	GSM_File_Image_PNG,
	GSM_File_Image_WBMP,
	GSM_File_Video_3GP,
	GSM_File_Sound_AMR,
	/**
	 * DCT4 binary format
	 */
	GSM_File_Sound_NRT,
	GSM_File_Sound_MIDI,
	GSM_File_MMS
} GSM_FileType;

/**
 * Structure for holding file information and data.
 *
 * \ingroup File
 */
typedef struct {
	/**
	 * How many bytes are used.
	 */
	size_t Used;
	/**
	 * Name in Unicode
	 */
	unsigned char Name[2 * (GSM_MAX_FILENAME_LENGTH + 1)];
	/**
	 * True, when folder
	 */
	gboolean Folder;
	/**
	 * How much file is nested on filesystem.
	 */
	int Level;
	/**
	 * Type of file.
	 */
	GSM_FileType Type;
	/**
	 * ID in Unicode
	 */
	unsigned char ID_FullName[2 * (GSM_MAX_FILENAME_ID_LENGTH + 1)];
	/**
	 * Pointer to file data.
	 */
	unsigned char *Buffer;
	/**
	 * Last modification date.
	 */
	GSM_DateTime Modified;
	/**
	 * Whether modification date is empty.
	 */
	gboolean ModifiedEmpty;

	/**
	 * Protected file attribute.
	 */
	gboolean Protected;
	/**
	 * Read only file attribute.
	 */
	gboolean ReadOnly;
	/**
	 * Hidden file attribute.
	 */
	gboolean Hidden;
	/**
	 * System file attribute.
	 */
	gboolean System;
} GSM_File;

/**
 * Parses JAD file.
 *
 * \param File JAD file data.
 * \param Vendor Buffer for vendor name.
 * \param Name Buffer for application name.
 * \param JAR Buffer for JAR URL.
 * \param Version Buffer for version of application.
 * \param Size Pointer to integer to store size.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_JADFindData(GSM_File *File, char *Vendor, char *Name, char *JAR,
			  char *Version, int *Size);

/**
 * Reads file from filesystem to \ref GSM_File structure.
 *
 * \param FileName File to read.
 * \param File Storage for data.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_ReadFile(const char *FileName, GSM_File * File);

/**
 * Identifies file format by checking it's content.
 *
 * \param File File data, Type member will be filled in.
 *
 * \ingroup File
 */
void GSM_IdentifyFileFormat(GSM_File * File);

/**
 * Gets next filename from filesystem.
 *
 * \param s State machine pointer.
 * \param File File structure where path will be stored, if start is
 * FALSE, it should contain data from previous reading (at least ID).
 * \param start Whether we're starting transfer.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_GetNextFileFolder(GSM_StateMachine * s, GSM_File * File,
				gboolean start);
/**
 * Gets listing of folder.
 *
 * \param s State machine pointer.
 * \param File File structure where path will be stored, if start is
 * FALSE, it should contain data from previous reading (at least ID). On
 * start it should contain path to directory.
 * \param start Whether we're starting transfer.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_GetFolderListing(GSM_StateMachine * s, GSM_File * File,
			       gboolean start);
/**
 * Gets next root folder.
 *
 * \param s State machine pointer.
 * \param File File structure where path will be stored.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_GetNextRootFolder(GSM_StateMachine * s, GSM_File * File);

/**
 * Sets file system attributes.
 *
 * \param s State machine pointer.
 * \param File File structure with path and attributes.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_SetFileAttributes(GSM_StateMachine * s, GSM_File * File);

/**
 * Retrieves file part.
 *
 * \param s State machine pointer.
 * \param File File structure with path, data will be stored here.
 * \param Size Size of transmitted data.
 * \param Handle Handle for saving file, some drivers need this
 * information to be kept between function calls.
 *
 * \return Error code, \ref ERR_EMPTY after transfer end.
 *
 * \ingroup File
 */
GSM_Error GSM_GetFilePart(GSM_StateMachine * s, GSM_File * File, int *Handle,
			  int *Size);

/**
 * Adds file to filesystem. Call repeatedly until function returns
 * \ref ERR_EMPTY.
 *
 * \param s State machine pointer.
 * \param File File structure and data.
 * \param Pos Position of transmitted data. Should be 0 on start.
 * \param Handle Handle for saving file, some drivers need this
 * information to be kept between function calls.
 *
 * \return Error code, \ref ERR_EMPTY after transfer end.
 *
 * \ingroup File
 */
GSM_Error GSM_AddFilePart(GSM_StateMachine * s, GSM_File * File, int *Pos,
			  int *Handle);
/**
 * Sends file to phone, it's up to phone to decide what to do with it.
 * It is usually same as when you receive file over Bluetooth from other
 * phone. Use in same way as \ref GSM_AddFilePart.
 *
 * \param s State machine pointer.
 * \param File File structure and data.
 * \param Pos Position of transmitted data. Should be 0 on start.
 * \param Handle Handle for saving file, some drivers need this
 * information to be kept between function calls.
 *
 * \return Error code, \ref ERR_EMPTY after transfer end.
 *
 * \ingroup File
 */
GSM_Error GSM_SendFilePart(GSM_StateMachine * s, GSM_File * File, int *Pos,
			   int *Handle);
/**
 * Acquires filesystem status.
 *
 * \param s State machine pointer.
 * \param Status Storage for status information.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_GetFileSystemStatus(GSM_StateMachine * s,
				  GSM_FileSystemStatus * Status);
/**
 * Deletes file from filesystem.
 *
 * \param s State machine pointer.
 * \param ID ID of folder.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_DeleteFile(GSM_StateMachine * s, unsigned char *ID);

/**
 * Adds folder to filesystem.
 *
 * \param s State machine pointer.
 * \param File Structure containing information about new folder (Name
 * and FullName).
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_AddFolder(GSM_StateMachine * s, GSM_File * File);

/**
 * Deletes folder from filesystem.
 *
 * \param s State machine pointer.
 * \param ID ID of folder.
 *
 * \return Error code.
 *
 * \ingroup File
 */
GSM_Error GSM_DeleteFolder(GSM_StateMachine * s, unsigned char *ID);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
