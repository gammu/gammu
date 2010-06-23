/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

/* Prototypes */

int WINAPI myinitialize (char *port, char *model, int connectiontype);
int WINAPI myterminate ();

int WINAPI mygetnetworkname(char *NetworkName, char *NetworkCode);
int WINAPI mygetnetworkcode(char *NetworkName, char *NetworkCode);
int WINAPI mygetnetworkinfo (GSM_NetworkInfo *NetworkInfo);
int WINAPI mygetnetworkdata(int number, char *Name, char *Code);

int WINAPI mynetmonitor(int mode, char *Screen);

int WINAPI mygetmemorylocation(GSM_PhonebookEntry *entry);
int WINAPI mywritephonebooklocation(GSM_PhonebookEntry *entry);
int WINAPI mygetmemorystatus(GSM_MemoryStatus *Status);

int WINAPI myreadringtonefile(char *FileName, GSM_Ringtone *ringtone);
int WINAPI mysetringtone(GSM_Ringtone *ringtone, int *notes);
int WINAPI myplaynote(GSM_RingtoneNote note);
int WINAPI myplayringtone(GSM_Ringtone *ringtone);
int WINAPI mygettempo(int beats);
int WINAPI myplaytone(int Herz, u8 Volume);
int WINAPI myreadbinringtonefile(char *FileName, GSM_BinRingtone *ringtone);
int WINAPI mysetbinringtone(GSM_BinRingtone *ringtone);
int WINAPI mygetphoneringtone(GSM_BinRingtone *ringtone,GSM_Ringtone *SMringtone);
int WINAPI mysaveringtonefile(char *FileName, GSM_Ringtone *ringtone);
int WINAPI mysavebinringtonefile(char *FileName, GSM_BinRingtone *ringtone);

int WINAPI myreadbitmapfile(char *FileName, GSM_Bitmap *bitmap);
int WINAPI mysavebitmapfile(char *FileName, GSM_Bitmap *bitmap);
int WINAPI mysetpointbitmap(GSM_Bitmap *bmp, int x, int y);
int WINAPI myclearpointbitmap(GSM_Bitmap *bmp, int x, int y);
int WINAPI myclearbitmap(GSM_Bitmap *bmp);
int WINAPI myresizebitmap (GSM_Bitmap *bitmap, GSM_Bitmap_Types target);
int WINAPI mysetbitmap(GSM_Bitmap *Bitmap);
int WINAPI mygetbitmap(GSM_Bitmap *Bitmap);
bool WINAPI myispointbitmap(GSM_Bitmap *bitmap,int x, int y);

int WINAPI mygetimei(char *IMEI);
int WINAPI mygetrevision(char *revision);
int WINAPI mygetmodel(char *model);
int WINAPI mygetmodelnumber(char *model,char *model2);

int WINAPI mygetdllversion(char *version);

int WINAPI mygetspeeddial(GSM_SpeedDial *entry);

int WINAPI myreset(unsigned char type);

int WINAPI mygetcalendarnote(GSM_CalendarNote *note);
int WINAPI mywritecalendarnote(GSM_CalendarNote *note);
int WINAPI mydeletecalendarnote(GSM_CalendarNote *note);

int WINAPI mygetsmscenter(GSM_MessageCenter *center);
int WINAPI mysetsmscenter(GSM_MessageCenter *center);

int WINAPI mygetalarm(int alarm_number, GSM_DateTime *alarm);

int WINAPI mygetprofile(GSM_Profile *profile);

int WINAPI mygetsimlock(GSM_AllSimlocks *simlocks);

int WINAPI mygetsmsstatus(GSM_SMSStatus *status);
int WINAPI mygetsmsfolders(GSM_SMSFolders *folders);
int WINAPI mygetsmsmessage(GSM_SMSMessage *sms);
int WINAPI mydeletesmsmessage(GSM_SMSMessage *sms);
int WINAPI mysavesmsmessage(GSM_SMSMessage *sms);
int WINAPI mysendsmsmessage(GSM_SMSMessage *sms);
int WINAPI mymakemultipartsms2(GSM_MultiSMSMessage *SMS,
    u8 *MessageBuffer,int MessageLength, GSM_UDH UDHType, GSM_Coding_Type Coding);

int WINAPI mysaveringtonetosms(GSM_MultiSMSMessage *SMS,
                          GSM_Ringtone *ringtone, bool profilestyle);
int WINAPI mysavebitmaptosms(GSM_MultiSMSMessage *SMS, GSM_Bitmap *bitmap,
                        bool ScreenSaver, bool UnicodeText);
int WINAPI mysavecalendarnotetosms(GSM_MultiSMSMessage *SMS,
                              GSM_CalendarNote *note);
int WINAPI mysavephonebookentrytosms(GSM_MultiSMSMessage *SMS,
                                GSM_PhonebookEntry *entry, int version);
int WINAPI mysavewapbookmarktosms(GSM_MultiSMSMessage *SMS,
                                  GSM_WAPBookmark *bookmark);
int WINAPI mysavewapsettingstosms(GSM_MultiSMSMessage *SMS,
                                  GSM_WAPSettings *settings);

int WINAPI myreadconfig(char *model, char *port, char *initlength,
               char *connection, char *synchronizetime);

int WINAPI mysetlocale(char *locale);

int WINAPI mygetsecuritycode(GSM_SecurityCode *SecurityCode);

int WINAPI mygetmodelfeature (featnum_index num);

int WINAPI mygetwapbookmark(GSM_WAPBookmark *bookmark);
int WINAPI mysetwapbookmark(GSM_WAPBookmark *bookmark);

int WINAPI mygetwapsettings(GSM_WAPSettings *settings);

int WINAPI mycalldivert(GSM_CallDivert *divert);

int WINAPI mysavebackupfile(char *FileName, GSM_Backup *backup);
int WINAPI myreadbackupfile(char *FileName, GSM_Backup *backup);

int WINAPI mydialvoice(char *Number);
