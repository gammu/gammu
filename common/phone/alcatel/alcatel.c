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
 *
 * Notes for future features:
 *  - max phone number length is 61
 *  - max name length is 50
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ALCATEL

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "alcatel.h"

extern GSM_Reply_Function ALCATELReplyFunctions[];
extern GSM_Reply_Function ATGENReplyFunctions[];

extern GSM_Error ATGEN_Initialise		(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetIMEI 			(GSM_StateMachine *s, unsigned char *imei);
extern GSM_Error ATGEN_GetFirmware		(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetModel			(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetDateTime		(GSM_StateMachine *s, GSM_DateTime *date_time);
extern GSM_Error ATGEN_GetAlarm			(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number);
extern GSM_Error ATGEN_GetMemory		(GSM_StateMachine *s, GSM_PhonebookEntry *entry);
extern GSM_Error ATGEN_SetMemory		(GSM_StateMachine *s, GSM_PhonebookEntry *entry);
extern GSM_Error ATGEN_GetMemoryStatus		(GSM_StateMachine *s, GSM_MemoryStatus *Status);
extern GSM_Error ATGEN_GetSMSC			(GSM_StateMachine *s, GSM_SMSC *smsc);
extern GSM_Error ATGEN_GetSMSMessage		(GSM_StateMachine *s, GSM_MultiSMSMessage *sms);
extern GSM_Error ATGEN_GetBatteryLevel		(GSM_StateMachine *s, int *level);
extern GSM_Error ATGEN_GetNetworkLevel		(GSM_StateMachine *s, int *level);
extern GSM_Error ATGEN_GetSMSFolders		(GSM_StateMachine *s, GSM_SMSFolders *folders);
extern GSM_Error ATGEN_GetNextSMSMessage	(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start);
extern GSM_Error ATGEN_GetSMSStatus		(GSM_StateMachine *s, GSM_SMSMemoryStatus *status);
extern GSM_Error ATGEN_DialVoice		(GSM_StateMachine *s, char *number);
extern GSM_Error ATGEN_AnswerCall		(GSM_StateMachine *s);
extern GSM_Error ATGEN_CancelCall		(GSM_StateMachine *s);
extern GSM_Error ATGEN_SaveSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_SendSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_SetDateTime		(GSM_StateMachine *s, GSM_DateTime *date_time);
extern GSM_Error ATGEN_DeleteSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_SetSMSC			(GSM_StateMachine *s, GSM_SMSC *smsc);
extern GSM_Error ATGEN_EnterSecurityCode	(GSM_StateMachine *s, GSM_SecurityCode Code);
extern GSM_Error ATGEN_GetSecurityStatus	(GSM_StateMachine *s, GSM_SecurityCodeType *Status);
extern GSM_Error ATGEN_ResetPhoneSettings	(GSM_StateMachine *s, GSM_ResetSettingsType Type);
extern GSM_Error ATGEN_SendDTMF			(GSM_StateMachine *s, char *sequence);
extern GSM_Error ATGEN_GetSIMIMSI		(GSM_StateMachine *s, char *IMSI);

extern GSM_Error ATGEN_DispatchMessage		(GSM_StateMachine *s);

/* Alcatel uses some 8-bit characters in contacts, calendar etc.. This table
 * attempts to decode it, it is probably not complete, here are just chars
 * that I found...
 */
unsigned char GSM_AlcatelAlphabet[] =
{
    0x89,0x00,0xc1, /* A acute 		*/
    0xcf,0x00,0x5c, /* \ 		*/
    0xce,0x00,0x7e, /* ~ 		*/
    0xfc,0x20,0xac, /* Euro 		*/
    0x84,0x00,0xe7, /* c cedilla 	*/
    0x98,0x00,0xe1, /* a acute 		*/
    0x00,0x00,0x00
};

static GSM_Error ALCATEL_SetBinaryMode(GSM_StateMachine *s)
{
    GSM_Phone_ALCATELData   *Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               error;

    if (Priv->Mode == ModeBinary) return GE_NONE;

    dprintf ("Changing to binary mode\n");

    error=GSM_WaitFor (s, "AT+IFC=2,2\r", 11, 0x02, 4, ID_SetFlowControl);
    if (error != GE_NONE) return error;
    error=GSM_WaitFor (s, "AT+CPROT=16,\"V1.0\",16\r", 22, 0x00, 4, ID_AlcatelConnect);
    if (error != GE_NONE) return error;
	/* FIXME */
    //tcdrain(s->Device.Data.Serial.hPhone);

    dprintf ("Changing protocol to Alcabus\n");

    s->Protocol.Functions               = &ALCABUSProtocol;
    error = s->Protocol.Functions->Initialise(s);
    if (error != GE_NONE) {
        s->Protocol.Functions           = &ATProtocol;
        return error;
    }
    s->Phone.Functions->ReplyFunctions  = ALCATELReplyFunctions;
    Priv->Mode                          = ModeBinary;
    Priv->BinaryItem                    = 0;
    Priv->BinaryType                    = 0;

    Priv->BinaryState                   = StateAttached;
    return GE_NONE;
}

static GSM_Error ALCATEL_GoToBinaryState(GSM_StateMachine *s, GSM_Alcatel_BinaryState state, GSM_Alcatel_BinaryType type, int item) {
    GSM_Phone_ALCATELData   *Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               error;
    unsigned char attach_buffer[] = {0x00, 0x00, 0x7C ,0x20};
    unsigned char detach_buffer[] = {0x00, 0x01, 0x7C ,0x00};
    unsigned char start_buffer[] =
		{0x00, 0x04, 0x7C, 0x80, /* 4 byte database id follows */
		 0x12, 0x34, 0x56, 0x78};
    unsigned char close_buffer[] =
		{0x00, 0x04,
		 0x00, 			 /*type */
		 0x23, 0x01};
    unsigned char select1_buffer[] = 
		{0x00, 0x00,
		 0x00, 			 /*type */
		 0x20};
    unsigned char select2_buffer[] =
		{0x00, 0x04,
		 0x00, 			 /*type */
		 0x22, 0x01, 0x00};
    unsigned char begin_buffer[] =
		{0x00, 0x04, 0x7C, 0x81,
		 0x00,			 /*type */
		 0x00, 0x85, 0x00};
    unsigned char commit_buffer[] =
		{0x00, 0x04,
		 0x00, 			/*type */
		 0x20, 0x01};

    error = ALCATEL_SetBinaryMode(s);
    if (error != GE_NONE) return error;

    /* Do we need to do anything? */
    if ((state == Priv->BinaryState) && (type == Priv->BinaryType) && (item == Priv->BinaryItem)) return GE_NONE;

    /* We're editing, but the next state is not the same. so commit editing */
    if (Priv->BinaryState == StateEdit) {
        switch (Priv->BinaryType) {
            case TypeCalendar:
                commit_buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
                break;
            case TypeContacts:
                commit_buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
                break;
            case TypeTodo:
                commit_buffer[2] = ALCATEL_SYNC_TYPE_TODO;
                break;
        }
        dprintf ("Commiting edited record\n");
        error=GSM_WaitFor (s, commit_buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelCommit);
        if (error != GE_NONE) return error;
        error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelCommit2);
        if (error != GE_NONE) return error;
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
        return GE_NONE;
    }

    /* Now we can be only in Attached or Session state, so if states and types matches, just keep them as they are */
    if ((state == Priv->BinaryState) && (type == Priv->BinaryType)) {
        return GE_NONE;
    }

    /* Do we need to close session? */
    if (Priv->BinaryState == StateSession) {
        switch (Priv->BinaryType) {
            case TypeCalendar:
                close_buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
                break;
            case TypeContacts:
                close_buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
                break;
            case TypeTodo:
                close_buffer[2] = ALCATEL_SYNC_TYPE_TODO;
                break;
        }
        dprintf ("Closing session\n");
        error=GSM_WaitFor (s, close_buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelClose);
        if (error != GE_NONE) return error;

        dprintf ("Detaching binary mode\n");
        GSM_WaitFor (s, detach_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelDetach);
        
        Priv->BinaryState = StateAttached;
        Priv->BinaryType = 0;
    }

    /* Do we need to open session? */
    if (state == StateSession || state == StateEdit) {
        dprintf ("Starting session for %s\n",
                (type == TypeCalendar ? "Calendar" : 
                 (type == TypeTodo ? "Todo" :
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
            case TypeTodo:
                select1_buffer[2] 	= ALCATEL_SYNC_TYPE_TODO;
                select2_buffer[2] 	= ALCATEL_SYNC_TYPE_TODO;
                begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_TODO;
                break;
        }
        dprintf ("Attaching in binary mode\n");

        /* Communicate */
        error=GSM_WaitFor (s, attach_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelAttach);
        if (error != GE_NONE) return error;

        smprintf(s,"Start session\n");
        error=GSM_WaitFor (s, start_buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelStart);
        if (error != GE_NONE) return error;

        smprintf(s,"Select type\n");
        error=GSM_WaitFor (s, select1_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelSelect1);
        if (error != GE_NONE) return error;
        error=GSM_WaitFor (s, select2_buffer, 6, 0x02, ALCATEL_TIMEOUT, ID_AlcatelSelect2);
        if (error != GE_NONE) return error;
        error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelSelect3);
        if (error != GE_NONE) return error;

        smprintf(s,"Begin transfer\n");
        error=GSM_WaitFor (s, begin_buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelBegin1);
        if (error != GE_NONE) return error;
        error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelBegin2);
        if (error != GE_NONE) return error;

        Priv->BinaryState = StateSession;
        Priv->BinaryType = type;
        /* Do we want to edit something of same type? */
        if ((state == StateEdit) && (type == Priv->BinaryType)) {
            Priv->BinaryState = StateEdit;
            Priv->BinaryItem = item;
            return GE_NONE;
        }
    }
    return GE_NONE;
}

static GSM_Error ALCATEL_SetATMode(GSM_StateMachine *s)
{
    GSM_Phone_ALCATELData   *Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               error;

    if (Priv->Mode == ModeAT) return GE_NONE;

    error = ALCATEL_GoToBinaryState(s, StateAttached, 0, 0);
    if (error != GE_NONE) return error;

    error = s->Protocol.Functions->Terminate(s);

    dprintf ("Changing protocol to AT\n");
    s->Protocol.Functions               = &ATProtocol;
    s->Phone.Functions->ReplyFunctions  = ATGENReplyFunctions;
    Priv->Mode                          = ModeAT;
    mili_sleep(10);
    /* In case we don't send AT command short after closing binary mode, phone 
     * takes VERY long to react next time.
     */
	error = s->Protocol.Functions->WriteMessage(s, "AT\r", 3, 0x00);
	if (error!=GE_NONE) return error;
	mili_sleep(200);

    return error;
}

static GSM_Error ALCATEL_Initialise(GSM_StateMachine *s)
{
    GSM_Phone_ALCATELData   *Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               error;

    Priv->Mode                          = ModeAT;

    Priv->CalendarItems                 = NULL;
    Priv->ContactsItems                 = NULL;
    Priv->ToDoItems                     = NULL;
    Priv->CalendarItemsCount            = 0;
    Priv->ToDoItemsCount                = 0;
    Priv->ContactsItemsCount            = 0;
    Priv->CurrentFields[0]              = 0;
    Priv->CurrentFieldsCount            = 0;
    Priv->CurrentFieldsItem             = 0;
    Priv->CurrentFieldsType             = 0;

    s->Protocol.Functions               = &ATProtocol;
    s->Phone.Functions->ReplyFunctions  = ATGENReplyFunctions;

    if (ATGEN_Initialise(s) != GE_NONE) {
        smprintf(s,"AT initialisation failed, trying to stop binary mode...\n");
        s->Protocol.Functions           = &ALCABUSProtocol;
        error = s->Protocol.Functions->Terminate(s);
        s->Protocol.Functions           = &ATProtocol;
        
        error = ATGEN_Initialise(s);
        if (error != GE_NONE) return error;
    }

    return GE_NONE;
}

static GSM_Error ALCATEL_Terminate(GSM_StateMachine *s)
{
    GSM_Phone_ALCATELData   *Priv = &s->Phone.Data.Priv.ALCATEL;

    free(Priv->CalendarItems);
    free(Priv->ContactsItems);
    free(Priv->ToDoItems);
    return ALCATEL_SetATMode(s);
}

/* finds whether id is set in the phone */
static GSM_Error ALCATEL_IsIdAvailable(GSM_StateMachine *s, int id) {
    GSM_Phone_ALCATELData   	*Priv = &s->Phone.Data.Priv.ALCATEL;
    int 			i = 0;
    
    switch (Priv->BinaryType) {
        case TypeCalendar:
            Priv->CurrentList 	= &(Priv->CalendarItems);
            Priv->CurrentCount 	= &(Priv->CalendarItemsCount);
            break;
        case TypeContacts:
            Priv->CurrentList 	= &(Priv->ContactsItems);
            Priv->CurrentCount 	= &(Priv->ContactsItemsCount);
            break;
        case TypeTodo:
            Priv->CurrentList 	= &(Priv->ToDoItems);
            Priv->CurrentCount 	= &(Priv->ToDoItemsCount);
            break;
    }

    while ((*Priv->CurrentList)[i] != 0) {
        if ((*Priv->CurrentList)[i] == id) return GE_NONE;
        i++;
    }
    return GE_EMPTY;
}

static GSM_Error ALCATEL_ReplyGetIds(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
    GSM_Phone_ALCATELData 	*Priv = &Data->Priv.ALCATEL;
    int 			count,i,pos;
    
    count 		 = msg.Buffer[10];
    *Priv->CurrentCount += count;
    
    *Priv->CurrentList = (int *)realloc(*Priv->CurrentList, (*Priv->CurrentCount + 1)* sizeof(int));
    if (*Priv->CurrentList == NULL) return GE_MOREMEMORY;

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
    return GE_NONE;
}

static GSM_Error ALCATEL_GetAvailableIds(GSM_StateMachine *s)
{
    GSM_Phone_ALCATELData   	*Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               	error;
    int                     	i;
    unsigned char 		buffer[] =
		{0x00, 0x04,
		 0x00, 			/*type */
		 0x2F, 0x01};

    if (Priv->BinaryState != StateSession) return GE_UNKNOWN;
    
    switch (Priv->BinaryType) {
        case TypeCalendar:
            buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
            Priv->CurrentList  = &(Priv->CalendarItems);
            Priv->CurrentCount = &(Priv->CalendarItemsCount);
            break;
        case TypeContacts:
            buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
            Priv->CurrentList  = &(Priv->ContactsItems);
            Priv->CurrentCount = &(Priv->ContactsItemsCount);
            break;
        case TypeTodo:
            buffer[2] = ALCATEL_SYNC_TYPE_TODO;
            Priv->CurrentList  = &(Priv->ToDoItems);
            Priv->CurrentCount = &(Priv->ToDoItemsCount);
            break;
    }

    if (*Priv->CurrentList != NULL) return GE_NONE;
    smprintf(s,"Reading items list\n");

    *Priv->CurrentCount = 0;
    Priv->TransferCompleted = false;
    
    error=GSM_WaitFor (s, buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetIds1);
    if (error != GE_NONE) return error;
    
    while (!Priv->TransferCompleted) {
        error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetIds2);
        if (error != GE_NONE) return error;
    }

    i = 0;
    smprintf(s,"Received %d ids: ", *Priv->CurrentCount);
    while ((*Priv->CurrentList)[i] != 0) {
        smprintf(s,"%i ", (*Priv->CurrentList)[i]);
        i++;
    }
    smprintf(s,"\n");
    
    return GE_NONE;
}

static GSM_Error ALCATEL_ReplyGetFields(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
    GSM_Phone_ALCATELData 	*Priv = &Data->Priv.ALCATEL;
    int 			i;

    if (msg.Buffer[14] > GSM_PHONEBOOK_ENTRIES) {
        dprintf("WARNING: Field list truncated, you should increase GSM_PHONEBOOK_ENTRIES to at least %d\n", msg.Buffer[14]);
        Priv->CurrentFieldsCount = GSM_PHONEBOOK_ENTRIES;
    } else {
        Priv->CurrentFieldsCount = msg.Buffer[14];
    }
    
    Priv->CurrentFields[Priv->CurrentFieldsCount] = 0;
        
    for (i = 0; i < Priv->CurrentFieldsCount; i++) {
        Priv->CurrentFields[i] = msg.Buffer[15 + i];
    }

    return GE_NONE;
}

static GSM_Error ALCATEL_GetFields(GSM_StateMachine *s, int id) {
    GSM_Phone_ALCATELData   	*Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               	error;
    int                     	i;
    unsigned char 		buffer[] =
		{0x00, 0x04,
		 0x00, 			/*type */
		 0x30, 0x01, 		/*here follows 4byte id*/
		 0x00, 0x00, 0x00, 0x00};    

    if (Priv->BinaryState != StateSession) return GE_UNKNOWN;
    if ((Priv->CurrentFieldsItem == id) && (Priv->CurrentFieldsType == Priv->BinaryType)) return GE_NONE;

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
        case TypeTodo:
            buffer[2] = ALCATEL_SYNC_TYPE_TODO;
            break;
    }

    Priv->CurrentFieldsItem = id;
    Priv->CurrentFieldsType = Priv->BinaryType;
    
    error=GSM_WaitFor (s, buffer, 9, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetFields1);
    if (error != GE_NONE) return error;
    error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetFields2);
    if (error != GE_NONE) return error;
    
    i = 0;
    smprintf(s,"Received %d fields: ", Priv->CurrentFieldsCount);
    while (Priv->CurrentFields[i] != 0) {
        smprintf(s,"%i ", Priv->CurrentFields[i]);
        i++;
    }
    smprintf(s,"\n");

    return GE_NONE;
}

