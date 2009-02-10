/* (c) 2002-2005 by Michal Cihar */
/**
 * \file alcatel.c
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * \defgroup AlcatelPhone Alcatel phones communication
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
 *
 * Notes for future features:
 * - max phone number length is 61 (BE5)
 * - max name length is 50 (BE5)
 *
 * @{
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ALCATEL
#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../../misc/misc.h"
#include "../pfunc.h"
#include "../at/atfunc.h"
#include "alcatel.h"

/**
 * Timeout for GSM_WaitFor calls.
 */
#define ALCATEL_TIMEOUT			64

/**
 * \name Some magic numbers for protocol
 * @{
 */

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

/* Various flags */
#define ALCATEL_UNICODE_FLAG		0x80

/*@}*/

extern GSM_Reply_Function ALCATELReplyFunctions[];

/**
 * Alcatel uses some 8-bit characters in contacts, calendar etc.. This table
 * attempts to decode it, it is probably not complete, here are just chars
 * that I found...
 *
 * \todo Some characters are not correctly coded.
 */
unsigned char GSM_AlcatelAlphabet[] =
{
/*	in phone	unicode		description	*/
	0x80,		0x00,0x20,	/* empty	- probably control char */
	0x81,		0x00,0x20,	/* empty	*/
	0x82,		0x00,0x20,	/* empty	*/
	0x83,		0x00,0x20,	/* empty	*/

	0x84,		0x00,0xe7,	/* c cedilla	*/
	0x85,		0x20,0x26,	/* ...		*/
	0x86,		0x03,0xc0,	/* pi		*/
	0x87,		0x01,0x3e,	/* l caron	*/
	0x88,		0x00,0xc0,	/* A grave	*/
	0x89,		0x00,0xc1,	/* A acute	*/
	0x8a,		0x00,0xc2,	/* A circumflex	*/
	0x8b,		0x00,0xc3,	/* A tilde	*/
	0x8c,		0x00,0xc8,	/* E grave	*/
	0x8d,		0x00,0xca,	/* E circumflex	*/
	0x8e,		0x00,0xcb,	/* E diaresis	*/
	0x8f,		0x00,0xcc,	/* I grave	*/
	0x90,		0x00,0xcd,	/* I acute	*/
	0x91,		0x00,0xd0,	/* ETH		*/
	0x92,		0x00,0xd2,	/* O grave	*/
	0x93,		0x00,0xd3,	/* O acute	*/
	0x94,		0x00,0xd4,	/* O circumflex	*/
	0x95,		0x00,0xd5,	/* O tilde	*/
	0x96,		0x00,0xd9,	/* U grave	*/
	0x97,		0x00,0xda,	/* U acute	*/
	0x98,		0x00,0xe1,	/* a acute	*/
	0x99,		0x00,0xe2,	/* a circumflex	*/
	0x9a,		0x00,0xe3,	/* a tilde	*/
	0x9b,		0x00,0xea,	/* e circumflex	*/
	0x9c,		0x00,0xeb,	/* e diaresis	*/
	0x9d,		0x00,0xed,	/* i acute	*/
	0x9e,		0x00,0xee,	/* i circumflex	*/
	0x9f,		0x00,0xef,	/* i diaresis	*/
	0xa0,		0x00,0xf3,	/* o acute	*/
	0xa1,		0x00,0xf4,	/* o circumflex	*/
	0xa2,		0x00,0xf5,	/* o tilde	*/
	0xa3,		0x00,0xfa,	/* u acute	*/
	0xa4,		0x00,0xa2,	/* cent		*/
	0xa5,		0x00,0x5b,	/* [		*/
	0xa6,		0x01,0x59,	/* r caron	*/
	0xa7,		0x01,0x0d,	/* c caron	*/
	0xa8,		0x01,0x61,	/* s caron	*/
	0xa9,		0x01,0x1b,	/* e caron	*/
	0xaa,		0x01,0x6f,	/* u ring	*/
	0xab,		0x00,0xfd,	/* y acute	*/
	0xac,		0x00,0xf0,	/* eth		*/
	0xad,		0x01,0x07,	/* c acute	*/
	0xae,		0x01,0x19,	/* e ogonek	*/
	0xaf,		0x01,0x05,	/* a ogonek	*/
	0xb0,		0x01,0x7c,	/* z dot	*/
	0xb1,		0x01,0x7a,	/* z acute	*/
	0xb2,		0x01,0x5b,	/* s acute	*/
	0xb3,		0x01,0x44,	/* n acute	*/
	0xb4,		0x01,0x42,	/* l stroke	*/

	0xb5,		0x00,0x20,	/* empty	*/

	0xb6,		0x01,0x48,	/* n caron	*/
	0xb7,		0x01,0x65,	/* t caron	*/

	0xb8,		0x00,0x20,	/* empty	*/

	0xb9,		0x01,0x7e,	/* z caron	*/
	0xba,		0x01,0xe7,	/* g caron	*/

	0xbb,		0x00,0x20,	/* empty	*/
	0xbc,		0x00,0x20,	/* empty	*/

	0xbd,		0x1e,0x20,	/* G macron	*/
	0xbe,		0x1e,0x21,	/* g macron	*/
	0xbf,		0x01,0x5e,	/* S cedilla	*/
	0xc0,		0x01,0x5f,	/* s cedilla	*/
	0xc1,		0x01,0x2f,	/* i ogonek	*/ /* Not sure with this, it look like normal i */
	0xc2,		0x01,0x31,	/* i dotless	*/
	0xc3,		0x01,0x68,	/* U tilde	*/
	0xc4,		0x01,0x50,	/* O dbl acute	*/
	0xc5,		0x01,0x69,	/* u tilde	*/
	0xc6,		0x01,0x51,	/* o dbl acute	*/
	0xc7,		0x27,0xa9,	/* =>		*/
	0xc8,		0x27,0xa8,	/* filled =>	*/
	0xc9,		0x00,0xd7,	/* x		*/
	0xca,		0x00,0x5d,	/* ]		*/
	0xcb,		0x26,0x0f,	/* phone	*/
	0xcc,		0x01,0x0f,	/* d caron	*/

	0xcd,		0x00,0x20,	/* empty	*/

	0xce,		0x00,0x7e,	/* ~		*/
	0xcf,		0x00,0x5c,	/* \		*/
	0xd0,		0x00,0x5e,	/* ^		*/

	0xd1,		0x00,0x20,	/* empty	*/

	0xd2,		0x00,0x7b,	/* {		*/
	0xd3,		0x00,0x7c,	/* | 		*/
	0xd4,		0x00,0x7d,	/* }		*/

	0xd5,		0x00,0x20,	/* empty	*/

	0xd6,		0x01,0x63,	/* t cedilla	*/

	0xd7,		0x00,0x20,	/* empty	*/
	0xd8,		0x00,0x20,	/* empty	*/
	0xd9,		0x00,0x20,	/* empty	*/
	0xda,		0x00,0x20,	/* empty	*/
	0xdb,		0x00,0x20,	/* empty	*/
	0xdc,		0x00,0x20,	/* empty	*/
	0xdd,		0x00,0x20,	/* empty	*/
	0xde,		0x00,0x20,	/* empty	*/
	0xdf,		0x00,0x20,	/* empty	*/
	0xe0,		0x00,0x20,	/* empty	*/

	0xe1,		0x00,0x20,	/* two candles	*/ /* Does this have unicode representation? */

	0xe2,		0x00,0x20,	/* empty	*/
	0xe3,		0x00,0x20,	/* empty	*/
	0xe4,		0x00,0x20,	/* empty	*/

	0xe5,		0x01,0xce,	/* a caron	*/
	0xe6,		0x01,0x01,	/* a macron	*/
	0xe7,		0x01,0x13,	/* e macron	*/
	0xe8,		0x01,0x2b,	/* i macron	*/
	0xe9,		0x01,0x4d,	/* o macron	*/
	0xea,		0x01,0x6b,	/* u macron	*/
	0xeb,		0x00,0x41,	/* A		*/
	0xec,		0x00,0x40,	/* @		*/
	0xed,		0x00,0x20,	/* some strange char :-) */ /* Does this have unicode representation? */

	0xee,		0x00,0x20,	/* big key stroken	*/ /* Does this have unicode representation? */
	0xef,		0x00,0x20,	/* big key	*/ /* Does this have unicode representation? */

	0xf0,		0x00,0x20,	/* empty	*/

	0xf1,		0x00,0x31,	/* 1		*/
	0xf2,		0x00,0x21,	/* bold !	*/
	0xf3,		0x26,0x0e,	/* black phone	*/
	0xf4,		0x00,0x26,	/* &		*/
	0xf5,		0x23,0x7e,	/* bell		*/
	0xf6,		0x26,0x6a,	/* note		*/

	0xf7,		0x27,0x13,	/* okay inv	*/ /* Does this have unicode representation? */
	0xf8,		0x27,0x13,	/* okay		*/

	0xf9,		0x00,0x20,	/* empty	*/

	0xfa,		0x00,0x20,	/* key		*/ /* Does this have unicode representation? */

	0xfb,		0x00,0x20,	/* empty	*/

	0xfc,		0x20,0xac,	/* Euro		*/
	0xfd,		0x21,0x97,	/* NE arrow	*/
	0xfe,		0x21,0x98,	/* SE arrow	*/

	0xff,		0x00,0x20,	/* empty	*/

	0x00,		0x00,0x00
};

/**
 * \defgroup AlcatelProto Protocol switching
 * \ingroup AlcatelPhone
 * @{
 */

/**
 * This is being called from atgen.
 */
GSM_Error ALCATEL_ProtocolVersionReply	(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char			*str, *str2;
/*
 * Reply received here looks like:
 * 1 "AT+CPROT=?"
 * 2 "+CPROT: 0,"V1.0",1"
 * 3 "+CPROT: 16,"V1.1",16"
 * 4 "OK"
 */
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
			str = strstr(msg.Buffer, "\"V");
			if (str == NULL) return ERR_UNKNOWNRESPONSE;
			str += 2;
			while((str2 = strstr(str, "\"V")) != NULL) str = str2 + 2;
			if (strncmp(str, "1.0", 3) == 0) {
				s->Phone.Data.Priv.ALCATEL.ProtocolVersion = V_1_0;
			} else if (strncmp(str, "1.1", 3) == 0) {
				s->Phone.Data.Priv.ALCATEL.ProtocolVersion = V_1_1;
			} else {
				smprintf(s, "Unknown protocol version. Please send debug log and phone info to author.\n");
				return ERR_NOTIMPLEMENTED;
			}
			return ERR_NONE;
		case AT_Reply_Error:
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		default:
			return ERR_UNKNOWNRESPONSE;
	}
}

/**
 * Switches phone to Alcatel binary protocol
 */
static GSM_Error ALCATEL_SetBinaryMode(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	if (Priv->Mode == ModeBinary) return ERR_NONE;

	smprintf(s, "Changing to binary mode\n");

	error=GSM_WaitFor (s, "AT+IFC=2,2\r", 11, 0x02, 4, ID_SetFlowControl);
	if (error != ERR_NONE) return error;

	error=GSM_WaitFor (s, "AT+CPROT=?\r", 11, 0x02, 4, ID_AlcatelProtocol);
	if (error != ERR_NONE) return error;

	if (Priv->ProtocolVersion == V_1_0) {
		error=GSM_WaitFor (s, "AT+CPROT=16,\"V1.0\",16\r", 22, 0x00, 4, ID_AlcatelConnect);
	} else {
		error=GSM_WaitFor (s, "AT+CPROT=16,\"V1.1\",16\r", 22, 0x00, 4, ID_AlcatelConnect);
	}

	if (error == ERR_TIMEOUT && s->Speed != 19200) {
		smprintf(s, "HINT: Try changing speed to 19200, it is sometimes needed for Alcatel binary mode.\n");
	}

	if (error != ERR_NONE) return error;

	smprintf(s, "Changing protocol to Alcabus\n");

	s->Protocol.Functions = &ALCABUSProtocol;
	error = s->Protocol.Functions->Initialise(s);
	if (error != ERR_NONE) {
		s->Protocol.Functions = &ATProtocol;
		return error;
	}
	s->Phone.Functions->ReplyFunctions	= ALCATELReplyFunctions;
	Priv->Mode				= ModeBinary;
	Priv->BinaryItem			= 0;
	Priv->BinaryType			= 0;
	Priv->BinaryState			= StateAttached;
	return ERR_NONE;
}

/**
 * Changes state in binary protocol.
 */
