
#include <string.h>
#include <ctype.h>

#include "../gsmstate.h"
#include "../service/gsmsms.h"
#include "../misc/coding.h"

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
	255	/*	TPMR		*/,	255 /*  TPPID??		*/};

GSM_Error PHONE_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	folders->Number=2;
	EncodeUnicode(folders->Folder[0].Name,GetMsg(s->msg,"Inbox"),strlen(GetMsg(s->msg,"Inbox")));
	EncodeUnicode(folders->Folder[1].Name,GetMsg(s->msg,"Outbox"),strlen(GetMsg(s->msg,"Outbox")));
	return GE_NONE;
}

void GSM_CreateFirmwareNumber(GSM_Phone_Data *Data)
{
	bool 		before=true;
	double		ala = 0, multiply = 1;
	unsigned int 	i;

	for (i=0;i<strlen(Data->Version);i++) {
		if (isdigit(Data->Version[i])) {
			if (before) {
				ala=ala*10+(Data->Version[i]-'0');
			} else {
				multiply=multiply*0.1;
				ala=ala+(Data->Version[i]-'0')*multiply;
			}
		}
		if (Data->Version[i]=='.') {
			before=false;
		}
	}
	*Data->VersionNum=ala;
	dprintf("Number version is \"%f\"\n",*Data->VersionNum);
}

GSM_Error PHONE_EncodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear)
{
	GSM_Error error;

	if (SMS->SMSC.Location!=0) {
		error = s->Phone.Functions->GetSMSC(s, &SMS->SMSC);
		if (error != GE_NONE) return error;
		SMS->SMSC.Location = 0;
	}
	if (SMS->PDU == SMS_Deliver) {
		if (SMS->SMSC.Number[0] == 0x00 && SMS->SMSC.Number[1] == 0x00)
		{
			return GE_EMPTYSMSC;
		}
	}
	return GSM_EncodeSMSFrame(SMS, buffer, Layout, length, clear);
}

GSM_Error PHONE_Terminate(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.EnableIncomingCB==true) {
		error=s->Phone.Functions->SetIncomingCB(s,false);
		if (error!=GE_NONE) return error;
	}
	if (s->Phone.Data.EnableIncomingSMS==true) {
		error=s->Phone.Functions->SetIncomingSMS(s,false);
		if (error!=GE_NONE) return error;
	}
	return GE_NONE;
}

GSM_Error PHONE_RTTLPlayOneNote(GSM_StateMachine *s, GSM_RingNote note, bool first)
{
	int 		duration, Hz;
	GSM_Error 	error;

	Hz=GSM_RingNoteGetFrequency(note);
  	
	error=s->Phone.Functions->PlayTone(s,Hz,5,first);
	if (error!=GE_NONE) return error;

	duration = GSM_RingNoteGetFullDuration(note);

	/* Is it correct ? Experimental values here */
	switch (note.Style) {
		case StaccatoStyle:
			mili_sleep (7500);
			error=s->Phone.Functions->PlayTone(s,0,0,false);	
			if (error != GE_NONE) return error;
			mili_sleep ((1500000/note.Tempo*duration)-(7500));
			break;
		case ContinuousStyle:
			mili_sleep  (1500000/note.Tempo*duration);
			break;
		case NaturalStyle:
			mili_sleep  (1500000/note.Tempo*duration-50);
			error=s->Phone.Functions->PlayTone(s,0,0,false);	
			if (error != GE_NONE) return error;
			mili_sleep (50);
			break;	
	}
	return GE_NONE;
}

GSM_Error PHONE_Beep(GSM_StateMachine *s)
{
	GSM_Error error;

	error=s->Phone.Functions->PlayTone(s, 4000, 5,true);
	if (error!=GE_NONE) return error;

	mili_sleep(500);

	return s->Phone.Functions->PlayTone(s,255*255,0,false);
}