static GSM_Error ALCATEL_ReplyGetFieldValue(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
    GSM_Phone_ALCATELData 	*Priv = &Data->Priv.ALCATEL;
    unsigned char 		*buffer = &(msg.Buffer[16]);

    if (buffer[1] == 0x05 && buffer[2] == 0x67) {
        /* date */
        Priv->ReturnType 		= Alcatel_date;
        Priv->ReturnDateTime.Day 	= buffer[4];
        Priv->ReturnDateTime.Month 	= buffer[5];
        Priv->ReturnDateTime.Year 	= buffer[7] + (buffer[6] << 8); 
    } else if (buffer[1] == 0x06 && buffer[2] == 0x68) {
        /* time */
        Priv->ReturnType 		= Alcatel_time;
        Priv->ReturnDateTime.Hour 	= buffer[4];
        Priv->ReturnDateTime.Minute 	= buffer[5];
        Priv->ReturnDateTime.Second 	= buffer[6];
    } else if (buffer[1] == 0x08 && buffer[2] == 0x3C) {
        /* string */
        Priv->ReturnType = Alcatel_string;
        if (GSM_PHONEBOOK_TEXT_LENGTH < buffer[3])
            dprintf("WARNING: Text truncated, you should increase GSM_PHONEBOOK_TEXT_LENGTH to at least %d\n", buffer[3] + 1);
        DecodeDefault( Priv->ReturnString, buffer + 4, MIN(GSM_PHONEBOOK_TEXT_LENGTH, buffer[3]), false, GSM_AlcatelAlphabet);
    } else if (buffer[1] == 0x07 && buffer[2] == 0x3C) {
        /* phone */
        Priv->ReturnType = Alcatel_phone;
        if (GSM_PHONEBOOK_TEXT_LENGTH < buffer[3])
            dprintf("WARNING: Text truncated, you should increase GSM_PHONEBOOK_TEXT_LENGTH to at least %d\n", buffer[3] + 1);
        DecodeDefault( Priv->ReturnString, buffer + 4, MIN(GSM_PHONEBOOK_TEXT_LENGTH, buffer[3]), false, GSM_AlcatelAlphabet);
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
        dprintf("WARNING: Uknown data type received (%02X,%02X)\n", buffer[1], buffer[2]);
        return GE_UNKNOWNRESPONSE;
    }
    return GE_NONE;
}

