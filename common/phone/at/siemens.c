                                           
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

GSM_Error ATGEN_CMS35ReplySetFunction (GSM_Protocol_Message msg, GSM_StateMachine *s,char *function)
{
	if (s->Protocol.Data.AT.EditMode)
	{
	    s->Protocol.Data.AT.EditMode = false;
	    return GE_NONE;
	}
	dprintf ("Written %s",function);
  	if (s->Phone.Data.Priv.ATGEN.ReplyState == AT_Reply_OK){
  		dprintf (" - OK\n");
  		return GE_NONE;
	} else {
  		dprintf (" - error\n");
  		return GE_UNKNOWN;
	}
}

GSM_Error GetSiemensFrame(GSM_Protocol_Message msg, GSM_StateMachine *s, char *templ,
			    unsigned char *buffer, int *len)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			i=2, pos=0, length=0;
	unsigned char 		buf[512];

	if (strstr(GetLineString(msg.Buffer,Priv->Lines,2),"OK")) return GE_EMPTY;
        if (!strstr(GetLineString(msg.Buffer,Priv->Lines,2),templ)) return GE_UNKNOWN;

	while (1) {
		if (Priv->Lines.numbers[i*2+1]==0) break;
		if ((!strstr(GetLineString(msg.Buffer,Priv->Lines,i+1),templ)) && 
	            (strstr(GetLineString(msg.Buffer,Priv->Lines,i),templ))){
			length = strlen(GetLineString(msg.Buffer,Priv->Lines,i+1));
			DecodeHexBin(buf, GetLineString(msg.Buffer,Priv->Lines,i+1),length);
			length = length/2;
			memcpy (buffer+pos,buf,length);
			pos+=length;
		} 
		i++;
	}
	*len = pos;
       return GE_NONE;	
}

GSM_Error SetSiemensFrame (GSM_StateMachine *s, unsigned char *buff, char *templ,
			    int Location, GSM_Phone_RequestID RequestID, int len)
{
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	GSM_Error		error;
	unsigned char 		req[20],req1[512],hexreq[2096];
	int			MaxFrame,CurrentFrame,size,sz,pos=0;

	EncodeHexBin(hexreq,buff,len);
	size	 = len * 2;
	MaxFrame = size / 352;
	if (size % 352) MaxFrame++;

	for (CurrentFrame=0;CurrentFrame<MaxFrame;CurrentFrame++)
	{
		pos=CurrentFrame*352;
	 	if (pos+352 < size) sz = 352; else sz = size - pos;
		sprintf(req, "AT^SBNW=\"%s\",%i,%i,%i\r",templ,Location,CurrentFrame+1,MaxFrame);
		s->Protocol.Data.AT.EditMode = true;
		error = GSM_WaitFor (s, req, strlen(req), 0x00, 3, RequestID);
		s->Phone.Data.DispatchError=GE_TIMEOUT;
		s->Phone.Data.RequestID=RequestID;
	     	if (error!=GE_NONE) return error;
	 	memcpy (req1,hexreq+pos,sz);
	 	error = s->Protocol.Functions->WriteMessage(s, req1, sz, 0x00);
	 	if (error!=GE_NONE) return error;
		error = s->Protocol.Functions->WriteMessage(s,"\x1A", 1, 0x00);
	 	if (error!=GE_NONE) return error;
		error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
	 	if (error == GE_TIMEOUT) return error;
	 }
	 return Phone->DispatchError;
}

GSM_Error ATGEN_CMS35ReplyGetBitmap(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 		buffer[4096];
	int			length;
	GSM_Error		error;
	
	error = GetSiemensFrame(msg,s,"bmp",buffer,&length);
	if (error!=GE_NONE) return error;
	dprintf ("Operator logo received lenght=%i\n",length);
	error = BMP2Bitmap (buffer,NULL,s->Phone.Data.Bitmap);
	if (error==GE_NONE) return error;
	else return GE_UNKNOWN;
}

GSM_Error ATGEN_CMS35ReplySetBitmap(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    return ATGEN_CMS35ReplySetFunction (msg, s, "Operator Logo");	
}

GSM_Error ATGEN_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char req[32];

	if (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	if (Bitmap->Type!=GSM_OperatorLogo) return GE_NOTSUPPORTED;
	if (Bitmap->Location-1 < 0) Bitmap->Location++;
	s->Phone.Data.Bitmap=Bitmap;
	sprintf(req, "AT^SBNR=\"bmp\",%i\r", Bitmap->Location-1);
	smprintf(s, "Getting Bitmap\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetBitmap);
}

GSM_Error ATGEN_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char 	buffer[4096];
	int 		length;
	GSM_Error		error;
	
	if (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	if (Bitmap->Type!=GSM_OperatorLogo) return GE_NOTSUPPORTED;

	error = Bitmap2BMP (buffer,NULL,Bitmap);
	if (error!=GE_NONE) return error;
	length = 0x100 * buffer[3] + buffer[2];
	printf ("WALEK Buff=%i\n",length);
	buffer[58]=0xff; buffer[59]=0xff; buffer[60]=0xff;
	if (Bitmap->Location-1 < 0) Bitmap->Location++;
	s->Phone.Data.Bitmap=Bitmap;
	return SetSiemensFrame(s, buffer,"bmp",Bitmap->Location-1,
				ID_SetBitmap,length);	

}

GSM_Error ATGEN_CMS35ReplyGetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 		buffer[32];
	int			length;
	GSM_Error		error;

        error = GetSiemensFrame(msg,s,"mid",s->Phone.Data.Ringtone->NokiaBinary.Frame,&length);
	if (error!=GE_NONE) return error;
	dprintf ("Midi ringtone received\n");
	
	s->Phone.Data.Ringtone->Format			= RING_MIDI;
	s->Phone.Data.Ringtone->NokiaBinary.Length	= length;
	sprintf(buffer,"Individual");
	EncodeUnicode (s->Phone.Data.Ringtone->Name,buffer,strlen(buffer));
	return GE_NONE;
}

