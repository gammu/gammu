
#include <string.h>
#include <ctype.h>

#include "../../cfg/config.h"
#include "../phone/nokia/nfunc.h"
#include "../misc/coding.h"
#include "../phone/nokia/dct3/n7110.h"
#include "gsmback.h"
#include "gsmlogo.h"

#ifdef GSM_ENABLE_BACKUP

/* -------- Some general functions ---------------------------------------- */

static void SaveLinkedBackupText(FILE *file, char *myname, char *myvalue)
{
	int 		w,current;
	unsigned char 	buffer2[300];

	current = strlen(myvalue); w = 0;
	while (true) {
		if (current > 200) {
			memcpy(buffer2,myvalue+(strlen(myvalue)-current),200);
			buffer2[200] = 0;
			current = current - 200;
		} else {
			memcpy(buffer2,myvalue+(strlen(myvalue)-current),current);
			buffer2[current] = 0;
			current = 0;
		}
		fprintf(file,"%s%02i = %s\n",myname,w,buffer2);
		if (current == 0) break;
		w++;
	}		
}

static void ReadLinkedBackupText(CFG_Header *file_info, char *section, char *myname, char *myvalue)
{
	unsigned char		buffer2[50];
	char			*readvalue;
	int			i;

	i=0;
	myvalue[0] = 0;
	while (true) {
		sprintf(buffer2,"%s%02i",myname,i);
		readvalue = CFG_Get(file_info, section, buffer2, false);
		if (readvalue!=NULL) {
			myvalue[strlen(myvalue)+strlen(readvalue)]=0;
			memcpy(myvalue+strlen(myvalue),readvalue,strlen(readvalue));
		} else break;
		i++;
	}
}

static void SaveBackupText(FILE *file, char *myname, char *myvalue)
{
	unsigned char buffer[10000];

	fprintf(file,"%s = \"%s\"\n",myname,DecodeUnicodeString(myvalue));
	EncodeHexBin(buffer,myvalue,strlen(DecodeUnicodeString(myvalue))*2);
	fprintf(file,"%sUnicode = %s\n",myname,buffer);
}

static void ReadBackupText(CFG_Header *file_info, char *section, char *myname, char *myvalue)
{
	unsigned char 	paramname[10000];
	char		*readvalue;

	strcpy(paramname,myname);
	strcat(paramname,"Unicode");
	readvalue = CFG_Get(file_info, section, paramname, false);
	if (readvalue!=NULL) {
		dprintf("%s %i\n",readvalue,strlen(readvalue));
		DecodeHexBin (myvalue, readvalue, strlen(readvalue));
		myvalue[strlen(readvalue)/2]=0;
		myvalue[strlen(readvalue)/2+1]=0;
	} else {
		strcpy(paramname,myname);
		readvalue = CFG_Get(file_info, section, paramname, false);
		if (readvalue!=NULL) {
			EncodeUnicode(myvalue,readvalue+1,strlen(readvalue)-2);
		} else {
			myvalue[0]=0;
			myvalue[1]=0;
		}
	}
}

static void ReadvCalTime(GSM_DateTime *dt, char *time)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	dt->Year=dt->Month=dt->Day=dt->Hour=dt->Minute=dt->Second=dt->Timezone=0;

	strncpy(year, 	time, 		4);
	strncpy(month, 	time+4, 	2);
	strncpy(day, 	time+6, 	2);
	strncpy(hour, 	time+9,		2);
	strncpy(minute, time+11, 	2);
	strncpy(second, time+13, 	2);

	/* FIXME: Should check ranges... */
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);
	dt->Hour	= atoi(hour);
	dt->Minute	= atoi(minute);
	dt->Second	= atoi(second);
	/* FIXME */
	dt->Timezone	= 0;
}

static void SaveVCalTime(FILE *file, GSM_DateTime *dt)
{
	fprintf(file, " = %04d%02d%02dT%02d%02d%02d\n",
		dt->Year, dt->Month, dt->Day,
		dt->Hour, dt->Minute, dt->Second);
}

/* ------------------- Backup text files ----------------------------------- */

static void SavePbkEntry(FILE *file, GSM_PhonebookEntry *Pbk)
{
	bool	text;
	char	buffer[50];
	int	j;

	fprintf(file,"Location = %03i\n",Pbk->Location);
	for (j=0;j<Pbk->EntriesNum;j++) {
		text = true;
		switch (Pbk->Entries[j].EntryType) {
			case PBK_Number_General:
				fprintf(file,"Entry%02iType = NumberGeneral\n",j);
				break;
			case PBK_Number_Mobile:
				fprintf(file,"Entry%02iType = NumberMobile\n",j);
				break;
			case PBK_Number_Work:
				fprintf(file,"Entry%02iType = NumberWork\n",j);
				break;
			case PBK_Number_Fax:
				fprintf(file,"Entry%02iType = NumberFax\n",j);
				break;
			case PBK_Number_Home:
				fprintf(file,"Entry%02iType = NumberHome\n",j);
				break;
			case PBK_Text_Note:
				fprintf(file,"Entry%02iType = Note\n",j);
				break;
			case PBK_Text_Postal:
				fprintf(file,"Entry%02iType = Postal\n",j);
				break;
			case PBK_Text_Email:
				fprintf(file,"Entry%02iType = Email\n",j);
				break;
			case PBK_Text_URL:
				fprintf(file,"Entry%02iType = URL\n",j);
				break;
			case PBK_Name:
				fprintf(file,"Entry%02iType = Name\n",j);
				break;
			case PBK_Caller_Group:
				fprintf(file,"Entry%02iType = CallerGroup\n",j);
				fprintf(file,"Entry%02iNumber = %i\n",j,Pbk->Entries[j].Number);
				text = false;
				break;
			case PBK_Date:
				break;
		}
		if (text) {
			sprintf(buffer,"Entry%02iText",j);
			SaveBackupText(file,buffer,Pbk->Entries[j].Text);
		}
		switch (Pbk->Entries[j].EntryType) {
			case PBK_Number_General:
			case PBK_Number_Mobile:
			case PBK_Number_Work:
			case PBK_Number_Fax:
			case PBK_Number_Home:
				if (Pbk->Entries[j].VoiceTag!=0)
					fprintf(file,"Entry%02iVoiceTag = %i\n",j,Pbk->Entries[j].VoiceTag);
				break;
			default:
				break;
		}
	}
	fprintf(file,"\n");
}

static void SaveCalendarEntry(FILE *file, GSM_CalendarNote *Note)
{
	fprintf(file,"Type = ");
	switch (Note->Type) {
		case GCN_REMINDER : fprintf(file,"Reminder\n");			break;
		case GCN_CALL     : fprintf(file,"Call\n");			break;
		case GCN_MEETING  : fprintf(file,"Meeting\n");			break;
		case GCN_BIRTHDAY : fprintf(file,"Birthday\n");			break;
		case GCN_T_ATHL   : fprintf(file,"Training/Athletism\n"); 	break;
       		case GCN_T_BALL   : fprintf(file,"Training/BallGames\n"); 	break;
                case GCN_T_CYCL   : fprintf(file,"Training/Cycling\n"); 	break;
                case GCN_T_BUDO   : fprintf(file,"Training/Budo\n"); 		break;
                case GCN_T_DANC   : fprintf(file,"Training/Dance\n"); 		break;
                case GCN_T_EXTR   : fprintf(file,"Training/ExtremeSports\n");	break;
                case GCN_T_FOOT   : fprintf(file,"Training/Football\n"); 	break;
                case GCN_T_GOLF   : fprintf(file,"Training/Golf\n"); 		break;
                case GCN_T_GYM    : fprintf(file,"Training/Gym\n"); 		break;
                case GCN_T_HORS   : fprintf(file,"Training/HorseRaces\n"); 	break;
                case GCN_T_HOCK   : fprintf(file,"Training/Hockey\n"); 		break;
                case GCN_T_RACE   : fprintf(file,"Training/Races\n"); 		break;
                case GCN_T_RUGB   : fprintf(file,"Training/Rugby\n"); 		break;
                case GCN_T_SAIL   : fprintf(file,"Training/Sailing\n"); 	break;
                case GCN_T_STRE   : fprintf(file,"Training/StreetGames\n"); 	break;
                case GCN_T_SWIM   : fprintf(file,"Training/Swimming\n"); 	break;
                case GCN_T_TENN   : fprintf(file,"Training/Tennis\n"); 		break;
                case GCN_T_TRAV   : fprintf(file,"Training/Travels\n"); 	break;
                case GCN_T_WINT   : fprintf(file,"Training/WinterGames\n"); 	break;
	}
	fprintf(file, "Time");
	SaveVCalTime(file, &Note->Time);
	if (Note->Alarm.Year != 0) {
		fprintf(file, "Alarm");
		SaveVCalTime(file, &Note->Alarm);
		if (Note->SilentAlarm) {
			fprintf(file, "AlarmType = Silent\n");
		} else {
			fprintf(file, "AlarmType = Tone\n");
		}
	}
	if (Note->Recurrance != 0) {
		fprintf(file, "Recurrance = %d\n",Note->Recurrance/24);
	}
	SaveBackupText(file, "Text", Note->Text);
	if (Note->Type == GCN_CALL) {
		SaveBackupText(file, "Phone", Note->Phone);
	}
	fprintf(file,"\n");
}