static GSM_Error ALCATEL_GetFieldValue(GSM_StateMachine *s, int id, int field)
{
    GSM_Phone_ALCATELData   	*Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error               	error;
    unsigned char 		buffer[] =
		{0x00, 0x04,
		 0x00, 				/*type */
		 0x1f, 0x01,  			/*here follows 4byte id*/
		 0x00, 0x00, 0x00, 0x00, 0x00};	/*field*/

    smprintf(s,"Reading item value (%08x.%02x)\n", id, field);
    
    switch (Priv->BinaryType) {
        case TypeCalendar:
            buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
            break;
        case TypeContacts:
            buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
            break;
        case TypeTodo:
            buffer[2] = ALCATEL_SYNC_TYPE_TODO;
            break;
    }

    buffer[5] = (id >> 24);
    buffer[6] = ((id >> 16) & 0xff);
    buffer[7] = ((id >> 8) & 0xff);
    buffer[8] = (id & 0xff);
    buffer[9] = (field & 0xff);

    error=GSM_WaitFor (s, buffer, 10, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetFieldValue1);
    if (error != GE_NONE) return error;
    error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetFieldValue2);
    if (error != GE_NONE) return error;

    return GE_NONE;
}

static GSM_Error ALCATEL_GetManufacturer(GSM_StateMachine *s, char *manufacturer)
{
    EncodeUnicode(manufacturer,"Alcatel",7);
    return GE_NONE;
}