static GSM_Error ALCATEL_GoToBinaryState(GSM_StateMachine *s, GSM_Alcatel_BinaryState state, GSM_Alcatel_BinaryType type, int item) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char		attach_buffer[] = {0x00, 0x00, 0x7C ,0x20};
	unsigned char		detach_buffer[] = {0x00, 0x01, 0x7C ,0x00};
	unsigned char		start_buffer[] =
					{0x00, 0x04, 0x7C, 0x80, /* 4 byte database id follows */
					 0x12, 0x34, 0x56, 0x78};
	unsigned char		end_buffer[] =
					{0x00, 0x04, 0x7C, 0x82,
					 0x00,			/* type */
					 0x00, 0x00, 0x00, 0x00}; /* TimeStamp */
	unsigned char		close_buffer[] =
					{0x00, 0x04,
					 0x00, 			 /*type */
					 0x23, 0x01};
	unsigned char		select1_buffer[] =
					{0x00, 0x00,
					 0x00, 			 /*type */
					 0x20};
	unsigned char		select2_buffer[] =
					{0x00, 0x04,
					 0x00, 			 /*type */
					 0x22, 0x01, 0x00};
	unsigned char		begin_buffer[] =
					{0x00, 0x04, 0x7C, 0x81,
					 0x00,			 /*type */
					 0x00, 0x85, 0x00};
	unsigned char		commit_buffer[] =
					{0x00, 0x04,
					 0x00, 			/*type */
					 0x20, 0x01};

	smprintf(s, "Alcatel state switcher: %d -> %d, %d -> %d, %d -> %d\n", Priv->BinaryState, state, Priv->BinaryType, type, Priv->BinaryItem, item);
	error = ALCATEL_SetBinaryMode(s);
	if (error != ERR_NONE) return error;

	/* Do we need to do anything? */
	if ((state == Priv->BinaryState) && (type == Priv->BinaryType) && (item == Priv->BinaryItem)) return ERR_NONE;

	/* We're editing, but the next state is not the same. so commit editing */
	if (Priv->BinaryState == StateEdit) {
		/* Something has changed, we will have to reread fields! */
		Priv->CurrentFieldsItem  = -1;
		switch (Priv->BinaryType) {
			case TypeCalendar:
				commit_buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
				break;
			case TypeContacts:
				commit_buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
				break;
			case TypeToDo:
				commit_buffer[2] = ALCATEL_SYNC_TYPE_TODO;
				break;
		}
		smprintf(s, "Commiting edited record\n");
		error=GSM_WaitFor (s, commit_buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelCommit);
		if (error != ERR_NONE) return error;
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelCommit2);
		if (error != ERR_NONE) return error;
		Priv->BinaryState = StateSession;
		Priv->BinaryItem = 0;
	}

	/* Do we want to edit something of same type? */
	if ((state == StateEdit) && (type == Priv->BinaryType)) {
		/* Edit state doesn't need any switching, it is needed only for
		 * indication that e have to commit record before we switch to other
		 * mode.
		 */
		Priv->BinaryState = StateEdit;
		Priv->BinaryItem = item;
		return ERR_NONE;
	}

	/* Now we can be only in Attached or Session state, so if states and types matches, just keep them as they are */
	if ((state == Priv->BinaryState) && (type == Priv->BinaryType)) {
		return ERR_NONE;
	}

	/* Do we need to close session? */
	if (Priv->BinaryState == StateSession) {
		smprintf(s, "Ending session\n");
		switch (Priv->BinaryType) {
			case TypeCalendar:
				end_buffer[4] 	= ALCATEL_BEGIN_SYNC_CALENDAR;
				break;
			case TypeContacts:
				end_buffer[4] 	= ALCATEL_BEGIN_SYNC_CONTACTS;
				break;
			case TypeToDo:
				end_buffer[4] 	= ALCATEL_BEGIN_SYNC_TODO;
				break;
		}
		error=GSM_WaitFor (s, end_buffer, 9, 0x02, ALCATEL_TIMEOUT, ID_AlcatelEnd);
		if (error != ERR_NONE) return error;

		switch (Priv->BinaryType) {
			case TypeCalendar:
				close_buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
				break;
			case TypeContacts:
				close_buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
				break;
			case TypeToDo:
				close_buffer[2] = ALCATEL_SYNC_TYPE_TODO;
				break;
		}
		smprintf(s, "Closing session\n");
		error=GSM_WaitFor (s, close_buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelClose);
		if (error != ERR_NONE) return error;

		smprintf(s, "Detaching binary mode\n");
		error = GSM_WaitFor (s, detach_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelDetach);
		if (error != ERR_NONE) return error;

		Priv->BinaryState = StateAttached;
		Priv->BinaryType = 0;
	}

	/* Do we need to open session? */
	if (state == StateSession || state == StateEdit) {
		smprintf(s, "Starting session for %s\n",
				(type == TypeCalendar ? "Calendar" :
				(type == TypeToDo ? "Todo" :
				(type == TypeContacts ? "Contacts" :
				"Unknown!"))));
		/* Fill up buffers */
		switch (type) {
			case TypeCalendar:
				select1_buffer[2] 	= ALCATEL_SYNC_TYPE_CALENDAR;
				select2_buffer[2] 	= ALCATEL_SYNC_TYPE_CALENDAR;
				begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_CALENDAR;
				break;
			case TypeContacts:
				select1_buffer[2] 	= ALCATEL_SYNC_TYPE_CONTACTS;
				select2_buffer[2] 	= ALCATEL_SYNC_TYPE_CONTACTS;
				begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_CONTACTS;
				break;
			case TypeToDo:
				select1_buffer[2] 	= ALCATEL_SYNC_TYPE_TODO;
				select2_buffer[2] 	= ALCATEL_SYNC_TYPE_TODO;
				begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_TODO;
				break;
		}
		smprintf(s, "Attaching in binary mode\n");

		/* Communicate */
		error=GSM_WaitFor (s, attach_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelAttach);
		if (error != ERR_NONE) return error;

		smprintf(s,"Start session\n");
		error=GSM_WaitFor (s, start_buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelStart);
		if (error != ERR_NONE) return error;

		smprintf(s,"Select type\n");
		error=GSM_WaitFor (s, select1_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelSelect1);
		if (error != ERR_NONE) return error;
		error=GSM_WaitFor (s, select2_buffer, 6, 0x02, ALCATEL_TIMEOUT, ID_AlcatelSelect2);
		if (error != ERR_NONE) return error;
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelSelect3);
		if (error != ERR_NONE) return error;

		smprintf(s,"Begin transfer\n");
		error=GSM_WaitFor (s, begin_buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelBegin1);
		if (error != ERR_NONE) return error;
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelBegin2);
		if (error != ERR_NONE) return error;

		Priv->BinaryState = StateSession;
		Priv->BinaryType = type;
		/* Do we want to edit something of same type? */
		if ((state == StateEdit) && (type == Priv->BinaryType)) {
			Priv->BinaryState = StateEdit;
			Priv->BinaryItem = item;
			return ERR_NONE;
		}
	}
	return ERR_NONE;
}

/**
 * Switches back to AT mode.
 */
static GSM_Error ALCATEL_SetATMode(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	if (Priv->Mode == ModeAT) return ERR_NONE;

	error = ALCATEL_GoToBinaryState(s, StateAttached, 0, 0);
	if (error != ERR_NONE) return error;

	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	smprintf(s, "Changing protocol to AT\n");
	s->Protocol.Functions			= &ATProtocol;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;
	Priv->Mode				= ModeAT;
	s->Phone.Data.Priv.ATGEN.Charset	= 0;
	s->Phone.Data.Priv.ATGEN.PBKMemory	= 0;

	usleep(100000);

	/* In case we don't send AT command short after closing binary mode,
	 * phone takes VERY long to react next time. The error code in
	 * intetionally ignored.
	 */
	error = GSM_WaitFor (s, "AT\r", 3, 0x00, 0, ID_IncomingFrame);

	return ERR_NONE;
}

/*@}*/

/**
 * Initialisation of Alcatel module
 */
static GSM_Error ALCATEL_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	Priv->Mode				= ModeAT;

	Priv->CalendarItems			= NULL;
	Priv->ContactsItems			= NULL;
	Priv->ToDoItems				= NULL;
	Priv->CalendarItemsCount		= 0;
	Priv->ToDoItemsCount			= 0;
	Priv->ContactsItemsCount		= 0;
	Priv->CurrentFields[0]			= 0;
	Priv->CurrentFieldsCount		= 0;
	Priv->CurrentFieldsItem			= 0;
	Priv->CurrentFieldsType			= 0;
	Priv->ProtocolVersion			= V_1_0;
	Priv->CurrentFieldsItem			= -1;

	Priv->CurrentCategoriesCount		= 0;
	Priv->CurrentCategoriesType		= 0;

	s->Protocol.Functions			= &ATProtocol;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;

	if (ATGEN_Initialise(s) != ERR_NONE || GSM_WaitFor (s, "AT\r", 3, 0x00, 2, ID_IncomingFrame) != ERR_NONE) {
		smprintf(s,"AT initialisation failed, trying to stop binary mode...\n");
		s->Protocol.Functions		= &ALCABUSProtocol;
		error = s->Protocol.Functions->Terminate(s);
		s->Protocol.Functions		= &ATProtocol;

		error = ATGEN_Initialise(s);
		if (error != ERR_NONE) return error;
	}

	return ERR_NONE;
}

static GSM_Error ALCATEL_Terminate(GSM_StateMachine *s)
{
	GSM_Error 		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	free(Priv->CalendarItems);
	free(Priv->ContactsItems);
	free(Priv->ToDoItems);
	error = ALCATEL_SetATMode(s);
	if (error != ERR_NONE) return error;
	return ATGEN_Terminate(s);
}

/**
 * Finds whether id is set in the phone
 */
static GSM_Error ALCATEL_IsIdAvailable(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;

	if (id > ALCATEL_MAX_LOCATION) return ERR_INVALIDLOCATION;

	switch (Priv->BinaryType) {
		case TypeCalendar:
			Priv->CurrentList 	= &(Priv->CalendarItems);
			Priv->CurrentCount 	= &(Priv->CalendarItemsCount);
			break;
		case TypeContacts:
			Priv->CurrentList 	= &(Priv->ContactsItems);
			Priv->CurrentCount 	= &(Priv->ContactsItemsCount);
			break;
		case TypeToDo:
			Priv->CurrentList 	= &(Priv->ToDoItems);
			Priv->CurrentCount 	= &(Priv->ToDoItemsCount);
			break;
	}

	for (i=0; i<*Priv->CurrentCount; i++) {
		if ((*Priv->CurrentList)[i] == id) return ERR_NONE;
	}

	return ERR_EMPTY;
}

/**
 * Finds next id that is available in the phone
 */
static GSM_Error ALCATEL_GetNextId(GSM_StateMachine *s, int *id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i = 0;
	int			next = ALCATEL_MAX_LOCATION;

	switch (Priv->BinaryType) {
		case TypeCalendar:
			Priv->CurrentList 	= &(Priv->CalendarItems);
			Priv->CurrentCount 	= &(Priv->CalendarItemsCount);
			break;
		case TypeContacts:
			Priv->CurrentList 	= &(Priv->ContactsItems);
			Priv->CurrentCount 	= &(Priv->ContactsItemsCount);
			break;
		case TypeToDo:
			Priv->CurrentList 	= &(Priv->ToDoItems);
			Priv->CurrentCount 	= &(Priv->ToDoItemsCount);
			break;
	}

	for (i=0; i<*Priv->CurrentCount; i++) {
		if (((*Priv->CurrentList)[i] > *id) && ((*Priv->CurrentList)[i] < next )) {
			next = (*Priv->CurrentList)[i];
		}
	}
	if (next == ALCATEL_MAX_LOCATION) {
		return ERR_EMPTY;
	} else {
		*id = next;
		return ERR_NONE;
	}
}

static GSM_Error ALCATEL_ReplyGetIds(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			count,i,pos;

	count 		 = msg.Buffer[10];
	*Priv->CurrentCount += count;

	*Priv->CurrentList = (int *)realloc(*Priv->CurrentList, (*Priv->CurrentCount + 1)* sizeof(int));
	if (*Priv->CurrentList == NULL) return ERR_MOREMEMORY;

	for (i = 0; i < count; i++) {
		pos = 11 + (4 * i);
		(*Priv->CurrentList)[*Priv->CurrentCount - count + i] = msg.Buffer[pos + 3] +
							(msg.Buffer[pos + 2] << 8) +
							(msg.Buffer[pos + 1] << 16) +
							(msg.Buffer[pos] << 24);
	}
	(*Priv->CurrentList)[*Priv->CurrentCount] = 0;

	/* If last byte is 0, then we transmitted all items */
	Priv->TransferCompleted = msg.Buffer[4 + msg.Buffer[4]] == 0;
	return ERR_NONE;
}

/**
 * Lists available ids from phone.
 */
static GSM_Error ALCATEL_GetAvailableIds(GSM_StateMachine *s, bool refresh)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	int			i;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 			/*type */
					 0x2F, 0x01};

	if (Priv->BinaryState != StateSession) return ERR_UNKNOWN;

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2]		= ALCATEL_SYNC_TYPE_CALENDAR;
			Priv->CurrentList	= &(Priv->CalendarItems);
			Priv->CurrentCount	= &(Priv->CalendarItemsCount);
			break;
		case TypeContacts:
			buffer[2]		= ALCATEL_SYNC_TYPE_CONTACTS;
			Priv->CurrentList	= &(Priv->ContactsItems);
			Priv->CurrentCount	= &(Priv->ContactsItemsCount);
			break;
		case TypeToDo:
			buffer[2]		= ALCATEL_SYNC_TYPE_TODO;
			Priv->CurrentList	= &(Priv->ToDoItems);
			Priv->CurrentCount	= &(Priv->ToDoItemsCount);
			break;
	}

	if (*Priv->CurrentList != NULL) {
		if (!refresh) return ERR_NONE;
		free(*Priv->CurrentList);
		*Priv->CurrentList = NULL;
	}
	smprintf(s,"Reading items list\n");

	*Priv->CurrentCount = 0;
	Priv->TransferCompleted = false;

	error=GSM_WaitFor (s, buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetIds1);
	if (error != ERR_NONE) return error;

	while (!Priv->TransferCompleted) {
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetIds2);
		if (error != ERR_NONE) return error;
	}

	i = 0;
	smprintf(s,"Received %d ids: ", *Priv->CurrentCount);
	for (i=0; i < *Priv->CurrentCount; i++) {
		smprintf(s,"%x ", (*Priv->CurrentList)[i]);
	}
	smprintf(s,"\n");

	return ERR_NONE;
}

