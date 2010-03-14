// This is header file for gnokiiapi.dll
// Latest version: http://marcin-wiacek.topnet.pl

unit GnokiiAPI;

interface

uses Classes,Forms,SysUtils;

const
  GSM_General = 10;
  GSM_General2= 11;
  GSM_Mobile  = 3;
  GSM_Work    = 6;
  GSM_Fax     = 4;
  GSM_Home    = 2;

  GSM_Date       = 19;
  GSM_Number     = 11;
  GSM_Note       = 10;
  GSM_Postal     = 9;
  GSM_Email      = 8;

const
  GMT_ME=0;
  GMT_SM=1;
  GMT_FD=2;
  GMT_ON=3;
  GMT_EN=4;
  GMT_DC=5;
  GMT_RC=6;
  GMT_MC=7;
  GMT_LD=8;
  GMT_MT=9;
  GMT_TA=10;
  GMT_CB=11;
  GMT_CG=12;

  GCT_FBUS=0;
  GCT_MBUS=3;

  GSS_SENTREAD=1;
  GSS_NOTSENTREAD=3;

const
  GSM_None=0;
  GSM_StartupLogo=1;      //size 84*48
  GSM_OperatorLogo=2;     //size 72*14
  GSM_CallerLogo=3;       //size 72*14
  GSM_PictureImage=4;     //size 72*28
  GSM_7110OperatorLogo=5; //size 78*21
  GSM_7110StartupLogo=6;  //size 96*65
  GSM_6210StartupLogo=7;  //size 96*60
  GSM_WelcomeNoteText=8;
  GSM_DealerNoteText=9;

const
  GSM_SoftReset=3;

const
  GCN_REMINDER=1;
  GCN_CALL=2;
  GCN_MEETING=3;
  GCN_BIRTHDAY=4;

const
  GSMF_Text   = 0;
  GSMF_Fax    = 34;
  GSMF_Voice  = 36;
  GSMF_ERMES  = 37;
  GSMF_Paging = 38;
  GSMF_UCI    = 45;
  GSMF_Email  = 50;
  GSMF_X400   = 49;

  GSMV_1_Hour   = 11;
  GSMV_6_Hours  = 71;
  GSMV_24_Hours = 167;
  GSMV_72_Hours = 169;
  GSMV_1_Week   = 173;
  GSMV_Max_Time = 255;

  GST_SMS=0;
  GST_DR=1;

  GSM_Coding_Unicode = 1;
  GSM_Coding_Default = 2;

  GSCT_SecurityCode = 1;

  GSM_NoUDH=0;

const
  FN_CALENDAR=0;
  FN_CALLERGROUPS=2;
  FN_PHONEBOOK=3;
  FN_SMS=11;
  FN_STARTUP=13;
  FN_PROFILES=14;
  FN_RINGTONES=15;
  FN_WAP=16;
  FN_RINGNUMBER=17;

  F_CAL61=1;   //Calendar in 6110 style - 4 types,...
  F_CAL33=2;   //Calendar in 3310 style - 10 reminders, Unicode, 3 coding types
  F_CAL71=3;   //Calendar in 7110 style - 4 types, recurrance, etc.
  F_PBK33SIM=4;//Phonebook in Unicode(3310 style).Name and number.No internal
  F_PBK33INT=5;//Phonebook in Unicode(3310 style).Name and number.Internal available
  F_PBK71INT=6;//Extended phonebook-multiple numbers,Unicode(7110 style).Internal available
  F_PBK61INT=7;//Name and number,no Unicode(6110 style).Internal available
  F_SMS  =10;  //SMS with Inbox, Outbox
  F_SMS71=11;  //SMS with SMS folders like in 7110
  F_STANIM=21; //Startup logo animated or static
  F_STA=22;    //Startup logo static
  F_STA62=23;  //Startup logo static size 6210
  F_STA71=24;  //Startup logo static size 7110
  F_PROF61=25; //Profiles numbered like in N6110
  F_PROF51=26; //Profiles numbered like in N5110
  F_PROF33=27; //Profiles numbered like in N3310.Unicode names
  F_RING_SM=28;//Binary ringtone in Smart Messaging
  F_RINGBIN=29;//Binary ringtone in binary format

