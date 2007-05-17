/* (c) 2001-2004 by Marcin Wiacek */

#ifndef __gsm_wap_h
#define __gsm_wap_h

#include "gsmmisc.h"
#include "../misc/misc.h"

#include <gammu-wap.h>


void NOKIA_EncodeWAPMMSSettingsSMSText(unsigned char *Buffer, int *Length, GSM_WAPSettings *settings, bool MMS);

/* -------------------------------- WAP Bookmark --------------------------- */


void 		NOKIA_EncodeWAPBookmarkSMSText	(unsigned char *Buffer, int *Length, GSM_WAPBookmark *bookmark);
GSM_Error 	GSM_EncodeURLFile		(unsigned char *Buffer, int *Length, GSM_WAPBookmark *bookmark);

/* ------------------------------ MMS Indicator ---------------------------- */

typedef struct {
	char		Address[500];
	char		Title[200];
	char		Sender[200];
} GSM_MMSIndicator;

void GSM_EncodeMMSIndicatorSMSText(unsigned char *Buffer, int *Length, GSM_MMSIndicator Indicator);

void GSM_EncodeWAPIndicatorSMSText(unsigned char *Buffer, int *Length, char *Text, char *URL);

/* ------------------------------ MMS file --------------------------------- */

#define MAX_MULTI_MMS 20

typedef enum {
	MMSADDRESS_PHONE,
	MMSADDRESS_UNKNOWN
} MMSAddressType;

typedef struct {
	GSM_File		File;
	unsigned char		ContentType[400];	// CT in Unicode
	unsigned char		SMIL[400];		// Smil ID in Unicode
} EncodedMultiPartMMSEntry2;

typedef struct {
	/* Subparts */
	EncodedMultiPartMMSEntry2 Entries[MAX_MULTI_MMS];
	int			  EntriesNum;

	unsigned char		  Source[200];		// in Unicode
	MMSAddressType		  SourceType;
	unsigned char		  Destination[200];	// in Unicode
	MMSAddressType		  DestinationType;
	unsigned char	  	  CC[200];		// in Unicode
	MMSAddressType		  CCType;

	unsigned char		  Subject[200];		// in Unicode
	unsigned char		  ContentType[400];	// CT in Unicode
	unsigned char		  MSGType[50];		// no Unicode

	bool			  DateTimeAvailable;
	GSM_DateTime		  DateTime;

	bool			  MMSReportAvailable;
	bool			  MMSReport;
} GSM_EncodedMultiPartMMSInfo2;

GSM_Error GSM_DecodeMMSFileToMultiPart(GSM_File *file, GSM_EncodedMultiPartMMSInfo2 *info);
GSM_Error GSM_ClearMMSMultiPart(GSM_EncodedMultiPartMMSInfo2 *info);

/* ---------------------- MMS folders ------------------------------------- */

/**
 * Number of possible MMS folders.
 */
#define GSM_MAX_MMS_FOLDERS	     	10
/**
 * Maximal length of MMS folder name.
 */
#define GSM_MAX_MMS_FOLDER_NAME_LEN     20

/**
 * Information about MMS folder.
 */
typedef struct {
	/**
	 * Whether it is really inbox.
	 */
	bool		    	InboxFolder;
        /**
  	 * Name for MMS folder.
	 */
        char	    		Name[(GSM_MAX_MMS_FOLDER_NAME_LEN+1)*2];
} GSM_OneMMSFolder;

/**
 * List of MMS folders.
 */
typedef struct {
        /**
	 * Array of structures holding information about each folder.
	 */
        GSM_OneMMSFolder	Folder[GSM_MAX_MMS_FOLDERS];
        /**
 	 * Number of MMS folders.
	 */
        unsigned char	   	Number;
} GSM_MMSFolders;

/* ------------------------------------------------------------------------ */


#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
