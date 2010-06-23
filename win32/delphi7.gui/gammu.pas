unit Gammu;

interface

uses Classes,Forms,SysUtils;

{$MINENUMSIZE 4}

type GSM_Error = (
        GE_NONE = 1,
        GE_DEVICEOPENERROR,		// Error during opening device
	GE_DEVICELOCKED,
        GE_DEVICEDTRRTSERROR,		// Error during setting DTR/RTS in device
        GE_DEVICECHANGESPEEDERROR,	// Error during changing speed in device
        GE_DEVICEWRITEERROR,		// Error during writing device 
	GE_DEVICEREADERROR,		// Error during reading device
	GE_DEVICEPARITYERROR,		// Can't set parity on device
        GE_TIMEOUT,			// Command timed out 
        GE_FRAMENOTREQUESTED,		// Frame handled, but not requested in this moment //10
        GE_UNKNOWNRESPONSE,		// Response not handled by gammu
	GE_UNKNOWNFRAME,		// Frame not handled by gammu
	GE_UNKNOWNCONNECTIONTYPESTRING,	// Unknown connection type given by user 
	GE_UNKNOWNMODELSTRING,		// Unknown model given by user 
	GE_SOURCENOTAVAILABLE,		// Some functions not compiled in your OS
	GE_NOTSUPPORTED,		// Not supported by phone
	GE_EMPTY,			// Empty phonebook entry, ...
	GE_SECURITYERROR,		// Not allowed
	GE_INVALIDLOCATION,		// Too high or too low location...
	GE_NOTIMPLEMENTED,		// Function not implemented //20
	GE_FULL,			// Memory is full 
	GE_UNKNOWN,
	GE_CANTOPENFILE, 		// Error during opening file
	GE_MOREMEMORY,			// More memory required
	GE_PERMISSION,			// No permission
	GE_EMPTYSMSC,			// SMSC number is empty
	GE_INSIDEPHONEMENU,		// Inside phone menu - can't make something
	GE_NOTCONNECTED,		// Phone NOT connected - can't make something 
	GE_WORKINPROGRESS,		// Work in progress
      	GE_PHONEOFF,			// Phone is disabled and connected to charger //30
	GE_FILENOTSUPPORTED,		// File format not supported by Gammu
	GE_BUG,                  	// Found bug in implementation or phone //32
	GE_CANCELED,
	GE_NEEDANOTHERANSWER);


const
	GSM_MAX_SMSC_NAME_LENGTH	= 30;
	GSM_MAX_SMS_NAME_LENGTH		= 40;
	GSM_MAX_NUMBER_LENGTH		= 50;

	GSM_MAX_UDH_LENGTH		= 140;
	GSM_MAX_SMS_LENGTH		= 160;
	GSM_MAX_8BIT_SMS_LENGTH		= 140;

