unit main;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, ComCtrls, Menus, StdCtrls, Config, About,
  Gammu, IniFiles, Math, DateUtils, ShellAPI, Send_SMS, ImgList, ToolWin;

type
  TMainForm = class(TForm)
    StatusBar: TStatusBar;
    GroupBox1: TGroupBox;
    IncomingSMSListView: TListView;
    Splitter1: TSplitter;
    GroupBox2: TGroupBox;
    OutgoingSMSListView: TListView;
    MainMenu1: TMainMenu;
    File1: TMenuItem;
    Exit1: TMenuItem;
    ools1: TMenuItem;
    Options1: TMenuItem;
    Help1: TMenuItem;
    About1: TMenuItem;
    Splitter2: TSplitter;
    GroupBox3: TGroupBox;
    LogListView: TListView;
    SendSMSTimer: TTimer;
    SendSMS1: TMenuItem;
    Getinfoaboutdevices1: TMenuItem;
    Restartallconnections1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    Readme1: TMenuItem;
    N1: TMenuItem;
    procedure Options1Click(Sender: TObject);
    procedure Exit1Click(Sender: TObject);
    procedure About1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure SendSMSTimerTimer(Sender: TObject);
    procedure About2Click(Sender: TObject);
    procedure SendSMS1Click(Sender: TObject);
    procedure Getinfoaboutdevices1Click(Sender: TObject);
    procedure IncomingSMSListViewColumnClick(Sender: TObject;
      Column: TListColumn);
    procedure IncomingSMSListViewCompare(Sender: TObject; Item1,
      Item2: TListItem; Data: Integer; var Compare: Integer);
    procedure OutgoingSMSListViewColumnClick(Sender: TObject;
      Column: TListColumn);
    procedure LogListViewColumnClick(Sender: TObject; Column: TListColumn);
    procedure OutgoingSMSListViewCompare(Sender: TObject; Item1,
      Item2: TListItem; Data: Integer; var Compare: Integer);
    procedure LogListViewCompare(Sender: TObject; Item1, Item2: TListItem;
      Data: Integer; var Compare: Integer);
    procedure Restartallconnections1Click(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure Readme1Click(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
  private
    procedure WndProc(var Msg : TMessage); override;
    { Private declarations }
  public
    TrayIconData : TNotifyIconData;
  end;

  TGSMModem = class(TGammuGSMDevice)
    fCFGReceive        : Boolean;
    fCFGSend           : Boolean;
    fCFGPort           : String;
    fCFGConnection     : String;
    fCFGPIN            : String;
    fCFGReceiveLog     : Boolean;
    fCFGReceiveLogFile : String;
    fCFGSendLog        : Boolean;
    fCFGSendLogFile    : String;
    fCFGGammuLog       : Boolean;
    fCFGGammuLogFormat : String;
    fCFGGammuLogFile   : String;
    fPINWarning        : Boolean;
  public
    property CFGReceive        : Boolean read fCFGReceive write fCFGReceive;
    property CFGSend           : Boolean read fCFGSend write fCFGSend;
    property CFGPort           : String  read fCFGPort write fCFGPort;
    property CFGConnection     : String  read fCFGConnection write fCFGConnection;
    property CFGPIN            : String  read fCFGPIN write fCFGPIN;
    property CFGReceiveLog     : Boolean read fCFGReceiveLog write fCFGReceiveLog;
    property CFGReceiveLogFile : String  read fCFGReceiveLogFile write fCFGReceiveLogFile;
    property CFGSendLog        : Boolean read fCFGSendLog write fCFGSendLog;
    property CFGSendLogFile    : String  read fCFGSendLogFile write fCFGSendLogFile;
    property CFGGammuLogFile   : String  read fCFGGammuLogFile write fCFGGammuLogFile;
    property CFGGammuLogFormat : String  read fCFGGammuLogFormat write fCFGGammuLogFormat;
    property CFGGammuLog       : Boolean read fCFGGammuLog write fCFGGammuLog;
    property PINWarning        : Boolean read fPINWarning write fPINWarning;
  end;
  TSendSMSThread = class(TThread)
  public
    procedure Execute; override;
  end;

const MODEMNUM = 2;

var
  MainForm          : TMainForm;
  GatewayIniFile    : TIniFile;
  GSMDevice         : array[1..MODEMNUM] of TGSMModem;
  SendSMSThread     : TSendSMSThread;
  ReceivedNum       : integer;
  SentOKNum         : integer;
  SentWrongNum      : integer;
  SMSPrice          : extended;
  IconVisible       : boolean;

procedure ChangeSecurityState(x:integer;ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
procedure ChangePhoneState(x:integer;ID:integer;Connected:WordBool);stdcall;
procedure HandleIncomingSMS(x:integer;ID:integer);stdcall;
procedure RestartAllConnections(PhoneCallBack:PPhoneCallBackProcClass;SecurityCallBack:PSecurityCallBackProcClass;SMSCallBack:PSMSCallBackProcClass);
function  SearchPBKEntry(s:string):string;
function FindPrice(number:string):Extended;
function ExecuteFile(const FileName, Params, DefaultDir: string; ShowCmd: Integer): THandle;

implementation

uses dev_info;

{$R *.dfm}

{ Convert any integer type to a string }
function Inttostr2(I: Longint;digits:integer): string;
var
  S: string[11];
  j:integer;
begin
  Str(I:digits, S);
  for  j:=1 to 11 do
  begin
    if s[j]<>' ' then break;
    s[j]:='0';
  end;
  Result:= S;
end;

function FindPrice(number:string):Extended;
var i:integer;
    P:Extended;
begin
  if not TryStrToFloat(GatewayINIFile.ReadString('general', 'OtherSMSCost',''),P) then P:=0;
  FindPrice:=P;

  i:=1;
  while true do
  begin
    if GatewayINIFile.ReadString('general', 'SpecialSMSNumber'+inttostr(i),'') = '' then break;
    if GatewayINIFile.ReadString('general', 'SpecialSMSCost'+inttostr(i),'') = '' then break;
    if strpos(PChar(number),PChar(GatewayINIFile.ReadString('general', 'SpecialSMSNumber'+inttostr(i),''))) = number then
    begin
      if not TryStrToFloat(GatewayINIFile.ReadString('general', 'SpecialSMSCost'+inttostr(i),''),P) then P:=0;
      FindPrice:=P;
      break;
    end;
    i:=i+1;
  end;
end;

procedure ShowStatistics();
var
  i,Receiving,Sending:integer;
begin
  MainForm.GroupBox1.Caption:='Incoming messages ('+inttostr(ReceivedNum)+' received';
  MainForm.GroupBox2.Caption:=
      'Outgoing messages ('+inttostr(SentOKNum)+' sent OK with price '+
      FloatToStr(SMSPrice)+' '+
      GatewayINIFile.ReadString('general', 'SMSCurrency','')+', '+inttostr(SentWrongNum)+' sent incorrectly';
  Receiving:=0;
  Sending:=0;
  for i:=1 to MODEMNUM do
  begin
    if GatewayINIFile.ReadBool('modem'+inttostr(i), 'Receive', false) then Receiving:=Receiving+1;
    if GatewayINIFile.ReadBool('modem'+inttostr(i), 'Send', false) then Sending:=Sending+1;
  end;
  if Receiving = 0 then
  begin
    MainForm.GroupBox1.Caption:=MainForm.GroupBox1.Caption+', receiving disabled';
  end else
  begin
    MainForm.GroupBox1.Caption:=MainForm.GroupBox1.Caption+', '+inttostr(Receiving)+' device(s) for receiving';
  end;
  if Sending = 0 then
  begin
    MainForm.GroupBox2.Caption:=MainForm.GroupBox2.Caption+', sending disabled';
  end else
  begin
    MainForm.GroupBox2.Caption:=MainForm.GroupBox2.Caption+', '+inttostr(Sending)+' device(s) for sending';
  end;
  MainForm.GroupBox1.Caption:=MainForm.GroupBox1.Caption+')';
  MainForm.GroupBox2.Caption:=MainForm.GroupBox2.Caption+')';
end;

function SearchPBKEntry(s:string):string;
var
  FoundNumber : boolean;
  z           : integer;
begin
  FoundNumber:=false;
  z:=1;
  while true do
  begin
    if GatewayINIFile.ReadString('general', 'PhonebookNumber'+inttostr(z),'') = '' then break;
    if GatewayINIFile.ReadString('general', 'PhonebookName'+inttostr(z),'') = '' then break;
    if GatewayINIFile.ReadString('general', 'PhonebookNumber'+inttostr(z),'') = S then
    begin
      SearchPBKEntry:=GatewayINIFile.ReadString('general', 'PhonebookName'+inttostr(z),'');
      FoundNumber:=true;
      break;
    end;
    z:=z+1;
  end;
  if not FoundNumber then SearchPBKEntry:=S;
end;

procedure TMainForm.WndProc(var Msg : TMessage);
var
  p : TPoint;
begin
  case Msg.Msg of
  WM_USER + 1:
    case Msg.LParam of
    WM_RBUTTONDOWN:
      begin
        MainForm.Show;
        Application.BringToFront;
      end;
    end;
  end;
  inherited;
end;

procedure AddTextToGatewayLog(num:integer;s:string);
var
  CurrentDate,CurrentTime : TDateTime;
  F                       : TextFile;
begin
  if (GatewayIniFile.ReadBool('general', 'UseGatewayLog', True) = True) and
     (GatewayIniFile.ReadString('general', 'GatewayLog', ExtractFilePath(Application.ExeName)+'gatelog.txt')<>'') then
  begin
    MainForm.GroupBox3.Visible:=true;

    CurrentDate:=Date;
    CurrentTime:=Time;

    //adding info to main screen
    with MainForm.LogListView do
    begin
      SortType:=stNone;
      if Items.Count>500 then Items.Delete(0);
      Items.Add;
      if num <> -1 then
      begin
        Items.Item[Items.Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(num), 'Port', 'com1:');
      end else
        Items.Item[Items.Count-1].Caption:='';
      begin
      end;
      Items.Item[Items.Count-1].SubItems.Add(inttostr2(YearOf(CurrentDate),4)+'-'+inttostr2(MonthOf(CurrentDate),2)+'-'+inttostr2(DayOf(CurrentDate),2)+' '+inttostr2(HourOf(CurrentTime),2)+':'+inttostr2(MinuteOf(CurrentTime),2)+':'+inttostr2(SecondOf(CurrentTime),2));
      Items.Item[Items.Count-1].SubItems.Add(S);
      SortType:=stBoth;
    end;

    //adding info to log file
    {$I-}
    AssignFile(F, GatewayIniFile.ReadString('general', 'GatewayLog', ''));
    Append(F);
    if (IoResult <> 0) then Rewrite(F);
    if (IoResult <> 0) then
    begin
        GatewayIniFile.WriteString('general', 'GatewayLog', ExtractFilePath(Application.ExeName)+'gatelog.txt');
        AssignFile(F, GatewayIniFile.ReadString('general', 'GatewayLog', ''));
        Rewrite(F);
    end;
    {$I+}

    if (IoResult <> 0) then
    begin
      //adding info to main screen
      with MainForm.LogListView do
      begin
        SortType:=stNone;
        if Items.Count>500 then Items.Delete(0);
        Items.Add;
        Items.Item[Items.Count-1].Caption:='';
        Items.Item[Items.Count-1].SubItems.Add(inttostr2(YearOf(CurrentDate),4)+'-'+inttostr2(MonthOf(CurrentDate),2)+'-'+inttostr2(DayOf(CurrentDate),2)+' '+inttostr2(HourOf(CurrentTime),2)+':'+inttostr2(MinuteOf(CurrentTime),2)+':'+inttostr2(SecondOf(CurrentTime),2));
        Items.Item[Items.Count-1].SubItems.Add('CAN NOT WRITE TO MAIN LOG FILE');
        SortType:=stBoth;
      end;
    end else
    begin
      Write(F,inttostr2(YearOf(CurrentDate),4)+'-'+inttostr2(MonthOf(CurrentDate),2)+'-'+inttostr2(DayOf(CurrentDate),2));
      Write(F,';'+inttostr2(HourOf(CurrentTime),2)+':'+inttostr2(MinuteOf(CurrentTime),2)+':'+inttostr2(SecondOf(CurrentTime),2));
      if num <> -1 then
      begin
        Write(F,';'+GatewayINIFile.ReadString('modem'+inttostr(num), 'Port', 'com1:'));
      end else
      begin
        Write(F,';');
      end;
      WriteLn(F,';'+PChar(S));
      Flush(F);
      CloseFile(F);
    end;
  end else
  begin
    MainForm.GroupBox3.Visible:=false;
  end;
end;

function FindGammuDevice(ID:integer):integer;
var i : integer;
begin
  FindGammuDevice:=-1;
  for i:=1 to MODEMNUM do
  begin
    if (GSMDevice[i].Used) and (GSMDevice[i].DeviceID=ID) then
    begin
      FindGammuDevice:=i;
      break;
    end;
  end;
end;

procedure RestartAllConnections(PhoneCallBack:PPhoneCallBackProcClass;SecurityCallBack:PSecurityCallBackProcClass;SMSCallBack:PSMSCallBackProcClass);
var
  i     : integer;
  F     : TextFile;
begin
  for i:=1 to MODEMNUM do
  begin
    with GSMDevice[i] do
    begin
      if Used then
      begin
        AddTextToGatewayLog(i,'Terminating connection');
        EndConnection;
        MainForm.StatusBar.Panels.Items[i-1].Text:='';
        Used:=False;
        Connected:=False;
      end;
      if (GatewayIniFile.ReadBool('modem'+inttostr(i), 'Send', false) = True) or
         (GatewayIniFile.ReadBool('modem'+inttostr(i), 'Receive', false) = True)
      then
      begin
        Used:=True;
        AddTextToGatewayLog(i,'Starting connection');
        if GatewayIniFile.ReadBool('modem'+inttostr(i), 'UseGammuLog',false) then
        begin
          {$I-}
          AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(i), 'GammuLog', ExtractFilePath(Application.ExeName)+'gammu'+inttostr(i)+'.txt'));
          Append(F);
          if IOResult <> 0 then Rewrite(F);
          if IOResult <> 0 then
          begin
            AddTextToGatewayLog(i,'Resetting communication log file for device to default');
            GatewayIniFile.WriteString('modem'+inttostr(i), 'GammuLog', ExtractFilePath(Application.ExeName)+'gammu'+inttostr(i)+'.txt');
            AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(i), 'GammuLog', ExtractFilePath(Application.ExeName)+'gammu'+inttostr(i)+'.txt'));
            Append(F);
            if IOResult <> 0 then Rewrite(F);
          end;
          {$I+}
          if IoResult <> 0 then
          begin
            AddTextToGatewayLog(i,'CAN NOT WRITE TO COMMUNICATION LOG FILE FOR DEVICE');
            StartConnection(GatewayIniFile.ReadString('modem'+inttostr(i), 'Port', 'com1:'),GatewayIniFile.ReadString('modem'+inttostr(i), 'Connection', 'fbus'),PhoneCallBack,SecurityCallBack,SMSCallBack,'','');
          end else
          begin
            CloseFile(F);
            StartConnection(GatewayIniFile.ReadString('modem'+inttostr(i), 'Port', 'com1:'),GatewayIniFile.ReadString('modem'+inttostr(i), 'Connection', 'fbus'),PhoneCallBack,SecurityCallBack,SMSCallBack,GatewayIniFile.ReadString('modem'+inttostr(i), 'GammuLog', ExtractFilePath(Application.ExeName)+'gammu'+inttostr(i)+'.txt'),GatewayIniFile.ReadString('modem'+inttostr(i), 'GammuLogLevel', ''));
          end;
        end else
        begin
          StartConnection(GatewayIniFile.ReadString('modem'+inttostr(i), 'Port', 'com1:'),GatewayIniFile.ReadString('modem'+inttostr(i), 'Connection', 'fbus'),PhoneCallBack,SecurityCallBack,SMSCallBack,'','');
        end;
      end;
    end;
  end;
  ShowStatistics()
