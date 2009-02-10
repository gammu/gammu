unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Gammu, ExtCtrls, ComCtrls;

type
  TForm1 = class(TForm)
    GroupBox2: TGroupBox;
    InitButton: TButton;
    GetNetInfoButton: TButton;
    InfoListBox: TListBox;
    GetAllInboxSMSButton: TButton;
    StatusBar1: TStatusBar;
    DeviceComboBox: TComboBox;
    Pin: TLabel;
    PINEdit: TEdit;
    ConnectionLabel: TLabel;
    GroupBox1: TGroupBox;
    Label2: TLabel;
    DestinationNumberEdit: TEdit;
    DeliveryReportCheckBox: TCheckBox;
    Label3: TLabel;
    SMSTextEdit: TEdit;
    SendButton: TButton;
    GetInfoButton: TButton;
    ResetButton: TButton;
    TerminateButton: TButton;
    Button1: TButton;
    procedure GetNetInfoButtonClick(Sender: TObject);
    procedure GetAllInboxSMSButtonClick(Sender: TObject);
    procedure InitButtonClick(Sender: TObject);
    procedure SendButtonClick(Sender: TObject);
    procedure GetInfoButtonClick(Sender: TObject);
    procedure ResetButtonClick(Sender: TObject);
    procedure TerminateButtonClick(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
        PhoneID                        : integer;
        SendSMS                        : GSM_SMSMessage;
        PhoneCallBackPointer           : PPhoneCallBackProc;
        SecurityCallBackPointer        : PSecurityCallBackProc;
        SMSCallBackPointer             : PSMSCallBackProc;
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

  implementation

{$R *.dfm}

//user pressed GetNetInfo button - we get network info
procedure TForm1.GetNetInfoButtonClick(Sender: TObject);
var
        NetInfo : GSM_NetworkInfo;
        error   : GSM_Error;
begin
        InfoListBox.Items.Add('Getting network info');
        error:=GSM_GetNetworkInfo(PhoneID,@NetInfo);
        if (error = ERR_NONE) then
        begin
                if ((NetInfo.State = GSM_HomeNetwork) or
                    (NetInfo.State = GSM_RoamingNetwork)) then
                begin
                        InfoListBox.Items.Add('Network code ' + NetInfo.NetworkCode);
                end;
                GSM_GetNetworkName(@NetInfo.NetworkCode,@NetInfo.NetworkName);
                InfoListBox.Items.Add('Network name ' + GetGammuUnicodeString(NetInfo.NetworkName));
        end else begin
                InfoListBox.Items.Add('Error '+inttostr(integer(error)));
        end;
end;

//user pressed GetALLInboxSMS button - we get all sms from phone
//and display info about Inbox SMS only
procedure TForm1.GetAllInboxSMSButtonClick(Sender: TObject);
var
        error   : GSM_Error;
        sms     : GSM_MultiSMSMessage;
        start   : Boolean;
begin
        InfoListBox.Items.Add('Getting all sms');
        error := ERR_NONE;
        start := True; //first set to true to allow init some internal DLL variables
        while error = ERR_NONE do
        begin
                sms.SMS[1].Folder := 0;
                error := GSM_GetNextSMSMessage (PhoneID,@sms,start);
                if (error = ERR_NONE) then
                begin
                        if (sms.SMS[1].InboxFolder) then
                        begin
                                InfoListBox.Items.Add('sms from '+GetGammuUnicodeString(sms.SMS[1].Number));
                                InfoListBox.Items.Add('Text '+GetGammuUnicodeString(sms.SMS[1].Text));
                                InfoListBox.Items.Add('Length '+inttostr(sms.SMS[1].Length));

                                //now we delete our read SMS after reading
                                sms.SMS[1].Folder:=0;
                                GSM_DeleteSMSMessage(PhoneID,@sms.SMS[1]);
                        end;
                end;
                start := false;
        end;
end;

procedure TForm1.GetInfoButtonClick(Sender: TObject);
var
        buffer : array[1..100] of char;
        error  : GSM_Error;
        ver    : Double;
begin
  GSM_GetGammuVersion(@buffer);
  InfoListBox.Items.Add('Gammu DLL version is '+buffer);
  error:=GSM_GetIMEI(PhoneID,@buffer);
  if (error = ERR_NONE) then
  begin
          InfoListBox.Items.Add('Device IMEI sent to network is '+buffer);
  end;
  error:=GSM_GetManufacturer(PhoneID,@buffer);
  if (error = ERR_NONE) then
  begin
          InfoListBox.Items.Add('Device manufacturer is '+buffer);
  end;
  error:=GSM_GetModel(PhoneID,@buffer);
  if (error = ERR_NONE) then
  begin
          InfoListBox.Items.Add('Device model is '+buffer);
  end;
  error:=GSM_GetFirmwareVersion(PhoneID,@ver);
  if (error = ERR_NONE) then
  begin
          InfoListBox.Items.Add('Device firmware version is '+floattostr(ver));
  end;

end;

//called, when phone is connected or disconnected
procedure ChangePhoneState1(x:integer;ID:integer;status:boolean);stdcall;
begin
   Form1.StatusBar1.Panels.Items[0].Text:='Not connected';
   if (status=True) then
   begin
        Form1.StatusBar1.Panels.Items[0].Text:='Connected';
        Form1.GetInfoButtonClick(NIL);
   end;
end;

//called, when phone needs PIN, PUK, etc.
procedure ChangeSecurityState(x:integer;ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
var
        Code    : GSM_SecurityCode;
        i       : integer;
        error   : GSM_Error;
begin
   //we show type of required code
   Form1.StatusBar1.Panels.Items[1].Text:='';
   case SecurityState of
        SEC_SecurityCode:Form1.StatusBar1.Panels.Items[1].Text:='Security code';
        SEC_Pin         :Form1.StatusBar1.Panels.Items[1].Text:='PIN';
        SEC_Pin2        :Form1.StatusBar1.Panels.Items[1].Text:='PIN2';
        SEC_Puk         :Form1.StatusBar1.Panels.Items[1].Text:='PUK';
        SEC_Puk2        :Form1.StatusBar1.Panels.Items[1].Text:='PUK2';
        SEC_None        :Form1.StatusBar1.Panels.Items[1].Text:='';
   end;

   //we want to enter PIN
   if (SecurityState = SEC_Pin) then
   begin
        //in PINEdit there was some text possibly with PIN
        if (Form1.PINEdit.Text<>'') then
        begin
                Code.CodeType:=SEC_Pin;
                for i:=1 to 4 do Code.Code[i]:=Form1.PINEdit.Text[i];
                Code.Code[5]:=chr(0);
                error := GSM_EnterSecurityCode(ID,@Code);
                //code was probably incorrect
                if (error <> ERR_NONE) then
                begin
                        Form1.InfoListBox.Items.Add('Error '+inttostr(integer(error)));
                        Form1.PINEdit.Text:='';
                end;
        end;
   end;
end;

//called, where there are ANY SMS on SIM
procedure HandleIncomingSMS(x:integer;ID:integer);stdcall;
begin
        Form1.InfoListBox.Items.Add('SMS on sim');
        Form1.GetAllInboxSMSButtonClick(nil);
end;

//after pressing Init button
procedure TForm1.InitButtonClick(Sender: TObject);
var
        Device: PChar;
        Connection: PChar;
        error: GSM_Error;
begin
   GetMem(Device,Length(DeviceComboBox.Text) + 1);
   StrCopy(Device, PChar(DeviceComboBox.Text));

   GetMem(Connection,50);
   Connection[0] := chr(0);
   PhoneCallBackPointer    := @ChangePhoneState1;
   SecurityCallBackPointer := @ChangeSecurityState;
   SMSCallBackPointer      := @HandleIncomingSMS;
   error:=GSM_StartConnection(@PhoneID,Device,Connection,'','logfile','text',false,@PhoneCallBackPointer,@SecurityCallBackPointer,@SMSCallBackPointer);
   if (error=ERR_NONE) then
   begin
           ConnectionLabel.Caption:=Connection;
           InitButton.Enabled:=False;
           DeviceComboBox.Enabled:=False;
           PINEdit.Enabled:=False;
           Pin.Enabled:=False;
           ConnectionLabel.Enabled:=False;
           TerminateButton.Enabled:=True;
   end else
   begin
        application.MessageBox(pchar('GSM device not found, error '+inttostr(integer(error))),'',0);
   end;
   FreeMem(Device);
   FreeMem(Connection);
   Form1.GetInfoButtonClick(Sender);
end;

//After pressing Send Button
procedure TForm1.SendButtonClick(Sender: TObject);
var     i:word;
        error:GSM_Error;
begin
   //destination number
   i:=0;
   while i<strlen(PChar(DestinationNumberEdit.Text)) do
   begin
        SendSMS.Number[i*2+1]:=DestinationNumberEdit.Text[i+1];
        SendSMS.Number[i*2+2]:=chr(0);
        i:=i+1;
   end;
   SendSMS.Number[i*2+1]:=chr(0);
   SendSMS.Number[i*2+2]:=chr(0);

   //sms text
   SendSMS.Length:=strlen(PChar(SMSTextEdit.Text));
   for i:=1 to 20 do SendSMS.Text[i]:='a';
   i:=0;
   while i<strlen(PChar(SMSTextEdit.Text)) do
   begin
        SendSMS.Text[i*2+1]:=SMSTextEdit.Text[i+1];
        SendSMS.Text[i*2+2]:=chr(0);
        i:=i+1;
   end;
   SendSMS.Text[i*2+1]:=chr(0);
   SendSMS.Text[i*2+2]:=chr(0);
   SendSMS.Coding:=GSM_Coding_Default;
   SendSMS.UDH.UDHType:=UDH_NoUDH;

   SendSMS.SMSC.Location:=1; //we will get SMSC settings from phone
   SendSMS.ReplyViaSameSMSC:=false; //don't allow to reply for the same SMSC
   if form1.DeliveryReportCheckBox.Checked then
   begin
           SendSMS.PDU:=SMS_Status_Report; //sms with delivery report
   end else
   begin
           SendSMS.PDU:=SMS_Submit; //sms without delivery report
   end;
   SendSMS.SMSClass:=-1; //no sms class - will be saved to SIM in destination
   SendSMS.RejectDuplicates:=false;
   SendSMS.MessageReference:=chr(0);
   SendSMS.ReplaceMessage:=chr(0);

//   SendSMS.Name[1]:=chr(0);
//   SendSMS.Name[2]:=chr(0);
//   SendSMS.PDU:=SMS_Deliver;
//   SendSMS.Folder:=1;
//   SendSMS.Location:=0;
//   error:=GSM_SaveSMSMessage(PhoneID,@SendSMS);

   //30 is timeout in seconds. Phone waits so long time for network answer
   error:=GSM_SendSMSMessage(PhoneID,@SendSMS,30);
   InfoListBox.Items.Add('Error '+inttostr(integer(error)));
end;

procedure TForm1.ResetButtonClick(Sender: TObject);
var error:GSM_Error;
begin
   error:=GSM_Reset(PhoneID,true);
   if (error<>ERR_NONE) then InfoListBox.Items.Add('Error '+inttostr(integer(error)));
end;

procedure TForm1.TerminateButtonClick(Sender: TObject);
var error: GSM_Error;
begin
   error:=GSM_EndConnection(PhoneID);
   if (error=ERR_NONE) then
   begin
           ConnectionLabel.Caption:='>>>connection<<<';
           InitButton.Enabled:=True;
           DeviceComboBox.Enabled:=True;
           PINEdit.Enabled:=True;
           Pin.Enabled:=True;
           ConnectionLabel.Enabled:=True;
           TerminateButton.Enabled:=False;
   end else
   begin
        application.MessageBox(pchar('GSM device not found, error '+inttostr(integer(error))),'',0);
   end;
end;

//called, when phone is connected or disconnected
procedure ChangePhoneState2(ID:integer;status:boolean);stdcall;
begin
   Form1.StatusBar1.Panels.Items[0].Text:='Not connected - CallBack2';
   if (status=True) then
   begin
        Form1.StatusBar1.Panels.Items[0].Text:='Connected - CallBack2';
   end;
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
   PhoneCallBackPointer := @ChangePhoneState2;
end;

end.