type
        GSM_NetworkInfo_State = (
        	GSM_HomeNetwork = 1,    //phone logged into home network
        	GSM_RoamingNetwork,     //phone logged into other than home network
        	GSM_RequestingNetwork,  //phone tries to log into network
        	GSM_NoNetwork);         //no network found

	GSM_NetworkInfo = record
		NetworkName	: array[1..30] of char;  // network name showed in some phones on display
		State		: GSM_NetworkInfo_State; // network state
      		NetworkCode 	: array[1..10] of char;  // GSM network code
		CellID	  	: array[1..10] of char;  // CellID
		LAC	  	: array[1..10] of char;  // LAC
	end;
	PGSM_NetworkInfo = ^GSM_NetworkInfo;

	GSM_DateTime = record
		Year	 : integer; //full year (for example,2002)
		Month	 : integer;
		Day	 : integer;
		Hour	 : integer;
		Minute	 : integer;
		Second	 : integer;
		Timezone : integer;
	end;
	PGSM_DateTime = ^GSM_DateTime;

	GSM_ValidityPeriodFormat = (
		GSM_NoValidityPeriod = 1, //sms doesn't have validity info
		GSM_RelativeFormat);      //sms has relative validity (starting from the sending moment). Example, one day, one week, 1 hour

        // Validity of SMS Messages.
        GSM_ValidityPeriod = (
        	GSMV_1_Hour   = 11,
        	GSMV_6_Hours  = 71,
        	GSMV_24_Hours = 167,
        	GSMV_72_Hours = 169,
        	GSMV_1_Week   = 173,
        	GSMV_Max_Time = 255);

        GSM_SMSValidity = record
                VPF      :      GSM_ValidityPeriodFormat; //type of sms validity
                Relative :      GSM_ValidityPeriod;       //how long is validity. example: 1 week or 3 days
        end;

	// SMS Messages sent as...
	GSM_SMSFormat = (
		GSMF_Pager = 1,
		GSMF_Fax,
		GSMF_Email,
		GSMF_Text);     //sms sent as normal text

	// Define datatype for SMS Message Center
	GSM_SMSC = record
		Location 	: integer;		                            // Number of the SMSC in the phone memory.
		Name		: array[1..(GSM_MAX_SMSC_NAME_LENGTH+1)*2] of char; // Name of the SMSC.
		Format		: GSM_SMSFormat;	                            // SMS is sent as text/fax/paging/email.
		Validity	: GSM_SMSValidity;	                            // Validity of SMS Message.
		Number		: array[1..(GSM_MAX_NUMBER_LENGTH+1)*2] of char;    // Number of the SMSC.
		DefaultNumber	: array[1..(GSM_MAX_NUMBER_LENGTH+1)*2] of char;    // Number of default recipient
	end;

	// types of UDH (User Data Header)
	GSM_UDH = (
		UDH_NoUDH = 1, //no user header. SMS has only "normal" text
		UDH_ConcatenatedMessages,
		UDH_DisableVoice,
		UDH_DisableFax,
		UDH_DisableEmail,
		UDH_EnableVoice,
		UDH_EnableFax,
		UDH_EnableEmail,
		UDH_VoidSMS,
		UDH_NokiaRingtone,
		UDH_NokiaOperatorLogo,
		UDH_NokiaCallerLogo,
		UDH_NokiaWAPBookmark,
		UDH_NokiaCalendarLong,
		UDH_NokiaOperatorLogoLong,
		UDH_NokiaProfileLong,
		UDH_NokiaWAPLong,
		UDH_NokiaPhonebookLong,
		UDH_UserUDH,			// Other user UDH
		UDH_MMSIndicatorLong);

        // Structure to hold UDH Header
        GSM_UDHHeader = record
                UDHType         : GSM_UDH;                              //Type
                Length          : integer;                              //Length
                Text            : array[1..GSM_MAX_UDH_LENGTH] of char; // Text
 		ID		: integer;
		PartNumber	: integer;
		AllParts	: integer;
        end;

	GSM_SMS_State = (
		GSM_Sent = 1, //for outbox sms - sent
		GSM_UnSent,   //for outbox sms - unsent
		GSM_Read,     //for inbox sms - read
		GSM_UnRead);  //for inbox sms - unread

	GSM_Coding_Type = (
		GSM_Coding_Unicode = 1, // 70 Unicode chars
		GSM_Coding_Default,     // 160 chars from Default GSM alphabet
		GSM_Coding_8bit);       // 140 8 bit chars

	GSM_SMSMessageType = (
		SMS_Deliver = 1, 	// when we save SMS in Inbox
		SMS_Status_Report,	// Delivery Report received by phone
		SMS_Submit);		// when we send SMS or save it in Outbox

	GSM_SMSMessage = record
		SMSC		 : GSM_SMSC;		// Message center
		UDH		 : GSM_UDHHeader;       // User Data Header
		Folder		 : integer;             // Inbox or Outbox message
		InboxFolder	 : LongBool;		// true, when sms is from inbox
		Location	 : integer;             // location of sms in sim memory (for example)
		Length		 : integer;		// Length of the SMS message.
                                                        // for 8 bit sms number of 8 bit chars
		Name		 : array[1..(GSM_MAX_SMS_NAME_LENGTH+1)*2] of char; // Name in Nokia 6210/7110, etc. Ignored in other
		Number		 : array[1..(GSM_MAX_NUMBER_LENGTH+1)*2]   of char; // Sender or recipient number
		Text		 : array[1..(GSM_MAX_SMS_LENGTH+1)*2] 	   of char; // Text for SMS
		PDU		 : GSM_SMSMessageType;	// Type of message
		Coding		 : GSM_Coding_Type;	// Type of coding
		DateTime	 : GSM_DateTime;	// Date of reception/response of messages.
		SMSCTime	 : GSM_DateTime;	// Date of SMSC response if DeliveryReport messages.
		DeliveryStatus	 : char;		// In delivery reports: status
		ReplyViaSameSMSC : LongBool;		// Indicates whether "Reply via same center" is set
		State		 : GSM_SMS_State;	// Read, UnRead, etc.
		SMSClass	 : shortint;		// SMS class. Normally -1.
                MessageReference : char;		// SMS Reference Number in SMS_Submit. 0, when don't used
                ReplaceMessage   : char;		// 0, when don't use this feature. 1 - 7 set SMS ID
		RejectDuplicates : LongBool;		// true, if set this flag. Normally false
	end;
	PGSM_SMSMessage = ^GSM_SMSMessage;

const
	MAX_MULTI_SMS			= 6;

