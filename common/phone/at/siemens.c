                                           
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>
#include <ctype.h>

#include "../../gsmcomon.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"

extern GSM_Error ATGEN_HandleCMSError(GSM_StateMachine *s);

static int cut_str (const unsigned char *source, unsigned char *dest, const unsigned char *findstr)
{
	unsigned char 	*find;
	int 	  	pos;

	find = (unsigned char *)strstr(source,findstr); 
	if (find==NULL) return -1;
	pos = (int)(find - source);
	while ((*dest++=*find++)!='\n');
	*(--dest)='\0';
	return pos;
}

GSM_Error ATGEN_CMS35ReplyGetBitmap(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	dprintf ("CMS35ReplyGetBitmap \n");
	return GE_NONE;
}

GSM_Error ATGEN_CMS35ReplySetBitmap(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	dprintf ("CMS35ReplySetBitmap \n");
	return GE_NONE;
}

GSM_Error ATGEN_CMS35ReplyGetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		buffer[512];
	int			i=2, pos=0, length=0;

        if (!strstr(GetLineString(msg.Buffer,Priv->Lines,2),"mid")) return GE_INVALIDLOCATION;
	dprintf ("Midi ringtone received\n");
	while (1) {
		if (Priv->Lines.numbers[i*2+1]==0) break;
		if ((!strstr(GetLineString(msg.Buffer,Priv->Lines,i+1),"mid")) && 
	            (strstr(GetLineString(msg.Buffer,Priv->Lines,i),"mid")))
		{
			length = strlen(GetLineString(msg.Buffer,Priv->Lines,i+1));
			DecodeHexBin(buffer, GetLineString(msg.Buffer,Priv->Lines,i+1),length);
			length = length/2;
			memcpy (s->Phone.Data.Ringtone->NokiaBinary.Frame+pos,buffer,length);
			pos+=length;
		} 
		i++;
	}
	s->Phone.Data.Ringtone->Format			= RING_MIDI;
	s->Phone.Data.Ringtone->NokiaBinary.Length	= pos;
	sprintf(buffer,"Individual");
	EncodeUnicode (s->Phone.Data.Ringtone->Name,buffer,strlen(buffer));
	return GE_NONE;
}

GSM_Error ATGEN_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		req[32];

	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	
	s->Phone.Data.Ringtone=Ringtone;
	sprintf(req, "AT^SBNR=\"mid\",%i\r", Ringtone->Location-1);
	smprintf(s, "Getting RingTone\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetRingtone);
}

GSM_Error ATGEN_CMS35ReplySetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	dprintf ("Written Ringtone");
  	if (Priv->ReplyState == AT_Reply_OK)
  	{
  		dprintf (" - OK\n");
  		return GE_NONE;
	} else {
  		dprintf (" - error\n");
  		return GE_UNKNOWN;
	}
}
  
GSM_Error ATGEN_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Phone_ATGENData	*Priv  = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	GSM_Error		error;
	unsigned char 		req[20],req1[500],hexreq[2000];
	int			MaxFrame,CurrentFrame,size,sz,pos=0;
	 
	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	s->Phone.Data.Ringtone=Ringtone;
	 
	EncodeHexBin(hexreq,Ringtone->NokiaBinary.Frame,Ringtone->NokiaBinary.Length);
	size 	 = Ringtone->NokiaBinary.Length*2;
	MaxFrame = size / 352;
	if (size % 352) MaxFrame++;
	 
	for (CurrentFrame=0;CurrentFrame<MaxFrame;CurrentFrame++)
	{
		pos=CurrentFrame*352;
	 	if (pos+352 < size) sz = 352; else sz = size - pos;
	 
	 	Phone->DispatchError 	= GE_TIMEOUT;
	 	Phone->RequestID 	= ID_SetRingtone;
	 	Phone->Ringtone		= Ringtone;
		sprintf(req, "AT^SBNW=\"mid\",%i,%i,%i\r",
	 		Ringtone->Location-1, CurrentFrame+1,MaxFrame);
	 	error = s->Protocol.Functions->WriteMessage(s, req, strlen(req), 0x00);
	     	if (error!=GE_NONE) return error;
		my_sleep(200);
	 	    
	 	memcpy (req1,hexreq+pos,sz);
	 	    
	 	error = s->Protocol.Functions->WriteMessage(s, req1, sz, 0x00);
	 	if (error!=GE_NONE) return error;
	 
		error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
	 	if (error!=GE_NONE) return error;
	 	my_sleep (500);
	 	    
		error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
	 	if (error == GE_TIMEOUT) return error;
	 }
	 my_sleep (600);
	 return Phone->DispatchError;
}