end;

procedure TSendSMSThread.Execute;
var
  FoundFile,WasSearch,found : Boolean;
  FoundNumber               : Boolean;
  sr                        : TSearchRec;
  FileAttrs,num,retries     : Integer;
  i,j,z                     : word;
  S                         : String;
  S2                        : AnsiString;
  F                         : TextFile;
  SendText,Temp,SendNumber  : array[1..5000] of char;
  MultiSMS                  : GSM_MultiSMSMessage;
  error                     : GSM_Error;
  CurrentDate,CurrentTime   : TDateTime;
begin
  error:=ERR_UNKNOWN;
  WasSearch:=false;
  FileAttrs:=faAnyFile;
  num:=1;
  while not Terminated do
  begin
    found:=false;
    while true do
    begin
      if (GSMDevice[num].Used) and
         (GSMDevice[num].Connected) and
         (GatewayIniFile.ReadBool('modem'+inttostr(num), 'Send', false) = True) then
      begin
        found:=true;
        break;
      end;
      num:=num+1;
      if num>MODEMNUM then
      begin
        num:=1;
        break;
      end;
    end;
    if found then
    begin
      FoundFile:=false;
      if WasSearch then
      begin
        if FindNext(sr) = 0 then
        begin
          FoundFile:=true;
        end else
        begin
          FindClose(sr);
          WasSearch:=False;
        end;
      end else
      begin
        if FindFirst(GatewayINIFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+'OUTBOX*.txt', FileAttrs, sr) = 0 then FoundFile:=true;
      end;
      If foundfile then
      begin
        AssignFile(F, GatewayINIFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+sr.Name);
        Reset(F);
        Readln(F, S);
        Readln(F, S2);
        Readln(F, retries);
        CloseFile(F);

        //first create Unicode string in Gammu style
        StringToWideChar(S, @Temp, 5000);
        i:=0;
        while i<strlen(PChar(S)) do
        begin
          SendNumber[i*2+1]:=Temp[i*2+1];
          SendNumber[i*2+2]:=Temp[i*2+2];
          i:=i+1;
        end;
        SendNumber[i*2+1]:=chr(0);
        SendNumber[i*2+2]:=chr(0);

        StringToWideChar(S2, @Temp, 5000);
        i:=0;
        while i<strlen(PChar(S2)) do
        begin
          //Remi wanted converting ~ to #10
          if (Temp[i*2+2] = chr(0)) and (Temp[i*2+1] = '~') then
          begin
            SendText[i*2+1]:=chr(0);
            SendText[i*2+2]:=chr(10);
          end else
          begin
            SendText[i*2+1]:=Temp[i*2+2];
            SendText[i*2+2]:=Temp[i*2+1];
          end;
          i:=i+1;
        end;
        SendText[i*2+1]:=chr(0);
        SendText[i*2+2]:=chr(0);

        //now initialy split messages
        //if message needs too many sms, remove chars
        while true do
        begin
          MultiSMS.Number:=chr(0);
          GSM_MakeMultiPartSMS(@SendText,i,UDH_NoUDH,GSM_Coding_Default,-1,0,@MultiSMS);
          if Ord(MultiSMS.Number) > 1 then
          begin
            MultiSMS.Number:=chr(0);
            GSM_MakeMultiPartSMS(@SendText,i,UDH_ConcatenatedMessages,GSM_Coding_Default,-1,0,@MultiSMS);
          end;
          if Ord(MultiSMS.Number) <= StrToInt(GatewayINIFile.ReadString('general', 'SMSNumber','1')) then break;
          i:=i-1;
          SendText[i*2+1]:=chr(0);
          SendText[i*2+2]:=chr(0);
        end;