static GSM_Error ALCATEL_ReplyGetFields(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i;

	if (msg.Buffer[14] > GSM_PHONEBOOK_ENTRIES) {
		smprintf(s, "WARNING: Field list truncated, you should increase GSM_PHONEBOOK_ENTRIES to at least %d\n", msg.Buffer[14]);
		Priv->CurrentFieldsCount = GSM_PHONEBOOK_ENTRIES;
	} else {
		Priv->CurrentFieldsCount = msg.Buffer[14];
	}

	Priv->CurrentFields[Priv->CurrentFieldsCount] = 0;

	for (i = 0; i < Priv->CurrentFieldsCount; i++) {
		Priv->CurrentFields[i] = msg.Buffer[15 + i];
	}

	return ERR_NONE;
}

static GSM_Error ALCATEL_GetFields(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	int			i;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 			/* type */
					 0x30, 0x01,
					 0x00, 0x00, 0x00, 0x00}; /* item */

	if (Priv->BinaryState != StateSession) return ERR_UNKNOWN;
	if ((Priv->CurrentFieldsItem == id) && (Priv->CurrentFieldsType == Priv->BinaryType)) return ERR_NONE;

	smprintf(s,"Reading item fields (%d)\n", id);

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}

	Priv->CurrentFieldsItem = id;
	Priv->CurrentFieldsType = Priv->BinaryType;

	error=GSM_WaitFor (s, buffer, 9, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetFields1);
	if (error != ERR_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetFields2);
	if (error != ERR_NONE) return error;

	i = 0;
	smprintf(s,"Received %d fields: ", Priv->CurrentFieldsCount);
	for (i=0; i < Priv->CurrentFieldsCount; i++) {
		smprintf(s,"%x ", Priv->CurrentFields[i]);
	}
	smprintf(s,"\n");

	return ERR_NONE;
}

static void ALCATEL_DecodeString(GSM_StateMachine *s, unsigned const char *buffer, unsigned char *target, int maxlen)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			len;

	len = buffer[0];
	if (Priv->ProtocolVersion == V_1_1 && (buffer[1] == ALCATEL_UNICODE_FLAG)) {
		/* UCS-2-BE string */
		if (GSM_PHONEBOOK_TEXT_LENGTH < len/2) {
			smprintf(s, "WARNING: Text truncated, to %d from %d\n", maxlen, len/2 + 1);
			len = GSM_PHONEBOOK_TEXT_LENGTH * 2;
		}
		memcpy(target, buffer + 2, len);
		Priv->ReturnString[len + 1] = 0;
		Priv->ReturnString[len + 2] = 0;
	} else {
		/* Alcatel alphabet string */
		if (GSM_PHONEBOOK_TEXT_LENGTH < len) {
			smprintf(s, "WARNING: Text truncated, to %d from %d\n", maxlen, len + 1);
			len = GSM_PHONEBOOK_TEXT_LENGTH;
		}
		DecodeDefault(target, buffer + 1, len, false, GSM_AlcatelAlphabet);
	}
}

static GSM_Error ALCATEL_EncodeString(GSM_StateMachine *s, unsigned const char *buffer, unsigned char *target, GSM_Alcatel_FieldType type)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	size_t			len;
	size_t			maxlen = 0;
	bool			unicode = false;
	unsigned char		text[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		utext[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];

	len = UnicodeLength(buffer);
	/* Maximal length is item and phone dependant, we use it only for guessing as phone will report too big entry correctly. */
	if (type == Alcatel_string) {
		if (Priv->ProtocolVersion == V_1_1) {
			maxlen = 151;
		} else {
			maxlen = 62;
		}
	} else if (type == Alcatel_phone) {
		if (Priv->ProtocolVersion == V_1_1) {
			maxlen = 61;
		} else {
			maxlen = 50;
		}
	} else if (type == 0) {
		/* We're creating category */
		maxlen = 60;
	}

	if (Priv->ProtocolVersion == V_1_1) {

		/* Compare if we would loose some information when not using
		 * unicode */
		EncodeDefault(text, buffer, &len, true, GSM_AlcatelAlphabet);
		DecodeDefault(utext, text, len, true, GSM_AlcatelAlphabet);
		if (!mywstrncmp(utext, buffer, len)) {
			/* Use unicode only when we have enough space */
			unicode = ((len * 2) < maxlen);
		}
	}

	if (unicode) {
		/* UCS-2-BE string */
		target[0] = len * 2 + 3;
		target[1] = ALCATEL_UNICODE_FLAG;
		memcpy(target + 2, buffer, len * 2 + 2);
	} else {
		/* Alcatel alphabet string */
		EncodeDefault(target + 1, buffer, &len, true, GSM_AlcatelAlphabet);
		target[0] = len;
		target[len + 1] = 0;
	}

	return ERR_NONE;
}

/**
 * Parses field value.
 *
 * \todo Timezone is not correctly set.
 */
static GSM_Error ALCATEL_ReplyGetFieldValue(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	unsigned char 		*buffer = &(msg.Buffer[16]);

	if (buffer[1] == 0x05 && buffer[2] == 0x67) {
		/* date */
		Priv->ReturnType		= Alcatel_date;
		Priv->ReturnDateTime.Day	= buffer[4];
		Priv->ReturnDateTime.Month	= buffer[5];
		Priv->ReturnDateTime.Year	= buffer[7] + (buffer[6] << 8);
		Priv->ReturnDateTime.Timezone	= 0; /* how to acquire this? */

		Priv->ReturnDateTime.Hour	= 0;
		Priv->ReturnDateTime.Minute	= 0;
		Priv->ReturnDateTime.Second	= 0;
	} else if (buffer[1] == 0x06 && buffer[2] == 0x68) {
		/* time */
		Priv->ReturnType		= Alcatel_time;
		Priv->ReturnDateTime.Hour	= buffer[4];
		Priv->ReturnDateTime.Minute	= buffer[5];
		Priv->ReturnDateTime.Second	= buffer[6];

		Priv->ReturnDateTime.Day	= 0;
		Priv->ReturnDateTime.Month	= 0;
		Priv->ReturnDateTime.Year	= 0;
		Priv->ReturnDateTime.Timezone	= 0;
	} else if (buffer[1] == 0x08 && buffer[2] == 0x3C) {
		/* string */
		Priv->ReturnType = Alcatel_string;
		ALCATEL_DecodeString(s, buffer + 3, Priv->ReturnString, GSM_PHONEBOOK_TEXT_LENGTH);
	} else if (buffer[1] == 0x07 && buffer[2] == 0x3C) {
		/* phone */
		Priv->ReturnType = Alcatel_phone;
		ALCATEL_DecodeString(s, buffer + 3, Priv->ReturnString, GSM_PHONEBOOK_TEXT_LENGTH);
	} else if (buffer[1] == 0x03 && buffer[2] == 0x3B) {
		/* boolean */
		Priv->ReturnType = Alcatel_bool;
		Priv->ReturnInt = buffer[3];
	} else if (buffer[1] == 0x02 && buffer[2] == 0x3A) {
		/* integer */
		Priv->ReturnType = Alcatel_int;
		Priv->ReturnInt = buffer[6] + (buffer[5] << 8) + (buffer[4] << 16) + (buffer[3] << 24);
	} else if (buffer[1] == 0x04 && buffer[2] == 0x38) {
		/* enumeration */
		Priv->ReturnType 	= Alcatel_enum;
		Priv->ReturnInt 	= buffer[3];
	} else if (buffer[1] == 0x00 && buffer[2] == 0x38) {
		/* byte */
		Priv->ReturnType 	= Alcatel_byte;
		Priv->ReturnInt 	= buffer[3];
	} else {
		smprintf(s, "WARNING: Uknown data type received (%02X,%02X)\n", buffer[1], buffer[2]);
		return ERR_UNKNOWNRESPONSE;
	}
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetFieldValue(GSM_StateMachine *s, int id, int field)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 				/* type */
					 0x1f, 0x01,
					 0x00, 0x00, 0x00, 0x00,	/* here follows 4byte id */
					 0x00};				/* field */

	smprintf(s,"Reading item value (%08x.%02x)\n", id, field);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);
	buffer[9] = (field & 0xff);

	error=GSM_WaitFor (s, buffer, 10, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetFieldValue1);
	if (error != ERR_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetFieldValue2);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}

static GSM_Error ALCATEL_ReplyGetCategories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i;

	/* Did we get any category? */
	if (msg.Buffer[4] == 6) {
		Priv->CurrentCategoriesCount = 0;
		return ERR_NONE;
	}
	if (msg.Buffer[12] > ALCATEL_MAX_CATEGORIES) {
		smprintf(s, "WARNING: Field list truncated, you should increase ALCATEL_MAX_CATEGORIES to at least %d\n", msg.Buffer[12]);
		Priv->CurrentCategoriesCount = ALCATEL_MAX_CATEGORIES;
	} else {
		Priv->CurrentCategoriesCount = msg.Buffer[12];
	}

	for (i = 0; i < Priv->CurrentCategoriesCount; i++) {
		Priv->CurrentCategories[i] = msg.Buffer[13 + i];
		Priv->CurrentCategoriesCache[i][0] = '\000';
		Priv->CurrentCategoriesCache[i][1] = '\000';
	}

	return ERR_NONE;
}

static GSM_Error ALCATEL_GetAvailableCategoryIds(GSM_StateMachine *s) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	int		 	i;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00 /*type */,
					 0x0b,
					 0x00 /* list */};

	if (Priv->BinaryState != StateSession) return ERR_UNKNOWN;
	if (Priv->CurrentCategoriesType == Priv->BinaryType) return ERR_NONE;
	switch (Priv->BinaryType) {
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			buffer[4] = ALCATEL_LIST_CONTACTS_CAT;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			buffer[4] = ALCATEL_LIST_TODO_CAT;
			break;
		default:
			return ERR_NOTSUPPORTED;
	}

	Priv->CurrentCategoriesType = Priv->BinaryType;

	smprintf(s,"Reading category list\n");

	error=GSM_WaitFor (s, buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetCategories1);
	if (error != ERR_NONE) return error;

	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetCategories2);
	if (error != ERR_NONE) return error;

	i = 0;
	smprintf(s,"Received %d ids: ", Priv->CurrentCategoriesCount);
	for (i=0; i < Priv->CurrentCategoriesCount; i++) {
		smprintf(s,"%i ", Priv->CurrentCategories[i]);
	}
	smprintf(s,"\n");

	return ERR_NONE;
}

static GSM_Error ALCATEL_IsCategoryIdAvailable(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i = 0;

	if (Priv->CurrentCategoriesType != Priv->BinaryType) return ERR_UNKNOWN;

	for (i = 0; i< Priv->CurrentCategoriesCount; i++) {
		if (Priv->CurrentCategories[i] == id) return ERR_NONE;
	}
	return ERR_EMPTY;
}

static GSM_Error ALCATEL_ReplyAddCategoryText(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;

	Priv->ReturnInt = msg.Buffer[12];

	return ERR_NONE;
}

static GSM_Error ALCATEL_AddCategoryText(GSM_StateMachine *s, const unsigned char *str) {
	unsigned char		buffer[200] = {0x00, 0x04, 0x00 /*type*/, 0x0d, 0x00 /*list*/, 0x0b };
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;


	smprintf(s,"Creating category\n");
	ALCATEL_EncodeString(s, str, buffer + 7, 0);
	buffer[6] = buffer[7] + 1;

	switch (Priv->BinaryType) {
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			buffer[4] = ALCATEL_LIST_CONTACTS_CAT;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			buffer[4] = ALCATEL_LIST_TODO_CAT;
			break;
		default:
			return ERR_NOTSUPPORTED;
	}

	error=GSM_WaitFor (s, buffer, 8 + buffer[7], 0x02, ALCATEL_TIMEOUT, ID_AlcatelAddCategoryText1);
	if (error != ERR_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelAddCategoryText2);
	if (error != ERR_NONE) return error;

	/* Refresh list */
	Priv->CurrentCategoriesType = 0;
	return ALCATEL_GetAvailableCategoryIds(s);
}

static GSM_Error ALCATEL_ReplyGetCategoryText(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;

	ALCATEL_DecodeString(s,msg.Buffer + 14, Priv->ReturnString, GSM_MAX_CATEGORY_NAME_LENGTH);

	return ERR_NONE;
}

static GSM_Error ALCATEL_GetCategoryText(GSM_StateMachine *s, int id) {
	unsigned char		buffer[] = {0x00, 0x04, 0x00 /*type*/, 0x0c, 0x00 /*list*/, 0x0A, 0x01, 0x00 /*item*/ };
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	if (Priv->CurrentCategoriesCache[id][0] != '\000' || Priv->CurrentCategoriesCache[id][1] != '\000') {
		CopyUnicodeString(Priv->ReturnString, Priv->CurrentCategoriesCache[id]);
		return ERR_NONE;
	}

	smprintf(s,"Reading category %d\n", id);

	switch (Priv->BinaryType) {
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			buffer[4] = ALCATEL_LIST_CONTACTS_CAT;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			buffer[4] = ALCATEL_LIST_TODO_CAT;
			break;
		default:
			return ERR_NOTSUPPORTED;
	}

	buffer[7] = (id & 0xff);

	error=GSM_WaitFor (s, buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetCategoryText1);
	if (error != ERR_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetCategoryText2);
	if (error != ERR_NONE) return error;

	CopyUnicodeString(Priv->CurrentCategoriesCache[id], Priv->ReturnString);

	return ERR_NONE;
}

static GSM_Error ALCATEL_DeleteField(GSM_StateMachine *s, int id, int field) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 				/* type */
					 0x26, 0x01,
					 0x00, 0x00, 0x00, 0x00,	/* here follows 4byte id */
					 0x65, 0x01,
					 0x00,				/* field */
					 0x01};

	smprintf(s,"Deleting field (%08x.%02x)\n", id, field);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);
	buffer[11] = (field & 0xff);

	error=GSM_WaitFor (s, buffer, 13, 0x02, ALCATEL_TIMEOUT, ID_AlcatelDeleteField);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}

