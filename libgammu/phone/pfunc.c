/* (c) 2002-2003 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include <gammu-ringtone.h>

#include "../gsmstate.h"
#include "../misc/coding/coding.h"
#include "../misc/locales.h"
#include "../service/gsmring.h"

/* These SMS layouts are used exactly as written in Nokia DCT3 phones.
 * In AT module(s) we have to use some layouts to convert AT frame to format
 * understod by SMS module. To share source the same layouts are used */
GSM_SMSMessageLayout PHONE_SMSDeliver = {
	35 	/*	SMS Text	*/,	16  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	14  /*	TPDCS		*/,
	28 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	15  /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	255	/*	TPMR		*/,	13  /*  TPPID		*/};
GSM_SMSMessageLayout PHONE_SMSSubmit = {
	36 	/*	SMS Text	*/,	17  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	15  /*	TPDCS		*/,
	255 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	16  /*	TPUDL		*/,
	29	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	14  /*  TPPID		*/};
GSM_SMSMessageLayout PHONE_SMSStatusReport = {
	255 	/*	SMS Text	*/,	15  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	255 /*	TPDCS		*/,
	27 	/*	SendingDateTime	*/,	34  /*	SMSCDateTime	*/,
	14	/*	TPStatus	*/,	255 /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	255 /*  TPPID??		*/};

GSM_Error PHONE_GetSMSFolders(GSM_StateMachine *s UNUSED, GSM_SMSFolders *folders)
{
	folders->Number=2;
	EncodeUnicode(folders->Folder[0].Name,_("Inbox"),strlen(_("Inbox")));
	EncodeUnicode(folders->Folder[1].Name,_("Outbox"),strlen(_("Outbox")));
	folders->Folder[0].InboxFolder = TRUE;
	folders->Folder[1].InboxFolder = FALSE;
	folders->Folder[0].OutboxFolder 	= FALSE;
	folders->Folder[1].OutboxFolder 	= TRUE;
	folders->Folder[0].Memory      = MEM_SM;
	folders->Folder[1].Memory      = MEM_SM;
	return ERR_NONE;
}

void GSM_CreateFirmwareNumber(GSM_StateMachine *s)
{
	StringToDouble(s->Phone.Data.Version, &s->Phone.Data.VerNum);
	smprintf(s, "Number version is \"%f\"\n", s->Phone.Data.VerNum);
}

GSM_Error PHONE_EncodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, gboolean clear)
{
	GSM_Error error;

	if (SMS->SMSC.Location != 0) {
		smprintf(s, "Getting SMSC from phone, location %d\n", SMS->SMSC.Location);
		error = s->Phone.Functions->GetSMSC(s, &SMS->SMSC);
		if (error != ERR_NONE) {
			return ERR_GETTING_SMSC;
		}
		SMS->SMSC.Location = 0;
	}
	if (SMS->PDU == SMS_Deliver) {
		if (SMS->SMSC.Number[0] == 0x00 && SMS->SMSC.Number[1] == 0x00) {
			smprintf(s,"No SMSC in SMS Deliver\n");
			return ERR_EMPTYSMSC;
		}
	}
	return GSM_EncodeSMSFrame(GSM_GetDI(s), SMS, buffer, Layout, length, clear);
}

GSM_Error PHONE_Terminate(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.EnableIncomingCB==TRUE) {
		error=s->Phone.Functions->SetIncomingCB(s,FALSE);
		if (error!=ERR_NONE) return error;
	}
	if (s->Phone.Data.EnableIncomingSMS==TRUE) {
		error=s->Phone.Functions->SetIncomingSMS(s,FALSE);
		if (error!=ERR_NONE) return error;
	}
	return ERR_NONE;
}

GSM_Error PHONE_RTTLPlayOneNote(GSM_StateMachine *s, GSM_RingNote note, gboolean first)
{
	int 		duration, Hz;
	GSM_Error 	error;

	Hz=GSM_RingNoteGetFrequency(note);

	error=s->Phone.Functions->PlayTone(s,Hz,5,first);
	if (error!=ERR_NONE) return error;

	duration = GSM_RingNoteGetFullDuration(note);

	/* Is it correct ? Experimental values here */
	switch (note.Style) {
		case INVALIDStyle:
			break;
		case StaccatoStyle:
			usleep(7500000);
			error=s->Phone.Functions->PlayTone(s,0,0,FALSE);
			if (error != ERR_NONE) return error;
			usleep ((1400000000L/note.Tempo*duration)-(7500000));
			break;
		case ContinuousStyle:
			usleep(1400000000L/note.Tempo*duration);
			break;
		case NaturalStyle:
			usleep(1400000000L/note.Tempo*duration-50000);
			error=s->Phone.Functions->PlayTone(s,0,0,FALSE);
			if (error != ERR_NONE) return error;
			usleep(50000);
			break;
	}
	return ERR_NONE;
}

GSM_Error PHONE_Beep(GSM_StateMachine *s)
{
	GSM_Error error;

	error=s->Phone.Functions->PlayTone(s, 4000, 5,TRUE);
	if (error!=ERR_NONE) return error;

	usleep(500000);

	return s->Phone.Functions->PlayTone(s,255*255,0,FALSE);
}

GSM_Error PHONE_FindDataFile(GSM_StateMachine *s, GSM_File * File, const char *ExtraPath, const char *filename)
{
	char *path;
	GSM_Error error;

	EncodeUnicode(File->Name, filename, strlen(filename));

	path = malloc(MAX(strlen(GAMMU_DATA_PATH), ExtraPath == NULL ? 0 : strlen(ExtraPath)) + 50);
	if (path == NULL) {
		return ERR_MOREMEMORY;
	}

	if (ExtraPath != NULL) {
		sprintf(path, "%s/%s", ExtraPath, filename);
		smprintf(s, "Trying to load from extra path: %s\n", path);

		error = GSM_ReadFile(path, File);
		if (error == ERR_NONE) {
			free(path);
			return error;
		}
	}

	sprintf(path, "%s/%s", GAMMU_DATA_PATH, filename);
	smprintf(s, "Trying to load from data path: %s\n", path);

	error = GSM_ReadFile(path, File);
	free(path);

	return error;
}

GSM_Error PHONE_UploadFile(GSM_StateMachine *s, GSM_File * File)
{
	size_t Pos = 0;
	int Handle = 0;
	GSM_Error error = ERR_NONE;;

	while (error == ERR_NONE) {
		error = GSM_SendFilePart(s, File, &Pos, &Handle);
	}
	if (error == ERR_EMPTY) {
		return ERR_NONE;
	}
	return error;
}

GSM_Error NoneReply(GSM_Protocol_Message *msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s,"None answer\n");
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