const
  GE_SMSTOOLONG=22;

const
  PROFILE_MESSAGE_NOTONE=0;
  PROFILE_MESSAGE_STANDARD=1;
  PROFILE_MESSAGE_SPECIAL=2;
  PROFILE_MESSAGE_BEEPONCE=3;
  PROFILE_MESSAGE_ASCENDING=4;

  PROFILE_WARNING_OFF=255;
  PROFILE_WARNING_ON =4;

  PROFILE_CALLALERT_RINGING=1;
  PROFILE_CALLALERT_BEEPONCE=2;
  PROFILE_CALLALERT_OFF=4;
  PROFILE_CALLALERT_RINGONCE=5;
  PROFILE_CALLALERT_ASCENDING=6;
  PROFILE_CALLALERT_CALLERGROUPS=7;

  PROFILE_KEYPAD_OFF=255;
  PROFILE_KEYPAD_LEVEL1=0;
  PROFILE_KEYPAD_LEVEL2=1;
  PROFILE_KEYPAD_LEVEL3=2;

  PROFILE_VOLUME_LEVEL1=6;
  PROFILE_VOLUME_LEVEL2=7;
  PROFILE_VOLUME_LEVEL3=8;
  PROFILE_VOLUME_LEVEL4=9;
  PROFILE_VOLUME_LEVEL5=10;

  PROFILE_CALLERGROUPS_ALL=255;
  PROFILE_CALLERGROUPS_FAMILY=1;
  PROFILE_CALLERGROUPS_VIP=2;
  PROFILE_CALLERGROUPS_FRIENDS=4;
  PROFILE_CALLERGROUPS_COLLEAGUES=8;
  PROFILE_CALLERGROUPS_OTHERS=16;

  PROFILE_VIBRATION_OFF=0;
  PROFILE_VIBRATION_ON=1;
  PROFILE_VIBRATION_FIRST=2;

