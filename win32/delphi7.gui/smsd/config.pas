unit config;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, ExtCtrls, StdCtrls, Disk_Dir, Pbk;

type
  TConfigForm = class(TForm)
    Panel1: TPanel;
    TreeView1: TTreeView;
    PageControl1: TPageControl;
    ModemTabSheet: TTabSheet;
    Button3: TButton;
    Button4: TButton;
    GroupBox6: TGroupBox;
    Label4: TLabel;
    PortComboBox: TComboBox;
    Label5: TLabel;
    ConnectionComboBox: TComboBox;
    GroupBox1: TGroupBox;
    IncomingLogCheckBox: TCheckBox;
    Label1: TLabel;
    ReceiveLogEdit: TEdit;
    Button1: TButton;
    OutgoingLogCheckBox: TCheckBox;
    Label2: TLabel;
    SendLogEdit: TEdit;
    Button2: TButton;
    Label10: TLabel;
    PINEdit: TEdit;
    OtherTabSheet: TTabSheet;
    OpenDialog1: TOpenDialog;
    TransmissionCheckBox: TCheckBox;
    Label14: TLabel;
    TransmissionFileEdit: TEdit;
    Button6: TButton;
    ReceiveCheckBox: TCheckBox;
    SendCheckBox: TCheckBox;
    Panel3: TPanel;
    Panel4: TPanel;
    Label15: TLabel;
    TransmissionFormatComboBox: TComboBox;
    GroupBox3: TGroupBox;
    Label17: TLabel;
    TLabel1: TLabel;
    Label18: TLabel;
    SMSNum: TEdit;
    CollectCheckBox: TCheckBox;
    SendingRetriesEdit: TEdit;
    SendingTimeoutEdit: TEdit;
    GroupBox5: TGroupBox;
    Label20: TLabel;
    OutgoingEdit: TEdit;
    Button12: TButton;
    GroupBox7: TGroupBox;
    PhonebookListBox: TListBox;
    PhonebookAddButton: TButton;
    PhonebookEditButton: TButton;
    PhonebookRemoveButton: TButton;
    GroupBox4: TGroupBox;
    Label16: TLabel;
    GatewayLogCheckBox: TCheckBox;
    GatewayLogFileEdit: TEdit;
    Button5: TButton;
    procedure FormCreate(Sender: TObject);
    procedure TreeView1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure RefreshScreenVariables;
    procedure Button3Click(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure ReceiveLogEditChange(Sender: TObject);
    procedure SendLogEditChange(Sender: TObject);
    procedure IncomingLogCheckBoxClick(Sender: TObject);
    procedure OutgoingLogCheckBoxClick(Sender: TObject);
    procedure ReceiveCheckBoxClick(Sender: TObject);
    procedure SendCheckBoxClick(Sender: TObject);
    procedure ConnectionComboBoxSelect(Sender: TObject);
    procedure PortComboBoxSelect(Sender: TObject);
    procedure PINEditChange(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure TransmissionFormatComboBoxChange(Sender: TObject);
    procedure TransmissionFileEditChange(Sender: TObject);
    procedure TransmissionCheckBoxClick(Sender: TObject);
    procedure Button12Click(Sender: TObject);
    procedure PhonebookAddButtonClick(Sender: TObject);
    procedure PhonebookEditButtonClick(Sender: TObject);
    procedure PhonebookListBoxClick(Sender: TObject);
    procedure PhonebookRemoveButtonClick(Sender: TObject);
    procedure FormShow(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  ConfigForm : TConfigForm;
  PhonebookNames            : array[1..200] of string;
  PhonebookNumbers          : array[1..200] of string;
  PhonebookEntriesNum       : integer;

implementation

uses Main;

{$R *.dfm}

procedure TConfigForm.RefreshScreenVariables;
var i:integer;
begin
  with GatewayIniFile do
  begin
    //CommonSettings sheet can be read to screen
    OutgoingEdit.Text:=ReadString('general', 'senddir', ExtractFilePath(Application.ExeName));

    PhonebookEntriesNum:=0;
    PhonebookListBox.Items.Clear;
    i:=1;
    while true do
    begin
      if ReadString('general', 'PhonebookName'+inttostr(i),'') = '' then break;
      if ReadString('general', 'PhonebookNumber'+inttostr(i),'') = '' then break;
      PhonebookListBox.Items.Add(ReadString('general', 'PhonebookName'+inttostr(i),''));
      PhonebookEntriesNum:=PhonebookEntriesNum+1;
      PhonebookNames[PhonebookEntriesNum]:=ReadString('general', 'PhonebookName'+inttostr(i),'');
      PhonebookNumbers[PhonebookEntriesNum]:=ReadString('general', 'PhonebookNumber'+inttostr(i),'');
      i:=i+1;
    end;
    PhonebookRemoveButton.Enabled   :=False;
    PhonebookEditButton.Enabled     :=False;

    GatewayLogCheckBox.Checked      :=ReadBool  ('general', 'UseGatewayLog'   , True);
    GatewayLogFileEdit.Text         :=ReadString('general', 'GatewayLog'      , ExtractFilePath(Application.ExeName)+'gatelog.txt');
    SMSNum.Text                     :=ReadString('general', 'SMSNumber','1');
    CollectCheckBox.Checked         :=ReadBool  ('general', 'CollectSMS', false);
    SendingRetriesEdit.Text         :=ReadString('general', 'SMSSendingRetries','2');
    SendingTimeoutEdit.Text         :=ReadString('general', 'SMSSendingTimeout','30');

    //modem options should be readed separately
    //there is one screen tab and MODEMNUM modems
    //we can't mix it
    for i:=1 to MODEMNUM do
    begin
      GSMDevice[i].CFGReceive       :=ReadBool  ('modem'+inttostr(i), 'Receive'      , false);
      GSMDevice[i].CFGSend          :=ReadBool  ('modem'+inttostr(i), 'Send'         , false);
      GSMDevice[i].CFGPort          :=ReadString('modem'+inttostr(i), 'Port'         , 'com1:');
      GSMDevice[i].CFGConnection    :=ReadString('modem'+inttostr(i), 'Connection'   , 'fbus');
      GSMDevice[i].CFGPIN           :=ReadString('modem'+inttostr(i), 'PIN'          , '1234');
      GSMDevice[i].CFGSendLog       :=ReadBool  ('modem'+inttostr(i), 'UseSendLog'   , false);
      GSMDevice[i].CFGReceiveLog    :=ReadBool  ('modem'+inttostr(i), 'UseReceiveLog', false);
      GSMDevice[i].CFGReceiveLogFile:=ReadString('modem'+inttostr(i), 'ReceiveLog'   , ExtractFilePath(Application.ExeName)+'receive'+inttostr(i)+'.txt');
      GSMDevice[i].CFGSendLogFile   :=ReadString('modem'+inttostr(i), 'SendLog'      , ExtractFilePath(Application.ExeName)+'send'+inttostr(i)+'.txt');
      GSMDevice[i].CFGGammuLogFile  :=ReadString('modem'+inttostr(i), 'GammuLog'     , ExtractFilePath(Application.ExeName)+'gammu'+inttostr(i)+'.txt');
      GSMDevice[i].CFGGammuLogFormat:=ReadString('modem'+inttostr(i), 'GammuLogLevel', '');
      GSMDevice[i].CFGGammuLog      :=ReadBool  ('modem'+inttostr(i), 'UseGammuLog'  , false);
    end;
  end;
end;

procedure ShowConfigValues(i:integer);
begin
  with ConfigForm do
  begin
    case i of
      1..MODEMNUM :
             begin
               ModemTabSheet.Caption:=inttostr(i);
               with GSMDevice[i] do
               begin
                 ReceiveCheckBox.Checked:=CFGReceive;
                 SendCheckBox.Checked:=CFGSend;
                 IncomingLogCheckBox.Checked:=CFGReceiveLog;
                 OutgoingLogCheckBox.Checked:=CFGSendLog;

                 PortComboBox.ItemIndex:=0;
                 if CFGPort='com2:' then PortComboBox.ItemIndex:=1;
                 if CFGPort='com3:' then PortComboBox.ItemIndex:=2;
                 if CFGPort='com4:' then PortComboBox.ItemIndex:=3;

                 ConnectionComboBox.ItemIndex:=0;
                 if CFGConnection = 'mbus' then ConnectionComboBox.ItemIndex:=1;
                 if CFGConnection = 'dlr3' then ConnectionComboBox.ItemIndex:=2;
                 if CFGConnection = 'at19200' then ConnectionComboBox.ItemIndex:=3;
                 if CFGConnection = 'at115200' then ConnectionComboBox.ItemIndex:=4;

                 TransmissionFileEdit.Text:=CFGGammuLogFile;
                 TransmissionFormatComboBox.ItemIndex:=3;
                 if CFGGammuLogFormat = 'binary'      then TransmissionFormatComboBox.ItemIndex:=0;
                 if CFGGammuLogFormat = 'textdate'    then TransmissionFormatComboBox.ItemIndex:=1;
                 if CFGGammuLogFormat = 'textalldate' then TransmissionFormatComboBox.ItemIndex:=2;
                 if CFGGammuLogFormat = 'errorsdate'  then TransmissionFormatComboBox.ItemIndex:=3;
                 TransmissionCheckBox.Checked:=CFGGammuLog;

                 PINEdit.Text:=CFGPIN;
                 ReceiveLogEdit.Text:=CFGReceiveLogFile;
                 SendLogEdit.Text:=CFGSendLogFile;
                 ConnectionComboBox.Text:=CFGConnection;
               end;
               PageControl1.ActivePage:=ModemTabSheet;
             end;
      6    : PageControl1.ActivePage:=OtherTabSheet;
    end;
  end;
end;

procedure TConfigForm.FormCreate(Sender: TObject);
var i:integer;
begin
  for i:=1 to ConfigForm.PageControl1.PageCount do
  begin
    ConfigForm.PageControl1.Pages[i-1].TabVisible:=false;
  end;
  RefreshScreenVariables;
end;

procedure TConfigForm.TreeView1MouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  ShowConfigValues(TreeView1.Selected.ImageIndex);
end;

procedure TConfigForm.Button3Click(Sender: TObject);
var
  i,j      : integer;
  TheSame  : Boolean;
begin
  TheSame:=False;
  for i:=1 to MODEMNUM do
  begin
    for j:=1 to MODEMNUM do
    begin
      if (GSMDevice[i].CFGPort = GSMDevice[j].CFGPort) and (i <> j)
      then begin
        if (GSMDevice[i].CFGReceive or GSMDevice[i].CFGSend) and
           (GSMDevice[j].CFGReceive or GSMDevice[j].CFGSend)
        then begin
          TheSame:=True;
          Break;
        end;
      end;
    end;
  end;
  if TheSame then
  begin
    if MessageDlg('The same serial port set for more than one device. Continue ?',
          mtConfirmation, [mbYes, mbNo], 0) <> mrYes then
    begin
      exit;
    end;
  end;
  with GatewayIniFile do
  begin
    for i:=1 to PhonebookEntriesNum do
    begin
      WriteString('general', 'PhonebookName'+inttostr(i),PhonebookNames[i]);
      WriteString('general', 'PhonebookNumber'+inttostr(i),PhonebookNumbers[i]);
    end;
    for i:=PhonebookEntriesNum+1 to 100 do
    begin
       DeleteKey('general', 'PhonebookName'+inttostr(i));
       DeleteKey('general', 'PhonebookNumber'+inttostr(i));
    end;

    WriteString('general', 'senddir', OutgoingEdit.Text);

    WriteBool  ('general', 'UseGatewayLog'       , GatewayLogCheckBox.Checked);
    WriteString('general', 'GatewayLog'          , GatewayLogFileEdit.Text);
    WriteString('general', 'SMSNumber'           , SMSNum.Text);
    WriteBool  ('general', 'CollectSMS'          , CollectCheckBox.Checked);
    WriteString('general', 'SMSSendingRetries'   , SendingRetriesEdit.Text);
    WriteString('general', 'SMSSendingTimeout'   , SendingTimeoutEdit.Text);

    for i:=1 to MODEMNUM do
    begin
      WriteBool  ('modem'+inttostr(i), 'Receive'      , GSMDevice[i].CFGReceive);
      WriteBool  ('modem'+inttostr(i), 'Send'         , GSMDevice[i].CFGSend);
      WriteString('modem'+inttostr(i), 'Port'         , GSMDevice[i].CFGPort);
      WriteString('modem'+inttostr(i), 'Connection'   , GSMDevice[i].CFGConnection);
      WriteString('modem'+inttostr(i), 'PIN'          , GSMDevice[i].CFGPIN);
      WriteBool  ('modem'+inttostr(i), 'UseSendLog'   , GSMDevice[i].CFGSendLog);
      WriteBool  ('modem'+inttostr(i), 'UseReceiveLog', GSMDevice[i].CFGReceiveLog);
      WriteString('modem'+inttostr(i), 'ReceiveLog'   , GSMDevice[i].CFGReceiveLogFile);
      WriteString('modem'+inttostr(i), 'SendLog'      , GSMDevice[i].CFGSendLogFile);
      WriteString('modem'+inttostr(i), 'GammuLog'     , GSMDevice[i].CFGGammuLogFile);
      WriteString('modem'+inttostr(i), 'GammuLogLevel', GSMDevice[i].CFGGammuLogFormat);
      WriteBool  ('modem'+inttostr(i), 'UseGammuLog'  , GSMDevice[i].CFGGammuLog);
    end;
  end;
  ModalResult:=mrOk;
end;

procedure TConfigForm.Button5Click(Sender: TObject);
begin
  OpenDialog1.InitialDir:=ExtractFilePath(GatewayLogFileEdit.Text);
  OpenDialog1.FileName:=ExtractFileName(GatewayLogFileEdit.Text);
  if OpenDialog1.Execute then
  begin
    GatewayLogFileEdit.Text:=OpenDialog1.FileName;
  end;
end;

procedure TConfigForm.Button1Click(Sender: TObject);
begin
  OpenDialog1.InitialDir:=ExtractFilePath(ReceiveLogEdit.Text);
  OpenDialog1.FileName:=ExtractFileName(ReceiveLogEdit.Text);
  if OpenDialog1.Execute then
  begin
    ReceiveLogEdit.Text:=OpenDialog1.FileName;
  end;
end;

procedure TConfigForm.Button2Click(Sender: TObject);
begin
  OpenDialog1.InitialDir:=ExtractFilePath(SendLogEdit.Text);
  OpenDialog1.FileName:=ExtractFileName(SendLogEdit.Text);
  if OpenDialog1.Execute then
  begin
    SendLogEdit.Text:=OpenDialog1.FileName;
  end;
end;

procedure TConfigForm.ReceiveLogEditChange(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGReceiveLogFile:=ReceiveLogEdit.Text;
end;

procedure TConfigForm.SendLogEditChange(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGSendLogFile:=SendLogEdit.Text;
end;

procedure TConfigForm.IncomingLogCheckBoxClick(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGReceiveLog:=IncomingLogCheckBox.Checked;
end;

procedure TConfigForm.OutgoingLogCheckBoxClick(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGSendLog:=OutgoingLogCheckBox.Checked;
end;

procedure TConfigForm.ReceiveCheckBoxClick(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGReceive:=ReceiveCheckBox.Checked;
end;

procedure TConfigForm.SendCheckBoxClick(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGSend:=SendCheckBox.Checked;
end;

procedure TConfigForm.ConnectionComboBoxSelect(Sender: TObject);
begin
  with GSMDevice[StrToInt(ModemTabSheet.Caption)] do
  begin
    case ConnectionComboBox.ItemIndex of
      0: CFGConnection := 'fbus';
      1: CFGConnection := 'mbus';
      2: CFGConnection := 'dlr3';
      3: CFGConnection := 'at19200';
      4: CFGConnection := 'at115200';
    end;
  end;
end;

procedure TConfigForm.PortComboBoxSelect(Sender: TObject);
begin
  with GSMDevice[StrToInt(ModemTabSheet.Caption)] do
  begin
    case PortComboBox.ItemIndex of
      0: CFGPort := 'com1:';
      1: CFGPort := 'com2:';
      2: CFGPort := 'com3:';
      3: CFGPort := 'com4:';
    end;
  end;
end;

procedure TConfigForm.PINEditChange(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGPIN:=PINEdit.Text;
end;

procedure TConfigForm.Button6Click(Sender: TObject);
begin
  OpenDialog1.InitialDir:=ExtractFilePath(TransmissionFileEdit.Text);
  OpenDialog1.FileName:=ExtractFileName(TransmissionFileEdit.Text);
  if OpenDialog1.Execute then
  begin
    TransmissionFileEdit.Text:=OpenDialog1.FileName;
  end;
end;

procedure TConfigForm.TransmissionFormatComboBoxChange(Sender: TObject);
begin
  with GSMDevice[StrToInt(ModemTabSheet.Caption)] do
  begin
    case TransmissionFormatComboBox.ItemIndex of
      0: CFGGammuLogFormat := 'binary';
      1: CFGGammuLogFormat := 'textdate';
      2: CFGGammuLogFormat := 'textalldate';
      3: CFGGammuLogFormat := 'errorsdate';
    end;
  end;
end;

procedure TConfigForm.TransmissionFileEditChange(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGGammuLogFile:=TransmissionFileEdit.Text;
end;

procedure TConfigForm.TransmissionCheckBoxClick(Sender: TObject);
begin
  GSMDevice[StrToInt(ModemTabSheet.Caption)].CFGGammuLog:=TransmissionCheckBox.Checked;
end;

procedure TConfigForm.Button12Click(Sender: TObject);
begin
  with DirectoryForm do
  begin
    ShellTreeView1.Root:='rfMyComputer';
    ShellTreeView1.FullCollapse;
    ShellTreeView1.Path:='c:\';
    if DirectoryExists(OutgoingEdit.Text) then ShellTreeView1.Path:=OutgoingEdit.Text;
    if ShowModal = mrOK then
    begin
      OutgoingEdit.Text:=ShellTreeView1.Path;
      if (OutgoingEdit.Text[strlen(PChar(OutgoingEdit.Text))] <> '\') then
      begin
        OutgoingEdit.Text:=OutgoingEdit.Text+'\';
      end;
    end;
  end;
end;

procedure TConfigForm.PhonebookAddButtonClick(Sender: TObject);
begin
  with PhonebookForm do
  begin
    Caption:='New phonebook entry';
    NameEdit.Text:='';
    NumberEdit.Text:='';
    if ShowModal = mrOK then
    begin
      PhonebookEntriesNum:=PhonebookEntriesNum+1;
      PhonebookNames[PhonebookEntriesNum]:=NameEdit.Text;
      PhonebookNumbers[PhonebookEntriesNum]:=NumberEdit.Text;
      PhonebookListBox.Items.Add(NameEdit.Text);
    end;
  end;
end;

procedure TConfigForm.PhonebookEditButtonClick(Sender: TObject);
var i:integer;
begin
  i:=1;
  while true do
  begin
    if i>PhonebookEntriesNum then break;
    if PhonebookListBox.Selected[i-1] then
    begin
      with PhonebookForm do
      begin
        Caption:='Edit phonebook entry';
        NameEdit.Text:=PhonebookNames[i];
        NumberEdit.Text:=PhonebookNumbers[i];
        if ShowModal = mrOK then
        begin
          PhonebookNames[i]:=NameEdit.Text;
          PhonebookNumbers[i]:=NumberEdit.Text;
          PhonebookListBox.Items.Strings[i-1]:=NameEdit.Text;
        end;
      end;
      break;
    end else
    begin
      i:=i+1;
    end;
  end;
end;

procedure TConfigForm.PhonebookListBoxClick(Sender: TObject);
var i,j:integer;
begin
  j:=0;
  PhonebookRemoveButton.Enabled:=False;
  for i:=1 to PhonebookListBox.Items.Count do
  begin
    if PhonebookListBox.Selected[i-1] then
    begin
      PhonebookRemoveButton.Enabled:=True;
      j:=j+1;
    end;
  end;
  PhonebookEditButton.Enabled:=false;
  if (j=1) then PhonebookEditButton.Enabled:=true;
end;

procedure TConfigForm.PhonebookRemoveButtonClick(Sender: TObject);
var i,j:integer;
begin
  i:=1;
  while true do
  begin
    if i>PhonebookListBox.Items.Count then break;
    if PhonebookListBox.Selected[i-1] then
    begin
      for j:=i to PhonebookEntriesNum-1 do
      begin
        PhonebookNames[j]:=PhonebookNames[j+1];
        PhonebookNumbers[j]:=PhonebookNumbers[j+1];
      end;
      PhonebookEntriesNum:=PhonebookEntriesNum-1;
      PhonebookListBox.Items.Delete(i-1);
      i:=1;
    end else
    begin
      i:=i+1;
    end;
  end;
  PhonebookListBox.OnClick(Sender);
end;

procedure TConfigForm.FormShow(Sender: TObject);
begin
  RefreshScreenVariables;
end;

end.
