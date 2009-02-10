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
    SendingTabSheet: TTabSheet;
    OpenDialog1: TOpenDialog;
    TransmissionCheckBox: TCheckBox;
    Label14: TLabel;
    TransmissionFileEdit: TEdit;
    Button6: TButton;
    OtherTabSheet: TTabSheet;
    GroupBox4: TGroupBox;
    GatewayLogCheckBox: TCheckBox;
    Label16: TLabel;
    GatewayLogFileEdit: TEdit;
    Button5: TButton;
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
    GroupBox7: TGroupBox;
    PhonebookListBox: TListBox;
    PhonebookAddButton: TButton;
    PhonebookEditButton: TButton;
    PhonebookRemoveButton: TButton;
    Panel5: TPanel;
    Label8: TLabel;
    SpecialSMSCostListView: TListView;
    Label9: TLabel;
    SpecialSMSCostAddButton: TButton;
    SpecialSMSCostEditButton: TButton;
    SpecialSMSCostRemoveButton: TButton;
    OtherSMSCostEdit: TEdit;
    Panel6: TPanel;
    Label11: TLabel;
    SMSCostCurrencyEdit: TEdit;
    GroupBox5: TGroupBox;
    Label19: TLabel;
    Label20: TLabel;
    IncomingEdit: TEdit;
    Button10: TButton;
    OutgoingEdit: TEdit;
    Button12: TButton;
    TrayCheckBox: TCheckBox;
    procedure FormCreate(Sender: TObject);
    procedure TreeView1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure RefreshScreenVariables;
    procedure Button3Click(Sender: TObject);
    procedure Button7Click(Sender: TObject);
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
    procedure TreeView1KeyPress(Sender: TObject; var Key: Char);
    procedure SpecialSMSCostAddButtonClick(Sender: TObject);
    procedure SpecialSMSCostEditButtonClick(Sender: TObject);
    procedure SpecialSMSCostRemoveButtonClick(Sender: TObject);
    procedure SpecialSMSCostListViewClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  ConfigForm : TConfigForm;
  PhonebookNames              : array[1..200] of string;
  PhonebookNumbers            : array[1..200] of string;
  PhonebookEntriesNum         : integer;
  SpecialSMSNumber            : array[1..200] of string;
  SpecialSMSCost              : array[1..200] of string;
  SpecialSMSNum               : integer;

implementation

uses Main, sms_cost;

{$R *.dfm}