//        for j:=1 to ord(multisms.Number) do
//        begin
//          if multisms.SMS[j].RejectDuplicates then application.messagebox('reject','',0);
//          application.messagebox(
//            pchar(
//               inttostr(Multisms.SMS[j].SMSClass)+' '+
//              inttostr(ord(Multisms.SMS[j].ReplaceMessage))+' '+inttostr(ord(Multisms.SMS[j].MessageReference))),'',0);
//        end;

        CurrentDate:=Date;
        CurrentTime:=Time;

        //adding info to main screen
        with MainForm.OutgoingSMSListView do
        begin
          SortType:=stNone;
          if Items.Count>500 then Items.Delete(0);
          Items.Add;
          Items.Item[Items.Count-1].Caption:=S2;
          Items.Item[Items.Count-1].SubItems.Add(GatewayINIFile.ReadString('modem'+inttostr(num), 'Port', 'com1:'));
          Items.Item[Items.Count-1].SubItems.Add(inttostr2(YearOf(CurrentDate),4)+'-'+inttostr2(MonthOf(CurrentDate),2)+'-'+inttostr2(DayOf(CurrentDate),2)+' '+inttostr2(HourOf(CurrentTime),2)+':'+inttostr2(MinuteOf(CurrentTime),2)+':'+inttostr2(SecondOf(CurrentTime),2));
          Items.Item[Items.Count-1].SubItems.Add(SearchPBKEntry(S));
          Items.Item[Items.Count-1].SubItems.Add('1/'+IntToStr(Ord(MultiSMS.Number)));
        end;
        //adding info to log file
        if GatewayIniFile.ReadBool('modem'+inttostr(num), 'UseSendLog', false) = True then
        begin
          {$I-}
          AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(num), 'SendLog',ExtractFilePath(Application.ExeName)+'send'+inttostr(num)+'.txt'));
          Append(F);
          if (IoResult <> 0) then Rewrite(F);
          if IOResult <> 0 then
          begin
            AddTextToGatewayLog(num,'Resetting SendLog for modem in options to default');
            GatewayIniFile.WriteString('modem'+inttostr(num), 'SendLog',ExtractFilePath(Application.ExeName)+'send'+inttostr(num)+'.txt');
            AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(num), 'SendLog',ExtractFilePath(Application.ExeName)+'send'+inttostr(num)+'.txt'));
            Append(F);
            if (IoResult <> 0) then Rewrite(F);
          end;
          {$I+}
          if IoResult <> 0 then
          begin
            AddTextToGatewayLog(num,'CAN NOT WRITE TO SEND LOG FOR MODEM');
          end else
          begin
            Write(F,inttostr2(YearOf(CurrentDate),4)+'-'+inttostr2(MonthOf(CurrentDate),2)+'-'+inttostr2(DayOf(CurrentDate),2));
            Write(F,';'+inttostr2(HourOf(CurrentTime),2)+':'+inttostr2(MinuteOf(CurrentTime),2)+':'+inttostr2(SecondOf(CurrentTime),2));
            Write(F,';'+GatewayINIFile.ReadString('modem'+inttostr(num), 'Port', 'com1:'));
            Write(F,';'+S);
            Write(F,';'+s2);
            Flush(F);
            CloseFile(F);
          end;
        end;

        for j:=1 to Ord(MultiSMS.Number) do
        begin
          //copy destination number to sms
          i:=0;
          while (SendNumber[i*2+1]<>chr(0)) or (SendNumber[i*2+2]<>chr(0)) do
          begin
            MultiSMS.SMS[j].Number[i*2+1]:=SendNumber[i*2+1];
            MultiSMS.SMS[j].Number[i*2+2]:=SendNumber[i*2+2];
            i:=i+1;
          end;
          MultiSMS.SMS[j].Number[i*2+1]:=chr(0);
          MultiSMS.SMS[j].Number[i*2+2]:=chr(0);
        end;

        for j:=1 to Ord(MultiSMS.Number) do
        begin
          MainForm.OutgoingSMSListView.Items.Item[MainForm.OutgoingSMSListView.Items.Count-1].SubItems.Strings[3]:=IntToStr(j)+'/'+IntToStr(Ord(MultiSMS.Number));

          //MultiSMS.SMS[j].PDU:=SMS_Submit;
          //MultiSMS.SMS[j].Folder:=1;
          //MultiSMS.SMS[j].Location:=0;
          //error:=GSM_SaveSMSMessage(GSMDevice[num].DeviceID,@MultiSMS.SMS[j]);

          //30 is timeout in seconds. Phone waits so long time for network answer
          error:=GSM_SendSMSMessage(GSMDevice[num].DeviceID,@MultiSMS.SMS[j],StrToInt(GatewayINIFile.ReadString('general', 'SMSSendingTimeout','30')));

          if error <> ERR_NONE then
          begin
            //adding info to main screen
            MainForm.OutgoingSMSListView.Items.Item[MainForm.OutgoingSMSListView.Items.Count-1].SubItems.Strings[3]:='ERROR '+IntTostr(integer(error))+', SMS '+IntToStr(j);
            //adding info to log file
            if GatewayIniFile.ReadBool('modem'+inttostr(num), 'UseSendLog', false) = True then
            begin
              {$I-}
              AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(num), 'SendLog',ExtractFilePath(Application.ExeName)+'send'+inttostr(num)+'.txt'));
              Append(F);
              if (IoResult <> 0) then Rewrite(F);
              {$I+}
              WriteLn(F,';ERROR '+IntTostr(integer(error))+', SMS '+IntToStr(j));
              Flush(F);
              CloseFile(F);
            end;
            SentWrongNum:=SentWrongNum+1;
            ShowStatistics();
            break;
          end else
          begin
            SentOKNum:=SentOKNum+1;
            SMSPrice:=SMSPrice+FindPrice(S);
            ShowStatistics();
          end;
        end;
        if (error = ERR_NONE) then
        begin
          DeleteFile(GatewayINIFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+sr.Name);
          //adding info to main screen
          MainForm.OutgoingSMSListView.Items.Item[MainForm.OutgoingSMSListView.Items.Count-1].SubItems.Strings[3]:='OK';
          //adding info to log file
          if GatewayIniFile.ReadBool('modem'+inttostr(num), 'UseSendLog', false) = True then
          begin
            {$I-}
            AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(num), 'SendLog',ExtractFilePath(Application.ExeName)+'send'+inttostr(num)+'.txt'));
            Append(F);
            if (IoResult <> 0) then Rewrite(F);
            {$I+}
            WriteLn(F,';OK');
            Flush(F);
            CloseFile(F);
          end;
        end else
        begin
          if retries = StrToInt(GatewayINIFile.ReadString('general', 'SMSSendingRetries','2'))-1 then
          begin
            DeleteFile(GatewayINIFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+sr.Name);
            AddTextToGatewayLog(-1,'Maximal number of SMS retries reached. SMS deleted');
          end else
          begin
            AssignFile(F, GatewayINIFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+sr.Name);
            Rewrite(F);
            Writeln(F, S);
            Writeln(F, S2);
            Writeln(F, retries+1);
            CloseFile(F);
          end;
        end;
        MainForm.OutgoingSMSListView.SortType:=stBoth;
      end;
    end;
    SendSMSThread.Suspend;
  end;