static void SaveWAPSettingsEntry(FILE *file, GSM_MultiWAPSettings *settings)
{
	int 	i;
	char 	buffer[50];

	for (i=0;i<settings->Number;i++) {
		sprintf(buffer,"Title%02i",i);
		SaveBackupText(file, buffer, settings->Settings[i].Title);
		sprintf(buffer,"HomePage%02i",i);
		SaveBackupText(file, buffer, settings->Settings[i].HomePage);
		if (settings->Settings[i].IsContinuous) {
			fprintf(file,"Type%02i = Continuous\n",i);
		} else {
			fprintf(file,"Type%02i = Temporary\n",i);
		}
		if (settings->Settings[i].IsSecurity) {
			fprintf(file,"Security%02i = On\n",i);
		} else {
			fprintf(file,"Security%02i = Off\n",i);
		}
		switch (settings->Settings[i].Bearer) {
			case WAPSETTINGS_BEARER_SMS:
				fprintf(file,"Bearer%02i = SMS\n",i);
				sprintf(buffer,"Server%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Server);
				sprintf(buffer,"Service%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Service);
				break;
			case WAPSETTINGS_BEARER_GPRS:
				fprintf(file,"Bearer%02i = GPRS\n",i);
			case WAPSETTINGS_BEARER_DATA:
				if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_DATA) {
					fprintf(file,"Bearer%02i = Data\n",i);
					if (settings->Settings[i].IsISDNCall) {
						fprintf(file,"CallType%02i = ISDN\n",i);
					} else {
						fprintf(file,"CallType%02i = Analogue\n",i);
					}
				}
				sprintf(buffer,"Number%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].DialUp);
				sprintf(buffer,"IP%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].IPAddress);
				if (settings->Settings[i].ManualLogin) {
					fprintf(file,"Login%02i = Manual\n",i);
				} else {
					fprintf(file,"Login%02i = Automatic\n",i);
				}
				if (settings->Settings[i].IsNormalAuthentication) {
					fprintf(file,"Authentication%02i = Normal\n",i);
				} else {
					fprintf(file,"Authentication%02i = Secure\n",i);
				}
				switch (settings->Settings[i].Speed) {
					case WAPSETTINGS_SPEED_9600 : fprintf(file,"CallSpeed%02i = 9600\n",i);  break;
					case WAPSETTINGS_SPEED_14400: fprintf(file,"CallSpeed%02i = 14400\n",i); break;
					case WAPSETTINGS_SPEED_AUTO : fprintf(file,"CallSpeed%02i = auto\n",i);  break;
				}
				sprintf(buffer,"User%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].User);
				sprintf(buffer,"Password%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Password);
				break;
			case WAPSETTINGS_BEARER_USSD:
				fprintf(file,"Bearer%02i = USSD\n",i);
				sprintf(buffer,"ServiceCode%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Code);
				if (settings->Settings[i].IsIP) {
					sprintf(buffer,"IP%02i",i);
					SaveBackupText(file, buffer, settings->Settings[i].Service);
				} else {
					sprintf(buffer,"Number%02i",i);
					SaveBackupText(file, buffer, settings->Settings[i].Service);
				}
		}
		fprintf(file,"\n");
	}
}

static void SaveBitmapEntry(FILE *file, GSM_Bitmap *bitmap)
{
	unsigned char 	buffer[10000];
	int		x,y;

	fprintf(file,"Width = %i\n",bitmap->Width);
	fprintf(file,"Height = %i\n",bitmap->Height);
	for (y=0;y<bitmap->Height;y++) {
		for (x=0;x<bitmap->Width;x++) {
			buffer[x] = ' ';
			if (GSM_IsPointBitmap(bitmap,x,y)) buffer[x]='#';
		}
		buffer[bitmap->Width] = 0;
		fprintf(file,"Bitmap%02i = \"%s\"\n",y,buffer);
	}
}

static void SaveCallerEntry(FILE *file, GSM_Bitmap *bitmap)
{
	fprintf(file,"Location = %03i\n",bitmap->Location);
	if (!bitmap->DefaultName) SaveBackupText(file, "Name", bitmap->Text);
	fprintf(file,"Ringtone = %02x\n",bitmap->Ringtone);
	if (bitmap->Enabled) {
		fprintf(file,"Enabled = True\n");
	} else {
		fprintf(file,"Enabled = False\n");
	}
	SaveBitmapEntry(file, bitmap);
	fprintf(file,"\n");
}

static void SaveWAPBookmarkEntry(FILE *file, GSM_WAPBookmark *bookmark)
{
	SaveBackupText(file, "URL", bookmark->Address);
	SaveBackupText(file, "Title", bookmark->Title);
	fprintf(file,"\n");
}

static void SaveStartupEntry(FILE *file, GSM_Bitmap *bitmap)
{
	fprintf(file,"[Startup]\n");	
	if (bitmap->Type == GSM_WelcomeNoteText) {
		SaveBackupText(file, "Text", bitmap->Text);
	}
	if (bitmap->Type == GSM_StartupLogo) {
		SaveBitmapEntry(file, bitmap);
	}
	fprintf(file,"\n");
}

static void SaveSMSCEntry(FILE *file, GSM_SMSC *SMSC)
{
	fprintf(file,"Location = %03i\n",SMSC->Location);
	SaveBackupText(file, "Name", SMSC->Name);
	SaveBackupText(file, "Number", SMSC->Number);
	SaveBackupText(file, "DefaultNumber", SMSC->DefaultNumber);
	fprintf(file,"Format = ");
	switch (SMSC->Format) {
		case GSMF_Text	: fprintf(file,"Text");		break;
		case GSMF_Fax	: fprintf(file,"Fax");		break;
		case GSMF_Email	: fprintf(file,"Email");	break;
		case GSMF_Pager	: fprintf(file,"Pager");	break;
	}
	fprintf(file,"\nValidity = ");
	switch (SMSC->Validity.Relative) {
		case GSMV_1_Hour	: fprintf(file,"1hour");	break;
		case GSMV_6_Hours 	: fprintf(file,"6hours");	break;
		case GSMV_24_Hours	: fprintf(file,"24hours");	break;
		case GSMV_72_Hours	: fprintf(file,"72hours");	break;
		case GSMV_1_Week  	: fprintf(file,"1week");	break;
		case GSMV_Max_Time	: fprintf(file,"MaximumTime");	break;
		default			: fprintf(file,"MaximumTime");	break;
	}
	fprintf(file,"\n\n");
}

static void SaveRingtoneEntry(FILE *file, GSM_Ringtone *ringtone)
{
	unsigned char 	buffer[10000];
	int		i,j;

	fprintf(file,"Location = %i\n",ringtone->Location);
	SaveBackupText(file, "Name", ringtone->Name);
	switch (ringtone->Format) {
	case RING_NOKIABINARY:
		j = 0; i = 0;
		EncodeHexBin(buffer,ringtone->NokiaBinary.Frame,ringtone->NokiaBinary.Length);
		SaveLinkedBackupText(file, "NokiaBinary", buffer);
		break;
	case RING_NOTETONE:
		break;
	}
	fprintf(file,"\n");
}

static void SaveOperatorEntry(FILE *file, GSM_Bitmap *bitmap)
{
	fprintf(file,"[Operator]\n");
	fprintf(file,"Network = \"%s\"\n", bitmap->NetworkCode);
	SaveBitmapEntry(file, bitmap);
	fprintf(file,"\n");
}

static void SaveToDoEntry(FILE *file, GSM_TODO *ToDo)
{
	fprintf(file,"Location = %i\n",ToDo->Location);
	SaveBackupText(file, "Text", ToDo->Text);
	fprintf(file,"Priority = %02x\n\n",ToDo->Priority);
}

static void SaveProfileEntry(FILE *file, GSM_Profile *Profile)
{
	int	j,k;
	bool	special;
		
	fprintf(file,"Location = %i\n",Profile->Location);
	SaveBackupText(file, "Name",Profile->Name);

	if (Profile->DefaultName) fprintf(file,"DefaultName = true\n");
	if (Profile->HeadSetProfile) fprintf(file,"HeadSetProfile = true\n");
	if (Profile->CarKitProfile) fprintf(file,"CarKitProfile = true\n");

	for (j=0;j<Profile->FeaturesNumber;j++) {
		fprintf(file,"Feature%02i = ",j);
		special = false;
		switch (Profile->FeatureID[j]) {
		case Profile_MessageToneID:
		case Profile_RingtoneID:
			special = true;
			if (Profile->FeatureID[j] == Profile_RingtoneID) {
				fprintf(file,"RingtoneID\n");
			} else {
				fprintf(file,"MessageToneID\n");
			}
			fprintf(file,"Value%02i = %i\n",j,Profile->FeatureValue[j]);
			break;	
		case Profile_CallerGroups:
			special = true;
			fprintf(file,"CallerGroups\n");
			fprintf(file,"Value%02i = ",j);
			for (k=0;k<5;k++) {
				if (Profile->CallerGroups[k]) {
					fprintf(file,"%i",k);
				}
			}
			fprintf(file,"\n");
			break;
		case Profile_ScreenSaverNumber:
			special = true;
			fprintf(file,"ScreenSaverNumber\n");
			fprintf(file,"Value%02i = %i\n",j,Profile->FeatureValue[j]);
			break;
		case Profile_CallAlert  	: fprintf(file,"IncomingCallAlert\n"); 	break;
		case Profile_RingtoneVolume 	: fprintf(file,"RingtoneVolume\n"); 	break;
		case Profile_Vibration		: fprintf(file,"Vibrating\n"); 		break;
		case Profile_MessageTone	: fprintf(file,"MessageTone\n"); 	break;
		case Profile_KeypadTone		: fprintf(file,"KeypadTones\n"); 	break;
		case Profile_WarningTone	: fprintf(file,"WarningTones\n"); 	break;
		case Profile_ScreenSaver	: fprintf(file,"ScreenSaver\n"); 	break;
		case Profile_ScreenSaverTime	: fprintf(file,"ScreenSaverTimeout\n"); break;
		case Profile_AutoAnswer		: fprintf(file,"AutomaticAnswer\n"); 	break;
		case Profile_Lights		: fprintf(file,"Lights\n"); 		break;
		default				: special = true;
		}
		if (!special) {
			fprintf(file,"Value%02i = ",j);
			switch (Profile->FeatureValue[j]) {
			case PROFILE_VOLUME_LEVEL1 		:
			case PROFILE_KEYPAD_LEVEL1 		: fprintf(file,"Level1\n"); 		break;
			case PROFILE_VOLUME_LEVEL2 		:
			case PROFILE_KEYPAD_LEVEL2 		: fprintf(file,"Level2\n");		break;
			case PROFILE_VOLUME_LEVEL3 		:
			case PROFILE_KEYPAD_LEVEL3 		: fprintf(file,"Level3\n"); 		break;
			case PROFILE_VOLUME_LEVEL4 		: fprintf(file,"Level4\n"); 		break;
			case PROFILE_VOLUME_LEVEL5 		: fprintf(file,"Level5\n"); 		break;
			case PROFILE_MESSAGE_NOTONE 		:
			case PROFILE_AUTOANSWER_OFF		:
			case PROFILE_LIGHTS_OFF  		:
			case PROFILE_SAVER_OFF			:
			case PROFILE_WARNING_OFF		:
			case PROFILE_CALLALERT_OFF	 	:
			case PROFILE_VIBRATION_OFF 		:
			case PROFILE_KEYPAD_OFF	   		: fprintf(file,"Off\n");	  	break;
			case PROFILE_CALLALERT_RINGING   	: fprintf(file,"Ringing\n");		break;
			case PROFILE_CALLALERT_RINGONCE  	: fprintf(file,"RingOnce\n");		break;
			case PROFILE_CALLALERT_ASCENDING 	: fprintf(file,"Ascending\n");        	break;
			case PROFILE_CALLALERT_CALLERGROUPS	: fprintf(file,"CallerGroups\n");	break;
			case PROFILE_MESSAGE_STANDARD 		: fprintf(file,"Standard\n");  		break;
			case PROFILE_MESSAGE_SPECIAL 		: fprintf(file,"Special\n");	 	break;
			case PROFILE_MESSAGE_BEEPONCE		:
			case PROFILE_CALLALERT_BEEPONCE  	: fprintf(file,"BeepOnce\n");		break;
			case PROFILE_MESSAGE_ASCENDING		: fprintf(file,"Ascending\n"); 		break;
			case PROFILE_MESSAGE_PERSONAL		: fprintf(file,"Personal\n");		break;
			case PROFILE_AUTOANSWER_ON		:
			case PROFILE_WARNING_ON			:
			case PROFILE_SAVER_ON			:
			case PROFILE_VIBRATION_ON 		: fprintf(file,"On\n");  		break;
			case PROFILE_VIBRATION_FIRST 		: fprintf(file,"VibrateFirst\n");	break;
			case PROFILE_LIGHTS_AUTO 		: fprintf(file,"Auto\n"); 		break;
			case PROFILE_SAVER_TIMEOUT_5SEC	 	: fprintf(file,"5Seconds\n"); 		break;
			case PROFILE_SAVER_TIMEOUT_20SEC 	: fprintf(file,"20Seconds\n"); 		break;
			case PROFILE_SAVER_TIMEOUT_1MIN	 	: fprintf(file,"1Minute\n");		break;
			case PROFILE_SAVER_TIMEOUT_2MIN	 	: fprintf(file,"2Minutes\n");		break;
			case PROFILE_SAVER_TIMEOUT_5MIN	 	: fprintf(file,"5Minutes\n");		break;
			case PROFILE_SAVER_TIMEOUT_10MIN 	: fprintf(file,"10Minutes\n");		break;
			default					: fprintf(file,"UNKNOWN\n");
			}	
		}
	}
    	fprintf(file,"\n");
}

static GSM_Error SaveBackup(FILE *file, GSM_Backup *backup)
{
	int i;

	fprintf(file,"[Backup]\n");
	fprintf(file,"IMEI = \"%s\"\n",backup->IMEI);
	fprintf(file,"Phone = \"%s\"\n",backup->Model);
	fprintf(file,"Time = \"%s\"\n",backup->DateTime);
	fprintf(file,"Format = 1.01\n");
	fprintf(file,"\n");

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
		fprintf(file,"[PhonePBK%03i]\n",i);
		SavePbkEntry(file, backup->PhonePhonebook[i]);
		i++;
	}
	i=0;
	while (backup->SIMPhonebook[i]!=NULL) {
		fprintf(file,"[SIMPBK%03i]\n",i);
		SavePbkEntry(file, backup->SIMPhonebook[i]);
		i++;
	}
	i=0;
	while (backup->Calendar[i]!=NULL) {
		fprintf(file,"[Calendar%03i]\n",i);
		SaveCalendarEntry(file, backup->Calendar[i]);
		i++;
	}
	i=0;
	while (backup->CallerLogos[i]!=NULL) {
		fprintf(file,"[Caller%03i]\n",i);
		SaveCallerEntry(file, backup->CallerLogos[i]);
		i++;
	}
	i=0;
	while (backup->SMSC[i]!=NULL) {
		fprintf(file,"[SMSC%03i]\n",i);
		SaveSMSCEntry(file, backup->SMSC[i]);
		i++;
	}
	i=0;
	while (backup->WAPBookmark[i]!=NULL) {
		fprintf(file,"[Bookmark%03i]\n",i);
		SaveWAPBookmarkEntry(file, backup->WAPBookmark[i]);
		i++;
	}
	i=0;
	while (backup->WAPSettings[i]!=NULL) {
		fprintf(file,"[Settings%03i]\n",i);
		SaveWAPSettingsEntry(file, backup->WAPSettings[i]);
		i++;
	}
	i=0;
	while (backup->Ringtone[i]!=NULL) {
		fprintf(file,"[Ringtone%03i]\n",i);
		SaveRingtoneEntry(file, backup->Ringtone[i]);
		i++;
	}
	i=0;
	while (backup->ToDo[i]!=NULL) {
		fprintf(file,"[TODO%03i]\n",i);
		SaveToDoEntry(file, backup->ToDo[i]);
		i++;
	}
	i=0;
	while (backup->Profiles[i]!=NULL) {
		fprintf(file,"[Profile%03i]\n",i);
		SaveProfileEntry(file, backup->Profiles[i]);
		i++;
	}

	if (backup->StartupLogo!=NULL) {
		SaveStartupEntry(file, backup->StartupLogo);
	}
	if (backup->OperatorLogo!=NULL) {
		SaveOperatorEntry(file, backup->OperatorLogo);
	}

	return GE_NONE;
}

static void SaveLMBStartupEntry(FILE *file, GSM_Bitmap bitmap)
{  
	int 			count=13;
	GSM_Phone_Bitmap_Types 	Type;
	/* Welcome note and logo header block */
	char req[1000] = {
		'W','E','L',' ',    /*block identifier*/
		00,00,              /*block data size*/
		0x02,00,00,00,00,00,
		0x00};              /*number of blocks (like in 6110 frame)*/

	if (bitmap.Type == GSM_StartupLogo) {
		req[count++] = 0x01;
		req[count++] = bitmap.Height;
		req[count++] = bitmap.Width;
		Type = GSM_NokiaStartupLogo;
	        switch (bitmap.Height) {
			case 65: Type = GSM_Nokia7110StartupLogo; break;
			case 60: Type = GSM_Nokia6210StartupLogo; break;
		}
		PHONE_EncodeBitmap(Type, req+count, &bitmap);
		count = count + PHONE_GetBitmapSize(Type);

		req[12]++;
	}
	if (bitmap.Type == GSM_WelcomeNoteText) {
		req[count++]=0x02;
		req[count++]=strlen(DecodeUnicodeString(bitmap.Text));
		memcpy(req+count,DecodeUnicodeString(bitmap.Text),strlen(DecodeUnicodeString(bitmap.Text)));
		count=count+strlen(DecodeUnicodeString(bitmap.Text));

		req[12]++;
	}

	req[4]=(count-12)%256;
	req[5]=(count-12)/256;

	fwrite(req, 1, count, file);
}		     

static void SaveLMBCallerEntry(FILE *file, GSM_Bitmap bitmap)
{  
	int count=12, textlen;
	char req[500] = {
		'C','G','R',' ',    /*block identifier*/
		00,00,              /*block data size*/
		02,00,              
		00,                 /*group number=0,1,etc.*/
		00,00,00};

	req[count++] = bitmap.Location - 1;
	if (bitmap.DefaultName) {
		textlen = 0;
	} else {
		textlen = strlen(DecodeUnicodeString(bitmap.Text));
	}
	req[count++] = textlen;
	if (!bitmap.DefaultName) {
		memcpy(req+count,DecodeUnicodeString(bitmap.Text),textlen);
		count += textlen;
	}
	req[count++] = bitmap.Ringtone;
	if (bitmap.Enabled) req[count++] = 0x01; else req[count++] = 0x00;
	req[count++] = (PHONE_GetBitmapSize(GSM_NokiaCallerLogo) + 4) >> 8;
	req[count++] = (PHONE_GetBitmapSize(GSM_NokiaCallerLogo) + 4) % 0xff;
	NOKIA_CopyBitmap(GSM_NokiaCallerLogo, &bitmap, req, &count);
	req[count++]=0;

	req[4]=(count-12)%256;
	req[5]=(count-12)/256;
	req[8]=bitmap.Location;

	fwrite(req, 1, count, file);
}		     

static void SaveLMBPBKEntry(FILE *file, GSM_PhonebookEntry entry)
{
	int count = 16, blocks;
	char req[500] = {
		'P','B','E','2', /*block identifier*/
		00,00,           /*block data size*/
		00,00,           
		00,00,           /*position of phonebook entry*/		                 
		03,              /*memory type. ME=02;SM=03*/
		00,
		00,00,           /*position of phonebook entry*/                   
		03,              /*memory type. ME=02;SM=03*/
		00};

	count=count+N71_65_EncodePhonebookFrame(req+16, entry, &blocks, true);

	req[4]=(count-12)%256;
	req[5]=(count-12)/256;
	req[8]=req[12] = entry.Location & 0xff;
	req[9]=req[13] = (entry.Location >> 8);
	if (entry.MemoryType==GMT_ME) req[10]=req[14]=2;
            
	fwrite(req, 1, count, file);	    
}

static GSM_Error SaveLMB(FILE *file, GSM_Backup *backup)
{
	int i;
	char LMBHeader[] = {'L','M','B',' '}; /*file identifier*/    
	char PBKHeader[] = {		      /* Phonebook header block */
		'P','B','K',' ', /*block identifier*/
		0x08,00,         /*block data size*/
		0x02,00,         
		03,              /*memory type. ME=02;SM=03*/
		00,00,00,
		00,00,           /*size of phonebook*/
		14,              /*max length of each position*/
		00,00,00,00,00};

	/* Write the header of the file. */		    		      
	fwrite(LMBHeader, 1, sizeof(LMBHeader), file);

	if (backup->PhonePhonebook[0]!=NULL) {
		PBKHeader[8]	= 2;		/* memory type=GMT_ME */
		PBKHeader[12] 	= (unsigned char)(500 % 256);
		PBKHeader[13] 	= 500 / 256;
		fwrite(PBKHeader, 1, sizeof(PBKHeader), file);
		i=0;
		while (backup->PhonePhonebook[i]!=NULL) {
			SaveLMBPBKEntry(file, *backup->PhonePhonebook[i]);
			i++;
		}
	}
	if (backup->SIMPhonebook[0]!=NULL) {
		PBKHeader[8]	= 3;		/* memory type=GMT_SM */
		PBKHeader[12] 	= (unsigned char)(250 % 256);
		PBKHeader[13] 	= 250 / 256;
		PBKHeader[14]	= 0x16;		/* max size of one entry */
		fwrite(PBKHeader, 1, sizeof(PBKHeader), file);
		i=0;
		while (backup->SIMPhonebook[i]!=NULL) {
			SaveLMBPBKEntry(file, *backup->SIMPhonebook[i]);
			i++;
		}
	}
	i=0;
	while (backup->CallerLogos[i]!=NULL) {
		SaveLMBCallerEntry(file, *backup->CallerLogos[i]);
		i++;
	}
	if (backup->StartupLogo!=NULL) {
		SaveLMBStartupEntry(file, *backup->StartupLogo);
	}

	return GE_NONE;
}

void GSM_GetBackupFeatures(char *FileName, GSM_Backup_Info *backup)
{
	if (strstr(FileName,".lmb")) {
		backup->IMEI 		= false;
		backup->Model 		= false;
		backup->DateTime 	= false;
		backup->PhonePhonebook 	= true;
		backup->SIMPhonebook 	= true;
		backup->ToDo		= false;
		backup->Calendar 	= false;
		backup->CallerLogos 	= true;
		backup->SMSC 		= false;
		backup->WAPBookmark 	= false;
		backup->WAPSettings 	= false;
		backup->Ringtone 	= false;
		backup->StartupLogo 	= true;
		backup->OperatorLogo 	= false;
		backup->Profiles 	= false;
	} else {
		backup->IMEI 		= true;
		backup->Model 		= true;
		backup->DateTime 	= true;
		backup->PhonePhonebook 	= true;
		backup->SIMPhonebook 	= true;
		backup->ToDo		= true;
		backup->Calendar 	= true;
		backup->CallerLogos 	= true;
		backup->SMSC 		= true;
		backup->WAPBookmark 	= true;
		backup->WAPSettings 	= true;
		backup->Ringtone 	= true;
		backup->StartupLogo 	= true;
		backup->OperatorLogo 	= true;
		backup->Profiles 	= true;
	}
}

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup)
{
	FILE *file;
  
	file = fopen(FileName, "wb");      
	if (!file) return(GE_CANTOPENFILE);

	if (strstr(FileName,".lmb")) {
		SaveLMB(file,backup);
	} else {
		SaveBackup(file,backup);
	}

	fclose(file);
   
	return GE_NONE;
}

static void ReadPbkEntry(CFG_Header *file_info, char *section, GSM_PhonebookEntry *Pbk)
{
	unsigned char		buffer[10000];
	char			*readvalue;
	int			num;
	CFG_Entry		*e;

	Pbk->EntriesNum = 0;
	e = CFG_FindLastSectionEntry(file_info, section, false);
	while (1) {
		if (e == NULL) break;
		num = -1;
		if (strlen(e->key) == 11) {
			if (mystrncasecmp("Entry", e->key,   5) &&
			    mystrncasecmp("Type",  e->key+7, 4)) {
				num = atoi(e->key+5);
			}
		}
		e = e->prev;
		if (num != -1) {
			sprintf(buffer,"Entry%02iType",num);
			readvalue = CFG_Get(file_info, section, buffer, false);
			if (mystrncasecmp(readvalue,"NumberGeneral",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
			} else if (mystrncasecmp(readvalue,"NumberMobile",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
			} else if (mystrncasecmp(readvalue,"NumberWork",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
			} else if (mystrncasecmp(readvalue,"NumberFax",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
			} else if (mystrncasecmp(readvalue,"NumberHome",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
			} else if (mystrncasecmp(readvalue,"Note",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
			} else if (mystrncasecmp(readvalue,"Postal",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
			} else if (mystrncasecmp(readvalue,"Email",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
			} else if (mystrncasecmp(readvalue,"URL",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
			} else if (mystrncasecmp(readvalue,"Name",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Name;
			} else if (mystrncasecmp(readvalue,"CallerGroup",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group;
				Pbk->Entries[Pbk->EntriesNum].Number = 0;
				sprintf(buffer,"Entry%02iNumber",num);
				readvalue = CFG_Get(file_info, section, buffer, false);
				if (readvalue!=NULL) {
					Pbk->Entries[Pbk->EntriesNum].Number = atoi(readvalue);
				}
				Pbk->EntriesNum ++;
				continue;
			}
			sprintf(buffer,"Entry%02iText",num);
			ReadBackupText(file_info, section, buffer, Pbk->Entries[Pbk->EntriesNum].Text);
			dprintf("text \"%s\", type %i\n",DecodeUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text),Pbk->Entries[Pbk->EntriesNum].EntryType);
			Pbk->Entries[Pbk->EntriesNum].VoiceTag = 0;
			sprintf(buffer,"Entry%02iVoiceTag",num);
			readvalue = CFG_Get(file_info, section, buffer, false);
			if (readvalue!=NULL) {
				Pbk->Entries[Pbk->EntriesNum].VoiceTag = atoi(readvalue);
			}
			Pbk->EntriesNum ++;
		}
	}
}

static void ReadCalendarEntry(CFG_Header *file_info, char *section, GSM_CalendarNote *note)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Type");
	readvalue = CFG_Get(file_info, section, buffer, false);
	note->Type = GCN_REMINDER;
	if (readvalue!=NULL)
	{
		if (mystrncasecmp(readvalue,"Call",0)) {
			note->Type = GCN_CALL;
		} else if (mystrncasecmp(readvalue,"Meeting",0)) {
			note->Type = GCN_MEETING;
		} else if (mystrncasecmp(readvalue,"Birthday",0)) {
			note->Type = GCN_BIRTHDAY;
		} else if (mystrncasecmp(readvalue,"Training/Athletism",0)) {
			note->Type = GCN_T_ATHL;
		} else if (mystrncasecmp(readvalue,"Training/BallGames",0)) {
			note->Type = GCN_T_BALL;
		} else if (mystrncasecmp(readvalue,"Training/Cycling",0)) {
			note->Type = GCN_T_CYCL;
		} else if (mystrncasecmp(readvalue,"Training/Budo",0)) {
			note->Type = GCN_T_BUDO;
		} else if (mystrncasecmp(readvalue,"Training/Dance",0)) {
			note->Type = GCN_T_DANC;
		} else if (mystrncasecmp(readvalue,"Training/ExtremeSports",0)) {
			note->Type = GCN_T_EXTR;
		} else if (mystrncasecmp(readvalue,"Training/Football",0)) {
			note->Type = GCN_T_FOOT;
		} else if (mystrncasecmp(readvalue,"Training/Golf",0)) {
			note->Type = GCN_T_GOLF;
		} else if (mystrncasecmp(readvalue,"Training/Gym",0)) {
			note->Type = GCN_T_GYM;
		} else if (mystrncasecmp(readvalue,"Training/HorseRaces",0)) {
			note->Type = GCN_T_HORS;
		} else if (mystrncasecmp(readvalue,"Training/Hockey",0)) {
			note->Type = GCN_T_HOCK;
		} else if (mystrncasecmp(readvalue,"Training/Races",0)) {
			note->Type = GCN_T_RACE;
		} else if (mystrncasecmp(readvalue,"Training/Rugby",0)) {
			note->Type = GCN_T_RUGB;
		} else if (mystrncasecmp(readvalue,"Training/Sailing",0)) {
			note->Type = GCN_T_SAIL;
		} else if (mystrncasecmp(readvalue,"Training/StreetGames",0)) {
			note->Type = GCN_T_STRE;
		} else if (mystrncasecmp(readvalue,"Training/Swimming",0)) {
			note->Type = GCN_T_SWIM;
		} else if (mystrncasecmp(readvalue,"Training/Tennis",0)) {
			note->Type = GCN_T_TENN;
		} else if (mystrncasecmp(readvalue,"Training/Travels",0)) {
			note->Type = GCN_T_TRAV;
		} else if (mystrncasecmp(readvalue,"Training/WinterGames",0)) {
			note->Type = GCN_T_WINT;
		}
	}
	sprintf(buffer,"Text");
	ReadBackupText(file_info, section, buffer, note->Text);
	sprintf(buffer,"Phone");
	ReadBackupText(file_info, section, buffer, note->Phone);
	note->Recurrance = 0;
	sprintf(buffer,"Recurrance");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) note->Recurrance = atoi(readvalue) * 24;
	sprintf(buffer,"Time");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) ReadvCalTime(&note->Time, readvalue);
	note->Alarm.Year = 0;
	sprintf(buffer,"Alarm");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL)
	{
		ReadvCalTime(&note->Alarm, readvalue);
		note->SilentAlarm = false;
		sprintf(buffer,"AlarmType");
		readvalue = CFG_Get(file_info, section, buffer, false);
		if (readvalue!=NULL)
		{
			if (mystrncasecmp(readvalue,"Silent",0)) note->SilentAlarm = true;
		}
	}
}

static void ReadToDoEntry(CFG_Header *file_info, char *section, GSM_TODO *ToDo)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Text");
	ReadBackupText(file_info, section, buffer, ToDo->Text);
	ToDo->Priority = 1;
	sprintf(buffer,"Priority");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) ToDo->Priority = atoi(readvalue);
}

static void ReadBitmapEntry(CFG_Header *file_info, char *section, GSM_Bitmap *bitmap)
{
	char		*readvalue;
	unsigned char	buffer[10000];
	unsigned char 	Width, Height;
	int 		x, y;

	GSM_GetMaxBitmapWidthHeight(bitmap->Type, &Width, &Height);
	sprintf(buffer,"Width");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue==NULL) bitmap->Width = Width; else bitmap->Width = atoi(readvalue);
	sprintf(buffer,"Height");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue==NULL) bitmap->Height = Height; else bitmap->Height = atoi(readvalue);
	GSM_ClearBitmap(bitmap);
	for (y=0;y<bitmap->Height;y++) {
		sprintf(buffer,"Bitmap%02i",y);
		readvalue = CFG_Get(file_info, section, buffer, false);
		if (readvalue!=NULL) {
			for (x=0;x<bitmap->Width;x++) {
				if (readvalue[x+1]=='#') GSM_SetPointBitmap(bitmap,x,y);
			}
		}
	}
}

static void ReadCallerEntry(CFG_Header *file_info, char *section, GSM_Bitmap *bitmap)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	bitmap->Type = GSM_CallerLogo;
	ReadBitmapEntry(file_info, section, bitmap);
	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, bitmap->Text);
	if (bitmap->Text[0] == 0x00 && bitmap->Text[1] == 0x00) {
		bitmap->DefaultName = true;
	} else {
		bitmap->DefaultName = false;
	}
	sprintf(buffer,"Ringtone");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue==NULL) bitmap->Ringtone = 0xff; else DecodeHexBin (&bitmap->Ringtone, readvalue, 2);
	sprintf(buffer,"Enabled");
	readvalue = CFG_Get(file_info, section, buffer, false);
        bitmap->Enabled = true;
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"False",0)) bitmap->Enabled = false;
	}
}

