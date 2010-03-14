/*
  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#ifndef VC6
  #if defined(__svr4__) || defined(__FreeBSD__)
  #  include <strings.h>	/* for bzero */
  #endif
#endif

#ifdef WIN32

  #include <windows.h>
  #define sleep(x) Sleep((x) * 1000)
  #define usleep(x) Sleep(((x) < 1000) ? 1 : ((x) / 1000))
  #include "getopt.h"

  #define DEV_CONSOLE "CON:"

#else

  #include <unistd.h>
  #include <termios.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/time.h>
  #include <getopt.h>

  #define DEV_CONSOLE "/dev/tty"
#endif

#include "misc.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "gsm-networks.h"
#include "files/cfgreader.h"
#include "gnokiiapi.h"
#include "files/gsm-filetypes.h"
#include "gsm-ringtones.h"
#include "gsm-bitmaps.h"
#include "gsm-sms.h"
#include "newmodules/n6110.h"

#include "mversion.h"

#ifdef USE_NLS
  #include <locale.h>
#endif

extern GSM_Network GSM_Networks[];

int WINAPI myinitialize (char *port, char *model, int connectiontype)
{
  return GSM_Initialise(model, port, "30", connectiontype, NULL, "no");
}

int WINAPI mygetnetworkinfo (GSM_NetworkInfo *NetworkInfo)
{
  return GSM->GetNetworkInfo(NetworkInfo);
}

int WINAPI myterminate ()
{
  GSM->Terminate();
  return 0;
}

int WINAPI mygetnetworkname(char *NetworkName, char *NetworkCode)
{
  strcpy(NetworkName,GSM_GetNetworkName(NetworkCode));
  return 0;
}

int WINAPI mynetmonitor(int mode, char *Screen)
{
  return GSM->NetMonitor(mode, Screen);
}

int WINAPI mygetmemorylocation(GSM_PhonebookEntry *entry)
{
  return GSM->GetMemoryLocation(entry);
}

int WINAPI mywritephonebooklocation(GSM_PhonebookEntry *entry)
{
  return GSM->WritePhonebookLocation(entry);
}

int WINAPI mygetmemorystatus(GSM_MemoryStatus *Status)
{
  return GSM->GetMemoryStatus(Status);
}

int WINAPI myreadringtonefile(char *FileName, GSM_Ringtone *ringtone)
{
  return GSM_ReadRingtoneFile(FileName, ringtone);
}

int WINAPI myplayringtone(GSM_Ringtone *ringtone)
{
  GSM_PlayRingtone(ringtone);
  return 0;
}

int WINAPI myplaynote(GSM_RingtoneNote note)
{
  GSM_PlayOneNote(note);
  return 0;
}

int WINAPI myreadbitmapfile(char *FileName, GSM_Bitmap *bitmap)
{
  return GSM_ReadBitmapFile(FileName, bitmap);
}

int WINAPI mysavebitmapfile(char *FileName, GSM_Bitmap *bitmap)
{
  return GSM_SaveBitmapFile(FileName, bitmap);
}

bool WINAPI myispointbitmap(GSM_Bitmap *bitmap,int x, int y)
{
  return GSM_IsPointBitmap(bitmap,x,y);
}

int WINAPI mygetimei(char *IMEI)
{
  return GSM->GetIMEI(IMEI);
}

int WINAPI mygetrevision(char *revision)
{
  return GSM->GetRevision(revision);
}

int WINAPI mygetmodel(char *model)
{
  return GSM->GetModel(model);
}

int WINAPI mygetmodelnumber(char *model,char *model2)
{
  strcpy(model2,GetModelName(model));
  return 0;
}

int WINAPI mysetringtone(GSM_Ringtone *ringtone, int *notes)
{
  return GSM->SetRingtone(ringtone,notes);
}

int WINAPI mygetdllversion(char *version)
{
  strcpy(version,VERSION);
  return 0;
}

int WINAPI mysetpointbitmap(GSM_Bitmap *bmp, int x, int y)
{
  GSM_SetPointBitmap(bmp,x,y);
  return 0;
}