end;

//called, when phone is connected or disconnected
procedure ChangePhoneState(x:integer;ID:integer;Connected:WordBool);stdcall;
var
   num,i  : integer;
   error  : GSM_Error;
   buffer : array[1..100] of char;
begin
  num:=FindGammuDevice(ID);
  GSMDevice[num].Connected:=Connected;

  //enables or disables all menus
  MainForm.SendSMS1.Enabled:=False;
  MainForm.Getinfoaboutdevices1.Enabled:=False;
  for i:=1 to MODEMNUM do
  begin
    if (GSMDevice[i].Used) and (GSMDevice[i].Connected) then
    begin
      MainForm.Getinfoaboutdevices1.Enabled:=True;
      if GatewayIniFile.ReadBool('modem'+inttostr(num), 'Send', false) then
      begin
        MainForm.SendSMS1.Enabled:=True;
      end;
    end;
  end;

  if Connected then
  begin
    GSMDevice[num].PINWarning:=False;

    error:=GSM_GetManufacturer(ID,@buffer);
    if (error = ERR_NONE) then
    begin
      MainForm.StatusBar.Panels.Items[num-1].Text:=PChar(@buffer);
      error:=GSM_GetModelName(ID,@buffer);
      if (error = ERR_NONE) then
      begin
        MainForm.StatusBar.Panels.Items[num-1].Text:=MainForm.StatusBar.Panels.Items[num-1].Text+' '+buffer;
      end;
    end;
    if error <> ERR_NONE then
    begin
      MainForm.StatusBar.Panels.Items[num-1].Text:='Connected';
      AddTextToGatewayLog(num,'Device connected');
    end else
    begin
      error:=GSM_GetIMEI(ID,@buffer);
      if (error = ERR_NONE) then
      begin
        AddTextToGatewayLog(num,PChar('Connected '+MainForm.StatusBar.Panels.Items[num-1].Text)+' with IMEI '+buffer);
      end else
      begin
        AddTextToGatewayLog(num,PChar('Connected '+MainForm.StatusBar.Panels.Items[num-1].Text));
      end;
    end;
  end else
  begin
    MainForm.StatusBar.Panels.Items[num-1].Text:='DISCONNECTED';
    AddTextToGatewayLog(num,'No connection with device');
  end;