GSM_Error ATGEN_CMS35ReplyGetNextCal(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	unsigned char 		buffer[354], tmp_buff[354];
	int			pos, len, no=0;

	if (Priv->ReplyState == AT_Reply_OK) {
		if (strstr(GetLineString(msg.Buffer,Priv->Lines,2),"OK")) return GE_EMPTY;
	 	DecodeHexBin(buffer, GetLineString(msg.Buffer,Priv->Lines,3), 
			     strlen(GetLineString(msg.Buffer,Priv->Lines,3)));
		if (strstr(GetLineString(msg.Buffer,Priv->Lines,4),"^SBNR"))
		{
		    DecodeHexBin(tmp_buff, GetLineString(msg.Buffer,Priv->Lines,5), strlen(GetLineString(msg.Buffer,Priv->Lines,5)));
		    pos = strlen(GetLineString(msg.Buffer,Priv->Lines,3)) / 2;
		    len = strlen(GetLineString(msg.Buffer,Priv->Lines,5)) / 2;
		    memcpy (buffer+pos,tmp_buff,len+1);
		}
 	 	pos = cut_str(buffer,tmp_buff,"CATEGORIES:");
	 	if (pos > -1) {
	    		if (strstr(tmp_buff,"MISCELLANEOUS"))   Data->Cal->Type=GCN_REMINDER;
	   		if (strstr(tmp_buff,"MEETING")) 	Data->Cal->Type=GCN_MEETING;
	    		if (strstr(tmp_buff,"PHONE CALL"))	Data->Cal->Type=GCN_CALL;
	    		if (strstr(tmp_buff,"ANNIVERSARY"))	Data->Cal->Type=GCN_BIRTHDAY;
	    	}
		pos = cut_str (buffer,tmp_buff,"DESCRIPTION:");
	 	if (pos > -1) {
	 		len=strlen(tmp_buff)-13;
	 		memcpy (tmp_buff,buffer+pos+12,len);
	 		tmp_buff[len]='\0';
	 		if (Data->Cal->Type==GCN_CALL) Data->Cal->Entries[no].EntryType=CAL_PHONE;
			else Data->Cal->Entries[no].EntryType=CAL_TEXT;
	 		EncodeUnicode (Data->Cal->Entries[no].Text,tmp_buff,strlen(tmp_buff));
			no++;
		}
		pos = cut_str (buffer,tmp_buff,"DTSTART:");
		if (pos > -1) {
			Data->Cal->Entries[no].EntryType = CAL_START_DATETIME;
	    		memcpy (tmp_buff,buffer+pos+8,4);
			tmp_buff[4]='\0';
	    		Data->Cal->Entries[no].Date.Year = atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+12,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Month = atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+14,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Day = atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+17,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Hour = atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+19,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Minute = atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+21,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Second = atoi(tmp_buff);
			no++;
	 	}
		pos = cut_str (buffer,tmp_buff,"DALARM:");
		if (pos > -1) {
			Data->Cal->Entries[no].EntryType = CAL_ALARM_DATETIME;
	    		memcpy (tmp_buff,buffer+pos+7,4);
			tmp_buff[4]='\0';
	    		Data->Cal->Entries[no].Date.Year= atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+11,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Month= atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+13,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Day= atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+16,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Hour= atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+18,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Minute= atoi(tmp_buff);
	    		memcpy (tmp_buff,buffer+pos+20,2);
			tmp_buff[2]='\0';
	    		Data->Cal->Entries[no].Date.Second= atoi(tmp_buff);
			no++;			
	 	}
		pos = cut_str (buffer,tmp_buff,"RRULE:");
		if (pos > -1) {
			if (strstr(tmp_buff,"D1")) 	//daily
			{
			    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
			    Data->Cal->Entries[no].Number 	= 24;
			}
	   		if (strstr(tmp_buff,"D7"))  	//weekly
			{
			    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
			    Data->Cal->Entries[no].Number 	= 7 * 24;
			}
	    		if (strstr(tmp_buff,"MD1"))  	//monthly 
			{
			    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
			    Data->Cal->Entries[no].Number 	= 30 * 24; //fix-me
			}
	    		if (strstr(tmp_buff,"YD1"))  	//yearly
			{
			    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
			    Data->Cal->Entries[no].Number 	= 365 * 24; //fix-me
			}
			no++;			
	    	}
       	}
	Data->Cal->EntriesNum = no;
	return GE_NONE;
}