procedure TConfigForm.RefreshScreenVariables;
var i,j:integer;
begin
  with GatewayIniFile do
  begin
    //CommonSettings sheet can be read to screen
    IncomingEdit.Text:=ReadString('general', 'receivedir', ExtractFilePath(Application.ExeName));
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
    TrayCheckBox.Checked            :=ReadBool  ('general', 'UseTray', false);

    SpecialSMSNum := 0;
    SpecialSMScostListView.Items.Clear;
    i:=1;
    while true do
    begin
      if ReadString('general', 'SpecialSMSNumber'+inttostr(i),'') = '' then break;
      if ReadString('general', 'SpecialSMSCost'+inttostr(i),'') = '' then break;
      SpecialSMSCostListView.Items.Add;
      SpecialSMSCostListView.Items.Item[SpecialSMSCostListView.Items.Count-1].Caption:=ReadString('general', 'SpecialSMSNumber'+inttostr(i),'');
      SpecialSMSCostListView.Items.Item[SpecialSMSCostListView.Items.Count-1].SubItems.Add(ReadString('general', 'SpecialSMSCost'+inttostr(i),''));
      SpecialSMSNum:=SpecialSMSNum+1;
      SpecialSMSNumber[SpecialSMSNum]:=ReadString('general', 'SpecialSMSNumber'+inttostr(i),'');
      SpecialSMSCost[SpecialSMSNum]:=ReadString('general', 'SpecialSMSCost'+inttostr(i),'');
      i:=i+1;
    end;
    SpecialSMSCostRemoveButton.Enabled   :=False;
    SpecialSMSCostEditButton.Enabled     :=False;
    OtherSMSCostEdit.Text:=ReadString('general', 'OtherSMSCost','');
    SMSCostCurrencyEdit.Text:=ReadString('general', 'SMSCurrency','');

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

                 PortComboBox.ItemIndex:=-1;
                 if CFGPort='com1:' then PortComboBox.ItemIndex:=0;
                 if CFGPort='com2:' then PortComboBox.ItemIndex:=1;
                 if CFGPort='com3:' then PortComboBox.ItemIndex:=2;
                 if CFGPort='com4:' then PortComboBox.ItemIndex:=3;
                 if CFGPort='com5:' then PortComboBox.ItemIndex:=4;
                 if CFGPort='com6:' then PortComboBox.ItemIndex:=5;
                 if CFGPort='com7:' then PortComboBox.ItemIndex:=6;
                 if CFGPort='com8:' then PortComboBox.ItemIndex:=7;
                 if CFGPort='com9:' then PortComboBox.ItemIndex:=8;
                 if CFGPort='com10:' then PortComboBox.ItemIndex:=9;

                 ConnectionComboBox.ItemIndex:=0;
                 if CFGConnection = 'mbus' then ConnectionComboBox.ItemIndex:=1;
                 if CFGConnection = 'dlr3' then ConnectionComboBox.ItemIndex:=2;
                 if CFGConnection = 'bluephonet' then ConnectionComboBox.ItemIndex:=3;
                 if CFGConnection = 'irdaphonet' then ConnectionComboBox.ItemIndex:=4;
                 if CFGConnection = 'at19200' then ConnectionComboBox.ItemIndex:=5;
                 if CFGConnection = 'at115200' then ConnectionComboBox.ItemIndex:=6;
                 if CFGConnection = 'blueat' then ConnectionComboBox.ItemIndex:=7;
                 if CFGConnection = 'irdaat' then ConnectionComboBox.ItemIndex:=8;

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
      6    : PageControl1.ActivePage:=SendingTabSheet;
      7    : PageControl1.ActivePage:=OtherTabSheet;
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
  P        : Extended;
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
    i:=1;
    for i:=1 to PhonebookEntriesNum do
    begin
      WriteString('general', 'PhonebookName'+inttostr(i),PhonebookNames[i]);
      WriteString('general', 'PhonebookNumber'+inttostr(i),PhonebookNumbers[i]);
    end;
    DeleteKey('general', 'PhonebookName'+inttostr(i+1));
    DeleteKey('general', 'PhonebookNumber'+inttostr(i+1));

    WriteString('general', 'receivedir', IncomingEdit.Text);
    WriteString('general', 'senddir', OutgoingEdit.Text);

    WriteBool  ('general', 'UseGatewayLog'       , GatewayLogCheckBox.Checked);
    WriteString('general', 'GatewayLog'          , GatewayLogFileEdit.Text);
    WriteString('general', 'SMSNumber'           , SMSNum.Text);
    WriteBool  ('general', 'CollectSMS'          , CollectCheckBox.Checked);
    WriteString('general', 'SMSSendingRetries'   , SendingRetriesEdit.Text);
    WriteString('general', 'SMSSendingTimeout'   , SendingTimeoutEdit.Text);
    WriteBool  ('general', 'UseTray'             , TrayCheckBox.Checked);

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

    i:=1;
    for i:=1 to SpecialSMSNum do
    begin
      if (SPecialSMSCost[i] <> '') then
      begin
        if not TryStrToFloat(SpecialSMSCost[i],P) then
        begin
          Application.MessageBox(PChar(SpecialSMSCost[i]+' SMS cost is not valid'),'',0);
          exit;
        end;
      end;
      WriteString('general', 'SpecialSMSNumber'+inttostr(i),SpecialSMSNumber[i]);
      WriteString('general', 'SpecialSMSCost'+inttostr(i),SpecialSMSCost[i]);
    end;
    DeleteKey('general', 'SpecialSMSNumber'+inttostr(i+1));
    DeleteKey('general', 'SpecialSMSCost'+inttostr(i+1));
    if OtherSMSCostEdit.Text <> '' then
    begin
      if not TryStrToFloat(OtherSMSCostEdit.Text,P) then
      begin
        Application.MessageBox(PChar(OtherSMSCostEdit.Text+' SMS cost is not valid'),'',0);
        exit;
      end;
    end;
    WriteString('general', 'OtherSMSCost',OtherSMSCostEdit.Text);
    WriteString('general', 'SMSCurrency',SMSCostCurrencyEdit.Text);
  end;
  ModalResult:=mrOk;
end;