int WINAPI myclearpointbitmap(GSM_Bitmap *bmp, int x, int y)
{
  GSM_ClearPointBitmap(bmp,x,y);
  return 0;
}

int WINAPI myclearbitmap(GSM_Bitmap *bmp)
{
  GSM_ClearBitmap(bmp);
  return 0;
}

int WINAPI myresizebitmap (GSM_Bitmap *bitmap, GSM_Bitmap_Types target)
{
  GSM_ResizeBitmap(bitmap,target);
  return 0;
}

int WINAPI mysetbitmap(GSM_Bitmap *Bitmap)
{
  return GSM->SetBitmap(Bitmap);
}

int WINAPI mygetbitmap(GSM_Bitmap *Bitmap)
{
  return GSM->GetBitmap(Bitmap);
}

int WINAPI mygetnetworkdata(int number, char *Name, char *Code)
{
  strcpy(Name,GSM_Networks[number].Name);
  strcpy(Code,GSM_Networks[number].Code);
  return 0;
}

int WINAPI mygetnetworkcode(char *NetworkName, char *NetworkCode)
{
  strcpy(NetworkCode,GSM_GetNetworkCode(NetworkName));
  return 0;
}

int WINAPI mygetspeeddial(GSM_SpeedDial *entry)
{
  return GSM->GetSpeedDial(entry);
}

int WINAPI myreset(unsigned char type)
{
  return GSM->Reset(type);
}

int WINAPI mygetcalendarnote(GSM_CalendarNote *note)
{
  return GSM->GetCalendarNote(note);
}

int WINAPI mygetsmscenter(GSM_MessageCenter *center)
{
  return GSM->GetSMSCenter(center);
}

int WINAPI mysetsmscenter(GSM_MessageCenter *center)
{
  return GSM->SetSMSCenter(center);
}

int WINAPI mygetalarm(int alarm_number, GSM_DateTime *alarm)
{
  return GSM->GetAlarm(alarm_number,alarm);
}

int WINAPI mygetprofile(GSM_Profile *profile)
{
  return GSM->GetProfile(profile);
}

int WINAPI mygetsimlock(GSM_AllSimlocks *simlocks)
{
  return GSM->SimlockInfo(simlocks);
}

int WINAPI mygetsmsstatus(GSM_SMSStatus *status)
{
  return GSM->GetSMSStatus(status);
}

int WINAPI mygetsmsfolders(GSM_SMSFolders *folders)
{
  return GSM->GetSMSFolders(folders);
}

int WINAPI mygetsmsmessage(GSM_SMSMessage *sms)
{
  return GSM->GetSMSMessage(sms);
}

int WINAPI mywritecalendarnote(GSM_CalendarNote *note)
{
  return GSM->WriteCalendarNote(note);
}

int WINAPI mydeletecalendarnote(GSM_CalendarNote *note)
{
  return GSM->DeleteCalendarNote(note);
}

int WINAPI mysavesmsmessage(GSM_SMSMessage *sms)
{
  return GSM->SaveSMSMessage(sms);
}

int WINAPI mysendsmsmessage(GSM_SMSMessage *sms)
{
  return GSM->SendSMSMessage(sms);
}

int WINAPI mydeletesmsmessage(GSM_SMSMessage *sms)
{
  return GSM->DeleteSMSMessage(sms);
}

int WINAPI mygettempo(int beats)
{
  GSM_GetTempo(beats);
  return 0;
}

int WINAPI myplaytone(int Herz, u8 Volume)
{
  return GSM->PlayTone(Herz,Volume);
}

int WINAPI myreadconfig(char *model, char *port, char *initlength,
               char *connection, char *synchronizetime)
{
  char *model0;
  char *port0;
  char *initlength0;
  char *connection0;
  char *bindir0;
  char *synchronizetime0;

  CFG_ReadConfig(&model0, &port0, &initlength0,&connection0,&bindir0,&synchronizetime0,false);

  strcpy(model,model0);
  strcpy(port,port0);
  strcpy(initlength,initlength0);
  *connection=GetConnectionTypeFromString(connection0);
  strcpy(synchronizetime,synchronizetime0);  

  return 0;
}