static void ReadStartupEntry(CFG_Header *file_info, char *section, GSM_Bitmap *bitmap)
{
	unsigned char		buffer[10000];

	sprintf(buffer,"Text");
	ReadBackupText(file_info, section, buffer, bitmap->Text);
	if (bitmap->Text[0]!=0 || bitmap->Text[1]!=0) {
		bitmap->Type = GSM_WelcomeNoteText;
	} else {
		bitmap->Type 	 = GSM_StartupLogo;
		bitmap->Location = 1;
		ReadBitmapEntry(file_info, section, bitmap);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL) GSM_PrintBitmap(di.df,bitmap);
#endif
	}
}

static void ReadWAPBookmarkEntry(CFG_Header *file_info, char *section, GSM_WAPBookmark *bookmark)
{
	unsigned char		buffer[10000];

	sprintf(buffer,"URL");
	ReadBackupText(file_info, section, buffer, bookmark->Address);
	sprintf(buffer,"Title");
	ReadBackupText(file_info, section, buffer, bookmark->Title);
}

static void ReadOperatorEntry(CFG_Header *file_info, char *section, GSM_Bitmap *bitmap)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Network");
	readvalue = CFG_Get(file_info, section, buffer, false);
	memcpy(bitmap->NetworkCode, readvalue + 1, 6);
	bitmap->NetworkCode[6] = 0;
	bitmap->Type = GSM_OperatorLogo;
	ReadBitmapEntry(file_info, section, bitmap);
}

