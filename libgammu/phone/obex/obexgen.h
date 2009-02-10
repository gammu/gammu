/* (c) 2003 by Marcin Wiacek */
/* (c) 2006 by Michal Cihar */

/**
 * \file obexgen.h
 * @author Michal Čihař
 * @author Marcin Wiacek
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @addtogroup OBEXPhone
 * @{
 */
#ifndef obexgen_h
#define obexgen_h

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

/**
 * Description of what is IrMC end point able to do.
 */
typedef struct {
	/**
	 * Information Exchange Level.
	 */
	int IEL;
	/**
	 * Support for hard deletes?
	 */
	bool HD;
} IRMC_Capability;

typedef struct {
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;
	GSM_File			Files[500];
	bool				FileLastPart;

	int				FrameSize;
	OBEX_Service			Service;
	/**
	 * Initial service used in configuration (this will be used for filesystem browsing)
	 */
	OBEX_Service			InitialService;

	/**
	 * Number of entries in phonebook
	 */
	int				PbCount;
	/**
	 * Offsets of entries in phonebook data
	 */
	int				*PbOffsets;
	/**
	 * Index - location translation map for phonebook
	 */
	int				*PbIndex;
	/**
	 * Number of entries in PbIndex list.
	 */
	int				PbIndexCount;
	/**
	 * LUID - location translation map for phonebook
	 */
	char				**PbLUID;
	/**
	 * Number of entries in PbLUID list.
	 */
	int				PbLUIDCount;
	/**
	 * Whether to update PbLUID after this request.
	 */
	bool				UpdatePbLUID;
	/**
	 * Capabilities for phonebook.
	 */
	IRMC_Capability			PbCap;
	/**
	 * Complete phonebook data.
	 */
	char				*PbData;
	/**
	 * Number of read phonebook entries.
	 */
	int				ReadPhonebook;
	/**
	 * Number of entries in todo
	 */
	int				TodoCount;
	/**
	 * Offsets of todo entries in calendar data
	 */
	int				*TodoOffsets;
	/**
	 * Index - location translation map for todo
	 */
	int				*TodoIndex;
	/**
	 * Number of entries in TodoIndex list.
	 */
	int				TodoIndexCount;
	/**
	 * LUID - location translation map for todo
	 */
	char				**TodoLUID;
	/**
	 * Number of entries in TodoLUID list.
	 */
	int				TodoLUIDCount;
	/**
	 * Whether to update TodoLUID after this request.
	 */
	bool				UpdateTodoLUID;
	/**
	 * Number of read todo entries.
	 */
	int				ReadTodo;
	/**
	 * Number of entries in calendar
	 */
	int				CalCount;
	/**
	 * Offsets of calendar entries in calendar data
	 */
	int				*CalOffsets;
	/**
	 * Index - location translation map for calendar
	 */
	int				*CalIndex;
	/**
	 * Number of entries in CalIndex list.
	 */
	int				CalIndexCount;
	/**
	 * LUID - location translation map for calendar
	 */
	char				**CalLUID;
	/**
	 * Number of entries in CalLUID list.
	 */
	int				CalLUIDCount;
	/**
	 * Whether to update CalLUID after this request.
	 */
	bool				UpdateCalLUID;
	/**
	 * Capabilities for calendar.
	 */
	IRMC_Capability			CalCap;
	/**
	 * Complete calendar data.
	 */
	char				*CalData;
	/**
	 * Number of read calendar entries.
	 */
	int				ReadCalendar;
	/**
	 * Number of entries in note
	 */
	int				NoteCount;
	/**
	 * Offsets of entries in note data
	 */
	int				*NoteOffsets;
	/**
	 * Index - location translation map for note
	 */
	int				*NoteIndex;
	/**
	 * Number of entries in NoteIndex list.
	 */
	int				NoteIndexCount;
	/**
	 * LUID - location translation map for note
	 */
	char				**NoteLUID;
	/**
	 * Number of entries in NoteLUID list.
	 */
	int				NoteLUIDCount;
	/**
	 * Whether to update NoteLUID after this request.
	 */
	bool				UpdateNoteLUID;
	/**
	 * Capabilities for note.
	 */
	IRMC_Capability			NoteCap;
	/**
	 * Complete note data.
	 */
	char				*NoteData;
	/**
	 * Capability data.
	 */
	char				*OBEXCapability;
	/**
	 * Devinfo data.
	 */
	char				*OBEXDevinfo;
} GSM_Phone_OBEXGENData;

#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