GSM_Error ATGEN_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	unsigned char req[32];

	if (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	
	s->Phone.Data.Ringtone=Ringtone;
	sprintf(req, "AT^SBNR=\"mid\",%i\r", Ringtone->Location-1);
	smprintf(s, "Getting RingTone\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetRingtone);
}

GSM_Error ATGEN_CMS35ReplySetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    return ATGEN_CMS35ReplySetFunction (msg, s, "Ringtone");
}
  
GSM_Error ATGEN_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Phone_Data *Phone = &s->Phone.Data;
	 
	if (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;

	if (Ringtone->Location==255) Ringtone->Location=1; 
	if (Ringtone->Location-1 > 1) return GE_INVALIDLOCATION;

	s->Phone.Data.Ringtone	= Ringtone;
 	Phone->Ringtone		= Ringtone;
	return SetSiemensFrame(s, Ringtone->NokiaBinary.Frame,"mid",Ringtone->Location-1,
				ID_SetRingtone,Ringtone->NokiaBinary.Length);
}

GSM_Error ATGEN_CMS35ReplyGetNextCal(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	GSM_Error		error;
	unsigned char 		buffer[354], tmp_buff[354];
	int			pos, len, no=0;

	if (Data->Priv.ATGEN.ReplyState != AT_Reply_OK) return GE_UNKNOWN;

	error = GetSiemensFrame(msg,s,"vcs",buffer,&len);
	if (error!=GE_NONE) return error;

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
		if (strstr(tmp_buff,"D1")) { 	//daily
		    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
		    Data->Cal->Entries[no].Number 	= 24;
		}
   		if (strstr(tmp_buff,"D7")) { 	//weekly
		    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
		    Data->Cal->Entries[no].Number 	= 7 * 24;
		}
    		if (strstr(tmp_buff,"MD1")) { 	//monthly 
		    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
		    Data->Cal->Entries[no].Number 	= 30 * 24; //fix-me
		}
    		if (strstr(tmp_buff,"YD1")) { 	//yearly
		    Data->Cal->Entries[no].EntryType 	= CAL_RECURRANCE;
		    Data->Cal->Entries[no].Number 	= 365 * 24; //fix-me
		}
		no++;			
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
	while (1){
	    Location++;	    	
	    sprintf(req, "AT^SBNR=\"vcs\",%i\r",Location);  
	    error = GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetCalendarNote);
	    if ((error!=GE_NONE) && (error!=GE_EMPTY)) return GE_INVALIDLOCATION;
	    Note->Location 		= Location;
	    Priv->FirstCalendarPos 	= Location;
	    if (Location > MAX_VCALENDAR_LOCATION) return GE_EMPTY;
	    if (error==GE_NONE) return error;
	}
	return error;
}

GSM_Error ATGEN_CMS35ReplySetCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    return ATGEN_CMS35ReplySetFunction (msg, s, "Calendar Note");
}

GSM_Error ATGEN_CMS35ReplyDeleteCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;
	
	if (Data->Cal->Location > MAX_VCALENDAR_LOCATION) return GE_UNKNOWN;
	
	if (Data->Priv.ATGEN.ReplyState== AT_Reply_OK) {
		smprintf(s, "Calendar note deleted\n");
		return GE_NONE;
	} else {
		smprintf(s, "Can't delete calendar note\n");
		return GE_UNKNOWN;
	}
}

GSM_Error ATGEN_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	unsigned char req[32];

	if (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	s->Phone.Data.Cal = Note;
	sprintf(req, "AT^SBNW=\"vcs\",%i,0\r",Note->Location);
	smprintf(s, "Deleting calendar note\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_DeleteCalendarNote);
}

GSM_Error ATGEN_AddCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool Past)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	unsigned char 		req[500], category[20], recurr[20], datetime[20],
				datealarm[20], description[20];
	int			size, day=0, no=0;
	bool			alarm = false;

	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	if (Note->Location==0x00) return GE_INVALIDLOCATION;	
	if (!Past && IsCalendarNoteFromThePast(Note)) return GE_NONE;

	error = ATGEN_DelCalendarNote(s, Note);
    	if (error!=GE_NONE) return error;
	
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
	
	return SetSiemensFrame (s,req,"vcs",Note->Location,ID_SetCalendarNote,size);
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
		Memory->PreferUnicode = false;
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