GSM_Error ATGEN_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	unsigned char 		req[32];
	int			Location;

	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;

	if (start) Note->Location=Priv->FirstCalendarPos;
	s->Phone.Data.Cal 	= Note;
	Note->EntriesNum 	= 0;
	smprintf(s, "Getting VCALENDAR\n");
	Location = Note->Location;
	while (1)
	{
		sprintf(req, "AT^SBNR=\"vcs\",%i\r",Location);  
		error = GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetCalendarNote);
		Location++;
		Priv->FirstCalendarPos 	= Location;
		Note->Location 		= Location;
	  	if (Location > 30) return GE_EMPTY;
	  	if (error==GE_NONE) return error;
	}
	return GE_NONE;
}

GSM_Error ATGEN_CMS35ReplySetCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	dprintf ("Written Calendar Note");
	if (Priv->ReplyState == AT_Reply_OK)
	{
		dprintf (" - OK\n");
		return GE_NONE;
	} else {
		dprintf (" - error\n");
		return GE_UNKNOWN;
	}
}

GSM_Error ATGEN_CMS35ReplyDeleteCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	if (Data->Cal->Location > 30) return GE_UNKNOWN;
	if (Priv->ReplyState== AT_Reply_OK) {
		smprintf(s, "Calendar note deleted\n");
		return GE_NONE;
	} else {
		smprintf(s, "Can't delete calendar note\n");
		return GE_UNKNOWN;
	}
}

GSM_Error ATGEN_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		req[32];

	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	s->Phone.Data.Cal = Note;
	sprintf(req, "AT^SBNW=\"vcs\",%i,0\r",Note->Location);
	smprintf(s, "Deleting calendar note\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_DeleteCalendarNote);
}