static void ReadSMSCEntry(CFG_Header *file_info, char *section, GSM_SMSC *SMSC)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, SMSC->Name);
	sprintf(buffer,"Number");
	ReadBackupText(file_info, section, buffer, SMSC->Number);
	sprintf(buffer,"DefaultNumber");
	ReadBackupText(file_info, section, buffer, SMSC->DefaultNumber);
	sprintf(buffer,"Format");
	SMSC->Format = GSMF_Text;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Fax",0)) {
			SMSC->Format = GSMF_Fax;
		} else if (mystrncasecmp(readvalue,"Email",0)) {
			SMSC->Format = GSMF_Email;
		} else if (mystrncasecmp(readvalue,"Pager",0)) {
			SMSC->Format = GSMF_Pager;
		}
	}
	sprintf(buffer,"Validity");
	SMSC->Validity.Relative = GSMV_Max_Time;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) 
	{
		if (mystrncasecmp(readvalue,"1hour",0)) {
			SMSC->Validity.Relative = GSMV_1_Hour;
		} else if (mystrncasecmp(readvalue,"6hours",0)) {
			SMSC->Validity.Relative = GSMV_6_Hours;
		} else if (mystrncasecmp(readvalue,"24hours",0)) {
			SMSC->Validity.Relative = GSMV_24_Hours;
		} else if (mystrncasecmp(readvalue,"72hours",0)) {
			SMSC->Validity.Relative = GSMV_72_Hours;
		} else if (mystrncasecmp(readvalue,"1week",0)) {
			SMSC->Validity.Relative = GSMV_1_Week;
		}
	}
}