end;

//called, when phone needs PIN, PUK, etc.
procedure ChangeSecurityState(x:integer;ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
var
  Code      : GSM_SecurityCode;
  num,i     : integer;
  buffer    : string;
  error     : GSM_Error;
begin
  num:=FindGammuDevice(ID);
  if (SecurityState = SEC_UNKNOWN) and (not GSMDevice[num].PINWarning) then
  begin
    AddTextToGatewayLog(num,'Unknown security state. Probably not supported by phone');
    GSMDevice[num].PINWarning:=True;
    exit;
  end;
  //we want to enter PIN
  if (SecurityState = SEC_Pin) then
  begin
    buffer:=GatewayIniFile.ReadString('modem'+inttostr(num), 'PIN', '1234');
    if buffer = '' then
    begin
      AddTextToGatewayLog(num,'PIN in config file is shorter than 4 chars. Can not enter');
      Exit;
    end;
    Code.CodeType:=SEC_Pin;
    for i:=1 to 4 do
    begin
      if buffer[i] = chr(0) then
      begin
        AddTextToGatewayLog(num,'PIN in config file is shorter than 4 chars. Can not enter');
        Exit;
      end;
      Code.Code[i]:=buffer[i];
    end;
    Code.Code[5]:=chr(0);
    AddTextToGatewayLog(num,'Entering PIN');
    error:=GSM_EnterSecurityCode(ID,@Code);
    if (error <> ERR_NONE) and (error <> ERR_TIMEOUT) then
    begin
      AddTextToGatewayLog(num,'Error entering PIN. Probably incorrect');
      AddTextToGatewayLog(num,'Deleting PIN from config file');
      GatewayINIFile.DeleteKey('modem'+inttostr(num), 'PIN');
    end;
  end;
end;

//called, when there is ANY SMS on SIM
procedure HandleIncomingSMS(x:integer;ID:integer);stdcall;
var
  num,i,j     : integer;
  error       : GSM_Error;
  sms         : GSM_MultiSMSMessage;
  start       : Boolean;
  F,F2        : TextFile;
  S4,S3,S5    : AnsiString;
  wasnumber   : boolean;
  KeyPhones   : array[1..2000] of string;
  KeyText     : array[1..2000] of string;
  KeyNum      : integer;
  FoundKey    : boolean;
begin
  num := FindGammuDevice(ID);
  if not GatewayIniFile.ReadBool('modem'+inttostr(num), 'Receive', false) then exit;
  error := ERR_NONE;
  start := True; //first set to true to allow init some internal DLL variables
  while error = ERR_NONE do
  begin
    sms.SMS[1].Folder := 0;
    error := GSM_GetNextSMSMessage(ID,@sms,start);
    if (error = ERR_NONE) then
    begin
      if (sms.SMS[1].InboxFolder) and (sms.SMS[1].Coding <> GSM_Coding_8bit) and (sms.SMS[1].PDU <> SMS_Status_Report) then
      begin
        ReceivedNum:=ReceivedNum+1;
        ShowStatistics();
        S4:=GetGammuUnicodeString(sms.SMS[1].Text);
        for i:=1 to length(s4) do
        begin
          case s4[i] of
            chr(13): s4[i]:=' ';
            chr(10): s4[i]:='~';
          end;
        end;
        //adding info to main screen
        with MainForm.IncomingSMSListView do
        begin
          SortType:=stNone;
          if Items.Count>500 then Items.Delete(0);
          Items.Add;
          Items.Item[Items.Count-1].Caption:=s4;
          Items.Item[Items.Count-1].SubItems.Add(GatewayINIFile.ReadString('modem'+inttostr(num), 'Port', 'com1:'));
          Items.Item[Items.Count-1].SubItems.Add(inttostr2(sms.SMS[1].DateTime.Year,4)+'-'+inttostr2(sms.SMS[1].DateTime.Month,2)+'-'+inttostr2(sms.SMS[1].DateTime.Day,2)+' '+inttostr2(sms.SMS[1].DateTime.Hour,2)+':'+inttostr2(sms.SMS[1].DateTime.Minute,2)+':'+inttostr2(sms.SMS[1].DateTime.Second,2));
          Items.Item[Items.Count-1].SubItems.Add(SearchPBKEntry(GetGammuUnicodeString(sms.SMS[1].Number)));
          SortType:=stBoth;
        end;
        //adding info to log file
        if GatewayIniFile.ReadBool('modem'+inttostr(num), 'UseReceiveLog', false) = True then
        begin
          {$I-}
          AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(num), 'ReceiveLog',ExtractFilePath(Application.ExeName)+'receive'+inttostr(num)+'.txt'));
          Append(F);
          if (IoResult <> 0) then Rewrite(F);
          if IOResult <> 0 then
          begin
            AddTextToGatewayLog(num,'Resetting ReceiveLog for modem in options to default');
            GatewayIniFile.WriteString('modem'+inttostr(num), 'ReceiveLog',ExtractFilePath(Application.ExeName)+'receive'+inttostr(num)+'.txt');
            AssignFile(F, GatewayIniFile.ReadString('modem'+inttostr(num), 'ReceiveLog',ExtractFilePath(Application.ExeName)+'receive'+inttostr(num)+'.txt'));
            Append(F);
            if (IoResult <> 0) then Rewrite(F);
          end;
          {$I+}
          if (IoResult <> 0) then
          begin
            AddTextToGatewayLog(num,'CAN NOT WRITE TO RECEIVELOG FOR MODEM');
          end else
          begin
            Write(F,inttostr2(sms.SMS[1].DateTime.Year,4)+'-'+inttostr2(sms.SMS[1].DateTime.Month,2)+'-'+inttostr2(sms.SMS[1].DateTime.Day,2));
            Write(F,';'+inttostr2(sms.SMS[1].DateTime.Hour,2)+':'+inttostr2(sms.SMS[1].DateTime.Minute,2)+':'+inttostr2(sms.SMS[1].DateTime.Second,2));
            Write(F,';'+GatewayINIFile.ReadString('modem'+inttostr(num), 'Port', 'com1:'));
            Write(F,';'+GetGammuUnicodeString(sms.SMS[1].Number));
            WriteLn(F,';'+S4);
            Flush(F);
            CloseFile(F);
          end;
        end;
      end;
      if sms.SMS[1].InboxFolder then
      begin
        //now we delete our read SMS after reading
        sms.SMS[1].Folder:=0;
        GSM_DeleteSMSMessage(ID,@sms.SMS[1]);
      end;
    end;
    start := false;
  end;