type
  GSM_NetworkInfo= record
      NetworkCode: array[1..10] of char; // GSM network code
      CellID: array[1..10] of char;      // CellID
      LAC: array[1..10] of char;         // LAC
  end;
  PGSM_NetworkInfo=^GSM_NetworkInfo;

  GSM_DateTime=record
    IsSet:integer;
    Year:integer;
    Month:integer;
    Day:integer;
    Hour:integer;
    Minute:integer;
    Second:integer;
    Timezone:integer;
  end;
  PGSM_DateTime=^GSM_DateTime;

  GSM_SubPhonebookEntry= record
    EntryType:integer;
    NumberType:integer;
    Number:array[1..61] of char;
    Date:GSM_DateTime;
    BlockNumber:integer;
  end;

  GSM_PhonebookEntry= record
    Empty:integer;
    Name:array[1..51] of char;
    Number:array[1..49] of char;
    MemoryType:integer;
    Group:integer;
    Location:integer;
    SubEntries:array[1..8] of GSM_SubPhonebookEntry;
    SubEntriesCount:integer;
  end;
  PGSM_PhonebookEntry=^GSM_PhonebookEntry;

  GSM_MemoryStatus=record
    MemoryType:integer;
    Used:integer;
    Free:integer;
  end;
  PGSM_memoryStatus=^GSM_MemoryStatus;

  GSM_RingtoneNote=record
    duration:byte;
    note:byte;
    tempo:integer;
    style:byte;
  end;

  GSM_Ringtone=record
    name:array[1..20] of char;
    Loop:byte;
    NrNotes:byte;
    notes:array[1..256] of GSM_RingtoneNote;
    allnotesscale:integer;
    location:integer;
  end;
  PGSM_Ringtone=^GSM_Ringtone;

  GSM_BinRingtone=record
    name:array[1..20] of char;
    frame:array[1..1000] of char;
    length:integer;
    location:integer;
  end;
  PGSM_BinRingtone=^GSM_BinRingtone;

  GSM_Bitmap=record
    heigh:byte;
    width:byte;
    size:word;
    type2:integer;
    netcode:array[1..7] of char;
    text:array[1..256] of char;
    bitmap:array[1..864] of char;
    number:byte;
    ringtone:byte;
    enabled:integer;
    Sender:array[1..41] of char;
  end;
  PGSM_Bitmap=^GSM_Bitmap;

  GSM_SpeedDial=record
    Number:integer;
    MemoryType:integer;
    Location:integer;
  end;
  PGSM_SpeedDial=^GSM_SpeedDial;

  GSM_CalendarNote=record
    Location:integer;
    Type2:integer;
    Time:GSM_DateTime;
    Alarm:GSM_DateTime;
    Text:array[1..59] of char;
    Phone:array[1..37] of char;

    Recurrance:integer;
    YearOfBirth:integer;
    AlarmType:byte;
    ReadNotesInfo:integer;
  end;
  PGSM_CalendarNote=^GSM_CalendarNote;

  GSM_MessageCenter=record
    No:integer;
    Name:array[1..20] of char;
    Format:integer;
    Validity:integer;
    Number:array[1..40] of char;
    DefaultRecipient:array[1..40] of char;
  end;
  PGSM_MessageCenter=^GSM_MessageCenter;

  GSM_Profile=record
    Number:integer;
    Name:array[1..40] of char;
    DefaultName:integer;
    KeypadTone:integer;
    Lights:integer;
    CallAlert:integer;
    Ringtone:integer;
    Volume:integer;
    MessageTone:integer;
    WarningTone:integer;
    Vibration:integer;
    CallerGroups:integer;
    ScreenSaver:integer;
    AutomaticAnswer:integer;
  end;
  PGSM_Profile=^GSM_Profile;

  GSM_OneSimlock=record
    data:array[1..11] of char;
    enabled:integer;
    counter:integer;
    factory:integer;
  end;

  GSM_AllSimlocks=record
    simlocks:array[1..4] of GSM_OneSimlock;
  end;
  PGSM_AllSimlocks=^GSM_AllSimlocks;

  GSM_OneSMSFolder=record
    Name:array[1..15] of char;
    locations:array[1..160] of byte;
    number:byte;
  end;

  GSM_SMSFolders=record
    Folder: array[1..24] of GSM_OneSMSFolder;
    FoldersID: array[1..24] of byte;
    number:byte;
  end;
  PGSM_SMSFolders=^GSM_SMSFolders;

  GSM_SMSMessage=record
    Coding:integer;
    Time:GSM_DateTime;
    SMSCTime:GSM_DateTime;
    Length:integer;
    Validity:integer;
    UDHType:integer;
    UDH:array[1..140] of char;
    MessageText:array[1..161] of char;
    MessageCenter:GSM_MessageCenter;
    Sender:array[1..41] of char;
    Destination:array[1..41] of char;
    MessageNumber:integer;
    MemoryType:integer;
    Type2:integer;
    Status:integer;
    Class2:integer;
    Compression:integer;
    Location:integer;
    ReplyViaSameSMSC:integer;
    folder:integer;
    SMSData:integer;
    Name:array[1..26] of char;
  end;
  PGSM_SMSMessage=^GSM_SMSMessage;

  GSM_MultiSMSMessage=record
    number:integer;
    SMS:array[1..6] of GSM_SMSMessage;
  end;
  PGSM_MultiSMSMessage=^GSM_MultiSMSMessage;

  GSM_SMS2Foldertable=record
    smsnum:integer;
    folder:shortint;
    location:integer;
  end;

  GSM_SMSStatus=record
    UnRead:integer;
    Number:integer;
    foldertable:array[1..255] of GSM_SMS2FolderTable;
  end;
  PGSM_SMSStatus=^GSM_SMSStatus;

  GSM_SecurityCode=record
    Type2:integer;
    Code:array[1..10] of char;
  end;
  PGSM_SecurityCode=^GSM_SecurityCode;

  GSM_WAPBookmark=record
    adress:array[1..256] of char;
    title:array[1..51] of char;
    location:Word;
  end;
  PGSM_WAPBookmark=^GSM_WAPBookmark;

  GSM_WAPSettings=record
    location:byte;
    homepage:array[1..101] of char;
    title:array[1..21] of char;
    bearer:integer;
    issecurity:integer;
    iscontinuous:integer;

    //for data bearer
    ipaddress:array[1..21] of char;
    dialup:array[1..21] of char;
    user:array[1..51] of char;
    password:array[1..51] of char;
    isspeed14400:integer;
    isISDNcall:integer;
    isnormalauthentication:integer;

    //for sms bearer
    server:array[1..22] of char;

    //for sms or ussd bearer
    service:array[1..21] of char;
    isIP:integer;

    //for ussd bearer
    code:array[1..11] of char;
  end;
  PGSM_WAPSettings=^GSM_WAPSettings;