static GSM_Error ALCATEL_DeleteItem(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 				/* type */
					 0x27, 0x01,
					 0x00, 0x00, 0x00, 0x00,	/* here follows 4byte id */
					 0x42};

	smprintf(s,"Deleting item (%08x)\n", id);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);

	error=GSM_WaitFor (s, buffer, 10, 0x02, ALCATEL_TIMEOUT, ID_AlcatelDeleteItem1);
	if (error != ERR_NONE) return error;

	error=GSM_WaitFor (s, 0, 0, 0x0, ALCATEL_TIMEOUT, ID_AlcatelDeleteItem2);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}

static GSM_Error ALCATEL_ReplyDeleteItem(GSM_Protocol_Message msg, GSM_StateMachine *s UNUSED)
{
	if (msg.Buffer[8] != 0x25) return ERR_UNKNOWNRESPONSE;
	return ERR_NONE;
}

static GSM_Error ALCATEL_BuildWriteBuffer(GSM_StateMachine *s, unsigned char * buffer, GSM_Alcatel_FieldType type, int field, void *data) {
	GSM_Error		error;

	buffer[1] = field & 0xff;

	switch(type) {
		case Alcatel_date:
			if (!CheckDate((GSM_DateTime *)data)) return ERR_INVALIDDATETIME;

			buffer[3] = 0x05;
			buffer[4] = 0x67;

			buffer[0] = 0x09;
			buffer[5] = 0x04;
			buffer[6] = ((GSM_DateTime *)data)->Day & 0xff;
			buffer[7] = ((GSM_DateTime *)data)->Month & 0xff;
			buffer[8] = ((GSM_DateTime *)data)->Year >> 8;
			buffer[9] = ((GSM_DateTime *)data)->Year & 0xff;
			buffer[10] = 0x00;
			break;
		case Alcatel_time:
			if (!CheckTime((GSM_DateTime *)data)) return ERR_INVALIDDATETIME;

			buffer[3] = 0x06;
			buffer[4] = 0x68;

			buffer[0] = 0x08;
			buffer[5] = 0x03;
			buffer[6] = ((GSM_DateTime *)data)->Hour & 0xff;
			buffer[7] = ((GSM_DateTime *)data)->Minute & 0xff;
			buffer[8] = ((GSM_DateTime *)data)->Second & 0xff;
			buffer[9] = 0x00;
			break;
		case Alcatel_string:
			buffer[3] = 0x08;
			buffer[4] = 0x3c;

			error = ALCATEL_EncodeString(s, (char *)data, buffer + 5, Alcatel_string);
			if (error != ERR_NONE) return error;
			buffer[0] = 5 + buffer[5];
			break;
		case Alcatel_phone:
			buffer[3] = 0x07;
			buffer[4] = 0x3c;

			error = ALCATEL_EncodeString(s, (char *)data, buffer + 5, Alcatel_phone);
			if (error != ERR_NONE) return error;
			buffer[0] = 5 + buffer[5];
			break;
		case Alcatel_enum:
			buffer[3] = 0x04;
			buffer[4] = 0x38;

			buffer[0] = 0x05;
			buffer[5] = *(int *)data & 0xff;
			buffer[6] = 0x00;
			break;
		case Alcatel_bool:
			buffer[3] = 0x03;
			buffer[4] = 0x3b;

			buffer[0] = 0x05;
			buffer[5] = *(int *)data & 0xff;
			buffer[6] = 0x00;
			break;
		case Alcatel_int:
			buffer[3] = 0x02;
			buffer[4] = 0x3a;

			buffer[0] = 0x08;
			buffer[5] = *(unsigned int *)data >> 24;
			buffer[6] = (*(unsigned int *)data >> 16) & 0xff;
			buffer[7] = (*(unsigned int *)data >> 8) & 0xff;
			buffer[8] = *(unsigned int *)data & 0xff;
			buffer[9] = 0x00;
			break;
		case Alcatel_byte:
			buffer[3] = 0x00;
			buffer[4] = 0x38;

			buffer[0] = 0x05;
			buffer[5] = *(int *)data & 0xff;
			buffer[6] = 0x00;
			break;
	}
	return ERR_NONE;
}

static GSM_Error ALCATEL_CreateField(GSM_StateMachine *s, GSM_Alcatel_FieldType type, int field, void *data) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char 		buffer[200] =
					{0x00, 0x04,
					 0x00, 				/* type */
					 0x25, 0x01, 0x65,
					 0x00,				/* length of remaining part */
					 0x00,				/* field */
					 0x37};				/* data follows here */

	smprintf(s,"Creating field (%02x)\n", field);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}
	error = ALCATEL_BuildWriteBuffer(s, buffer + 6, type, field, data);
	if (error != ERR_NONE) return error;

	error = GSM_WaitFor (s, buffer, 8 + buffer[6], 0x02, ALCATEL_TIMEOUT, ID_AlcatelCreateField);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}

static GSM_Error ALCATEL_UpdateField(GSM_StateMachine *s, GSM_Alcatel_FieldType type, int id, int field, void *data) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char 		buffer[50 + GSM_PHONEBOOK_TEXT_LENGTH] =
					{0x00, 0x04,
					 0x00, 				/* type */
					 0x26, 0x01,
					 0x00, 0x00, 0x00, 0x00,	/* id */
					 0x65,
					 0x00,				/* length of remaining part */
					 0x00,				/* field */
					 0x37};				/* data follows here */

	smprintf(s,"Updating field (%08x.%02x)\n", id, field);

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}
	error = ALCATEL_BuildWriteBuffer(s, buffer + 10, type, field, data);
	if (error != ERR_NONE) return error;

	error = GSM_WaitFor (s, buffer, 12 + buffer[10], 0x02, ALCATEL_TIMEOUT, ID_AlcatelUpdateField);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}

static GSM_Error ALCATEL_GetManufacturer(GSM_StateMachine *s)
{
	strcpy(s->Phone.Data.Manufacturer, "Alcatel");
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetIMEI (GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetIMEI(s);
}

static GSM_Error ALCATEL_GetFirmware(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetFirmware(s);
}

static GSM_Error ALCATEL_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetModel(s);
}

static GSM_Error ALCATEL_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetDateTime(s, date_time);
}

static GSM_Error ALCATEL_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i;
	int			j = 0;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
		if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= ERR_NONE) {
			entry->EntriesNum = 0;
			return error;
		}
		if ((error = ALCATEL_GetFields(s, entry->Location))!= ERR_NONE) return error;

		entry->EntriesNum = Priv->CurrentFieldsCount;

		for (i=0; i<Priv->CurrentFieldsCount; i++) {
			if ((error = ALCATEL_GetFieldValue(s, entry->Location, Priv->CurrentFields[i]))!= ERR_NONE) return error;
			entry->Entries[i].VoiceTag   = 0;
			entry->Entries[i].SMSList[0] = 0;
			switch (Priv->CurrentFields[i]) {
				case 0:
					if (Priv->ReturnType != Alcatel_string) {
					smprintf(s,"WARNING: Received unexpected type %02X for field 0, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_LastName;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 1:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 1, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_FirstName;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 2:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 2, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Company;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 3:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 3, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_JobTitle;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 4:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 4, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Note;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 5:
					if (Priv->ReturnType != Alcatel_byte) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 5, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Category;
					entry->Entries[i - j].Number = Priv->ReturnInt;
					break;
				case 6:
					if (Priv->ReturnType != Alcatel_bool) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 6, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Private;
					entry->Entries[i - j].Number = Priv->ReturnInt;
					break;
				case 7:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 7, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Work;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 8:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 8, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_General;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 9:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 9, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Fax;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 10:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 10, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Other;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 11:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 11, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Pager;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 12:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 12, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Mobile;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 13:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 13, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Home;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 14:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 14, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Email;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 15:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 15, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Email2;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 16:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 16, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_StreetAddress;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 17:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 17, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_City;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 18:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 18, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_State;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 19:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 19, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Zip;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 20:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 20, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Country;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 21:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 21, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom1;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 22:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 22, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom2;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 23:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 23, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom3;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 24:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 24, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom4;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 25:
					if (Priv->ReturnType != Alcatel_int) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 25, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					if (Priv->ReturnInt != 0) {
						entry->Entries[i - j].EntryType = PBK_PictureID;
						entry->Entries[i - j].Number = Priv->ReturnInt;
					} else {
						entry->EntriesNum--;
						j++;
					}
					break;
				default:
					entry->EntriesNum--;
					j++;
					smprintf(s,"WARNING: Received unknown field %02X, ignoring. Type = %02X. Value = ", Priv->CurrentFields[i], Priv->ReturnType);
					switch (Priv->ReturnType) {
						case Alcatel_date:
							smprintf(s, "%d.%d.%d", Priv->ReturnDateTime.Day, Priv->ReturnDateTime.Month, Priv->ReturnDateTime.Year);
							break;
						case Alcatel_time:
							smprintf(s, "%d:%d:%d", Priv->ReturnDateTime.Hour, Priv->ReturnDateTime.Minute, Priv->ReturnDateTime.Second);
							break;
						case Alcatel_string:
						case Alcatel_phone:
							smprintf(s, "\"%s\"",DecodeUnicodeString(Priv->ReturnString));
							break;
						case Alcatel_enum:
						case Alcatel_bool:
						case Alcatel_int:
						case Alcatel_byte:
							smprintf(s, "%d", Priv->ReturnInt);
							break;
					}
					smprintf(s,"\n");
			}
		}
		return ERR_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_GetMemory(s, entry);
	}
}

static GSM_Error ALCATEL_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start)
{
	GSM_Error 		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
		if (Priv->ContactsItemsCount == 0) return ERR_EMPTY;

		if (start) entry->Location = 0;
		if ((error = ALCATEL_GetNextId(s, &(entry->Location))) != ERR_NONE) return error;

		return ALCATEL_GetMemory(s, entry);
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_GetNextMemory(s, entry, start);
	}
}

static GSM_Error ALCATEL_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			NamePosition = -1;
	bool			NameSet = false;
	int			i;


	if (entry->MemoryType == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GoToBinaryState(s, StateEdit, TypeContacts, 0))!= ERR_NONE) return error;
		for (i = 0; i < entry->EntriesNum; i++) {
			entry->Entries[i].AddError = ERR_NONE;
			switch (entry->Entries[i].EntryType) {
				case PBK_Number_General:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 8, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Mobile:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 12, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Work:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 7, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Fax:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 9, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Home:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 13, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Pager:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 11, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Other:
					if ((error = ALCATEL_CreateField(s, Alcatel_phone, 10, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Note:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 4, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Email:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 14, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Email2:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 15, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_LastName:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 0, entry->Entries[i].Text)) != ERR_NONE) return error;
					NameSet = true;
					break;
				case PBK_Text_FirstName:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 1, entry->Entries[i].Text)) != ERR_NONE) return error;
					NameSet = true;
					break;
				case PBK_Text_Company:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 2, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_JobTitle:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 3, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Category:
					if ((error = ALCATEL_CreateField(s, Alcatel_byte, 5, &(entry->Entries[i].Number))) != ERR_NONE) return error;
					break;
				case PBK_Private:
					if ((error = ALCATEL_CreateField(s, Alcatel_bool, 6, &(entry->Entries[i].Number))) != ERR_NONE) return error;
					break;
				case PBK_Text_StreetAddress:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 16, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_City:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 17, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_State:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 18, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Zip:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 19, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Country:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 20, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom1:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 21, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom2:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 22, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom3:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 23, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom4:
					if ((error = ALCATEL_CreateField(s, Alcatel_string, 24, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_PictureID:
					if (s->Phone.Data.Priv.ALCATEL.ProtocolVersion == V_1_1) {
					       if ((error = ALCATEL_CreateField(s, Alcatel_int, 25, &(entry->Entries[i].Number))) != ERR_NONE) return error;
					} else {
						entry->Entries[i].AddError = ERR_NOTSUPPORTED;
						smprintf(s,"WARNING: Ignoring entry %d, not supported by phone\n", entry->Entries[i].EntryType);
					}
					break;

				case PBK_Text_Name: NamePosition = i; break;
				/* Following fields are not supported: */
				case PBK_Text_UserID:
				case PBK_Date:
				case PBK_LastModified:
				case PBK_Caller_Group:
				case PBK_RingtoneID:
				case PBK_Text_Postal:
				case PBK_Text_URL:
				case PBK_Text_LUID:
				case PBK_CallLength:
				case PBK_Text_NickName:
				case PBK_Text_FormalName:
				case PBK_Text_WorkPostal:
				case PBK_Text_WorkStreetAddress:
				case PBK_Text_WorkCity:
				case PBK_Text_WorkState:
				case PBK_Text_WorkZip:
				case PBK_Text_WorkCountry:
				case PBK_Text_PictureName:
				case PBK_PushToTalkID:
				case PBK_Number_Messaging:
				case PBK_Photo:
					entry->Entries[i].AddError = ERR_NOTSUPPORTED;
					smprintf(s,"WARNING: Ignoring entry %d, not supported by phone\n", entry->Entries[i].EntryType);
					break;
			}
		}
		if (NamePosition != -1) {
			if (NameSet) {
				entry->Entries[NamePosition].AddError = ERR_NOTSUPPORTED;
				smprintf(s,"WARNING: Ignoring name, not supported by phone\n");
			} else {
				if ((error = ALCATEL_CreateField(s, Alcatel_string, 1, entry->Entries[i].Text)) != ERR_NONE) return error;
			}
		}
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		entry->Location = Priv->CommitedRecord;
		/* Refresh list */
		if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;
		return ERR_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_AddMemory(s, entry);
	}
}