static GSM_Error ALCATEL_GetIMEI (GSM_StateMachine *s, unsigned char *imei)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetIMEI(s, imei);
}

static GSM_Error ALCATEL_GetFirmware(GSM_StateMachine *s)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetFirmware(s);
}

static GSM_Error ALCATEL_GetModel(GSM_StateMachine *s)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetModel(s);
}

static GSM_Error ALCATEL_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetDateTime(s, date_time);
}

static GSM_Error ALCATEL_GetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
    GSM_Error error;

    if (alarm_number != 1) return GE_NOTSUPPORTED;
    /* XXX: In fact phone reports 5 alarms, but setting more than one really
     * confuses it */
    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    /* TODO: this maybe should be done in binary mode, because alarms over AT
     * commands seems to be a bit unreliable */
    return ATGEN_GetAlarm(s, alarm, alarm_number);
}

static GSM_Error ALCATEL_GetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
    GSM_Error 			error;
    GSM_Phone_ALCATELData   	*Priv = &s->Phone.Data.Priv.ALCATEL;
    int 			i;

    if (entry->MemoryType == GMT_ME) {
        if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= GE_NONE) return error;
        if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
        if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= GE_NONE) {
            entry->EntriesNum = 0;
            return error;
        }
        if ((error = ALCATEL_GetFields(s, entry->Location))!= GE_NONE) return error;

        entry->EntriesNum = Priv->CurrentFieldsCount;
        
        for (i=0; i<Priv->CurrentFieldsCount; i++) {
            if ((error = ALCATEL_GetFieldValue(s, entry->Location, Priv->CurrentFields[i]))!= GE_NONE) return error;
            entry->Entries[i].VoiceTag = 0;
            switch (Priv->CurrentFields[i]) {
                case 0:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_LastName;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 1:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_FirstName;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 2:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Company;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 3:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_JobTitle;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 4:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Note;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 5:
                    if (Priv->ReturnType != Alcatel_byte) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Category;
                    entry->Entries[i].Number = Priv->ReturnInt;
                    break;
                case 6:
                    if (Priv->ReturnType != Alcatel_bool) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Private;
                    entry->Entries[i].Number = Priv->ReturnInt;
                    break;
                case 7:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_Work;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 8:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_General;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 9:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_Fax;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 10:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_Other;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 11:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_Pager;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 12:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_Mobile;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 13:
                    if (Priv->ReturnType != Alcatel_phone) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Number_Home;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 14:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Email;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 15:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Email;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 16:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_StreetAddress;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 17:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_City;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 18:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_State;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 19:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Zip;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 20:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Country;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 21:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Custom1;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 22:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Custom2;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 23:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Custom3;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                case 24:
                    if (Priv->ReturnType != Alcatel_string) {
                        smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
                        break;
                    }
                    entry->Entries[i].EntryType = PBK_Text_Custom4;
                    CopyUnicodeString(entry->Entries[i].Text, Priv->ReturnString);
                    break;
                default:
                    smprintf(s,"WARNING: Received unknown field %02X, ignoring\n", Priv->CurrentFields[i]);
            }
        }
        return GE_NONE;
    } else {
        if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
        return ATGEN_GetMemory(s, entry);
    }
}