end;

procedure TMainForm.Options1Click(Sender: TObject);
begin
  if ConfigForm.ShowModal = mrOK then
  begin
    RestartAllConnections(@ChangePhoneState,@ChangeSecurityState,@HandleIncomingSMS);

    if GatewayIniFile.ReadBool('general', 'UseTray', false) <> IconVisible then
    begin
      Application.MessageBox('Please restart application to make System Tray change working','',0);
    end;
  end;
end;

procedure TMainForm.Exit1Click(Sender: TObject);
var i:integer;
begin
  if MessageDlg('Do you really want to close Gateway ?',
    mtConfirmation, [mbYes, mbNo], 0) = mrYes then
  begin
    for i:=1 to MODEMNUM do
    begin
      if GSMDevice[i].Used then
      begin
        AddTextToGatewayLog(i,'Closing gateway');
        GSMDevice[i].EndConnection;
      end;
    end;
    halt;
  end;
end;

procedure TMainForm.About1Click(Sender: TObject);
begin
  AboutForm.ShowModal;
end;

procedure TMainForm.FormCreate(Sender: TObject);
var
  i      : integer;
  buffer : array[1..100] of char;
begin
  if (CreateMutex(nil, false, 'GammuGatewayMutex') = 0) or (GetLastError = ERROR_ALREADY_EXISTS) then
  begin
    if MessageDlg('Another instance of Gateway active. Do you want to continue ?', mtConfirmation, [mbYes, mbNo], 0) = mrNo then halt;
  end;

  ReceivedNum    := 0;
  SentOKNum      := 0;
  SentWrongNum   := 0;
  SMSPrice       := 0;
  GatewayIniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'smsd.ini');

  if GatewayIniFile.ReadBool('general', 'UseTray', false) then
  begin
    with TrayIconData do
    begin
      cbSize := SizeOf(TrayIconData);
      Wnd := Handle;
      uID := 0;
      uFlags := NIF_MESSAGE or NIF_ICON or NIF_TIP;
      uCallbackMessage := WM_USER + 1;
      hIcon := Application.Icon.Handle;
      StrPCopy(szTip, Application.Title);
    end;
    Shell_NotifyIcon(NIM_ADD, @TrayIconData);
    Application.ProcessMessages;

    IconVisible:=true;
  end else
  begin
    IconVisible:=false;
    MainForm.BorderIcons := [biSystemMenu, biMinimize, biMaximize];
  end;
  MainForm.Show;

  GSM_GetGammuVersion(@buffer);
  AddTextToGatewayLog(-1,'Starting gateway, DLL version '+buffer);
  for i:=1 to MODEMNUM do GSMDevice[i]:=TGSMModem.Create;
  RestartAllConnections(@ChangePhoneState,@ChangeSecurityState,@HandleIncomingSMS);
  SendSMSThread:=TSendSMSThread.Create(True);
  SendSMSThread.Priority:=tpIdle;
  SendSMSThread.Resume;
  SendSMSTimer.Enabled:=True;
end;

procedure TMainForm.FormResize(Sender: TObject);
var
  w:real;
  i:integer;
begin
  w:=int(MainForm.Width/MODEMNUM);
  for i:=1 to MODEMNUM do StatusBar.Panels.Items[i-1].Width:=ceil(w);
end;

procedure TMainForm.SendSMSTimerTimer(Sender: TObject);
begin
  if SendSMSThread.Suspended then SendSMSThread.Resume;
  SendSMSTimer.Interval:=2000;
end;

procedure TMainForm.About2Click(Sender: TObject);
begin
  AboutForm.ShowModal;
end;