static GSM_Error ALCATEL_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			NamePosition = -1;
	bool			NameSet = false;
	int			i;
	bool			UpdatedFields[26];

	if (entry->Location == 0) return ERR_INVALIDLOCATION;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		/* Save modified entry */
		if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
		if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= ERR_NONE) {
			/* Entry doesn't exist, we will create new one */
			return ALCATEL_AddMemory(s, entry);
		}
		/* Get fields for current item */
		if ((error = ALCATEL_GetFields(s, entry->Location))!= ERR_NONE) return error;

		for (i = 0; i < 26; i++) { UpdatedFields[i] = false; }

		if ((error = ALCATEL_GoToBinaryState(s, StateEdit, TypeContacts, entry->Location))!= ERR_NONE) return error;
		for (i = 0; i < entry->EntriesNum; i++) {
			entry->Entries[i].AddError = ERR_NONE;
			switch (entry->Entries[i].EntryType) {
				case PBK_Number_General:
					UpdatedFields[8] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 8, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Mobile:
					UpdatedFields[12] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 12, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Work:
					UpdatedFields[7] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 7, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Fax:
					UpdatedFields[9] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 9, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Home:
					UpdatedFields[13] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 13, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Pager:
					UpdatedFields[11] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 11, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Number_Other:
					UpdatedFields[10] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_phone, entry->Location, 10, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Note:
					UpdatedFields[4] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 4, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Email:
					UpdatedFields[14] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 14, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Email2:
					UpdatedFields[15] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 15, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_LastName:
					UpdatedFields[0] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 0, entry->Entries[i].Text)) != ERR_NONE) return error; NameSet = true;
					break;
				case PBK_Text_FirstName:
					UpdatedFields[1] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 1, entry->Entries[i].Text)) != ERR_NONE) return error; NameSet = true;
					break;
				case PBK_Text_Company:
					UpdatedFields[2] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 2, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_JobTitle:
					UpdatedFields[3] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 3, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Category:
					UpdatedFields[5] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_byte, entry->Location, 5, &(entry->Entries[i].Number))) != ERR_NONE) return error;
					break;
				case PBK_Private:
					UpdatedFields[6] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_bool, entry->Location, 6, &(entry->Entries[i].Number))) != ERR_NONE) return error;
					break;
				case PBK_Text_StreetAddress:
					UpdatedFields[16] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 16, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_City:
					UpdatedFields[17] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 17, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_State:
					UpdatedFields[18] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 18, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Zip:
					UpdatedFields[19] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 19, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Country:
					UpdatedFields[20] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 20, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom1:
					UpdatedFields[21] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 21, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom2:
					UpdatedFields[22] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 22, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom3:
					UpdatedFields[23] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 23, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_Text_Custom4:
					UpdatedFields[24] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 24, entry->Entries[i].Text)) != ERR_NONE) return error;
					break;
				case PBK_PictureID:
					if (s->Phone.Data.Priv.ALCATEL.ProtocolVersion == V_1_1) {
					       UpdatedFields[25] = true;
					       if ((error = ALCATEL_UpdateField(s, Alcatel_int, entry->Location, 25, &(entry->Entries[i].Number))) != ERR_NONE) return error;
					} else {
						smprintf(s,"WARNING: Ignoring entry %d, not supported by phone\n", entry->Entries[i].EntryType);
						entry->Entries[i].AddError = ERR_NOTSUPPORTED;
					}
					break;
				case PBK_Text_Name: NamePosition = i; break;
				/* Following fields are not supported: */
				case PBK_Text_UserID:
				case PBK_Date:
				case PBK_LastModified:
				case PBK_Caller_Group:
				case PBK_RingtoneID:
				case PBK_Text_Postal:
				case PBK_Text_URL:
				case PBK_Text_LUID:
				case PBK_CallLength:
				case PBK_Text_NickName:
				case PBK_Text_FormalName:
				case PBK_Text_WorkPostal:
				case PBK_Text_WorkStreetAddress:
				case PBK_Text_WorkCity:
				case PBK_Text_WorkState:
				case PBK_Text_WorkZip:
				case PBK_Text_WorkCountry:
				case PBK_Text_PictureName:
				case PBK_PushToTalkID:
				case PBK_Number_Messaging:
				case PBK_Photo:
					entry->Entries[i].AddError = ERR_NOTSUPPORTED;
					smprintf(s,"WARNING: Ignoring entry %d, not supported by phone\n", entry->Entries[i].EntryType);
					break;
			}
		}
		if (NamePosition != -1) {
			if (NameSet) {
				entry->Entries[NamePosition].AddError = ERR_NOTSUPPORTED;
				smprintf(s,"WARNING: Ignoring name, not supported by phone\n");
			} else {
				/* mw: fixme ? */
				UpdatedFields[1] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_string, entry->Location, 1, entry->Entries[i].Text)) != ERR_NONE) return error;
			}
		}
		/* If we didn't update some field, we have to delete it... */
		for (i=0; i<Priv->CurrentFieldsCount; i++) {
			if (!UpdatedFields[Priv->CurrentFields[i]]) if ((error = ALCATEL_DeleteField(s, entry->Location, Priv->CurrentFields[i])) != ERR_NONE) return error;
		}
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		entry->Location = Priv->CommitedRecord;
		return ERR_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_SetMemory(s, entry);
	}
}

static GSM_Error ALCATEL_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
		if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= ERR_NONE) {
			/* Entry was empty => no error */
			return ERR_NONE;
		}
		/* Do real delete */
		error = ALCATEL_DeleteItem(s, entry->Location);
		if (error != ERR_NONE) return error;

		/* Refresh list */
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;

		return ERR_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_DeleteMemory(s, entry);
	}
}

static GSM_Error ALCATEL_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType type)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;

	if (type == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;

		for (i=0; i<Priv->ContactsItemsCount; i++) {
			error =  ALCATEL_DeleteItem(s, Priv->ContactsItems[i]);
			if (error != ERR_NONE) return error;
		}

		/* Refresh list */
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;

		return ERR_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_DeleteAllMemory(s, type);
	}
}

static GSM_Error ALCATEL_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSC(s, smsc);
}


static GSM_Error ALCATEL_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error 		error;

	if (Status->MemoryType == MEM_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= ERR_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;
		Status->MemoryUsed = Priv->ContactsItemsCount;
		Status->MemoryFree = ALCATEL_FREE_MEMORY;
		return ERR_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_GetMemoryStatus(s, Status);
	}
}

static GSM_Error ALCATEL_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMS(s, sms);
}

static GSM_Error ALCATEL_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DeleteSMS(s, sms);
}

static GSM_Error ALCATEL_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AddSMS(s, sms);
}

static GSM_Error ALCATEL_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetBatteryCharge(s, bat);
}

static GSM_Error ALCATEL_GetSignalStrength(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSignalQuality(s, sig);
}

static GSM_Error ALCATEL_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSFolders(s, folders);
}

static GSM_Error ALCATEL_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNextSMS(s, sms, start);
}

static GSM_Error ALCATEL_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSStatus(s, status);
}

static GSM_Error ALCATEL_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DialVoice(s, number, ShowNumber);
}

static GSM_Error ALCATEL_DialService(GSM_StateMachine *s, char *number)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DialService(s, number);
}

static GSM_Error ALCATEL_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AnswerCall(s,ID,all);
}

static GSM_Error ALCATEL_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNetworkInfo(s, netinfo);
}

static GSM_Error ALCATEL_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetDisplayStatus(s, features);
}

static GSM_Error ALCATEL_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetAutoNetworkLogin(s);
}

static GSM_Error ALCATEL_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_PressKey(s, Key, Press);
}

static GSM_Error ALCATEL_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_Reset(s, hard);
}

static GSM_Error ALCATEL_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_CancelCall(s,ID,all);
}

static GSM_Error ALCATEL_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendSavedSMS(s, Folder, Location);
}

static GSM_Error ALCATEL_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendSMS(s, sms);
}

static GSM_Error ALCATEL_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetDateTime(s, date_time);
}

static GSM_Error ALCATEL_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetSMSC(s, smsc);
}

static GSM_Error ALCATEL_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_EnterSecurityCode(s, Code);
}

static GSM_Error ALCATEL_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSecurityStatus(s, Status);
}

static GSM_Error ALCATEL_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_ResetPhoneSettings(s, Type);
}

static GSM_Error ALCATEL_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendDTMF(s, sequence);
}

static GSM_Error ALCATEL_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSIMIMSI(s, IMSI);
}

static GSM_Error ALCATEL_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *status)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	status->Used = 0;
	status->Free = ALCATEL_FREE_MEMORY;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;

	status->Used = Priv->CalendarItemsCount;
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error		error;
	GSM_DateTime		*dt = NULL;
	GSM_DateTime		EventDate = {0,0,0,0,0,0,0};
	GSM_DateTime		EventStart = {0,0,0,0,0,0,0};
	GSM_DateTime		EventEnd = {0,0,0,0,0,0,0};
	bool			EventDateSet = false;
	bool			EventStartSet = false;
	bool			EventEndSet = false;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;
	int			j=0;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, Note->Location))!= ERR_NONE) {
		Note->EntriesNum = 0;
		return error;
	}
	if ((error = ALCATEL_GetFields(s, Note->Location))!= ERR_NONE) return error;

	Note->EntriesNum = Priv->CurrentFieldsCount;

	for (i=0; i < Priv->CurrentFieldsCount; i++) {
		if ((error = ALCATEL_GetFieldValue(s, Note->Location, Priv->CurrentFields[i]))!= ERR_NONE) return error;
		switch (Priv->CurrentFields[i]) {
			case 0:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				/* Date and time is composed at the end, when we have all needed data */
				j++;
				Note->EntriesNum--;
				EventDate = Priv->ReturnDateTime;
				EventDateSet = true;
				break;
			case 1:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckTime(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid time in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				/* Date and time is composed at the end, when we have all needed data */
				j++;
				Note->EntriesNum--;
				EventStart = Priv->ReturnDateTime;
				EventStartSet = true;
				break;
			case 2:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckTime(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid time in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				/* Date and time is composed at the end, when we have all needed data */
				j++;
				Note->EntriesNum--;
				EventEnd = Priv->ReturnDateTime;
				EventEndSet = true;
				break;
			case 3:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					Note->Entries[i-j].EntryType = CAL_TONE_ALARM_DATETIME;
					Note->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(Note->Entries[i-j].Date);
				} else {
					j++;
					Note->EntriesNum--;
					dt->Day = Priv->ReturnDateTime.Day;
					dt->Month = Priv->ReturnDateTime.Month;
					dt->Year = Priv->ReturnDateTime.Year;
					dt->Timezone = Priv->ReturnDateTime.Timezone;
					dt = NULL;
				}
				break;
			case 4:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckTime(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid time in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					Note->Entries[i-j].EntryType = CAL_TONE_ALARM_DATETIME;
					Note->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(Note->Entries[i-j].Date);
				} else {
					j++;
					Note->EntriesNum--;
					dt->Hour = Priv->ReturnDateTime.Hour;
					dt->Minute = Priv->ReturnDateTime.Minute;
					dt->Second = Priv->ReturnDateTime.Second;
					dt = NULL;
				}
				break;
			case 5:
				if (Priv->ReturnType != Alcatel_string) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_TEXT;
				CopyUnicodeString(Note->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 6:
				if (Priv->ReturnType != Alcatel_bool) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_PRIVATE;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 7:
				if (Priv->ReturnType != Alcatel_enum) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				switch (Priv->ReturnInt) {
					case 0:
						Note->Type = GSM_CAL_MEETING;
						break;
					case 2:
						Note->Type = GSM_CAL_BIRTHDAY;
						break;
					case 3:
						Note->Type = GSM_CAL_CALL;
						break;
					case 4:
						Note->Type = GSM_CAL_ALARM;
						break;
					case 5:
						Note->Type = GSM_CAL_DAILY_ALARM;
						break;
					case 9:
						/* I'd call this repeating event, but it makes no sense creating one more type ... */
						Note->Type = GSM_CAL_MEETING;
						break;
					default:
						smprintf(s,"WARNING: Received unknown event type %02X!\n", Priv->ReturnInt);
						break;
				}
				j++;
				Note->EntriesNum--;
				break;
			case 8:
				if (Priv->ReturnType != Alcatel_int) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				/* 0xffffffff indicates that there is phone (BF5), 0 means none (BF5, BE5)*/
				if (Priv->ReturnInt == 0xffffffff || Priv->ReturnInt == 0) {
					j++;
					Note->EntriesNum--;
				} else {
					Note->Entries[i-j].EntryType = CAL_CONTACTID;
					Note->Entries[i-j].Number = Priv->ReturnInt;
				}
				break;
			case 9:
				if (Priv->ReturnType != Alcatel_phone) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_PHONE;
				CopyUnicodeString(Note->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 10:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_DAYOFWEEK;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 11:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_DAY;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 12:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_WEEKOFMONTH;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 13:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_MONTH;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 17:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				/* In BF5 birthday has frequency = 1 */
				if (Note->Type == GSM_CAL_BIRTHDAY) {
					Note->EntriesNum--;
					j++;
				} else {
					Note->Entries[i-j].EntryType = CAL_REPEAT_FREQUENCY;
					Note->Entries[i-j].Number = Priv->ReturnInt;
				}
				break;
			case 18:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_STARTDATE;
				Note->Entries[i-j].Date = Priv->ReturnDateTime;
				break;
			case 19:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_STOPDATE;
				Note->Entries[i-j].Date = Priv->ReturnDateTime;
				break;
			case 20:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 3rd (alarm date) */
				j++;
				Note->EntriesNum--;
				break;
			case 21:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (!CheckTime(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid time in phone, ignoring\n");
					Note->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 4th (alarm time) */
				j++;
				Note->EntriesNum--;
				break;
			default:
				Note->EntriesNum--;
				j++;
				smprintf(s,"WARNING: Received unknown field %02X, ignoring. Type = %02X. Value = ", Priv->CurrentFields[i], Priv->ReturnType);
				switch (Priv->ReturnType) {
					case Alcatel_date:
						smprintf(s, "%d.%d.%d", Priv->ReturnDateTime.Day, Priv->ReturnDateTime.Month, Priv->ReturnDateTime.Year);
						break;
					case Alcatel_time:
						smprintf(s, "%d:%d:%d", Priv->ReturnDateTime.Hour, Priv->ReturnDateTime.Minute, Priv->ReturnDateTime.Second);
						break;
					case Alcatel_string:
					case Alcatel_phone:
						smprintf(s, "\"%s\"",DecodeUnicodeString(Priv->ReturnString));
						break;
					case Alcatel_enum:
					case Alcatel_bool:
					case Alcatel_int:
					case Alcatel_byte:
						smprintf(s, "%d", Priv->ReturnInt);
						break;
				}
				smprintf(s,"\n");
		}
	}
	/* Fill start/stop datetime */
	if (EventDateSet) {
		if (EventStartSet) {
			Note->Entries[i-j].EntryType = CAL_START_DATETIME;
			Note->Entries[i-j].Date = EventDate;
			Note->Entries[i-j].Date.Timezone = EventStart.Timezone;
			Note->Entries[i-j].Date.Hour = EventStart.Hour;
			Note->Entries[i-j].Date.Minute = EventStart.Minute;
			Note->Entries[i-j].Date.Second = EventStart.Second;
			Note->EntriesNum++;
			i++;
		}
		if (EventEndSet) {
			Note->Entries[i-j].EntryType = CAL_END_DATETIME;
			Note->Entries[i-j].Date = EventDate;
			Note->Entries[i-j].Date.Timezone = EventEnd.Timezone;
			Note->Entries[i-j].Date.Hour = EventEnd.Hour;
			Note->Entries[i-j].Date.Minute = EventEnd.Minute;
			Note->Entries[i-j].Date.Second = EventEnd.Second;
			Note->EntriesNum++;
			i++;
		}
		if (!EventStartSet && !EventEndSet) {
			Note->Entries[i-j].EntryType = CAL_START_DATETIME;
			Note->Entries[i-j].Date = EventDate;
			Note->EntriesNum++;
			i++;
		}
	}
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Error 		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if (Priv->CalendarItemsCount == 0) return ERR_EMPTY;

	if (start) Note->Location = 0;
	if ((error = ALCATEL_GetNextId(s, &(Note->Location))) != ERR_NONE) return error;

	return ALCATEL_GetCalendar(s, Note);
}


static GSM_Error ALCATEL_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error		error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	/* Delete Calendar */
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, Note->Location))!= ERR_NONE) {
		/* Entry was empty => no error */
		return ERR_NONE;
	}
	error = ALCATEL_DeleteItem(s, Note->Location);
	if (error != ERR_NONE) return error;
	/* Refresh list */
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;
	return ERR_NONE;
}


