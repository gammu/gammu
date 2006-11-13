/* (c) 2003 by Marcin Wiacek */
/* (c) 2006 by Michal Cihar */

#ifndef obexgen_h
#define obexgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"
#include "../../service/sms/gsmsms.h"

#ifndef GSM_USED_IRDAOBEX
#  define GSM_USED_IRDAOBEX
#endif
#ifndef GSM_USED_BLUEOBEX
#  define GSM_USED_BLUEOBEX
#endif

/**
 * Service type we want to use on OBEX.
 */
typedef enum {
	/**
	 * No specific service requested.
	 */
	OBEX_None = 1,
	/**
	 * IrMC synchronisation service.
	 */
	OBEX_IRMC,
	/**
	 * Browsing for folders.
	 */
	OBEX_BrowsingFolders,
} OBEX_Service;

typedef struct {
	int				FileLev;
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;
	GSM_File			Files[500];
	bool				FileLastPart;

	int				FrameSize;
	OBEX_Service			Service;

	/**
	 * Number of entries in phonebook
	 */
	int				PbCount;
	/**
	 * Offsets of entries in phonebook data
	 */
	int				*PbOffsets;
	/**
	 * LUID - location translation map for phonebook
	 */
	char				**PbLUID;
	/**
	 * Number of entries in PbLUID list.
	 */
	int				PbLUIDCount;
	/**
	 * Information Exchange Level for phonebook.
	 */
	int				PbIEL;
	/**
	 * Complete phonebook data.
	 */
	char				*PbData;
	/**
	 * Number of entries in todo
	 */
	int				TodoCount;
	/**
	 * Offsets of todo entries in calendar data
	 */
	int				*TodoOffsets;
	/**
	 * Number of entries in calendar
	 */
	int				CalCount;
	/**
	 * Offsets of calendar entries in calendar data
	 */
	int				*CalOffsets;
	/**
	 * LUID - location translation map for calendar (and todo)
	 */
	char				**CalLUID;
	/**
	 * Number of entries in CalLUID list.
	 */
	int				CalLUIDCount;
	/**
	 * Information Exchange Level for calendar.
	 */
	int				CalIEL;
	/**
	 * Complete calendar data.
	 */
	char				*CalData;
} GSM_Phone_OBEXGENData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