procedure TMainForm.SendSMS1Click(Sender: TObject);
var
  i,j,z                   : integer;
  CurrentDate,CurrentTime : TDateTime;
  F                       : TextFile;
  buffer                  : array[1..10000] of char;
  FoundNumber             : boolean;
begin
  if not SendSMS1.Enabled then exit;
  with SendSMSForm do
  begin
    if ShowModal = mrOK then
    begin
      CurrentDate:=Date;
      CurrentTime:=Time;
      for i:=1 to RecipientsListBox.Items.Count do
      begin
        {$I-}
        AssignFile(F, GatewayIniFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+'OUTBOX_USER'+inttostr2(YearOf(CurrentDate),4)+inttostr2(MonthOf(CurrentDate),2)+inttostr2(DayOf(CurrentDate),2)+'_'+inttostr2(HourOf(CurrentTime),2)+inttostr2(MinuteOf(CurrentTime),2)+inttostr2(SecondOf(CurrentTime),2)+'_'+inttostr(i)+'.txt');
        Rewrite(F);
        if IOResult <> 0 then
        begin
          AddTextToGatewayLog(-1,'Resetting directory for temporary outgoing sms in options to default');
          GatewayIniFile.WriteString('general', 'senddir', ExtractFilePath(Application.ExeName));
          AssignFile(F, GatewayIniFile.ReadString('general', 'senddir', ExtractFilePath(Application.ExeName))+'OUTBOX_USER'+inttostr2(YearOf(CurrentDate),4)+inttostr2(MonthOf(CurrentDate),2)+inttostr2(DayOf(CurrentDate),2)+'_'+inttostr2(HourOf(CurrentTime),2)+inttostr2(MinuteOf(CurrentTime),2)+inttostr2(SecondOf(CurrentTime),2)+'_'+inttostr(i)+'.txt');
          Rewrite(F);
        end;
        {$I+}
        if (IOResult <> 0) then
        begin
          AddTextToGatewayLog(-1,'CAN NOT WRITE TO DIRECTORY WITH TEMPORARY OUTGOING SMS');
        end else
        begin
          FoundNumber:=false;
          z:=1;
          while true do
          begin
            if GatewayINIFile.ReadString('general', 'PhonebookNumber'+inttostr(z),'') = '' then break;
            if GatewayINIFile.ReadString('general', 'PhonebookName'+inttostr(z),'') = '' then break;
            if GatewayINIFile.ReadString('general', 'PhonebookName'+inttostr(z),'') = RecipientsListBox.Items.Strings[i-1] then
            begin
              WriteLn(F,PChar(GatewayINIFile.ReadString('general', 'PhonebookNumber'+inttostr(z),'')));
              FoundNumber:=true;
              break;
            end;
            z:=z+1;
          end;
          if not FoundNumber then WriteLn(F,PChar(RecipientsListBox.Items.Strings[i-1]));
          StrCopy(@buffer,PChar(Memo.Text));
          for j:=1 to length(buffer) do
          begin
            //Remi wanted new line char to be ~
            if buffer[j] = chr(13) then buffer[j]:=' ';
            if buffer[j] = chr(10) then buffer[j]:='~';
          end;
          WriteLn(F,PChar(@buffer));
          WriteLn(F,0);
          CloseFile(F);
        end;
      end;
    end;
  end;
end;

procedure TMainForm.Getinfoaboutdevices1Click(Sender: TObject);
var
  i       : integer;
  buffer  : array[1..100] of char;
  error   : GSM_Error;
  ver     : Double;
  NetInfo : GSM_NetworkInfo;
begin
  if not GetInfoAboutDevices1.Enabled then exit;
  with DeviceInfoForm.ListView.Items do
  begin
    Clear;
    for i:=1 to MODEMNUM do
    begin
      if (GSMDevice[i].Used) and (GSMDevice[i].Connected) then
      begin
        error:=GSM_GetManufacturer(GSMDevice[i].DeviceID,@buffer);
        if (error = ERR_NONE) then
        begin
          Add;
          Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
          Item[Count-1].SubItems.Add('Device');
          Item[Count-1].SubItems.Add(PChar(@buffer));
          error:=GSM_GetModelName(GSMDevice[i].DeviceID,@buffer);
          if (error = ERR_NONE) then
          begin
            Item[Count-1].SubItems.Strings[1]:=Item[Count-1].SubItems.Strings[1]+' '+buffer;
          end;
        end;
        error:=GSM_GetIMEI(GSMDevice[i].DeviceID,@buffer);
        if (error = ERR_NONE) then
        begin
          Add;
          Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
          Item[Count-1].SubItems.Add('IMEI');
          Item[Count-1].SubItems.Add(buffer);
        end;
        error:=GSM_GetFirmwareVersion(GSMDevice[i].DeviceID,@ver);
        if (error = ERR_NONE) then
        begin
          Add;
          Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
          Item[Count-1].SubItems.Add('Firmware');
          Item[Count-1].SubItems.Add(floattostr(ver));
        end;
        error:=GSM_GetNetworkInfo(GSMDevice[i].DeviceID,@NetInfo);
        if (error = ERR_NONE) then
        begin
          Add;
          Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
          Item[Count-1].SubItems.Add('Network state');

          case NetInfo.State of
            GSM_HomeNetwork:
              begin
                Item[Count-1].SubItems.Add('Home network '+ NetInfo.NetworkCode);
                GSM_GetNetworkName(@NetInfo.NetworkCode,@NetInfo.NetworkName);
                Add;
                Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
                Item[Count-1].SubItems.Add('Network name');
                Item[Count-1].SubItems.Add(GetGammuUnicodeString(NetInfo.NetworkName));
                Add;
                Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
                Item[Count-1].SubItems.Add('LAC');
                Item[Count-1].SubItems.Add(NetInfo.LAC);
                Add;
                Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
                Item[Count-1].SubItems.Add('CID');
                Item[Count-1].SubItems.Add(NetInfo.CID);
              end;
            GSM_RoamingNetwork:
              begin
                Item[Count-1].SubItems.Add('Roaming network '+ NetInfo.NetworkCode);
                GSM_GetNetworkName(@NetInfo.NetworkCode,@NetInfo.NetworkName);
                Add;
                Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
                Item[Count-1].SubItems.Add('Network name');
                Item[Count-1].SubItems.Add(GetGammuUnicodeString(NetInfo.NetworkName));
                Add;
                Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
                Item[Count-1].SubItems.Add('LAC');
                Item[Count-1].SubItems.Add(NetInfo.LAC);
                Add;
                Item[Count-1].Caption:=GatewayINIFile.ReadString('modem'+inttostr(i), 'Port', 'com1:');
                Item[Count-1].SubItems.Add('CID');
                Item[Count-1].SubItems.Add(NetInfo.CID);
              end;
            GSM_RequestingNetwork:
              Item[Count-1].SubItems.Add('Requesting network');
            GSM_NoNetwork:
              Item[Count-1].SubItems.Add('No network');
          else
              Item[Count-1].SubItems.Add('unknown '+inttostr(shortint(NetInfo.State)));
          end;
        end;
      end;
    end;
  end;
  DeviceInfoForm.ShowModal;