static void ReadWAPSettingsEntry(CFG_Header *file_info, char *section, GSM_MultiWAPSettings *settings)
{
	unsigned char		buffer[10000];
	char			*readvalue;
	int			num;
	CFG_Entry		*e;

	settings->Number = 0;
	e = CFG_FindLastSectionEntry(file_info, section, false);
	while (1) {
		if (e == NULL) break;
		num = -1;
		if (strlen(e->key) == 7) {
			if (mystrncasecmp("Title", e->key,5)) num = atoi(e->key+5);
		}
		e = e->prev;
		if (num != -1) {
			sprintf(buffer,"Title%02i",num);
			ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Title);
			sprintf(buffer,"HomePage%02i",num);
			ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].HomePage);
			sprintf(buffer,"Type%02i",num);
			settings->Settings[settings->Number].IsContinuous = true;
			readvalue = CFG_Get(file_info, section, buffer, false);
			if (readvalue!=NULL) 
			{
				if (mystrncasecmp(readvalue,"Temporary",0)) settings->Settings[settings->Number].IsContinuous = false;
			}
			sprintf(buffer,"Security%02i",num);
			settings->Settings[settings->Number].IsSecurity = true;
			readvalue = CFG_Get(file_info, section, buffer, false);
			if (readvalue!=NULL) 
			{
				if (mystrncasecmp(readvalue,"Off",0)) settings->Settings[settings->Number].IsSecurity = false;
			}
			sprintf(buffer,"Bearer%02i",num);
			readvalue = CFG_Get(file_info, section, buffer, false);
			if (readvalue!=NULL) 
			{
				if (mystrncasecmp(readvalue,"SMS",0)) {
					settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_SMS;
					sprintf(buffer,"Server%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Server);
					sprintf(buffer,"Service%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Service);
				} else if ((mystrncasecmp(readvalue,"Data",0) || mystrncasecmp(readvalue,"GPRS",0))) {
					settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_DATA;
					if (mystrncasecmp(readvalue,"GPRS",0)) settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_GPRS;
					sprintf(buffer,"Number%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].DialUp);
					sprintf(buffer,"IP%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].IPAddress);
					sprintf(buffer,"User%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].User);
					sprintf(buffer,"Password%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Password);
					sprintf(buffer,"Authentication%02i",num);
					settings->Settings[settings->Number].IsNormalAuthentication = true;
					readvalue = CFG_Get(file_info, section, buffer, false);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"Secure",0)) settings->Settings[settings->Number].IsNormalAuthentication = false;
					}
					sprintf(buffer,"CallSpeed%02i",num);
					settings->Settings[settings->Number].Speed = WAPSETTINGS_SPEED_14400;
					readvalue = CFG_Get(file_info, section, buffer, false);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"9600",0)) settings->Settings[settings->Number].Speed = WAPSETTINGS_SPEED_9600;
						if (mystrncasecmp(readvalue,"auto",0)) settings->Settings[settings->Number].Speed = WAPSETTINGS_SPEED_AUTO;
					}
					sprintf(buffer,"Login%02i",num);
					settings->Settings[settings->Number].ManualLogin = false;
					readvalue = CFG_Get(file_info, section, buffer, false);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"Manual",0)) settings->Settings[settings->Number].ManualLogin = true;
					}	
					sprintf(buffer,"CallType%02i",num);
					settings->Settings[settings->Number].IsISDNCall = true;
					readvalue = CFG_Get(file_info, section, buffer, false);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"Analogue",0)) settings->Settings[settings->Number].IsISDNCall = false;
					}
				} else if (mystrncasecmp(readvalue,"USSD",0)) {
					settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_USSD;
					sprintf(buffer,"ServiceCode%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Code);
					sprintf(buffer,"IP%02i",num);
					readvalue = CFG_Get(file_info, section, buffer, false);
					if (readvalue!=NULL) {
						settings->Settings[settings->Number].IsIP = true;
						sprintf(buffer,"IP%02i",num);
					} else {
						settings->Settings[settings->Number].IsIP = false;
						sprintf(buffer,"Number%02i",num);
					}
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Service);
				}
			}
			settings->Number++;
		}
	}
}

static void ReadRingtoneEntry(CFG_Header *file_info, char *section, GSM_Ringtone *ringtone)
{
	unsigned char		buffer[10000], buffer2[10000];
	char			*readvalue;

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, ringtone->Name);
	ringtone->Location = 0;
	sprintf(buffer,"Location");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) ringtone->Location = atoi(readvalue);
	sprintf(buffer,"NokiaBinary00");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		ringtone->Format = RING_NOKIABINARY;
		ReadLinkedBackupText(file_info, section, "NokiaBinary", buffer2);
		DecodeHexBin (ringtone->NokiaBinary.Frame, buffer2, strlen(buffer2));
		ringtone->NokiaBinary.Length = strlen(buffer2)/2;
	}
}