const
  WAPSETTINGS_BEARER_SMS=0;
  WAPSETTINGS_BEARER_DATA=1;
  WAPSETTINGS_BEARER_USSD=2;

const
  GSM_CDV_Busy       = 1; // Divert when busy
  GSM_CDV_NoAnswer   = 2; // Divert when not answered
  GSM_CDV_OutOfReach = 3; // Divert when phone off or no coverage
  GSM_CDV_AllTypes   = 4; // Divert all calls without ringing

  GSM_CDV_VoiceCalls = 1;
  GSM_CDV_FaxCalls   = 2;
  GSM_CDV_DataCalls  = 3;
  GSM_CDV_AllCalls   = 4;

  GSM_CDV_Disable  = 0;
  GSM_CDV_Enable   = 1;
  GSM_CDV_Query    = 2; // Is concrete divert enabled ?
  GSM_CDV_Register = 3; // Sets divert
  GSM_CDV_Erasure  = 4; // Erase concrete divert

type
  GSM_CallDivert=record
    DType:integer;
    CType:integer;
    Operation:integer;
    Number:array[1..41] of char;
    timeout:longword;
    enabled:integer;
  end;
  PGSM_CallDivert=^GSM_CallDivert;

  GSM_Backup=record
    SIMPhonebookUsed:integer;
    SIMPhonebookSize:integer;
    SIMPhonebook:array[1..250] of GSM_PhonebookEntry;

    PhonePhonebookUsed:integer;
    PhonePhonebookSize:integer;
    PhonePhonebook:array[1..500] of GSM_PhonebookEntry;

    CallerAvailable:integer;
    CallerGroups:array[1..5] of GSM_Bitmap;

    SpeedAvailable:integer;
    SpeedDials:array[1..8] of GSM_SpeedDial;

    OperatorLogoAvailable:integer;
    OperatorLogo:GSM_Bitmap;

    StartupLogoAvailable:integer;
    StartupLogo:GSM_Bitmap;

    StartupText:GSM_Bitmap;
  end;
  PGSM_Backup=^GSM_Backup;

var
  GSMPhoneConnected: boolean=false;
  GSMNetworkInfo: GSM_NetworkInfo;

  GSMPort: array[1..256] of Char;
  GSMModel: array[1..256] of Char;
  GSMInitLength: Array[1..256] of char;
  GSMSynchronizeTime: array[1..256] of char;
  GSMConnection: integer=GCT_FBUS;

type
  TSparkyThread = class(TThread)
    public
      procedure Execute; override;
  end;

var
  SparkyThread: TSparkyThread;

function GSM_Initialize(Port:Pchar;Model:Pchar;connectiontype:integer): integer; stdcall; external 'gnokiiapi.dll' name 'myinitialize';
function GSM_Terminate(): integer; stdcall; external 'gnokiiapi.dll' name 'myterminate';