GSM_Error ATGEN_AddCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool Past)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	GSM_Error		error;
	unsigned char 		req[500], hexreq[500], category[20], recurr [20],
				datetime[20], datealarm[20], description[20];
	int			MaxFrame,CurrentFrame,size,sz,day=0,no=0,pos=0;
	bool			alarm = false;

	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	if (Note->Location==0x00) return GE_INVALIDLOCATION;	
	if (!Past && IsNoteFromThePast(*Note)) return GE_NONE;

	error = ATGEN_DelCalendarNote(s, Note);
    	if (error!=GE_NONE) return error;
	my_sleep (500);
	
	s->Phone.Data.Cal = Note;
	
	switch(Note->Type) {
		case GCN_REMINDER : sprintf(category,"MISCELLANEOUS"); 	break;
		case GCN_CALL     : sprintf(category,"PHONE CALL"); 	break;
    		case GCN_MEETING  : sprintf(category,"MEETING");	break;
		case GCN_BIRTHDAY : sprintf(category,"ANNIVERSARY");	break;
		default		  : sprintf(category,"MISCELLANEOUS");  break;
	}
	recurr[0] = '\0';
	
	while (no < Note->EntriesNum) {
	switch (Note->Entries[no].EntryType) {
	    case CAL_ALARM_DATETIME:
	        sprintf(datealarm,"%04d%02d%02dT%02d%02d%02d", Note->Entries[no].Date.Year,
							Note->Entries[no].Date.Month,
							Note->Entries[no].Date.Day,
							Note->Entries[no].Date.Hour,
							Note->Entries[no].Date.Minute,
							Note->Entries[no].Date.Second);
		alarm = true;
		break;
	    case CAL_START_DATETIME:
	        sprintf(datetime,"%04d%02d%02dT%02d%02d%02d", Note->Entries[no].Date.Year,
							Note->Entries[no].Date.Month,
							Note->Entries[no].Date.Day,
							Note->Entries[no].Date.Hour,
							Note->Entries[no].Date.Minute,
							Note->Entries[no].Date.Second);
		day = Note->Entries[no].Date.Day;
		break;
	    case CAL_TEXT:
	        sprintf(description,DecodeUnicodeString(Note->Entries[no].Text));
	        break;
	    case CAL_RECURRANCE:
		switch (Note->Entries[no].Number){
		    case 24: 		sprintf (recurr,"RRULE:D1\r\n"); break;
		    case (7*24): 	sprintf (recurr,"RRULE:D7\r\n"); break;
		    case (30*24):	sprintf (recurr,"RRULE:MD1 %02d\r\n",day); break;
		    case (365*24): 	sprintf (recurr,"RRULE:YD1\r\n"); break;
		    default: break;
		}
		break;
	    default: break;
	    }
	    no++;
	}
	
	
	if (alarm)
	 sprintf(req,"BEGIN:VCALENDAR\r\nVERSION:1.0\r\nBEGIN:VEVENT\r\nCATEGORIES:%s\r\nDALARM:%s\r\nDTSTART:%s\r\n%sDESCRIPTION:%s\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n",
		 category,datealarm,datetime,recurr,description); 
	else 
	 sprintf(req,"BEGIN:VCALENDAR\r\nVERSION:1.0\r\nBEGIN:VEVENT\r\nCATEGORIES:%s\r\nDTSTART:%s\r\n%sDESCRIPTION:%s\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n",
		 category,datetime,recurr,description);
	size = strlen(req);
	EncodeHexBin(hexreq,req,size);
	size = size * 2;

	MaxFrame = size / 352;
	if (size % 352) MaxFrame++;
	 
	for (CurrentFrame=0;CurrentFrame<MaxFrame;CurrentFrame++)
	{
		pos=CurrentFrame*352;
	 	if (pos+352 < size) sz = 352; else sz = size - pos;
	 
	 	sprintf(req,"AT^SBNW=\"vcs\",%i,%i,%i\r",
			Note->Location,CurrentFrame+1,MaxFrame);

		Phone->DispatchError 	= GE_TIMEOUT;
    		Phone->RequestID 	= ID_SetCalendarNote;
		Phone->Cal = Note;
		error = s->Protocol.Functions->WriteMessage(s, req, strlen(req), 0x00);
    		if (error!=GE_NONE) return error;
		my_sleep (200);
    		error = s->Protocol.Functions->WriteMessage(s, hexreq+pos, sz, 0x00);
		if (error!=GE_NONE) return error;
		error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
		if (error!=GE_NONE) return error;
	 	my_sleep (600);
		error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
	 	if (error == GE_TIMEOUT) return error;
	 }
	 return Phone->DispatchError;
}

GSM_Error ATGEN_SL45ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
 	GSM_PhonebookEntry	*Memory = s->Phone.Data.Memory;
	unsigned char		buffer[500],buffer2[500];

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "Phonebook entry received\n");
		CopyLineString(buffer, msg.Buffer, Priv->Lines, 3);
		DecodeHexBin(buffer2,buffer,strlen(buffer));
 		Memory->EntriesNum = 0;
                DecodeVCARD21Text(buffer2, Memory);
		return GE_NONE;
	case AT_Reply_Error:
                smprintf(s, "Error - too high location ?\n");
                return GE_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