static GSM_Error ALCATEL_SetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
    GSM_Error error;

    if (entry->MemoryType == GMT_ME) {
        if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= GE_NONE) return error;
        if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
        if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= GE_NONE) return error;
        if ((error = ALCATEL_GetFields(s, entry->Location))!= GE_NONE) return error;
        /* TODO: do the real setting */
        return GE_WORKINPROGRESS;
    } else {
        if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
        return ATGEN_SetMemory(s, entry);
    }
}


static GSM_Error ALCATEL_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
    GSM_Phone_ALCATELData   	*Priv = &s->Phone.Data.Priv.ALCATEL;
    GSM_Error 			error;

    if (Status->MemoryType == GMT_ME) {
        if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= GE_NONE) return error;
        if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
        Status->Used = Priv->ContactsItemsCount;
        Status->Free = ALCATEL_FREE_MEMORY;
        return GE_NONE;
    } else {
        if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
        return ATGEN_GetMemoryStatus(s, Status);
    }
}

static GSM_Error ALCATEL_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetSMSC(s, smsc);
}

static GSM_Error ALCATEL_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetSMSMessage(s, sms);
}

static GSM_Error ALCATEL_GetBatteryLevel(GSM_StateMachine *s, int *level)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetBatteryLevel(s, level);
}