static GSM_Error ALCATEL_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error		error;
	unsigned int		val;
	bool			contact_set = false;
	bool			phone_set = false;
	bool			date_set = false;
	bool			repeating = false;
	int			i;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GoToBinaryState(s, StateEdit, TypeCalendar, 0))!= ERR_NONE) return error;

	for (i = 0; i < Note->EntriesNum; i++) {
		switch (Note->Entries[i].EntryType) {
			case CAL_START_DATETIME:
				if (!date_set) {
					if ((error = ALCATEL_CreateField(s, Alcatel_date, 0, &(Note->Entries[i].Date))) != ERR_NONE) return error;
					date_set = true;
				}
				if ((error = ALCATEL_CreateField(s, Alcatel_time, 1, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				break;
			case CAL_END_DATETIME:
				if (!date_set) {
					if ((error = ALCATEL_CreateField(s, Alcatel_date, 0, &(Note->Entries[i].Date))) != ERR_NONE) return error;
					date_set = true;
				}
				if ((error = ALCATEL_CreateField(s, Alcatel_time, 2, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				break;
			case CAL_TONE_ALARM_DATETIME:
				if ((error = ALCATEL_CreateField(s, Alcatel_date, 3, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				if ((error = ALCATEL_CreateField(s, Alcatel_time, 4, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				if (Note->Type == GSM_CAL_ALARM || Note->Type == GSM_CAL_DAILY_ALARM) {
					if ((error = ALCATEL_CreateField(s, Alcatel_date, 20, &(Note->Entries[i].Date))) != ERR_NONE) return error;
					if ((error = ALCATEL_CreateField(s, Alcatel_time, 21, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				}
				break;
			case CAL_TEXT:
				if ((error = ALCATEL_CreateField(s, Alcatel_string, 5, Note->Entries[i].Text)) != ERR_NONE) return error;
				break;
			case CAL_PRIVATE:
				if ((error = ALCATEL_CreateField(s, Alcatel_bool, 6, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				break;
			case CAL_CONTACTID:
				if ((error = ALCATEL_CreateField(s, Alcatel_int, 8, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				contact_set = true;
				break;
			case CAL_PHONE:
				if ((error = ALCATEL_CreateField(s, Alcatel_phone, 9, Note->Entries[i].Text)) != ERR_NONE) return error;
				phone_set = true;
				break;
			case CAL_REPEAT_DAYOFWEEK:
				if ((error = ALCATEL_CreateField(s, Alcatel_byte, 10, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_DAY:
				if ((error = ALCATEL_CreateField(s, Alcatel_byte, 11, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_WEEKOFMONTH:
				if ((error = ALCATEL_CreateField(s, Alcatel_byte, 12, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_MONTH:
				if ((error = ALCATEL_CreateField(s, Alcatel_byte, 13, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_FREQUENCY:
				if ((error = ALCATEL_CreateField(s, Alcatel_byte, 17, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			 case CAL_REPEAT_STARTDATE:
				if ((error = ALCATEL_CreateField(s, Alcatel_date, 18, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_STOPDATE:
				if ((error = ALCATEL_CreateField(s, Alcatel_date, 19, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_COUNT:
			case CAL_REPEAT_DAYOFYEAR:
			case CAL_SILENT_ALARM_DATETIME:
			case CAL_LOCATION:
			case CAL_DESCRIPTION:
			case CAL_LUID:
			case CAL_LAST_MODIFIED:
				smprintf(s,"WARNING: Ignoring entry %d, not supported by phone\n", Note->Entries[i].EntryType);
				break;
		}
	}

	switch (Note->Type) {
		case GSM_CAL_CALL:
			val = 3;
			break;
		case GSM_CAL_BIRTHDAY:
			val = 2;
			break;
		case GSM_CAL_ALARM:
			val = 4;
			break;
		case GSM_CAL_DAILY_ALARM:
			val = 5;
			break;
		default:
			if (repeating) {
				val = 9;
			} else {
				val = 0;
			}
	}
	if ((error = ALCATEL_CreateField(s, Alcatel_enum, 7, &val)) != ERR_NONE) return error;

	if (!contact_set) {
		if (phone_set) {
			val = 0xffffffff;
		} else {
			val = 0;
		}
		if ((error = ALCATEL_CreateField(s, Alcatel_int, 8, &val)) != ERR_NONE) return error;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	Note->Location = Priv->CommitedRecord;
	/* Refresh list */
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;
	return ERR_NONE;
}

static GSM_Error ALCATEL_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error		error;
	unsigned int		val;
	bool			contact_set = false;
	bool			phone_set = false;
	bool			date_set = false;
	bool			repeating = false;
	int			i;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	bool			UpdatedFields[22];

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, Note->Location))!= ERR_NONE) {
		/* Entry doesn't exist, we will create new one */
		return ALCATEL_AddCalendar(s, Note);
	}
	/* Get fields for current item */
	if ((error = ALCATEL_GetFields(s, Note->Location))!= ERR_NONE) return error;

	for (i = 0; i < 22; i++) { UpdatedFields[i] = false; }

	if ((error = ALCATEL_GoToBinaryState(s, StateEdit, TypeCalendar, Note->Location))!= ERR_NONE) return error;

	for (i = 0; i < Note->EntriesNum; i++) {
		switch (Note->Entries[i].EntryType) {
			case CAL_START_DATETIME:
				if (!date_set) {
					UpdatedFields[0] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_date, Note->Location, 0, &(Note->Entries[i].Date))) != ERR_NONE) return error;
					date_set = true;
				}
				UpdatedFields[1] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_time, Note->Location, 1, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				break;
			case CAL_END_DATETIME:
				if (!date_set) {
					UpdatedFields[0] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_date, Note->Location, 0, &(Note->Entries[i].Date))) != ERR_NONE) return error;
					date_set = true;
				}
				UpdatedFields[2] = true; if ((error = ALCATEL_UpdateField(s, Alcatel_time, Note->Location, 2, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				break;
			case CAL_TONE_ALARM_DATETIME:
				UpdatedFields[3] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_date, Note->Location, 3, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				UpdatedFields[4] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_time, Note->Location, 4, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				if (Note->Type == GSM_CAL_ALARM || Note->Type == GSM_CAL_DAILY_ALARM) {
					UpdatedFields[20] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_date, Note->Location, 20, &(Note->Entries[i].Date))) != ERR_NONE) return error;
					UpdatedFields[21] = true;
					if ((error = ALCATEL_UpdateField(s, Alcatel_time, Note->Location, 21, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				}
				break;
			case CAL_TEXT:
				UpdatedFields[5] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_string, Note->Location, 5, Note->Entries[i].Text)) != ERR_NONE) return error;
				break;
			case CAL_PRIVATE:
				UpdatedFields[6] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_bool, Note->Location, 6, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				break;
			case CAL_CONTACTID:
				UpdatedFields[8] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_int, Note->Location, 8, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				contact_set = true;
				break;
			case CAL_PHONE:
				UpdatedFields[9] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_phone, Note->Location, 9, Note->Entries[i].Text)) != ERR_NONE) return error;
				phone_set = true;
				break;
			case CAL_REPEAT_DAYOFWEEK:
				UpdatedFields[10] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_byte, Note->Location, 10, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_DAY:
				UpdatedFields[11] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_byte, Note->Location, 11, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_WEEKOFMONTH:
				UpdatedFields[12] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_byte, Note->Location, 12, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_MONTH:
				UpdatedFields[13] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_byte, Note->Location, 13, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_FREQUENCY:
				UpdatedFields[17] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_byte, Note->Location, 17, &(Note->Entries[i].Number))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			 case CAL_REPEAT_STARTDATE:
				UpdatedFields[18] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_date, Note->Location, 18, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_STOPDATE:
				UpdatedFields[19] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_date, Note->Location, 19, &(Note->Entries[i].Date))) != ERR_NONE) return error;
				repeating 		= true;
				break;
			case CAL_REPEAT_COUNT:
			case CAL_REPEAT_DAYOFYEAR:
			case CAL_SILENT_ALARM_DATETIME:
			case CAL_LOCATION:
			case CAL_DESCRIPTION:
			case CAL_LUID:
			case CAL_LAST_MODIFIED:
				smprintf(s,"WARNING: Ignoring entry %d, not supported by phone\n", Note->Entries[i].EntryType);
				break;
		}
	}

	switch (Note->Type) {
		case GSM_CAL_CALL:
			val = 3;
			break;
		case GSM_CAL_BIRTHDAY:
			val = 2;
			break;
		case GSM_CAL_ALARM:
			val = 4;
			break;
		case GSM_CAL_DAILY_ALARM:
			val = 5;
			break;
		default:
			if (repeating) {
				val = 9;
			} else {
				val = 0;
			}
	}
	UpdatedFields[7] = true;
	if ((error = ALCATEL_UpdateField(s, Alcatel_enum, Note->Location, 7, &val)) != ERR_NONE) return error;

	if (!contact_set) {
		if (phone_set) {
			val = 0xffffffff;
		} else {
			val = 0;
		}
		UpdatedFields[8] = true;
		if ((error = ALCATEL_UpdateField(s, Alcatel_int, Note->Location, 8, &val)) != ERR_NONE) return error;
	}
	/* If we didn't update some field, we have to delete it... */
	for (i=0; i<Priv->CurrentFieldsCount; i++) {
		if (!UpdatedFields[Priv->CurrentFields[i]]) if ((error = ALCATEL_DeleteField(s, Note->Location, Priv->CurrentFields[i])) != ERR_NONE) return error;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	return ERR_NONE;
}

static GSM_Error ALCATEL_DeleteAllCalendar (GSM_StateMachine *s)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;

	for (i=0; i<Priv->CalendarItemsCount; i++) {
		error =  ALCATEL_DeleteItem(s, Priv->CalendarItems[i]);
		if (error != ERR_NONE) return error;
	}

	/* Refresh list */
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;

	return ERR_NONE;
}


static GSM_Error ALCATEL_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error		error;
	GSM_CalendarEntry	Note;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;
	bool			Found = false;
	bool			DateSet = false;
	int			alarm_number = Alarm->Location;
	static GSM_DateTime	nulldt = {0,0,0,0,0,0,0};

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;

	for (i=0; i<Priv->CalendarItemsCount; i++) {
		if ((error = ALCATEL_GetFieldValue(s, Priv->CalendarItems[i], 7))!= ERR_NONE) return error;
		if (Priv->ReturnType != Alcatel_enum) {
			smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
			continue;
		}
		if (Priv->ReturnInt == 4 || Priv->ReturnInt == 5) {
			alarm_number--;
			if (alarm_number == 0) {
				Found = true;
				break;
			}
		}
	}

	if (!Found) return ERR_EMPTY;

	Note.Location = Priv->CalendarItems[i];

	if ((error = ALCATEL_GetCalendar(s, &Note))!= ERR_NONE) return error;

	if (Note.Type == GSM_CAL_ALARM) {
		Alarm->Repeating = false;
	} else {
		Alarm->Repeating = true;
	}

	Alarm->Text[0] = 0; Alarm->Text[1] = 0;


	for (i = 0; i < Note.EntriesNum; i++) {
		if (Note.Entries[i].EntryType == CAL_TEXT) {
			CopyUnicodeString(Alarm->Text, Note.Entries[i].Text);
		} else if (Note.Entries[i].EntryType == CAL_TONE_ALARM_DATETIME) {
			Alarm->DateTime = Note.Entries[i].Date;
			DateSet = false;
		}
	}
	if (!DateSet) {
		Alarm->DateTime = nulldt;
	}

	return ERR_NONE;
}


static GSM_Error ALCATEL_SetAlarm (GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error		error;
	GSM_CalendarEntry	Note;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_DateTime		dt;
	int			i;
	bool			Found = false;
	int			alarm_number = Alarm->Location;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;

	for (i=0; i<Priv->CalendarItemsCount; i++) {
		if ((error = ALCATEL_GetFieldValue(s, Priv->CalendarItems[i], 7))!= ERR_NONE) return error;
		if (Priv->ReturnType != Alcatel_enum) {
			smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
			continue;
		}
		if (Priv->ReturnInt == 4 || Priv->ReturnInt == 5) {
			alarm_number--;
			if (alarm_number == 0) {
				Found = true;
				break;
			}
		}
	}

	if (Found) {
		Note.Location = Priv->CalendarItems[i];
	}

	Note.EntriesNum = 1;

	Note.Entries[0].EntryType = CAL_TONE_ALARM_DATETIME;
	Note.Entries[0].Date = Alarm->DateTime;

	if (Alarm->Repeating) {
		Note.Type = GSM_CAL_DAILY_ALARM;
		GSM_GetCurrentDateTime(&dt);
		Note.Entries[0].Date.Day = dt.Day;
		Note.Entries[0].Date.Month = dt.Month;
		Note.Entries[0].Date.Year = dt.Year;
	} else {
		Note.Type = GSM_CAL_ALARM;
	}

	if (Alarm->Text[0] != 0 || Alarm->Text[1] != 0) {
		Note.EntriesNum++;
		Note.Entries[1].EntryType = CAL_TEXT;
		CopyUnicodeString(Note.Entries[1].Text, Alarm->Text);
	}

	if (Found) {
		return ALCATEL_SetCalendar(s, &Note);
	} else {
		return ALCATEL_AddCalendar(s, &Note);
	}
}


static GSM_Error ALCATEL_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	status->Used = 0;
	status->Free = ALCATEL_FREE_MEMORY;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;

	status->Used = Priv->ToDoItemsCount;
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error		error;
	GSM_DateTime		*dt = NULL;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;
	int			j=0;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, ToDo->Location))!= ERR_NONE) {
		ToDo->EntriesNum = 0;
		return error;
	}
	if ((error = ALCATEL_GetFields(s, ToDo->Location))!= ERR_NONE) return error;

	ToDo->EntriesNum = Priv->CurrentFieldsCount;
	ToDo->Priority = GSM_Priority_None;
	ToDo->Type = GSM_CAL_MEMO;

	for (i=0; i < Priv->CurrentFieldsCount; i++) {
		if ((error = ALCATEL_GetFieldValue(s, ToDo->Location, Priv->CurrentFields[i]))!= ERR_NONE) return error;
		switch (Priv->CurrentFields[i]) {
			case 0:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_END_DATETIME;
				ToDo->Entries[i-j].Date = Priv->ReturnDateTime;
				break;
			case 1:
				if (Priv->ReturnType != Alcatel_bool) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_COMPLETED;
				ToDo->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 2:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					ToDo->Entries[i-j].EntryType = TODO_ALARM_DATETIME;
					ToDo->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(ToDo->Entries[i-j].Date);
				} else {
					j++;
					ToDo->EntriesNum--;
					dt->Day = Priv->ReturnDateTime.Day;
					dt->Month = Priv->ReturnDateTime.Month;
					dt->Year = Priv->ReturnDateTime.Year;
					dt->Timezone = Priv->ReturnDateTime.Timezone;
					dt = NULL;
				}
				break;
			case 3:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (!CheckTime(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid time in phone, ignoring\n");
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					ToDo->Entries[i-j].EntryType = TODO_ALARM_DATETIME;
					ToDo->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(ToDo->Entries[i-j].Date);
				} else {
					j++;
					ToDo->EntriesNum--;
					dt->Hour = Priv->ReturnDateTime.Hour;
					dt->Minute = Priv->ReturnDateTime.Minute;
					dt->Second = Priv->ReturnDateTime.Second;
					dt = NULL;
				}
				break;
			case 4:
				if (Priv->ReturnType != Alcatel_string) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 5:
				if (Priv->ReturnType != Alcatel_bool) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_PRIVATE;
				ToDo->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 6:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (Priv->ReturnInt == 255) {
					/* 255 means no category */
					j++;
					ToDo->EntriesNum--;
				} else {
					ToDo->Entries[i-j].EntryType = TODO_CATEGORY;
					ToDo->Entries[i-j].Number = Priv->ReturnInt;
				}
				break;
			case 7:
				/* This one seems to be byte for BF5 and enum for BE5 */
				if (Priv->ReturnType != Alcatel_enum && Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				switch (Priv->ReturnInt) {
					case 0:
						ToDo->Priority = GSM_Priority_High;
						break;
					case 1:
						ToDo->Priority = GSM_Priority_Medium;
						break;
					case 2:
						ToDo->Priority = GSM_Priority_Low;
						break;
					default:
						smprintf(s,"WARNING: Received unexpected priority %02X, ignoring\n", Priv->ReturnInt);
				}
				j++;
				ToDo->EntriesNum--;
				break;
			case 8:
				if (Priv->ReturnType != Alcatel_int) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				/* 0xffffffff indicates that there is phone, 0 means none */
				if (Priv->ReturnInt == 0xffffffff || Priv->ReturnInt == 0) {
					j++;
					ToDo->EntriesNum--;
				} else {
					ToDo->Entries[i-j].EntryType = TODO_CONTACTID;
					ToDo->Entries[i-j].Number = Priv->ReturnInt;
					ToDo->Type = GSM_CAL_CALL;
				}
				break;
			case 9:
				if (Priv->ReturnType != Alcatel_phone) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_PHONE;
				ToDo->Type = GSM_CAL_CALL;
				CopyUnicodeString(ToDo->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 10:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (!CheckDate(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid date in phone, ignoring\n");
					ToDo->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 2nd (alarm date) */
				j++;
				ToDo->EntriesNum--;
				break;
			case 11:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (!CheckTime(&(Priv->ReturnDateTime))) {
					smprintf(s,"WARNING: Invalid time in phone, ignoring\n");
					ToDo->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 3rd (alarm time) */
				j++;
				ToDo->EntriesNum--;
				break;
			default:
				ToDo->EntriesNum--;
				j++;
				smprintf(s,"WARNING: Received unknown field %02X, ignoring. Type = %02X. Value = ", Priv->CurrentFields[i], Priv->ReturnType);
				switch (Priv->ReturnType) {
					case Alcatel_date:
						smprintf(s, "%d.%d.%d", Priv->ReturnDateTime.Day, Priv->ReturnDateTime.Month, Priv->ReturnDateTime.Year);
						break;
					case Alcatel_time:
						smprintf(s, "%d:%d:%d", Priv->ReturnDateTime.Hour, Priv->ReturnDateTime.Minute, Priv->ReturnDateTime.Second);
						break;
					case Alcatel_string:
					case Alcatel_phone:
						smprintf(s, "\"%s\"",DecodeUnicodeString(Priv->ReturnString));
						break;
					case Alcatel_enum:
					case Alcatel_bool:
					case Alcatel_int:
					case Alcatel_byte:
						smprintf(s, "%d", Priv->ReturnInt);
						break;
				}
				smprintf(s,"\n");
		}
	}
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start)
{
	GSM_Error 		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if (Priv->ToDoItemsCount == 0) return ERR_EMPTY;

	if (start) ToDo->Location = 0;
	if ((error = ALCATEL_GetNextId(s, &(ToDo->Location))) != ERR_NONE) return error;

	return ALCATEL_GetToDo(s, ToDo);
}

static GSM_Error ALCATEL_DeleteAllToDo (GSM_StateMachine *s)
{
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;

	for (i=0; i<Priv->ToDoItemsCount; i++) {
		error =  ALCATEL_DeleteItem(s, Priv->ToDoItems[i]);
		if (error != ERR_NONE) return error;
	}

	/* Refresh list */
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;

	return ERR_NONE;
}

static GSM_Error ALCATEL_AddToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error		error;
	unsigned int		val;
	bool			contact_set = false;
	bool			phone_set = false;
	int			i;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GoToBinaryState(s, StateEdit, TypeToDo, 0))!= ERR_NONE) return error;

	/* Set priority if wanted */
	switch (ToDo->Priority) {
		case GSM_Priority_High:
			val = 0;
			break;
		case GSM_Priority_Low:
			val = 2;
			break;
		case GSM_Priority_Medium:
			val = 1;
			break;
		case GSM_Priority_None:
		default:
			val = 0xff;
			break;
	}

	if (val != 0xff) {
		/* This one seems to be byte for BF5 and enum for BE5 */
		if (s->Phone.Data.Priv.ALCATEL.ProtocolVersion == V_1_1) {
		       if ((error = ALCATEL_CreateField(s, Alcatel_byte, 7, &val)) != ERR_NONE) return error;
		} else {
		       if ((error = ALCATEL_CreateField(s, Alcatel_enum, 7, &val)) != ERR_NONE) return error;
		}
	}

	for (i = 0; i < ToDo->EntriesNum; i++) {
		switch (ToDo->Entries[i].EntryType) {
			case TODO_END_DATETIME:
				if ((error = ALCATEL_CreateField(s, Alcatel_date, 0, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				break;
			case TODO_COMPLETED:
				if ((error = ALCATEL_CreateField(s, Alcatel_bool, 1, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				break;
			case TODO_ALARM_DATETIME:
				if ((error = ALCATEL_CreateField(s, Alcatel_date, 2, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				if ((error = ALCATEL_CreateField(s, Alcatel_time, 3, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				if ((error = ALCATEL_CreateField(s, Alcatel_date, 10, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				if ((error = ALCATEL_CreateField(s, Alcatel_time, 11, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				break;
			case TODO_TEXT:
				if ((error = ALCATEL_CreateField(s, Alcatel_string, 4, ToDo->Entries[i].Text)) != ERR_NONE) return error;
				break;
			case TODO_PRIVATE:
				if ((error = ALCATEL_CreateField(s, Alcatel_bool, 5, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				break;
			case TODO_CATEGORY:
				if ((error = ALCATEL_CreateField(s, Alcatel_byte, 6, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				break;
			case TODO_CONTACTID:
				if ((error = ALCATEL_CreateField(s, Alcatel_int, 8, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				contact_set = true;
				break;
			case TODO_PHONE:
				if ((error = ALCATEL_CreateField(s, Alcatel_phone, 9, ToDo->Entries[i].Text)) != ERR_NONE) return error;
				phone_set = true;
				break;
			default:
				break;
		}
	}
	if (!contact_set) {
		if (phone_set) {
			val = 0xffffffff;
		} else {
			val = 0;
		}
		if ((error = ALCATEL_CreateField(s, Alcatel_int, 8, &val)) != ERR_NONE) return error;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	ToDo->Location = Priv->CommitedRecord;
	/* Refresh list */
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;
	return ERR_NONE;
}

static GSM_Error ALCATEL_SetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error		error;
	unsigned int		val;
	bool			contact_set = false;
	bool			phone_set = false;
	bool			UpdatedFields[12];
	int			i;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	/* Save modified ToDo */
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, ToDo->Location))!= ERR_NONE) {
		/* Entry doesn't exist, we will create new one */
		return ALCATEL_AddToDo(s, ToDo);
	}
	/* Get fields for current item */
	if ((error = ALCATEL_GetFields(s, ToDo->Location))!= ERR_NONE) return error;

	for (i = 0; i < 12; i++) { UpdatedFields[i] = false; }

	if ((error = ALCATEL_GoToBinaryState(s, StateEdit, TypeToDo, ToDo->Location))!= ERR_NONE) return error;

	/* Set priority if wanted */
	switch (ToDo->Priority) {
		case GSM_Priority_High:
			val = 0;
			break;
		case GSM_Priority_Low:
			val = 2;
			break;
		case GSM_Priority_Medium:
			val = 1;
			break;
		case GSM_Priority_None:
		default:
			val = 0xff;
			break;
	}

	if (val != 0xff) {
		/* This one seems to be byte for BF5 and enum for BE5 */
		if (s->Phone.Data.Priv.ALCATEL.ProtocolVersion == V_1_1) {
		       if ((error = ALCATEL_UpdateField(s, Alcatel_byte, ToDo->Location, 7, &val)) != ERR_NONE) return error;
		} else {
		       if ((error = ALCATEL_UpdateField(s, Alcatel_enum, ToDo->Location, 7, &val)) != ERR_NONE) return error;
		}
		UpdatedFields[7] = true;
	}

	for (i = 0; i < ToDo->EntriesNum; i++) {
		switch (ToDo->Entries[i].EntryType) {
			case TODO_END_DATETIME:
				if ((error = ALCATEL_UpdateField(s, Alcatel_date, ToDo->Location, 0, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				UpdatedFields[0] = true;
				break;
			case TODO_COMPLETED:
				if ((error = ALCATEL_UpdateField(s, Alcatel_bool, ToDo->Location, 1, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				UpdatedFields[1] = true;
				break;
			case TODO_ALARM_DATETIME:
				if ((error = ALCATEL_UpdateField(s, Alcatel_date, ToDo->Location, 2, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				UpdatedFields[2] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_time, ToDo->Location, 3, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				UpdatedFields[3] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_date, ToDo->Location, 10, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				UpdatedFields[10] = true;
				if ((error = ALCATEL_UpdateField(s, Alcatel_time, ToDo->Location, 11, &(ToDo->Entries[i].Date))) != ERR_NONE) return error;
				UpdatedFields[11] = true;
				break;
			case TODO_TEXT:
				if ((error = ALCATEL_UpdateField(s, Alcatel_string, ToDo->Location, 4, ToDo->Entries[i].Text)) != ERR_NONE) return error;
				UpdatedFields[4] = true;
				break;
			case TODO_PRIVATE:
				if ((error = ALCATEL_UpdateField(s, Alcatel_bool, ToDo->Location, 5, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				UpdatedFields[5] = true;
				break;
			case TODO_CATEGORY:
				if ((error = ALCATEL_UpdateField(s, Alcatel_byte, ToDo->Location, 6, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				UpdatedFields[6] = true;
				break;
			case TODO_CONTACTID:
				if ((error = ALCATEL_UpdateField(s, Alcatel_int, ToDo->Location, 8, &(ToDo->Entries[i].Number))) != ERR_NONE) return error;
				UpdatedFields[8] = true;
				contact_set = true;
				break;
			case TODO_PHONE:
				if ((error = ALCATEL_UpdateField(s, Alcatel_phone, ToDo->Location, 9, ToDo->Entries[i].Text)) != ERR_NONE) return error;
				UpdatedFields[9] = true;
				phone_set = true;
				break;
			default:
				break;
		}
	}
	if (!contact_set) {
		if (phone_set) {
			val = 0xffffffff;
		} else {
			val = 0;
		}
		if ((error = ALCATEL_UpdateField(s, Alcatel_int, ToDo->Location, 8, &val)) != ERR_NONE) return error;
		UpdatedFields[8] = true;
	}


	/* If we didn't update some field, we have to delete it... */
	for (i=0; i<Priv->CurrentFieldsCount; i++) {
		if (!UpdatedFields[Priv->CurrentFields[i]]) if ((error = ALCATEL_DeleteField(s, ToDo->Location, Priv->CurrentFields[i])) != ERR_NONE) return error;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	return ERR_NONE;
}

static GSM_Error ALCATEL_DeleteToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error		error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	/* Delete ToDo */
	if ((error = ALCATEL_GetAvailableIds(s, false))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, ToDo->Location))!= ERR_NONE) {
		/* Entry was empty => no error */
		return ERR_NONE;
	}
	error = ALCATEL_DeleteItem(s, ToDo->Location);
	if (error != ERR_NONE) return error;
	/* Refresh list */
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, true))!= ERR_NONE) return error;
	return ERR_NONE;
}

static GSM_Error ALCATEL_GetCategoryStatus(GSM_StateMachine *s, GSM_CategoryStatus *Status)
{
	GSM_Alcatel_BinaryType	type;
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	switch (Status->Type) {
		case Category_ToDo: type = TypeToDo; break;
		case Category_Phonebook: type = TypeContacts; break;
		default: return ERR_NOTSUPPORTED;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, type, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableCategoryIds(s))!= ERR_NONE) return error;

	Status->Used = Priv->CurrentCategoriesCount;

	return ERR_NONE;
}

static GSM_Error ALCATEL_GetCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	GSM_Alcatel_BinaryType	type;
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	switch (Category->Type) {
		case Category_ToDo: type = TypeToDo; break;
		case Category_Phonebook: type = TypeContacts; break;
		default: return ERR_NOTSUPPORTED;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, type, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetAvailableCategoryIds(s))!= ERR_NONE) return error;
	if ((error = ALCATEL_IsCategoryIdAvailable(s, Category->Location))!= ERR_NONE) return error;
	if ((error = ALCATEL_GetCategoryText(s, Category->Location))!= ERR_NONE) return error;

	CopyUnicodeString(Category->Name, Priv->ReturnString);

	return ERR_NONE;
}

static GSM_Error ALCATEL_AddCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	GSM_Alcatel_BinaryType	type;
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	switch (Category->Type) {
		case Category_ToDo: type = TypeToDo; break;
		case Category_Phonebook: type = TypeContacts; break;
		default: return ERR_NOTSUPPORTED;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, type, 0))!= ERR_NONE) return error;
	if ((error = ALCATEL_AddCategoryText(s, Category->Name))!= ERR_NONE) return error;

	Category->Location = Priv->ReturnInt;

	return ERR_NONE;
}

static GSM_Error ALCATEL_GetProductCode(GSM_StateMachine *s, char *value)
{
       strcpy(value, s->Phone.Data.ModelInfo->model);
       return ERR_NONE;
}

static GSM_Error ALCATEL_DispatchMessage(GSM_StateMachine *s)
{
	if (s->Phone.Data.Priv.ALCATEL.Mode == ModeBinary) {
		return GSM_DispatchMessage(s);
	} else {
		return ATGEN_DispatchMessage(s);
	}
}

static GSM_Error ALCATEL_ReplyGeneric(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* All error values are just VERY wild guesses, but these seems to work
	 * almost as expected ...
	 */
	switch (msg.Buffer[8]) {
		case 0x00: /* no error */
			return ERR_NONE;
		case 0x0e: /* Opening session when not closed (might be also opened somewhere else) */
		case 0x10: /* Some thing opened in phone menus */
			return ERR_INSIDEPHONEMENU;
		case 0x13:
			/* This appears in more cases:
			 *	- phone needs PIN code
			 *	- we want to close not opened session
			 * For normal users the second case shouldn't occur...
			 */
			return ERR_SECURITYERROR;
		case 0x14: /* Bad data */
			return ERR_INVALIDDATA;
		case 0x2f: /* Closing session when not opened */
		case 0x1f: /* Bad in/out counter in packet/ack */
		case 0x0C: /* Bad id (item/database) */
		case 0x11: /* Bad list id */
		case 0x2A: /* Nonexistant field/item id */
			smprintf(s, "WARNING: Bug reply %02X\n", msg.Buffer[8]);
			return ERR_BUG;
		case 0x35: /* Too long text */
			return ERR_INVALIDDATA;
		case 0x23: /* Session opened */
		case 0x80: /* Transfer started */
			return ERR_NONE;
		case 0x82: /* Transfer canceled */
			return ERR_CANCELED;
		default:
			smprintf(s, "WARNING: Packet seems to indicate some status by %02X, ignoring!\n", msg.Buffer[8]);
			return ERR_NONE;
	}
}

static GSM_Error ALCATEL_ReplyCommit(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	s->Phone.Data.Priv.ALCATEL.CommitedRecord = msg.Buffer[12] + (msg.Buffer[11] << 8) + (msg.Buffer[10] << 16) + (msg.Buffer[9] << 24);
	smprintf(s, "Created record %08x\n", s->Phone.Data.Priv.ALCATEL.CommitedRecord);
	return ERR_NONE;
}

static GSM_Error ALCATEL_SetIncomingCB (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingCB(s, enable);
}

static GSM_Error ALCATEL_SetIncomingSMS (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingSMS(s, enable);
}

static GSM_Error ALCATEL_SetFastSMSSending(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetFastSMSSending(s, enable);
}

GSM_Reply_Function ALCATELReplyFunctions[] = {
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelAttach		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelDetach		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelCommit		},
{ALCATEL_ReplyCommit,		"\x02",0x00,0x00, ID_AlcatelCommit2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelEnd			},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelClose		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelStart		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelSelect1		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelSelect2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelSelect3		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelBegin1		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelBegin2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetIds1		},
{ALCATEL_ReplyGetIds,		"\x02",0x00,0x00, ID_AlcatelGetIds2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetCategories1	},
{ALCATEL_ReplyGetCategories,	"\x02",0x00,0x00, ID_AlcatelGetCategories2	},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetCategoryText1	},
{ALCATEL_ReplyGetCategoryText,	"\x02",0x00,0x00, ID_AlcatelGetCategoryText2	},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelAddCategoryText1	},
{ALCATEL_ReplyAddCategoryText,	"\x02",0x00,0x00, ID_AlcatelAddCategoryText2	},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetFields1		},
{ALCATEL_ReplyGetFields,	"\x02",0x00,0x00, ID_AlcatelGetFields2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetFieldValue1	},
{ALCATEL_ReplyGetFieldValue,	"\x02",0x00,0x00, ID_AlcatelGetFieldValue2	},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelDeleteField		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelDeleteItem1		},
{ALCATEL_ReplyDeleteItem,	"\x02",0x00,0x00, ID_AlcatelDeleteItem2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelCreateField		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelUpdateField		},
{NULL,				"\x00",0x00,0x00, ID_None			}
};

GSM_Phone_Functions ALCATELPhone = {
	/* AFAIK, any 50[0123] phone should work, but I'm not sure whether all
	 * they were ever really released, if yes add them here also.
	 */
	"alcatel|OT501|OT701|OT715|OT535|OT735|BE5|BF5|BH4",
	ALCATELReplyFunctions,
	ALCATEL_Initialise,
	ALCATEL_Terminate,
	ALCATEL_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	ALCATEL_GetManufacturer,
	ALCATEL_GetModel,
	ALCATEL_GetFirmware,
	ALCATEL_GetIMEI,
	NOTSUPPORTED,			/* 	GetOriginalIMEI		*/
	NOTSUPPORTED,			/* 	GetManufactureMonth	*/
        ALCATEL_GetProductCode,
	NOTSUPPORTED,			/* 	GetHardware		*/
	NOTSUPPORTED,			/* 	GetPPM			*/
	ALCATEL_GetSIMIMSI,
	ALCATEL_GetDateTime,
	ALCATEL_SetDateTime,
	ALCATEL_GetAlarm,
	ALCATEL_SetAlarm,
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	ALCATEL_PressKey,
	ALCATEL_Reset,
	ALCATEL_ResetPhoneSettings,
	ALCATEL_EnterSecurityCode,
	ALCATEL_GetSecurityStatus,
	ALCATEL_GetDisplayStatus,
	ALCATEL_SetAutoNetworkLogin,
	ALCATEL_GetBatteryCharge,
	ALCATEL_GetSignalStrength,
	ALCATEL_GetNetworkInfo,
	ALCATEL_GetCategory,
	ALCATEL_AddCategory,
	ALCATEL_GetCategoryStatus,
	ALCATEL_GetMemoryStatus,
	ALCATEL_GetMemory,
	ALCATEL_GetNextMemory,
	ALCATEL_SetMemory,
	ALCATEL_AddMemory,
	ALCATEL_DeleteMemory,
	ALCATEL_DeleteAllMemory,
	NOTSUPPORTED,			/* 	GetSpeedDial		*/
	NOTSUPPORTED,			/* 	SetSpeedDial		*/
	ALCATEL_GetSMSC,
	ALCATEL_SetSMSC,
	ALCATEL_GetSMSStatus,
	ALCATEL_GetSMS,
	ALCATEL_GetNextSMS,
	NOTSUPPORTED,			/*	SetSMS			*/
	ALCATEL_AddSMS,
	ALCATEL_DeleteSMS,
	ALCATEL_SendSMS,
	ALCATEL_SendSavedSMS,
	ALCATEL_SetFastSMSSending,
	ALCATEL_SetIncomingSMS,
	ALCATEL_SetIncomingCB,
	ALCATEL_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	ALCATEL_DialVoice,
	ALCATEL_DialService,
	ALCATEL_AnswerCall,
	ALCATEL_CancelCall,
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	NONEFUNCTION,			/* 	SetIncomingCall		*/
	NOTSUPPORTED,			/* 	SetIncomingUSSD		*/
	ALCATEL_SendDTMF,
	NOTSUPPORTED,			/* 	GetRingtone		*/
	NOTSUPPORTED,			/* 	SetRingtone		*/
	NOTSUPPORTED,			/* 	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/* 	PlayTone		*/
	NOTSUPPORTED,			/* 	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark		*/
	NOTSUPPORTED,			/* 	DeleteWAPBookmark	*/
	NOTSUPPORTED,			/* 	GetWAPSettings		*/
	NOTSUPPORTED,			/* 	SetWAPSettings		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	NOTSUPPORTED,			/* 	GetBitmap		*/
	NOTSUPPORTED,			/* 	SetBitmap		*/
	ALCATEL_GetToDoStatus,
	ALCATEL_GetToDo,
	ALCATEL_GetNextToDo,
	ALCATEL_SetToDo,
	ALCATEL_AddToDo,
	ALCATEL_DeleteToDo,
	ALCATEL_DeleteAllToDo,
	ALCATEL_GetCalendarStatus,
	ALCATEL_GetCalendar,
	ALCATEL_GetNextCalendar,
	ALCATEL_SetCalendar,
	ALCATEL_AddCalendar,
	ALCATEL_DeleteCalendar,
	ALCATEL_DeleteAllCalendar,
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	NOTSUPPORTED,			/* 	GetProfile		*/
	NOTSUPPORTED,			/* 	SetProfile		*/
	NOTSUPPORTED,			/* 	GetFMStation		*/
	NOTSUPPORTED,			/* 	SetFMStation		*/
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFilePart		*/
	NOTSUPPORTED,			/* 	SendFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif
#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