type
	GSM_MultiSMSMessage = record
		Number		 : char;  //how many sms we have in collection now
		SMS		 : array[1..MAX_MULTI_SMS] of GSM_SMSMessage;
	end;
	PGSM_MultiSMSMessage = ^GSM_MultiSMSMessage;

	GSM_SMSMemoryStatus = record
		SIMUnRead	 : integer; //unread sms on sim
		SIMUsed		 : integer; //all used (including unread) locations on sim
		SIMSize		 : integer; //size of sim memory
		PhoneUnRead	 : integer;
		PhoneUsed	 : integer;
		PhoneSize	 : integer;
		TemplatesUsed	 : integer;
	end;
	PGSM_SMSMemoryStatus = ^GSM_SMSMemoryStatus;

        GSM_SecurityCodeType = (
		GSCT_UNKNOWN,
                GSCT_SecurityCode, 		// Security code.
                GSCT_Pin,                 	// PIN.
                GSCT_Pin2,                	// PIN 2.
                GSCT_Puk,                 	// PUK.
                GSCT_Puk2,			// PUK 2.
                GSCT_None);			// Code not needed.

	GSM_SecurityCode = record
		CodeType	: GSM_SecurityCodeType; //type of code
		Code	        : array[1..10] of char; //code (without unicode !)
	end;
	PGSM_SecurityCode = ^GSM_SecurityCode;