static GSM_Error ALCATEL_GetNetworkLevel(GSM_StateMachine *s, int *level)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetNetworkLevel(s, level);
}

static GSM_Error ALCATEL_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetSMSFolders(s, folders);
}

static GSM_Error ALCATEL_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetNextSMSMessage(s, sms, start);
}

static GSM_Error ALCATEL_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetSMSStatus(s, status);
}

static GSM_Error ALCATEL_DialVoice(GSM_StateMachine *s, char *number)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_DialVoice(s, number);
}

static GSM_Error ALCATEL_AnswerCall(GSM_StateMachine *s)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_AnswerCall(s);
}

static GSM_Error ALCATEL_CancelCall(GSM_StateMachine *s)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_CancelCall(s);
}

static GSM_Error ALCATEL_SaveSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_SaveSMSMessage(s, sms);
}

static GSM_Error ALCATEL_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_SendSMSMessage(s, sms);
}

static GSM_Error ALCATEL_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_SetDateTime(s, date_time);
}

static GSM_Error ALCATEL_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_DeleteSMSMessage(s, sms);
}

static GSM_Error ALCATEL_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_SetSMSC(s, smsc);
}

static GSM_Error ALCATEL_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_EnterSecurityCode(s, Code);
}

static GSM_Error ALCATEL_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetSecurityStatus(s, Status);
}