static void ReadProfileEntry(CFG_Header *file_info, char *section, GSM_Profile *Profile)
{
	unsigned char		buffer[10000];
	char			*readvalue;
	bool			unknown;
	int			num,j;
	CFG_Entry		*e;

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, Profile->Name);

	sprintf(buffer,"Location");
	readvalue = CFG_Get(file_info, section, buffer, false);
	Profile->Location = atoi(readvalue);

	Profile->DefaultName = false;
	sprintf(buffer,"DefaultName");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL && mystrncasecmp(buffer,"true",0)) Profile->DefaultName = true;

	Profile->HeadSetProfile = false;
	sprintf(buffer,"HeadSetProfile");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL && mystrncasecmp(buffer,"true",0)) Profile->HeadSetProfile = true;

	Profile->CarKitProfile = false;
	sprintf(buffer,"CarKitProfile");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL && mystrncasecmp(buffer,"true",0)) Profile->CarKitProfile = true;

	Profile->FeaturesNumber = 0;
	e = CFG_FindLastSectionEntry(file_info, section, false);
	while (1) {
		if (e == NULL) break;
		num = -1;
		if (strlen(e->key) == 9) {
			if (mystrncasecmp("Feature", e->key, 7)) num = atoi(e->key+7);
		}
		e = e->prev;
		if (num != -1) {
			sprintf(buffer,"Feature%02i",num);
			readvalue = CFG_Get(file_info, section, buffer, false);
			if (readvalue==NULL) break;
			unknown = true;
			if (mystrncasecmp(readvalue,"RingtoneID",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_RingtoneID;
				sprintf(buffer,"Value%02i",num);
				readvalue = CFG_Get(file_info, section, buffer, false);
				Profile->FeatureValue[Profile->FeaturesNumber]=atoi(readvalue);
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"MessageToneID",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_MessageToneID;
				sprintf(buffer,"Value%02i",num);
				readvalue = CFG_Get(file_info, section, buffer, false);
				Profile->FeatureValue[Profile->FeaturesNumber]=atoi(readvalue);
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"ScreenSaverNumber",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_ScreenSaverNumber;
				sprintf(buffer,"Value%02i",num);
				readvalue = CFG_Get(file_info, section, buffer, false);
				Profile->FeatureValue[Profile->FeaturesNumber]=atoi(readvalue);
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"CallerGroups",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_CallerGroups;
				sprintf(buffer,"Value%02i",num);
				readvalue = CFG_Get(file_info, section, buffer, false);
				for (j=0;j<5;j++) {
					Profile->CallerGroups[j]=false;
					if (strstr(readvalue,"1"+j)!=NULL) Profile->CallerGroups[j]=true;
				}
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"IncomingCallAlert",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_CallAlert;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"RingtoneVolume",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_RingtoneVolume;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"Vibrating",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_Vibration;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"MessageTone",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_MessageTone;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"KeypadTones",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_KeypadTone;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"WarningTones",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_WarningTone;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"ScreenSaver",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_ScreenSaver;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"ScreenSaverTimeout",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_ScreenSaverTime;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"AutomaticAnswer",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_AutoAnswer;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"Lights",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_Lights;
				unknown = false;
			}
			if (!unknown) {
				sprintf(buffer,"Value%02i",num);
				readvalue = CFG_Get(file_info, section, buffer, false);
				if (mystrncasecmp(readvalue,"Level1",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL1;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_KeypadTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_LEVEL1;
					}
				} else if (mystrncasecmp(readvalue,"Level2",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL2;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_KeypadTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_LEVEL2;
					}
				} else if (mystrncasecmp(readvalue,"Level3",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL3;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_KeypadTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_LEVEL3;
					}
				} else if (mystrncasecmp(readvalue,"Level4",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL4;
				} else if (mystrncasecmp(readvalue,"Level5",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL5;
				} else if (mystrncasecmp(readvalue,"Off",0)) {
					switch (Profile->FeatureID[Profile->FeaturesNumber]) {
					case Profile_MessageTone:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_NOTONE;
						break;
					case Profile_AutoAnswer:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_AUTOANSWER_OFF;
						break;
					case Profile_Lights:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_LIGHTS_OFF;
						break;
					case Profile_ScreenSaver:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_OFF;
						break;
					case Profile_WarningTone:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_WARNING_OFF;
						break;
					case Profile_CallAlert:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_OFF;
						break;
					case Profile_Vibration:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VIBRATION_OFF;
						break;
					default:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_OFF;
						break;
					}
				} else if (mystrncasecmp(readvalue,"Ringing",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_RINGING;
				} else if (mystrncasecmp(readvalue,"BeepOnce",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_BEEPONCE;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_MessageTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_BEEPONCE;
					}
				} else if (mystrncasecmp(readvalue,"RingOnce",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_RINGONCE;
				} else if (mystrncasecmp(readvalue,"Ascending",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_ASCENDING;
				} else if (mystrncasecmp(readvalue,"CallerGroups",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_CALLERGROUPS;
				} else if (mystrncasecmp(readvalue,"Standard",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_STANDARD;
				} else if (mystrncasecmp(readvalue,"Special",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_SPECIAL;
				} else if (mystrncasecmp(readvalue,"Ascending",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_ASCENDING;
				} else if (mystrncasecmp(readvalue,"Personal",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_PERSONAL;
				} else if (mystrncasecmp(readvalue,"VibrateFirst",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VIBRATION_FIRST;
				} else if (mystrncasecmp(readvalue,"Auto",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_LIGHTS_AUTO;
				} else if (mystrncasecmp(readvalue,"5Seconds",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_5SEC;
				} else if (mystrncasecmp(readvalue,"20Seconds",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_20SEC;
				} else if (mystrncasecmp(readvalue,"1Minute",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_1MIN;
				} else if (mystrncasecmp(readvalue,"2Minutes",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_2MIN;
				} else if (mystrncasecmp(readvalue,"5Minutes",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_5MIN;
				} else if (mystrncasecmp(readvalue,"10Minutes",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_10MIN;
				} else if (mystrncasecmp(readvalue,"On",0)) {
					switch (Profile->FeatureID[Profile->FeaturesNumber]) {
					case Profile_AutoAnswer:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_AUTOANSWER_ON;
						break;
					case Profile_WarningTone:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_WARNING_ON;
						break;
					case Profile_ScreenSaver:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_ON;
						break;
					default:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VIBRATION_ON;
						break;
					}
				} else unknown = true;
			}
			if (!unknown) Profile->FeaturesNumber++;
		}
	}
}

static GSM_Error LoadBackup(char *FileName, GSM_Backup *backup)
{
	CFG_Header		*file_info, *h;
	char			buffer[100];
	char			*readvalue;
	int			num;
	GSM_PhonebookEntry 	PBK;

	file_info = CFG_ReadFile(FileName, false);

	sprintf(buffer,"Backup");

	readvalue = CFG_Get(file_info, buffer, "Format", false);
	/* Is this format version supported ? */
	if (strcmp(readvalue,"1.01")!=0) return GE_NOTSUPPORTED;

	GSM_ClearBackup(backup);

	readvalue = CFG_Get(file_info, buffer, "IMEI", false);
	if (readvalue!=NULL) strcpy(backup->IMEI,readvalue);
	readvalue = CFG_Get(file_info, buffer, "Phone", false);
	if (readvalue!=NULL) strcpy(backup->Model,readvalue);
	readvalue = CFG_Get(file_info, buffer, "Time", false);
	if (readvalue!=NULL) strcpy(backup->DateTime,readvalue);

	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("Profile", h->section, 7)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_PROFILES) {
				backup->Profiles[num] = malloc(sizeof(GSM_Profile));
			        if (backup->Profiles[num] == NULL) return GE_MOREMEMORY;
				backup->Profiles[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_PROFILES\n");
				return GE_MOREMEMORY;
			}
			ReadProfileEntry(file_info, h->section, backup->Profiles[num]);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("PhonePBK", h->section, 8)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
				backup->PhonePhonebook[num] = malloc(sizeof(GSM_PhonebookEntry));
			        if (backup->PhonePhonebook[num] == NULL) return GE_MOREMEMORY;
				backup->PhonePhonebook[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
				return GE_MOREMEMORY;
			}
			backup->PhonePhonebook[num]->Location	= atoi (readvalue);
			backup->PhonePhonebook[num]->MemoryType	= GMT_ME;
			ReadPbkEntry(file_info, h->section, backup->PhonePhonebook[num]);
			dprintf("number of entries = %i\n",backup->PhonePhonebook[num]->EntriesNum);
			num++;
                }
        }
	num = 0;
	while (0) {
		if (backup->PhonePhonebook[num] == NULL) break;
		if (backup->PhonePhonebook[num+1] != NULL) {
			if (backup->PhonePhonebook[num+1]->Location < backup->PhonePhonebook[num]->Location) {
				memcpy(&PBK,backup->PhonePhonebook[num+1],sizeof(GSM_PhonebookEntry));
				memcpy(backup->PhonePhonebook[num+1],backup->PhonePhonebook[num],sizeof(GSM_PhonebookEntry));
				memcpy(backup->PhonePhonebook[num],&PBK,sizeof(GSM_PhonebookEntry));
				num = 0;
				continue;
			}
		}
		num++;
	}
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("SIMPBK", h->section, 6)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_SIMPHONEBOOK) {
				backup->SIMPhonebook[num] = malloc(sizeof(GSM_PhonebookEntry));
			        if (backup->SIMPhonebook[num] == NULL) return GE_MOREMEMORY;
				backup->SIMPhonebook[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_SIMPHONEBOOK\n");
				return GE_MOREMEMORY;
			}
			backup->SIMPhonebook[num]->Location	= atoi (readvalue);
			backup->SIMPhonebook[num]->MemoryType	= GMT_SM;
			ReadPbkEntry(file_info, h->section, backup->SIMPhonebook[num]);
			num++;
                }
        }
	num = 0;
	while (0) {
		if (backup->SIMPhonebook[num] == NULL) break;
		if (backup->SIMPhonebook[num+1] != NULL) {
			if (backup->SIMPhonebook[num+1]->Location < backup->SIMPhonebook[num]->Location) {
				memcpy(&PBK,backup->SIMPhonebook[num+1],sizeof(GSM_PhonebookEntry));
				memcpy(backup->SIMPhonebook[num+1],backup->SIMPhonebook[num],sizeof(GSM_PhonebookEntry));
				memcpy(backup->SIMPhonebook[num],&PBK,sizeof(GSM_PhonebookEntry));
				num = 0;
				continue;
			}
		}
		num++;
	}
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("Calendar", h->section, 8)) {
			readvalue = CFG_Get(file_info, h->section, "Type", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_CALENDAR) {
				backup->Calendar[num] = malloc(sizeof(GSM_CalendarNote));
			        if (backup->Calendar[num] == NULL) return GE_MOREMEMORY;
				backup->Calendar[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_CALENDAR\n");
				return GE_MOREMEMORY;
			}
			backup->Calendar[num]->Location = num + 1;
			ReadCalendarEntry(file_info, h->section, backup->Calendar[num]);
			dprintf("Note text \"%s\"\n",DecodeUnicodeString(backup->Calendar[num]->Text));
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("Caller", h->section, 6)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_CALLER) {
				backup->CallerLogos[num] = malloc(sizeof(GSM_Bitmap));
			        if (backup->CallerLogos[num] == NULL) return GE_MOREMEMORY;
				backup->CallerLogos[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_CALLER\n");
				return GE_MOREMEMORY;
			}
			backup->CallerLogos[num]->Location = atoi (readvalue);
			ReadCallerEntry(file_info, h->section, backup->CallerLogos[num]);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("SMSC", h->section, 4)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_SMSC) {
				backup->SMSC[num] = malloc(sizeof(GSM_SMSC));
			        if (backup->SMSC[num] == NULL) return GE_MOREMEMORY;
				backup->SMSC[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_SMSC\n");
				return GE_MOREMEMORY;
			}
			backup->SMSC[num]->Location = atoi (readvalue);
			ReadSMSCEntry(file_info, h->section, backup->SMSC[num]);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("Bookmark", h->section, 8)) {
			readvalue = CFG_Get(file_info, h->section, "URL", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_WAPBOOKMARK) {
				backup->WAPBookmark[num] = malloc(sizeof(GSM_WAPBookmark));
			        if (backup->WAPBookmark[num] == NULL) return GE_MOREMEMORY;
				backup->WAPBookmark[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_WAPBOOKMARK\n");
				return GE_MOREMEMORY;
			}
			backup->WAPBookmark[num]->Location = num + 1;
			ReadWAPBookmarkEntry(file_info, h->section, backup->WAPBookmark[num]);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("Settings", h->section, 8)) {
			readvalue = CFG_Get(file_info, h->section, "Title00", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_WAPSETTINGS) {
				backup->WAPSettings[num] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (backup->WAPSettings[num] == NULL) return GE_MOREMEMORY;
				backup->WAPSettings[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_WAPSETTINGS\n");
				return GE_MOREMEMORY;
			}
			backup->WAPSettings[num]->Location = num + 1;
			ReadWAPSettingsEntry(file_info, h->section, backup->WAPSettings[num]);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("Ringtone", h->section, 8)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_RINGTONES) {
				backup->Ringtone[num] = malloc(sizeof(GSM_Ringtone));
			        if (backup->Ringtone[num] == NULL) return GE_MOREMEMORY;
				backup->Ringtone[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_RINGTONES\n");
				return GE_MOREMEMORY;
			}
			ReadRingtoneEntry(file_info, h->section, backup->Ringtone[num]);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("TODO", h->section, 4)) {
			readvalue = CFG_Get(file_info, h->section, "Location", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_TODO) {
				backup->ToDo[num] = malloc(sizeof(GSM_TODO));
			        if (backup->ToDo[num] == NULL) return GE_MOREMEMORY;
				backup->ToDo[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_TODO\n");
				return GE_MOREMEMORY;
			}
			backup->ToDo[num]->Location = num + 1;
			ReadToDoEntry(file_info, h->section, backup->ToDo[num]);
			num++;
                }
        }
	sprintf(buffer,"Startup");
	readvalue = CFG_Get(file_info, buffer, "Text", false);
	if (readvalue==NULL) {
		readvalue = CFG_Get(file_info, buffer, "Width", false);
	}
	if (readvalue!=NULL) {
		backup->StartupLogo = malloc(sizeof(GSM_Bitmap));
	        if (backup->StartupLogo == NULL) return GE_MOREMEMORY;
		ReadStartupEntry(file_info, buffer, backup->StartupLogo);
	}
	sprintf(buffer,"Operator");
	readvalue = CFG_Get(file_info, buffer, "Network", false);
	if (readvalue!=NULL) {
		backup->OperatorLogo = malloc(sizeof(GSM_Bitmap));
	        if (backup->OperatorLogo == NULL) return GE_MOREMEMORY;
		ReadOperatorEntry(file_info, buffer, backup->OperatorLogo);
	}

	return GE_NONE;
}

static GSM_Error LoadLMBCallerEntry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{ 
	GSM_Bitmap 	bitmap;
	int 		num;

#ifdef DEBUG
	dprintf("Number %i, name \"", buffer2[0]+1);
	for (num=0;num<buffer2[1];num++) dprintf("%c", buffer2[num+2]);
	dprintf("\"\n");
	dprintf("Ringtone ID=%i\n", buffer2[num+2]);
	if (buffer2[num+3]==1) {
		dprintf("Logo enabled\n");
	} else {
		dprintf("Logo disabled\n");
	}
#endif

  	bitmap.Location	= buffer2[0] + 1;
	bitmap.Type	= GSM_CallerLogo;
	bitmap.Ringtone	= buffer2[buffer2[1]+2];
	
	EncodeUnicode(bitmap.Text,buffer2+2,buffer2[1]);
	if (bitmap.Text[0] == 0x00 && bitmap.Text[1] == 0x00) {
		bitmap.DefaultName = true;
	} else {
		bitmap.DefaultName = false;
	}
	
	bitmap.Enabled=false;
	if (buffer2[buffer2[1]+3]==1) bitmap.Enabled=true;

	PHONE_DecodeBitmap(GSM_NokiaCallerLogo, buffer2+(buffer2[1]+10), &bitmap);

#ifdef DEBUG
	dprintf("Caller logo\n");
	if (di.dl == DL_TEXTALL) GSM_PrintBitmap(di.df,&bitmap);
#endif

	num = 0;
	while (backup->CallerLogos[num] != NULL) num++;
	if (num < GSM_BACKUP_MAX_CALLER) {
		backup->CallerLogos[num] = malloc(sizeof(GSM_Bitmap));
	        if (backup->CallerLogos[num] == NULL) return GE_MOREMEMORY;
		backup->CallerLogos[num + 1] = NULL;
	} else {
		dprintf("Increase GSM_BACKUP_MAX_CALLER\n");
		return GE_MOREMEMORY;
	}
	*backup->CallerLogos[num] = bitmap;

	return GE_NONE;
}		     

static GSM_Error LoadLMBStartupEntry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{
	int 			i,j;
#ifdef DEBUG
	int 			z;
#endif
	GSM_Phone_Bitmap_Types 	Type;

	j=1;
	for (i=0;i<buffer2[0];i++) {
		switch (buffer2[j++]) {
			case 1:
				dprintf("Block 1 - startup logo\n");
				backup->StartupLogo = malloc(sizeof(GSM_Bitmap));
			        if (backup->StartupLogo == NULL) return GE_MOREMEMORY;
				backup->StartupLogo->Location	= 1;
				backup->StartupLogo->Height	= buffer2[j++];
				backup->StartupLogo->Width	= buffer2[j++];
				Type = GSM_NokiaStartupLogo;
			        switch (backup->StartupLogo->Height) {
					case 65: Type = GSM_Nokia7110StartupLogo; break;
					case 60: Type = GSM_Nokia6210StartupLogo; break;
				}
				PHONE_DecodeBitmap(Type, buffer2+j, backup->StartupLogo);
#ifdef DEBUG
				if (di.dl == DL_TEXTALL) GSM_PrintBitmap(di.df,backup->StartupLogo);
#endif
				j = j + PHONE_GetBitmapSize(Type);
				break;            
			case 2:
#ifdef DEBUG
				dprintf("Block 2 - welcome note \"");
				for (z=0;z<buffer2[j];z++) dprintf("%c",buffer2[j+z+1]);
				dprintf("\"\n");
#endif
				if (backup->StartupLogo == NULL) {
					backup->StartupLogo = malloc(sizeof(GSM_Bitmap));
				        if (backup->StartupLogo == NULL) return GE_MOREMEMORY;
					backup->StartupLogo->Type = GSM_WelcomeNoteText;
					EncodeUnicode(backup->StartupLogo->Text,buffer2+j,buffer2[j]);
				}
				j = j + buffer2[j];
			        break;
			default:
			        dprintf("Unknown block %02x\n",buffer2[j]);
				break;
		}
	}
	return GE_NONE;
}

static GSM_Error LoadLMBPbkEntry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{
	GSM_PhonebookEntry 	pbk;
	int			num;

#ifdef DEBUG
	dprintf("Memory : ");
	switch(buffer[10]) {
		case 2 : dprintf("(internal)\n"); break;
		case 3 : dprintf("(sim)\n");	  break;
		default: dprintf("(unknown)\n");  break;
	}
	dprintf("Location : %i\n",buffer2[0]+buffer2[1]*256);
#endif

	N71_65_DecodePhonebook(&pbk, NULL,NULL,buffer2+4,(buffer[4]+buffer[5]*256)-4);

	pbk.MemoryType=GMT_SM;
	if (buffer[10]==2) pbk.MemoryType=GMT_ME;

	pbk.Location=buffer2[0]+256*buffer2[1];

	num = 0;
	if (buffer[10]==2) {
		while (backup->PhonePhonebook[num] != NULL) num++;
		if (num < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
			backup->PhonePhonebook[num] = malloc(sizeof(GSM_PhonebookEntry));
		        if (backup->PhonePhonebook[num] == NULL) return GE_MOREMEMORY;
			backup->PhonePhonebook[num + 1] = NULL;
		} else {
			dprintf("Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
			return GE_MOREMEMORY;
		}
		*backup->PhonePhonebook[num] = pbk;
	} else {
		while (backup->SIMPhonebook[num] != NULL) num++;
		if (num < GSM_BACKUP_MAX_SIMPHONEBOOK) {
			backup->SIMPhonebook[num] = malloc(sizeof(GSM_PhonebookEntry));
		        if (backup->SIMPhonebook[num] == NULL) return GE_MOREMEMORY;
			backup->SIMPhonebook[num + 1] = NULL;
		} else {
			dprintf("Increase GSM_BACKUP_MAX_SIMPHONEBOOK\n");
			return GE_MOREMEMORY;
		}
		*backup->SIMPhonebook[num] = pbk;
	}
	return GE_NONE;
}

static GSM_Error loadlmb(char *FileName, GSM_Backup *backup)
{
#ifdef DEBUG
	int 		i;
#endif
	unsigned char 	buffer[12], buffer2[1000];
	FILE		*file;
	GSM_Error	error;

	GSM_ClearBackup(backup);

	file = fopen(FileName, "rb");
	if (!file) return(GE_CANTOPENFILE);

	/* Read the header of the file. */
	fread(buffer, 1, 4, file);

	/* while we have something to read */
	while (fread(buffer, 1, 12, file)==12) {
#ifdef DEBUG
		/* Info about block in the file */
		dprintf("Block \"");
		for (i=0;i<4;i++) {dprintf("%c",buffer[i]);}
		dprintf("\" (");
		if (memcmp(buffer, "PBK ",4)==0) {	  dprintf("Phonebook");
		} else if (memcmp(buffer, "PBE2",4)==0) { dprintf("Phonebook entry");
		} else if (memcmp(buffer, "CGR ",4)==0) { dprintf("Caller group");
		} else if (memcmp(buffer, "SPD ",4)==0) { dprintf("Speed dial");
		} else if (memcmp(buffer, "OLG ",4)==0) { dprintf("Operator logo");
		} else if (memcmp(buffer, "WEL ",4)==0) { dprintf("Startup logo and welcome text");
		} else {				  dprintf("unknown - ignored");
		}
		dprintf(") - length %i\n", buffer[4]+buffer[5]*256);
#endif
      		/* reading block data */
		fread(buffer2, 1, buffer[4]+buffer[5]*256, file);

#ifdef DEBUG
		if (memcmp(buffer, "PBK ",4)==0) {
			dprintf("Size of phonebook %i, type %i ",(buffer2[0]+buffer2[1]*256),buffer[8]);
			switch(buffer[8]) {
				case 2 : dprintf("(internal)");break;
				case 3 : dprintf("(sim)")     ;break;
				default: dprintf("(unknown)") ;break;
			}
			dprintf(", length of each position - %i\n",buffer2[2]);
		}
#endif        
		if (memcmp(buffer, "PBE2",4)==0) {
			error = LoadLMBPbkEntry(buffer,buffer2,backup);
			if (error != GE_NONE) {
				fclose(file);
				return error;
			}
		}
		if (memcmp(buffer, "CGR ",4)==0) {
			error = LoadLMBCallerEntry(buffer, buffer2, backup);
			if (error != GE_NONE) {
				fclose(file);
				return error;
			}
		}
		if (memcmp(buffer, "WEL ",4)==0) {
			error = LoadLMBStartupEntry(buffer, buffer2, backup);
			if (error != GE_NONE) {
				fclose(file);
				return error;
			}
		}
        }

	fclose(file);

	return(GE_NONE);
}

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup)
{
	FILE		*file;
	unsigned char	buffer[300];
	GSM_Error	error;

	file = fopen(FileName, "rb");
	if (!file) return(GE_CANTOPENFILE);

	fread(buffer, 1, 9, file); /* Read the header of the file. */
	fclose(file);

	/* Attempt to identify filetype */
	if (memcmp(buffer, "LMB ",4)==0) {
		error=loadlmb(FileName,backup);
	} else {
		error=LoadBackup(FileName,backup);
	}

	return error;
}

void GSM_ClearBackup(GSM_Backup *backup)
{
	backup->PhonePhonebook	[0] = NULL;
	backup->SIMPhonebook	[0] = NULL;
	backup->Calendar	[0] = NULL;
	backup->CallerLogos	[0] = NULL;
	backup->SMSC		[0] = NULL;
	backup->WAPBookmark	[0] = NULL;
	backup->WAPSettings	[0] = NULL;
	backup->Ringtone	[0] = NULL;
	backup->Profiles	[0] = NULL;
	backup->ToDo		[0] = NULL;
	backup->StartupLogo	    = NULL;
	backup->OperatorLogo	    = NULL;

	backup->IMEI		[0] = 0;
	backup->Model		[0] = 0;
	backup->DateTime	[0] = 0;
}

/* ----------------------- SMS Backup functions ---------------------------- */

static void ReadSMSBackupEntry(CFG_Header *file_info, char *section, GSM_SMSMessage *SMS)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	GSM_SetDefaultSMSData(SMS);

	SMS->PDU = SMS_Submit;
	SMS->SMSC.Location = 0;
	sprintf(buffer,"SMSC");
	ReadBackupText(file_info, section, buffer, SMS->SMSC.Number);
	sprintf(buffer,"ReplySMSC");
	SMS->ReplyViaSameSMSC = false;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"True",0)) SMS->ReplyViaSameSMSC = true;
	}
	sprintf(buffer,"Class");
	SMS->Class = -1;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->Class = atoi(readvalue);
	sprintf(buffer,"Sent");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		ReadvCalTime(&SMS->DateTime, readvalue);
		SMS->PDU = SMS_Deliver;
	}
	sprintf(buffer,"RejectDuplicates");
	SMS->RejectDuplicates = false;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"True",0)) SMS->RejectDuplicates = true;
	}
	sprintf(buffer,"ReplaceMessage");
	SMS->ReplaceMessage = 0;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->ReplaceMessage = atoi(readvalue);
	sprintf(buffer,"MessageReference");
	SMS->MessageReference = 0;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->MessageReference = atoi(readvalue);
	sprintf(buffer,"State");
	SMS->State = GSM_UnRead;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Read",0))		SMS->State = GSM_Read;
		else if (mystrncasecmp(readvalue,"Sent",0))	SMS->State = GSM_Sent;
		else if (mystrncasecmp(readvalue,"UnSent",0))	SMS->State = GSM_UnSent;
	}
	sprintf(buffer,"Number");
	ReadBackupText(file_info, section, buffer, SMS->Number);
	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, SMS->Name);
	sprintf(buffer,"Length");
	SMS->Length = 0;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->Length = atoi(readvalue);
	sprintf(buffer,"Coding");
	SMS->Coding = GSM_Coding_Default;
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Unicode",0)) {
			SMS->Coding = GSM_Coding_Unicode;
		} else if (mystrncasecmp(readvalue,"8bit",0)) {
			SMS->Coding = GSM_Coding_8bit;
		}
	}
	ReadLinkedBackupText(file_info, section, "Text", buffer);
	DecodeHexBin (SMS->Text, buffer, strlen(buffer));
	SMS->Text[strlen(buffer)/2]	= 0;
	SMS->Text[strlen(buffer)/2+1] 	= 0;
	sprintf(buffer,"Folder");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->Folder = atoi(readvalue);
	SMS->UDH.Type		= UDH_NoUDH;
	SMS->UDH.Length 	= 0;
	SMS->UDH.ID	  	= -1;
	SMS->UDH.PartNumber	= -1;
	SMS->UDH.AllParts	= -1;
	sprintf(buffer,"UDH");
	readvalue = CFG_Get(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		DecodeHexBin (SMS->UDH.Text, readvalue, strlen(readvalue));
		SMS->UDH.Length = strlen(readvalue)/2;
		GSM_DecodeUDHHeader(&SMS->UDH);
	}
}

static GSM_Error GSM_ReadSMSBackupTextFile(char *FileName, GSM_SMS_Backup *backup)
{
	CFG_Header	*file_info, *h;
	char		*readvalue;
	int		num;

	backup->SMS[0] = NULL;

	file_info = CFG_ReadFile(FileName, false);

	num = 0;
        for (h = file_info; h != NULL; h = h->next) {
                if (mystrncasecmp("SMSBackup", h->section, 9)) {
			readvalue = CFG_Get(file_info, h->section, "Number", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_SMS) {
				backup->SMS[num] = malloc(sizeof(GSM_SMSMessage));
			        if (backup->SMS[num] == NULL) return GE_MOREMEMORY;
				backup->SMS[num + 1] = NULL;
			} else {
				dprintf("Increase GSM_BACKUP_MAX_SMS\n");
				return GE_MOREMEMORY;
			}
			backup->SMS[num]->Location = num + 1;
			ReadSMSBackupEntry(file_info, h->section, backup->SMS[num]);
			num++;
		}
        }
	return GE_NONE;
}

GSM_Error GSM_ReadSMSBackupFile(char *FileName, GSM_SMS_Backup *backup)
{
	FILE		*file;

	backup->SMS[0] = NULL;

	file = fopen(FileName, "rb");
	if (!file) return(GE_CANTOPENFILE);

	fclose(file);

	return GSM_ReadSMSBackupTextFile(FileName, backup);
}

GSM_Error SaveSMSBackupTextFile(FILE *file, GSM_SMS_Backup *backup)
{
	int 		i,w,current;
	unsigned char 	buffer[10000];

	i=0;
	while (backup->SMS[i]!=NULL) {
		fprintf(file,"[SMSBackup%03i]\n",i);
		switch (backup->SMS[i]->Coding) {
			case GSM_Coding_Unicode:
			case GSM_Coding_Default:
				sprintf(buffer,"%s",DecodeUnicodeString(backup->SMS[i]->Text));
				fprintf(file,"#");
				current = 0;
				for (w=0;w<(int)(strlen(buffer));w++) {
					switch (buffer[w]) {
						case 10:
							fprintf(file,"\n#");
							current = 0;
							break;
						case 13:
							break;
						default:
							if (isprint(buffer[w])) {
								fprintf(file,"%c",buffer[w]);
								current ++;
							}
							if (current == 75) {
								fprintf(file,"\n#");
								current = 0;
							}
					}
				}
				fprintf(file,"\n");	
				break;
			default:
				break;
		}
		if (backup->SMS[i]->PDU == SMS_Deliver) {
			SaveBackupText(file, "SMSC", backup->SMS[i]->SMSC.Number);
			if (backup->SMS[i]->ReplyViaSameSMSC) fprintf(file,"SMSCReply = true\n");
			fprintf(file,"Sent");
			SaveVCalTime(file,&backup->SMS[i]->DateTime);
		}
		fprintf(file,"State = ");
		switch (backup->SMS[i]->State) {
			case GSM_UnRead	: fprintf(file,"UnRead\n");	break;
			case GSM_Read	: fprintf(file,"Read\n");	break;
			case GSM_Sent	: fprintf(file,"Sent\n");	break;
			case GSM_UnSent	: fprintf(file,"UnSent\n");	break;
		}
		SaveBackupText(file, "Number", backup->SMS[i]->Number);
		SaveBackupText(file, "Name", backup->SMS[i]->Name);
		if (backup->SMS[i]->UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer,backup->SMS[i]->UDH.Text,backup->SMS[i]->UDH.Length);
			fprintf(file,"UDH = %s\n",buffer);
		}
		switch (backup->SMS[i]->Coding) {
			case GSM_Coding_Unicode:
			case GSM_Coding_Default:
				EncodeHexBin(buffer,backup->SMS[i]->Text,backup->SMS[i]->Length*2);
				break;
			default:
				EncodeHexBin(buffer,backup->SMS[i]->Text,backup->SMS[i]->Length);
				break;
		}
		SaveLinkedBackupText(file, "Text", buffer);
		switch (backup->SMS[i]->Coding) {
			case GSM_Coding_Unicode	: fprintf(file,"Coding = Unicode\n"); 	break;
			case GSM_Coding_Default	: fprintf(file,"Coding = Default\n"); 	break;
			case GSM_Coding_8bit	: fprintf(file,"Coding = 8bit\n"); 	break;
		}
		fprintf(file,"Folder = %i\n",backup->SMS[i]->Folder);
		fprintf(file,"Length = %i\n",backup->SMS[i]->Length);
		fprintf(file,"Class = %i\n",backup->SMS[i]->Class);
		fprintf(file,"ReplySMSC = ");
		if (backup->SMS[i]->ReplyViaSameSMSC) fprintf(file,"True\n"); else fprintf(file,"False\n");
		fprintf(file,"RejectDuplicates = ");
		if (backup->SMS[i]->RejectDuplicates) fprintf(file,"True\n"); else fprintf(file,"False\n");
		fprintf(file,"ReplaceMessage = %i\n",backup->SMS[i]->ReplaceMessage);
		fprintf(file,"MessageReference = %i\n",backup->SMS[i]->MessageReference);
		fprintf(file,"\n");
		i++;
	}
	return GE_NONE;
}

GSM_Error GSM_SaveSMSBackupFile(char *FileName, GSM_SMS_Backup *backup)
{
	FILE *file;
  
	file = fopen(FileName, "wb");      
	if (!file) return(GE_CANTOPENFILE);

	SaveSMSBackupTextFile(file,backup);

	fclose(file);
   
	return GE_NONE;
}

#endif
