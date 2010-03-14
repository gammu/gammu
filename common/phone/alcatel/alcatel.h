/*
 * High level functions for communication with Alcatel One Touch 501 and
 * compatible mobile phone.
 *
 * This code implements functions to communicate with Alcatel BE5 (One Touch
 * 500, 501, 701 and maybe others) phone. For some functions it uses normal AT
 * mode (not implemented here, look at ../at/atgen.[ch]) for others it
 * switches into binary mode and initialises underlaying protocol (see
 * ../../protocol/alcatel/alcabus.[ch]) and communicates over it. Don't ask
 * me why Alcatel uses such silly thing...
 */

#ifndef alcatel_h
#define alcatel_h

#include "../../gsmcomon.h"

#ifndef GSM_USED_AT
# define GSM_USED_AT
#endif
#ifndef GSM_USED_ALCABUS
# define GSM_USED_ALCABUS
#endif

/* in which mode we are currently? */
typedef enum {
	ModeAT = 1,
	ModeBinary
} GSM_Alcatel_Mode;

/* state of binary mode */
typedef enum {
	StateAttached = 1,	/* Attached */
	StateSession,	/* Active session of BinaryType type */
	StateEdit		/* Active editing of some type, BinaryItem incicates which item is being edited */
} GSM_Alcatel_BinaryState;

/* type of active binary session */
typedef enum {
	TypeCalendar = 1,
	TypeContacts,
	TypeToDo
} GSM_Alcatel_BinaryType;

/* Alcatel internal types */
typedef enum {
	Alcatel_date,	/* DATE	*/
	Alcatel_time,	/* TIME	*/
	Alcatel_string,	/* char	*/
	Alcatel_phone,	/* char	*/
	Alcatel_enum,	/* int	*/
	Alcatel_bool,	/* int	*/
	Alcatel_int,	/* int	*/
	Alcatel_byte	/* int	*/
} GSM_Alcatel_FieldType;

/* timeout for GSM_WaitFor */
#define ALCATEL_TIMEOUT			32
/* what is returned as free memory by GetMemoryStatus */
/* XXX: what should be returned by it, when there is no way how to determine
 * this from phone.
 */
#define ALCATEL_FREE_MEMORY		100
#define ALCATEL_MAX_LOCATION		0x0fffffff
#define ALCATEL_MAX_CATEGORIES		100

typedef struct {
	/***********************************/
	/* Communication state information */
	/***********************************/
	GSM_Alcatel_Mode	Mode;
	GSM_Alcatel_BinaryState	BinaryState;
	GSM_Alcatel_BinaryType	BinaryType;	/* 0 when N/A (StateAttached) */
	int			BinaryItem;	/* 0 when N/A (StateAttached, StateSession) */

	/*****************/
	/* Return values */
	/*****************/
	int			CommitedRecord;
	bool			TransferCompleted;
	GSM_Alcatel_FieldType	ReturnType;					/* Type of received data */
	GSM_DateTime		ReturnDateTime; 				/* For date and time */
	int			ReturnInt;					/* For enum, bool, int and byte */
	unsigned char		ReturnString[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];	/* For string and phone */

	/***********/
	/* Caches: */
	/***********/
	/* Listings of available items for each type */
	int			**CurrentList;
	int			*CurrentCount;
	int			*CalendarItems;
	int			*ToDoItems;
	int			*ContactsItems;
	int			CalendarItemsCount;
	int			ToDoItemsCount;
	int			ContactsItemsCount;
	int			CurrentFields[GSM_PHONEBOOK_ENTRIES+1];
	int			CurrentFieldsCount;
	int			CurrentFieldsItem;
	GSM_Alcatel_BinaryType	CurrentFieldsType;

	int			CurrentCategories[ALCATEL_MAX_CATEGORIES+1];
	int			CurrentCategoriesCount;
	GSM_Alcatel_BinaryType	CurrentCategoriesType;

} GSM_Phone_ALCATELData;

/*****************************/
/* Some magic numbers follow */
/*****************************/

/* synchronisation types (for everything except begin transfer): */
#define ALCATEL_SYNC_TYPE_CALENDAR	0x64
#define ALCATEL_SYNC_TYPE_TODO		0x68
#define ALCATEL_SYNC_TYPE_CONTACTS	0x6C

/* synchronisation types (for begin transfer): */
#define ALCATEL_BEGIN_SYNC_CALENDAR	0x00
#define ALCATEL_BEGIN_SYNC_TODO		0x02
#define ALCATEL_BEGIN_SYNC_CONTACTS	0x01

/* category types */
#define ALCATEL_LIST_TODO_CAT		0x9B
#define ALCATEL_LIST_CONTACTS_CAT	0x96

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