static GSM_Error ALCATEL_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
    /* XXX: This does probably nothing, but it doesn't return error and was easy
     * to implement ;-)
     */
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_ResetPhoneSettings(s, Type);
}

static GSM_Error ALCATEL_SendDTMF(GSM_StateMachine *s, char *sequence)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_SendDTMF(s, sequence);
}

static GSM_Error ALCATEL_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
    GSM_Error error;

    if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
    return ATGEN_GetSIMIMSI(s, IMSI);
}

static GSM_Error ALCATEL_GetCalendarNote (GSM_StateMachine *s, GSM_CalendarNote *Note, bool start)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    if ((error = ALCATEL_IsIdAvailable(s, Note->Location))!= GE_NONE) return error;
    if ((error = ALCATEL_GetFields(s, Note->Location))!= GE_NONE) return error;
    /* TODO: do the real getting */
    return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_SetAlarm (GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    /* TODO: do the real setting */
    return GE_WORKINPROGRESS;
}
static GSM_Error ALCATEL_DeleteCalendarNote  (GSM_StateMachine *s, GSM_CalendarNote *Note)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    /* TODO: do the real deleting */
    return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_SetCalendarNote (GSM_StateMachine *s, GSM_CalendarNote *Note)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    /* TODO: do the real setting */
    return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_GetToDo (GSM_StateMachine *s, GSM_TODO *ToDo, bool refresh)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeTodo, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    if ((error = ALCATEL_IsIdAvailable(s, ToDo->Location))!= GE_NONE) return error;
    if ((error = ALCATEL_GetFields(s, ToDo->Location))!= GE_NONE) return error;
    /* TODO: do the real reading */
    return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_DeleteAllToDo (GSM_StateMachine *s)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeTodo, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    /* TODO: do the real deleting */
    return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_SetToDo (GSM_StateMachine *s, GSM_TODO *ToDo)
{
    GSM_Error error;

    if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeTodo, 0))!= GE_NONE) return error;
    if ((error = ALCATEL_GetAvailableIds(s))!= GE_NONE) return error;
    /* TODO: do the real setting */
    return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_DispatchMessage(GSM_StateMachine *s)
{
    if (s->Phone.Data.Priv.ALCATEL.Mode == ModeBinary) {
        return GSM_DispatchMessage(s);
    } else {
        return ATGEN_DispatchMessage(s);
    }
}

static GSM_Error ALCATEL_ReplyGeneric(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
    unsigned char c;

    c = msg.Buffer[8];
    /* All error values are just VERY wild guesses, but these seems to work
     * almost as expected ...
     */
    if (c != 0x00) {
        switch (c) {
            case 0x10: /* same thing opened in phone menus */
                return GE_INSIDEPHONEMENU;
            case 0x14: /* Bad data */
            case 0x13: /* Closing bad session */
            case 0x2f: /* Closing session when not opened */
            case 0x1f: /* Bad in/out counter in packet/ack */
            case 0x0e: /* Openning session when not closed */
                return GE_BUG;
            case 0x0C: /* Bad id (item/database) */
            case 0x11: /* Bad list id */
            case 0x2A: /* Nonexistant field/item id */
                return GE_INVALIDLOCATION;
            case 0x23: /* Session opened */
            case 0x80: /* Transfer started */
                return GE_NONE;
            default:
                dprintf("WARNING: Packet seems to indicate some status by %02X, ignoring!\n", c);
        }
    }
    return GE_NONE;
}

static GSM_Error ALCATEL_ReplyCommit(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
    /* TODO: read received id */
    return GE_NONE;
}

