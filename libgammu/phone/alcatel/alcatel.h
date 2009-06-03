/* (c) 2002-2004 by Michal Cihar */

/**
 * \file alcatel.h
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @addtogroup AlcatelPhone
 * @{
 */
/*
 * High level functions for communication with Alcatel One Touch 501 and
 * compatible mobile phone.
 *
 * This code implements functions to communicate with Alcatel phones,
 * currently seem to work:
 *  - BE5 series (501/701)
 *  - BF5 series (715)
 *  - BH4 series (535/735)
 * For some functions it uses normal AT  mode (not implemented here, look at
 * ../at/atgen.[ch]) for others it  switches into binary mode and initialises
 * underlaying protocol (see ../../protocol/alcatel/alcabus.[ch]) and
 * communicates over it. Don't ask me why Alcatel uses such silly thing...
 */

#ifndef alcatel_h
#define alcatel_h

#include <gammu-limits.h>
#include <gammu-category.h>

#ifndef GSM_USED_AT
# define GSM_USED_AT
#endif
#ifndef GSM_USED_ALCABUS
# define GSM_USED_ALCABUS
#endif

/**
 * Determines which mode is phone currently using.
 */
typedef enum {
	/**
	 * Phone accepts normal AT commands.
	 */
	ModeAT = 1,
	/**
	 * Binary mode using proprietary protocol.
	 */
	ModeBinary
} GSM_Alcatel_Mode;

/**
 * Protocol version being used for binary mode.
 */
typedef enum {
	/**
	 * Version 1.0 used in BE5 phones (501, 701).
	 */
	V_1_0 = 1,
	/**
	 * Version 1.1 used in BF5 phones (715, 535, 735).
	 */
	V_1_1
} GSM_Alcatel_ProtocolVersion;

/**
 * State of binary mode.
 */
typedef enum {
	/**
	 * Binary mode is active. No type selected.
	 */
	StateAttached = 1,
	/**
	 * Opened session of some type.
	 */
	StateSession,
	/**
	 * Some item is being edited.
	 */
	StateEdit
} GSM_Alcatel_BinaryState;

/**
 * Type of active binary session.
 */
typedef enum {
	/**
	 * Calendar events.
	 */
	TypeCalendar = 1,
	/**
	 * Contacts.
	 */
	TypeContacts,
	/**
	 * To do items.
	 */
	TypeToDo
} GSM_Alcatel_BinaryType;

/**
 * Alcatel internal types.
 */
typedef enum {
	/**
	 * Date, stored as @ref GSM_DateTime.
	 */
	Alcatel_date,
	/**
	 * Time, stored as @ref GSM_DateTime.
	 */
	Alcatel_time,
	/**
	 * String, strored as chars
	 */
	Alcatel_string,	/* char	*/
	/**
	 * Phone number (can contain anything, but dialling it then may cause
	 * strange problems to phone) strored as chars.
	 */
	Alcatel_phone,
	/**
	 * Enumeration, usually used for user defined values (categories),
	 * stored as int.
	 */
	Alcatel_enum,
	/**
	 * Boolean, stored as int.
	 */
	Alcatel_bool,
	/**
	 * 32-bit unsigned integer, stored as int.
	 */
	Alcatel_int,
	/**
	 * 8-bit usigned integer, stored as int.
	 */
	Alcatel_byte
} GSM_Alcatel_FieldType;

/**
 * Return value for GetMemoryStatus. There is no way ho to determine free
 * memory so we have to return some fixed value.
 */
#define ALCATEL_FREE_MEMORY		100
/**
 * Maximum location, that will driver attempt to read. Because phone can have
 * up to 2^32 locations, we want to limit this a bit.
 */
/* There COULD be 0xffffffff on next line, but this is IMHO enough */
#define ALCATEL_MAX_LOCATION		0xffff
/**
 * Maximum category count. Used for static cache size.
 */
#define ALCATEL_MAX_CATEGORIES		100

/**
 * Alcatel driver private data.
 */
typedef struct {
	/***********************************/
	/* Communication state information */
	/***********************************/
	/**
	 * Mode of current communication.
	 */
	GSM_Alcatel_Mode	Mode;
	/**
	 * State of current communication if phone is in binary mode.
	 */
	GSM_Alcatel_BinaryState	BinaryState;
	/**
	 * Type of current communication if phone is in session or edit state,
	 * zero otherwise.
	 */
	GSM_Alcatel_BinaryType	BinaryType;
	/**
	 * Currently edited item when phone is in edit state, zero otherwise.
	 */
	int			BinaryItem;
	/**
	 * Protocol version being used.
	 */
	GSM_Alcatel_ProtocolVersion	ProtocolVersion;

	/*****************/
	/* Return values */
	/*****************/
	/**
	 * Return value for commited record position.
	 */
	int			CommitedRecord;
	/**
	 * Used for detecting end of large data data, that don't fit in one
	 * message.
	 */
	gboolean			TransferCompleted;
	/**
	 * Type of currently received field.
	 */
	GSM_Alcatel_FieldType	ReturnType;
	/**
	 * Value of currently returned field (if it can be saved in DateTime).
	 */
	GSM_DateTime		ReturnDateTime;
	/**
	 * Value of currently returned field (if it can be saved in int).
	 */
	unsigned int		ReturnInt;
	/**
	 * Value of currently returned field (if it can be saved in string).
	 */
	unsigned char		ReturnString[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];

	/***********/
	/* Caches: */
	/***********/
	/* Listings of available items for each type */
	/**
	 * Pointer to list of active items.
	 */
	int			**CurrentList;
	/**
	 * Pointer to currently count of active items.
	 */
	int			*CurrentCount;
	/**
	 * Items locations in calendar.
	 */
	int			*CalendarItems;
	/**
	 * Items locations in to do list.
	 */
	int			*ToDoItems;
	/**
	 * Items locations in contacts.
	 */
	int			*ContactsItems;
	/**
	 * Count of calendar items stored in @ref CalendarItems.
	 */
	int			CalendarItemsCount;
	/**
	 * Count of todo items stored in @ref ToDoItems.
	 */
	int			ToDoItemsCount;
	/**
	 * Count of contacts items stored in @ref ContactsItems.
	 */
	int			ContactsItemsCount;
	/**
	 * Fields of currently active item.
	 */
	int			CurrentFields[GSM_PHONEBOOK_ENTRIES+1];
	/**
	 * Count of fields listed in @ref CurrentFields.
	 */
	int			CurrentFieldsCount;
	/**
	 * Location of current (eg. which identifies @ref CurrentFieldsCount
	 * and @ref CurrentFields) item.
	 */
	int			CurrentFieldsItem;
	/**
	 * Type of current (eg. which identifies @ref CurrentFieldsCount
	 * and @ref CurrentFields) item.
	 */
	GSM_Alcatel_BinaryType	CurrentFieldsType;

	/**
	 * Listing of categories of current type.
	 */
	int			CurrentCategories[ALCATEL_MAX_CATEGORIES+1];
	/**
	 * Cache for category names of current type. The index here is not
	 * same as in @ref CurrentCategories, it is id of category here.
	 */
	char			CurrentCategoriesCache[ALCATEL_MAX_CATEGORIES+1][(GSM_MAX_CATEGORY_NAME_LENGTH + 1)*2];
	/**
	 * Count of entries in @ref CurrentCategories.
	 */
	int			CurrentCategoriesCount;
	/**
	 * Type of current categories in @ref CurrentCategories etc.
	 */
	GSM_Alcatel_BinaryType	CurrentCategoriesType;

} GSM_Phone_ALCATELData;
#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