function GSM_GetNetworkInfo(NetworkInfo:PGSM_NetworkInfo): integer; stdcall; external 'gnokiiapi.dll' name 'mygetnetworkinfo';
function GSM_GetNetworkName(NetworkName:Pchar;NetworkCode:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetnetworkname';
function GSM_GetNetworkCode(NetworkName:Pchar;NetworkCode:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetnetworkcode';
function GSM_GetNetworkData(number:integer; NetworkName:Pchar;NetworkCode:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetnetworkdata';

function GSM_Netmonitor(mode:integer;Screen:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mynetmonitor';

function GSM_GetMemoryLocation(entry:PGSM_Phonebookentry): integer; stdcall; external 'gnokiiapi.dll' name 'mygetmemorylocation';
function GSM_WritePhonebookLocation(entry:PGSM_Phonebookentry): integer; stdcall; external 'gnokiiapi.dll' name 'mywritephonebooklocation';
function GSM_GetMemoryStatus(status:PGSM_MemoryStatus): integer; stdcall; external 'gnokiiapi.dll' name 'mygetmemorystatus';

function GSM_ReadRingtoneFile(filename:Pchar;ringtone:PGSM_Ringtone): integer; stdcall; external 'gnokiiapi.dll' name 'myreadringtonefile';
function GSM_SetRingtone(ringtone:PGSM_Ringtone;notes:Pinteger): integer; stdcall; external 'gnokiiapi.dll' name 'mysetringtone';
function GSM_ReadBinRingtoneFile(filename:Pchar;ringtone:PGSM_BinRingtone): integer; stdcall; external 'gnokiiapi.dll' name 'myreadbinringtonefile';
function GSM_SetBinRingtone(ringtone:PGSM_BinRingtone): integer; stdcall; external 'gnokiiapi.dll' name 'mysetbinringtone';
function GSM_GetPhoneRingtone(ringtone:PGSM_BinRingtone;SMringtone:PGSM_Ringtone): integer; stdcall; external 'gnokiiapi.dll' name 'mygetphoneringtone';
function GSM_PlayRingtone(ringtone:PGSM_Ringtone): integer; stdcall; external 'gnokiiapi.dll' name 'myplayringtone';
function GSM_SaveRingtoneFile(filename:Pchar;ringtone:PGSM_Ringtone): integer; stdcall; external 'gnokiiapi.dll' name 'mysaveringtonefile';
function GSM_SaveBinRingtoneFile(filename:Pchar;ringtone:PGSM_BinRingtone): integer; stdcall; external 'gnokiiapi.dll' name 'mysavebinringtonefile';
function GSM_PlayNote(note:GSM_RingtoneNote): integer; stdcall; external 'gnokiiapi.dll' name 'myplaynote';
function GSM_PlayTone(herz:integer;volume:byte): integer; stdcall; external 'gnokiiapi.dll' name 'myplaytone';
function GSM_GetTempo(beats:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mygettempo';

function GSM_ReadBitmapFile(filename:Pchar;bitmap:PGSM_Bitmap): integer; stdcall; external 'gnokiiapi.dll' name 'myreadbitmapfile';
function GSM_SaveBitmapFile(filename:Pchar;bitmap:PGSM_Bitmap): integer; stdcall; external 'gnokiiapi.dll' name 'mysavebitmapfile';
function GSM_IsPointBitmap(bitmap:PGSM_Bitmap;x:integer;y:integer): integer; stdcall; external 'gnokiiapi.dll' name 'myispointbitmap';
function GSM_SetPointBitmap(bitmap:PGSM_Bitmap;x:integer;y:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mysetpointbitmap';
function GSM_ClearPointBitmap(bitmap:PGSM_Bitmap;x:integer;y:integer): integer; stdcall; external 'gnokiiapi.dll' name 'myclearpointbitmap';
function GSM_ClearBitmap(bitmap:PGSM_Bitmap): integer; stdcall; external 'gnokiiapi.dll' name 'myclearbitmap';
function GSM_GetBitmap(bitmap:PGSM_Bitmap): integer; stdcall; external 'gnokiiapi.dll' name 'mygetbitmap';
function GSM_SetBitmap(bitmap:PGSM_Bitmap): integer; stdcall; external 'gnokiiapi.dll' name 'mysetbitmap';
function GSM_ResizeBitmap(bitmap:PGSM_Bitmap;bitmaptype:integer): integer; stdcall; external 'gnokiiapi.dll' name 'myresizebitmap';

function GSM_GetImei(IMEI:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetimei';
function GSM_GetRevision(revision:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetrevision';
function GSM_GetModel(model:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetmodel';
function GSM_GetModelNumber(model:Pchar;model2:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetmodelnumber';

function GSM_GetDLLVersion(version:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mygetdllversion';

function GSM_GetSpeedDial(entry:PGSM_SpeedDial): integer; stdcall; external 'gnokiiapi.dll' name 'mygetspeeddial';

function GSM_Reset(mode:byte): integer; stdcall; external 'gnokiiapi.dll' name 'myreset';

function GSM_GetCalendarNote(entry:PGSM_CalendarNote): integer; stdcall; external 'gnokiiapi.dll' name 'mygetcalendarnote';
function GSM_WriteCalendarNote(entry:PGSM_CalendarNote): integer; stdcall; external 'gnokiiapi.dll' name 'mywritecalendarnote';
function GSM_DeleteCalendarNote(entry:PGSM_CalendarNote): integer; stdcall; external 'gnokiiapi.dll' name 'mydeletecalendarnote';

function GSM_GetAlarm(alarmnumber:integer;alarm:PGSM_DateTime): integer; stdcall; external 'gnokiiapi.dll' name 'mygetalarm';

function GSM_GetSMSCenter(center:PGSM_MessageCenter): integer; stdcall; external 'gnokiiapi.dll' name 'mygetsmscenter';
function GSM_SetSMSCenter(center:PGSM_MessageCenter): integer; stdcall; external 'gnokiiapi.dll' name 'mysetsmscenter';

function GSM_GetProfile(profile:PGSM_Profile): integer; stdcall; external 'gnokiiapi.dll' name 'mygetprofile';

function GSM_GetSimlock(simlock:PGSM_AllSimlocks): integer; stdcall; external 'gnokiiapi.dll' name 'mygetsimlock';

function GSM_GetSMSFolders(folders:PGSM_SMSFolders): integer; stdcall; external 'gnokiiapi.dll' name 'mygetsmsfolders';
function GSM_GetSMSMessage(sms:PGSM_SMSMessage): integer; stdcall; external 'gnokiiapi.dll' name 'mygetsmsmessage';
function GSM_SaveSMSMessage(sms:PGSM_SMSMessage): integer; stdcall; external 'gnokiiapi.dll' name 'mysavesmsmessage';
function GSM_SendSMSMessage(sms:PGSM_SMSMessage): integer; stdcall; external 'gnokiiapi.dll' name 'mysendsmsmessage';
function GSM_DeleteSMSMessage(sms:PGSM_SMSMessage): integer; stdcall; external 'gnokiiapi.dll' name 'mydeletesmsmessage';
function GSM_GetSMSStatus(folders:PGSM_SMSStatus): integer; stdcall; external 'gnokiiapi.dll' name 'mygetsmsstatus';
function GSM_MakeMultiPartSMS2(SMS:PGSM_MultiSMSMessage;MessageBuffer:PByte;MessageLength:Integer;UDHType:integer;Coding:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mymakemultipartsms2';

function GSM_SaveRingtoneToSMS(SMS:PGSM_MultiSMSMessage;ringtone:PGSM_Ringtone;profilestyle:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mysaveringtonetosms';
function GSM_SaveCalendarNoteToSMS(SMS:PGSM_MultiSMSMessage;note:PGSM_CalendarNote): integer; stdcall; external 'gnokiiapi.dll' name 'mysavecalendarnotetosms';
function GSM_SaveBitmapToSMS(SMS:PGSM_MultiSMSMessage;bitmap:PGSM_Bitmap;ScreenSaver:integer;Unicode:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mysavebitmaptosms';
function GSM_SavePhonebookEntryToSMS(SMS:PGSM_MultiSMSMessage;entry:PGSM_PhonebookEntry;version:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mysavephonebookentrytosms';
function GSM_SaveWAPBookmarkToSMS(SMS:PGSM_MultiSMSMessage;bookmark:PGSM_WAPBookmark): integer; stdcall; external 'gnokiiapi.dll' name 'mysavewapbookmarktosms';
function GSM_SaveWAPSettingsToSMS(SMS:PGSM_MultiSMSMessage;settings:PGSM_WAPSettings): integer; stdcall; external 'gnokiiapi.dll' name 'mysavewapsettingstosms';

function GSM_ReadConfig(model:Pchar;port:Pchar;initlength:Pchar;connection:Pinteger;synchronizetime:Pchar): integer; stdcall; external 'gnokiiapi.dll' name 'myreadconfig';

function GSM_SetLocale(locale:pchar): integer; stdcall; external 'gnokiiapi.dll' name 'mysetlocale';

function GSM_GetSecurityCode(code:PGSM_SecurityCode): integer; stdcall; external 'gnokiiapi.dll' name 'mygetsecuritycode';

function GSM_GetModelFeature(featurenumber:integer): integer; stdcall; external 'gnokiiapi.dll' name 'mygetmodelfeature';

function GSM_GetWAPBookmark(bookmark:PGSM_WAPBookmark): integer; stdcall; external 'gnokiiapi.dll' name 'mygetwapbookmark';
function GSM_SetWAPBookmark(bookmark:PGSM_WAPBookmark): integer; stdcall; external 'gnokiiapi.dll' name 'mysetwapbookmark';

function GSM_GetWAPSettings(settings:PGSM_WAPSettings): integer; stdcall; external 'gnokiiapi.dll' name 'mygetwapsettings';

function GSM_CallDivertOperation(divert:PGSM_CallDivert): integer; stdcall; external 'gnokiiapi.dll' name 'mycalldivert';

function GSM_SaveBackupFile(filename:Pchar;backup:PGSM_Backup): integer; stdcall; external 'gnokiiapi.dll' name 'mysavebackupfile';
function GSM_ReadBackupFile(filename:Pchar;backup:PGSM_Backup): integer; stdcall; external 'gnokiiapi.dll' name 'myreadbackupfile';

function GSM_DialVoice(Number:PChar): integer; stdcall; external 'gnokiiapi.dll' name 'mydialvoice';

implementation

procedure TSparkyThread.Execute;
begin
  while not Terminated do
  begin
    if GSMPhoneConnected then
    begin
      if GSM_GetNetworkInfo(@GSMNetworkInfo)<>0 then
      begin
        GSMPhoneConnected:=false;
        GSM_Terminate();
      end else
      begin
        if not SparkyThread.Suspended then SparkyThread.Suspend;
      end;
    end else
    begin
      if GSM_Initialize(@GSMPort,@GSMModel,GSMConnection)=0 then
      begin
        GSMPhoneConnected:=true;
      end else
        GSM_Terminate();
    end;
  end;
  GSMPhoneConnected:=false;
end;

begin
  GSM_ReadConfig(@GSMmodel,@GSMport,@GSMinitlength,@GSMconnection,@GSMsynchronizetime);
  GSM_SetLocale('.1250');
  GSMPhoneConnected:=false;
  SparkyThread:=TSparkyThread.Create(True);
  SparkyThread.Priority:=tpIdle;
//  SparkyThread.Priority:=tpLowest;
  SparkyThread.Resume;
end.