static GSM_Reply_Function ALCATELReplyFunctions[] = {
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelAttach		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelDetach		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelCommit		},
{ALCATEL_ReplyCommit,       "\x02",0x00,0x00, ID_AlcatelCommit2		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelClose		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelStart		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelSelect1		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelSelect2		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelSelect3		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelBegin1		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelBegin2		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelGetIds1		},
{ALCATEL_ReplyGetIds,       "\x02",0x00,0x00, ID_AlcatelGetIds2		},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelGetFields1	},
{ALCATEL_ReplyGetFields,    "\x02",0x00,0x00, ID_AlcatelGetFields2	},
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, ID_AlcatelGetFieldValue1	},
{ALCATEL_ReplyGetFieldValue,"\x02",0x00,0x00, ID_AlcatelGetFieldValue2	},
/*
{ALCATEL_ReplyGeneric,      "\x02",0x00,0x00, 				},
*/
{NULL,                      "\x00",0x00,0x00, ID_None			}
};

GSM_Phone_Functions ALCATELPhone = {
    /* AFAIK, any 50[0123] phone should work, but I'm not sure whether all
     * they were ever really released, if yes add them here also.
     */
    "alcatel|A500|A501|A701|501|701|BE5",
    ALCATELReplyFunctions,
    ALCATEL_Initialise,
    ALCATEL_Terminate,
    ALCATEL_DispatchMessage,
    ALCATEL_GetModel,
    ALCATEL_GetFirmware,
    ALCATEL_GetIMEI,
    ALCATEL_GetDateTime,
    ALCATEL_GetAlarm,
    ALCATEL_GetMemory,
    ALCATEL_GetMemoryStatus,
    ALCATEL_GetSMSC,
    ALCATEL_GetSMSMessage,
    ALCATEL_GetBatteryLevel,
    ALCATEL_GetNetworkLevel,
    ALCATEL_GetSMSFolders,
    ALCATEL_GetManufacturer,
    ALCATEL_GetNextSMSMessage,
    ALCATEL_GetSMSStatus,
    NOTIMPLEMENTED,             /* SetIncomingSMS       */ /* TODO: I don't know what should this do ;-), should be same as in AT */
    NOTSUPPORTED,               /* GetNetworkInfo       */
    NOTSUPPORTED,               /* Reset                */
    ALCATEL_DialVoice,
    ALCATEL_AnswerCall,
    ALCATEL_CancelCall,
    NOTSUPPORTED,               /*  GetRingtone         */
    ALCATEL_GetCalendarNote,
    NOTSUPPORTED,               /*  GetWAPBookmark      */
    NOTSUPPORTED,               /*  GetBitmap           */
    NOTSUPPORTED,               /*  SetRingtone         */
    ALCATEL_SaveSMSMessage,
    ALCATEL_SendSMSMessage,
    ALCATEL_SetDateTime,
    ALCATEL_SetAlarm,
    NOTSUPPORTED,               /*  SetBitmap           */
    ALCATEL_SetMemory,
    ALCATEL_DeleteSMSMessage,
    ALCATEL_DeleteCalendarNote,
    ALCATEL_SetCalendarNote,
    NOTSUPPORTED,               /*  SetWAPBookmark      */
    NOTSUPPORTED,               /*  DeleteWAPBookmark   */
    NOTSUPPORTED,               /*  GetWAPSettings      */
    NOTIMPLEMENTED,             /*  SetIncomingCB       */
    ALCATEL_SetSMSC,
    NOTSUPPORTED,               /*  GetManufactureMonth */
    NOTSUPPORTED,               /*  GetProductCode      */
    NOTSUPPORTED,               /*  GetOriginalIMEI     */
    NOTSUPPORTED,               /*  GetHardware         */
    NOTSUPPORTED,               /*  GetPPM              */
    NOTSUPPORTED,               /*  PressKey            */
    ALCATEL_GetToDo,
    ALCATEL_DeleteAllToDo,
    ALCATEL_SetToDo,
    NOTSUPPORTED,               /*  PlayTone            */
    ALCATEL_EnterSecurityCode,
    ALCATEL_GetSecurityStatus,
    NOTSUPPORTED,               /*  GetProfile          */
    NOTSUPPORTED,               /*  GetRingtonesInfo    */
    NOTSUPPORTED,               /*  SetWAPSettings      */
    NOTSUPPORTED,               /*  GetSpeedDial        */
    NOTSUPPORTED,               /*  SetSpeedDial        */
    ALCATEL_ResetPhoneSettings,
    ALCATEL_SendDTMF,
    NOTSUPPORTED,               /*  GetDisplayStatus    */
    NOTSUPPORTED,               /*  SetAutoNetworkLogin */
    NOTSUPPORTED,               /*  SetProfile          */
    ALCATEL_GetSIMIMSI,
    NONEFUNCTION,               /*  SetIncomingCall     */
    NOTIMPLEMENTED		/*  GetNextCalendarNote	*/
};

#endif