type
        //callback, which is called, when phone is connected or disconnected
        PhoneCallBackProc          = procedure(x:integer;ID:integer;connected:LongBool);stdcall;
        PPhoneCallBackProc         = ^PhoneCallBackProc;
        //this definition is used, when call back is defined under Class
        PhoneCallBackProcClass     = procedure(ID:integer;connected:LongBool);stdcall;
        PPhoneCallBackProcClass    = ^PhoneCallBackProcClass;

        //called, when phone needs PIN, PUK, etc.
        SecurityCallBackProc       = procedure(x:integer;ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
        PSecurityCallBackProc      = ^SecurityCallBackProc;
        //this definition is used, when call back is defined under Class
        SecurityCallBackProcClass  = procedure(ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
        PSecurityCallBackProcClass = ^SecurityCallBackProcClass;

        //called, when there are ANY SMS on sim
        SMSCallBackProc            = procedure(x:integer;ID:integer);stdcall;
        PSMSCallBackProc           = ^SMSCallBackProc;
        //this definition is used, when call back is defined under Class
        SMSCallBackProcClass       = procedure(ID:integer);stdcall;
        PSMSCallBackProcClass      = ^SMSCallBackProcClass;

function GSM_StartConnection	        (Phone : Pinteger; Device: PChar;Connection: PChar; Model : PChar; LogFile: PChar; LogFileType: PChar; CallBack1 : PPhoneCallBackProc; CallBack2: PSecurityCallBackProc; CallBack3: PSMSCallBackProc): GSM_Error; stdcall; external 'gammu.dll' name 'mystartconnection';
function GSM_StartConnectionClass	(Phone : Pinteger; Device: PChar;Connection: PChar; Model : PChar; LogFile: PChar; LogFileType: PChar; CallBack1 : PPhoneCallBackProcClass; CallBack2: PSecurityCallBackProcClass; CallBack3: PSMSCallBackProcClass): GSM_Error; stdcall; external 'gammu.dll' name 'mystartconnection';
function GSM_EndConnection	        (Phone : integer): GSM_Error; stdcall; external 'gammu.dll' name 'myendconnection';
function GSM_GetNetworkInfo	        (Phone : integer; NetworkInfo : PGSM_NetworkInfo): GSM_Error; stdcall; external 'gammu.dll' name 'mygetnetworkinfo';
function GSM_GetSMSStatus	        (Phone : integer; status : PGSM_SMSMemoryStatus): GSM_Error; stdcall; external 'gammu.dll' name 'mygetsmsstatus';
function GSM_GetNextSMSMessage	        (Phone : integer; SMS : PGSM_MultiSMSMessage;start : LongBool): GSM_Error; stdcall; external 'gammu.dll' name 'mygetnextsmsmessage';
function GSM_DeleteSMSMessage	        (Phone : integer; SMS : PGSM_SMSMessage): GSM_Error; stdcall; external 'gammu.dll' name 'mydeletesmsmessage';
function GSM_SendSMSMessage	        (Phone : integer; SMS : PGSM_SMSMessage;timeout:integer): GSM_Error; stdcall; external 'gammu.dll' name 'mysendsmsmessage';
function GSM_SaveSMSMessage	        (Phone : integer; SMS : PGSM_SMSMessage): GSM_Error; stdcall; external 'gammu.dll' name 'mysavesmsmessage';
function GSM_EnterSecurityCode	        (Phone : integer; Code : PGSM_SecurityCode): GSM_Error; stdcall	; external 'gammu.dll' name 'myentersecuritycode';
function GSM_GetIMEI	                (Phone : integer; IMEI:PAnsiString): GSM_Error; stdcall; external 'gammu.dll' name 'mygetimei';
function GSM_GetManufacturer	        (Phone : integer; IMEI:PAnsiString): GSM_Error; stdcall; external 'gammu.dll' name 'mygetmanufacturer';
function GSM_GetModel	                (Phone : integer; Model:PAnsiString): GSM_Error; stdcall; external 'gammu.dll' name 'mygetmodel';
function GSM_GetModelName	        (Phone : integer; Model:PAnsiString): GSM_Error; stdcall; external 'gammu.dll' name 'mygetmodelname';
function GSM_GetFirmwareVersion         (Phone : integer; Version: PDouble): GSM_Error; stdcall; external 'gammu.dll' name 'mygetfirmwareversion';
function GSM_Reset                      (Phone : integer; Hard: LongBool): GSM_Error; stdcall; external 'gammu.dll' name 'myreset';
function GSM_SMSCounter			(MessageLength:Integer;MessageBuffer:PAnsiString;UDH:GSM_UDHHeader;Coding:GSM_Coding_Type;SMSNum:PInteger;CharsLeft:PInteger): GSM_Error; stdcall; external 'gammu.dll' name 'mysmscounter';
function GSM_MakeMultiPartSMS           (MessageBuffer:PAnsiString;MessageLength:Integer;UDHType:GSM_UDH;Coding:GSM_Coding_Type;MyClass:Integer;ReplaceMessage:ShortInt;SMS:PGSM_MultiSMSMessage): GSM_Error; stdcall; external 'gammu.dll' name 'mymakemultipartsms';
procedure GSM_GetNetworkName	        (NetworkCode: PAnsiString; NetworkName: PAnsiString); stdcall; external 'gammu.dll' name 'mygetnetworkname';
procedure GSM_GetGammuVersion		(Version: PAnsiString); stdcall; external 'gammu.dll' name 'mygetgammuversion';

function GetGammuUnicodeString		(Source : AnsiString):string;

type
  TGammuGSMDevice = class(TObject)
  private
    fDeviceID                 : integer;
    fDevice                   : PChar;
    fConnection               : PChar;
    fUsed                     : Boolean;
    fConnected		      : Boolean;
    fPhoneCallBackPointer     : PPhoneCallBackProcClass;
    fSecurityCallBackPointer  : PSecurityCallBackProcClass;
    fSMSCallBackPointer       : PSMSCallBackProcClass;
  public
    function 			StartConnection(DeviceString:String;ConnectionString:String;PhoneCallBack:PPhoneCallBackProcClass;SecurityCallBack:PSecurityCallBackProcClass;SMSCallBack:PSMSCallBackProcClass;LogFile:String;LogFormat:String):GSM_Error;
    function 			EndConnection:GSM_Error;
    constructor 		Create;

    property Device           : PChar   read fDevice;
    property DeviceID         : integer read fDeviceID;
    property Connection       : PChar   read fConnection;
    property Used             : Boolean read fUsed write fUsed;
    property Connected        : Boolean read fConnected write fConnected;
  end;

implementation //here all private variables, stuff, etc.

constructor TGammuGSMDevice.Create;
begin
  Connected := False;
  Used := False;
  inherited;
end;

function TGammuGSMDevice.EndConnection():GSM_Error;
begin
  EndConnection:=GSM_EndConnection(fDeviceID);
end;

function TGammuGSMDevice.StartConnection(DeviceString:String;ConnectionString:String;PhoneCallBack:PPhoneCallBackProcClass;SecurityCallBack:PSecurityCallBackProcClass;SMSCallBack:PSMSCallBackProcClass;LogFile:String;LogFormat:String):GSM_Error;
begin
  GetMem(fDevice,Length(DeviceString) + 1);
  StrCopy(fDevice,PChar(DeviceString));
  GetMem(fConnection,50);
  StrCopy(fConnection,PChar(ConnectionString));

  fPhoneCallBackPointer    := PhoneCallBack;
  fSecurityCallBackPointer := SecurityCallBack;
  fSMSCallBackPointer      := SMSCallback;

  StartConnection:=GSM_StartConnection(@fDeviceID,fDevice,fConnection,'',PChar(LogFile),PChar(LogFormat),@fPhoneCallBackPointer,@fSecurityCallBackPointer,@fSMSCallBackPointer);
end;

function GetGammuUnicodeString(Source:AnsiString):string;
var
        i : integer;
        Dest : array[1..255] of Word;
begin
        i := 1;
        while ((Source[i*2-1]<>chr(0)) or (Source[i*2]<>chr(0))) do
        begin
                Dest[i] := ord(Source[i*2])*256+ord(Source[i*2-1]);
                i := i + 1;
        end;
        Dest[i] := 0;
        GetGammuUnicodeString := WideCharLenToString(@Dest,i-1);
end;

end.