int WINAPI mysetlocale(char *locale)
{
  setlocale(LC_ALL, locale);
  return 0;
}

int WINAPI mymakemultipartsms2(GSM_MultiSMSMessage *SMS,
    u8 *MessageBuffer,int MessageLength, GSM_UDH UDHType, GSM_Coding_Type Coding)
{
  GSM_MakeMultiPartSMS2(SMS,MessageBuffer,MessageLength,UDHType,Coding);
  return 0;
}

int WINAPI mysaveringtonetosms(GSM_MultiSMSMessage *SMS,
                          GSM_Ringtone *ringtone, bool profilestyle)
{
  return GSM_SaveRingtoneToSMS(SMS,ringtone,profilestyle);
}

int WINAPI mysavewapbookmarktosms(GSM_MultiSMSMessage *SMS,
                                  GSM_WAPBookmark *bookmark)
{
  return GSM_SaveWAPBookmarkToSMS(SMS,bookmark);
}

int WINAPI mysavewapsettingstosms(GSM_MultiSMSMessage *SMS,
                                  GSM_WAPSettings *settings)
{
  return GSM_SaveWAPSettingsToSMS(SMS,settings);
}

int WINAPI mysavebitmaptosms(GSM_MultiSMSMessage *SMS, GSM_Bitmap *bitmap,
                        bool ScreenSaver, bool UnicodeText)
{
  return GSM_SaveBitmapToSMS(SMS,bitmap,ScreenSaver,UnicodeText);
}

int WINAPI mysavecalendarnotetosms(GSM_MultiSMSMessage *SMS,
                              GSM_CalendarNote *note)
{
  return GSM_SaveCalendarNoteToSMS(SMS,note);
}

int WINAPI mysavephonebookentrytosms(GSM_MultiSMSMessage *SMS,
                                GSM_PhonebookEntry *entry, int version)
{
  return GSM_SavePhonebookEntryToSMS(SMS,entry,version);
}

int WINAPI mygetsecuritycode(GSM_SecurityCode *SecurityCode)
{
  return GSM->GetSecurityCode(SecurityCode);
}

int WINAPI myreadbinringtonefile(char *FileName, GSM_BinRingtone *ringtone)
{
  return GSM_ReadBinRingtoneFile(FileName,ringtone);
}

int WINAPI mysetbinringtone(GSM_BinRingtone *ringtone)
{
  return GSM->SetBinRingtone(ringtone);
}

int WINAPI mygetmodelfeature (featnum_index num)
{
  return GetModelFeature (num);
}

int WINAPI mygetwapbookmark(GSM_WAPBookmark *bookmark)
{
  return GSM->GetWAPBookmark(bookmark);
}

int WINAPI mygetwapsettings(GSM_WAPSettings *settings)
{
  return GSM->GetWAPSettings(settings);
}

int WINAPI mycalldivert(GSM_CallDivert *divert)
{
  return GSM->CallDivert(divert);
}

int WINAPI mysetwapbookmark(GSM_WAPBookmark *bookmark)
{
  return GSM->SetWAPBookmark(bookmark);
}

int WINAPI mysavebackupfile(char *FileName, GSM_Backup *backup)
{
  return GSM_SaveBackupFile(FileName,backup);
}

int WINAPI myreadbackupfile(char *FileName, GSM_Backup *backup)
{
  return GSM_ReadBackupFile(FileName,backup);
}

int WINAPI mygetphoneringtone(GSM_BinRingtone *ringtone,GSM_Ringtone *SMringtone)
{
  return GSM_GetPhoneRingtone(ringtone,SMringtone);
}

int WINAPI mysaveringtonefile(char *FileName, GSM_Ringtone *ringtone)
{
  return GSM_SaveRingtoneFile(FileName,ringtone);
}

int WINAPI mysavebinringtonefile(char *FileName, GSM_BinRingtone *ringtone)
{
  return GSM_SaveBinRingtoneFile(FileName,ringtone);
}

int WINAPI mydialvoice(char *Number)
{
  return GSM->DialVoice(Number);
}