procedure TConfigForm.Button7Click(Sender: TObject);
begin
  with DirectoryForm do
  begin
    ShellTreeView1.Root:='rfMyComputer';
    ShellTreeView1.FullCollapse;
    ShellTreeView1.Path:='c:\';
    if DirectoryExists(IncomingEdit.Text) then ShellTreeView1.Path:=IncomingEdit.Text;
    if ShowModal = mrOK then
    begin
      IncomingEdit.Text:=ShellTreeView1.Path;
      if (IncomingEdit.Text[strlen(PChar(IncomingEdit.Text))] <> '\') then
      begin
        IncomingEdit.Text:=IncomingEdit.Text+'\';
      end;
    end;
  end;
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
      3: CFGConnection := 'bluephonet';
      4: CFGConnection := 'irdaphonet';
      5: CFGConnection := 'at19200';
      6: CFGConnection := 'at115200';
      7: CFGConnection := 'blueat';
      8: CFGConnection := 'irdaat';
    end;
  end;
  Label4.Enabled:=true;
  PortComboBox.Enabled:=true;
  if (ConnectionComboBox.ItemIndex < 3) or
     (ConnectionComboBox.ItemIndex = 5) or
     (ConnectionComboBox.ItemIndex = 6) then
  begin
    Label4.Caption:='Serial port:';
  end;
  if (ConnectionComboBox.ItemIndex = 3) or
     (ConnectionComboBox.ItemIndex = 7) then
  begin
    Label4.Caption:='Device address:';
  end;
  if (ConnectionComboBox.ItemIndex = 4) or
     (ConnectionComboBox.ItemIndex = 8) then
  begin
    Label4.Enabled:=false;
    PortComboBox.Enabled:=false;
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
      4: CFGPort := 'com5:';
      5: CFGPort := 'com6:';
      6: CFGPort := 'com7:';
      7: CFGPort := 'com8:';
      8: CFGPort := 'com9:';
      9: CFGPort := 'com10:';
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

procedure TConfigForm.TreeView1KeyPress(Sender: TObject; var Key: Char);
begin
  ShowConfigValues(TreeView1.Selected.ImageIndex);
end;

procedure TConfigForm.SpecialSMSCostAddButtonClick(Sender: TObject);
var i,j:integer;
begin
  with SMSPriceForm do
  begin
    Caption:='New SMS price set';
    NumberEdit.Text:='';
    PriceEdit.Text:='';
    CurrencyLabel.Caption:=SMSCostCurrencyEdit.Text;
    if ShowModal = mrOK then
    begin
      SpecialSMSNum:=SpecialSMSNum+1;
      SpecialSMSCost[SpecialSMSNum]:=PriceEdit.Text;
      SpecialSMSNumber[SpecialSMSNum]:=NumberEdit.Text;
      SpecialSMSCostListView.Items.Add;
      SpecialSMSCostListView.Items.Item[SpecialSMSCostListView.Items.Count-1].Caption:=NumberEdit.Text;
      SpecialSMSCostListView.Items.Item[SpecialSMSCostListView.Items.Count-1].SubItems.Add(PriceEdit.Text);
    end;
  end;
end;

procedure TConfigForm.SpecialSMSCostEditButtonClick(Sender: TObject);
var i:integer;
begin
  i:=1;
  while true do
  begin
    if i>SpecialSMSNum then break;
    if SpecialSMSCostListView.Items[i-1].Selected then
    begin
      with SMSPriceForm do
      begin
        Caption:='Edit SMS price set';
        NumberEdit.Text:=SpecialSMSNumber[i];
        PriceEdit.Text:=SpecialSMSCost[i];
        CurrencyLabel.Caption:=SMSCostCurrencyEdit.Text;
        if ShowModal = mrOK then
        begin
          SpecialSMSNumber[i]:=NumberEdit.Text;
          SpecialSMSCost[i]:=PriceEdit.Text;
          SpecialSMSCostListView.Items.Delete(i-1);
          SpecialSMSCostListView.Items.Add;
          SpecialSMSCostListView.Items.Item[SpecialSMSCostListView.Items.Count-1].Caption:=NumberEdit.Text;
          SpecialSMSCostListView.Items.Item[SpecialSMSCostListView.Items.Count-1].SubItems.Add(PriceEdit.Text);
        end;
      end;
      break;
    end else
    begin
      i:=i+1;
    end;
  end;
end;

procedure TConfigForm.SpecialSMSCostRemoveButtonClick(Sender: TObject);
var i,j:integer;
begin
  i:=1;
  while true do
  begin
    if i>SpecialSMSNum then break;
    if SpecialSMSCostListView.Items[i-1].Selected then
    begin
      for j:=i to SpecialSMSNum-1 do
      begin
        SpecialSMSNumber[j]:=SpecialSMSNumber[j+1];
        SpecialSMSCost[j]:=SpecialSMSCost[j+1];
      end;
      SpecialSMSNum:=SpecialSMSNum-1;
      SpecialSMSCostListView.Items.Delete(i-1);
      i:=1;
    end else
    begin
      i:=i+1;
    end;
  end;
  SpecialSMSCostListView.OnClick(Sender);
end;

procedure TConfigForm.SpecialSMSCostListViewClick(Sender: TObject);
var i,j:integer;
begin
  j:=0;
  SpecialSMSCostRemoveButton.Enabled:=False;
  for i:=1 to SpecialSMSCostListView.Items.Count do
  begin
    if SpecialSMSCostListView.Items[i-1].Selected then
    begin
      SpecialSMSCostRemoveButton.Enabled:=True;
      j:=j+1;
    end;
  end;
  SpecialSMSCostEditButton.Enabled:=false;
  if (j=1) then SpecialSMSCostEditButton.Enabled:=true;
end;

end.