end;

procedure TMainForm.IncomingSMSListViewColumnClick(Sender: TObject;
  Column: TListColumn);
begin
  If Column.Index = GatewayINIFile.ReadInteger('general', 'IncomingMsgSort',0) then
  begin
    GatewayINIFile.WriteBool('general', 'IncomingMsgSortUp',not GatewayINIFile.ReadBool('general', 'IncomingMsgSortUp',false));
  end;
  GatewayINIFile.WriteInteger('general', 'IncomingMsgSort',Column.Index);
  (Sender as TCustomListView).AlphaSort;
end;

procedure TMainForm.IncomingSMSListViewCompare(Sender: TObject; Item1,
  Item2: TListItem; Data: Integer; var Compare: Integer);
var
  ix: Integer;
begin
  if GatewayINIFile.ReadInteger('general', 'IncomingMsgSort',0) = 0 then
  begin
    if GatewayINIFile.ReadBool('general', 'IncomingMsgSortUp',false) then
    begin
      Compare := CompareText(Item1.Caption,Item2.Caption)
    end else
    begin
      Compare := CompareText(Item2.Caption,Item1.Caption)
    end;
  end else
  begin
   ix := GatewayINIFile.ReadInteger('general', 'IncomingMsgSort',0) - 1;
   if GatewayINIFile.ReadBool('general', 'IncomingMsgSortUp',false) then
   begin
     Compare := CompareText(Item1.SubItems[ix],Item2.SubItems[ix]);
   end else
   begin
     Compare := CompareText(Item2.SubItems[ix],Item1.SubItems[ix]);
   end;
  end;
end;

procedure TMainForm.OutgoingSMSListViewColumnClick(Sender: TObject;
  Column: TListColumn);
begin
  If Column.Index = GatewayINIFile.ReadInteger('general', 'OutgoingMsgSort',0) then
  begin
    GatewayINIFile.WriteBool('general', 'OutgoingMsgSortUp',not GatewayINIFile.ReadBool('general', 'OutgoingMsgSortUp',false));
  end;
  GatewayINIFile.WriteInteger('general', 'OutgoingMsgSort',Column.Index);
  (Sender as TCustomListView).AlphaSort;
end;

procedure TMainForm.LogListViewColumnClick(Sender: TObject;
  Column: TListColumn);
begin
  If Column.Index = GatewayINIFile.ReadInteger('general', 'GatewayLogSort',0) then
  begin
    GatewayINIFile.WriteBool('general', 'GatewayLogSortUp',not GatewayINIFile.ReadBool('general', 'GatewayLogSortUp',false));
  end;
  GatewayINIFile.WriteInteger('general', 'GatewayLogSort',Column.Index);
  (Sender as TCustomListView).AlphaSort;
end;

procedure TMainForm.OutgoingSMSListViewCompare(Sender: TObject; Item1,
  Item2: TListItem; Data: Integer; var Compare: Integer);
var
  ix: Integer;
begin
  if GatewayINIFile.ReadInteger('general', 'OutgoingMsgSort',0) = 0 then
  begin
    if GatewayINIFile.ReadBool('general', 'OutgoingMsgSortUp',false) then
    begin
      Compare := CompareText(Item1.Caption,Item2.Caption)
    end else
    begin
      Compare := CompareText(Item2.Caption,Item1.Caption)
    end;
  end else
  begin
   ix := GatewayINIFile.ReadInteger('general', 'OutgoingMsgSort',0) - 1;
   if GatewayINIFile.ReadBool('general', 'OutgoingMsgSortUp',false) then
   begin
     Compare := CompareText(Item1.SubItems[ix],Item2.SubItems[ix]);
   end else
   begin
     Compare := CompareText(Item2.SubItems[ix],Item1.SubItems[ix]);
   end;
  end;
end;

procedure TMainForm.LogListViewCompare(Sender: TObject; Item1,
  Item2: TListItem; Data: Integer; var Compare: Integer);
var
  ix: Integer;
begin
  if GatewayINIFile.ReadInteger('general', 'GatewayLogSort',0) = 0 then
  begin
    if GatewayINIFile.ReadBool('general', 'GatewayLogSortUp',false) then
    begin
      Compare := CompareText(Item1.Caption,Item2.Caption)
    end else
    begin
      Compare := CompareText(Item2.Caption,Item1.Caption)
    end;
  end else
  begin
   ix := GatewayINIFile.ReadInteger('general', 'GatewayLogSort',0) - 1;
   if GatewayINIFile.ReadBool('general', 'GatewayLogSortUp',false) then
   begin
     Compare := CompareText(Item1.SubItems[ix],Item2.SubItems[ix]);
   end else
   begin
     Compare := CompareText(Item2.SubItems[ix],Item1.SubItems[ix]);
   end;
  end;
end;

procedure TMainForm.Restartallconnections1Click(Sender: TObject);
begin
  RestartAllConnections(@ChangePhoneState,@ChangeSecurityState,@HandleIncomingSMS);
end;

function ExecuteFile(const FileName, Params, DefaultDir: string;
  ShowCmd: Integer): THandle;
var
  zFileName, zParams, zDir: array[0..79] of Char;
begin
  Result := ShellExecute(Application.MainForm.Handle, nil,
    StrPCopy(zFileName, FileName), StrPCopy(zParams, Params),
    StrPCopy(zDir, DefaultDir), ShowCmd);
end;

procedure TMainForm.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if (Key = 112) then
  begin
    ExecuteFile('readme.htm','',ExtractFilePath(Application.ExeName), SW_SHOW);
  end;
end;

procedure TMainForm.Readme1Click(Sender: TObject);
begin
  ExecuteFile('readme.htm','',ExtractFilePath(Application.ExeName), SW_SHOW);
end;

procedure TMainForm.FormDestroy(Sender: TObject);
begin
  Shell_NotifyIcon(NIM_DELETE, @TrayIconData);
  Application.ProcessMessages;
  Application.Terminate;
end;

procedure TMainForm.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  CanClose:=false;
  if IconVisible then
  begin
    MainForm.Hide;
  end else
  begin
    Exit1Click(Sender);
  end;
end;

end.
