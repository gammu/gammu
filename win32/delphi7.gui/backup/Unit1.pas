unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Gammu, ExtCtrls, ComCtrls, StrUtils;

type
  TForm1 = class(TForm)
    Button1: TButton;
    PageControl1: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    TabSheet4: TTabSheet;
    NextButton: TButton;
    Memo1: TMemo;
    ProgressBar1: TProgressBar;
    PrevButton: TButton;
    GroupBox3: TGroupBox;
    ConnectionComboBox: TComboBox;
    DeviceComboBox: TComboBox;
    GroupBox4: TGroupBox;
    Label1: TLabel;
    FileNameEdit: TEdit;
    Button6: TButton;
    Label2: TLabel;
    Edit1: TEdit;
    Label3: TLabel;
    Edit4: TEdit;
    Label4: TLabel;
    Edit7: TEdit;
    GroupBox5: TGroupBox;
    CheckBox1: TCheckBox;
    CheckBox2: TCheckBox;
    CheckBox3: TCheckBox;
    CheckBox4: TCheckBox;
    OpenRestoreDialog: TOpenDialog;
    Button5: TButton;
    Button7: TButton;
    Button8: TButton;
    StatusBar1: TStatusBar;
    GroupBox1: TGroupBox;
    ModelEdit: TEdit;
    IMEIEdit: TEdit;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    TabSheet0: TTabSheet;
    GroupBox2: TGroupBox;
    BackupRadioButton: TRadioButton;
    RestoreRadioButton: TRadioButton;
    SaveBackupDialog: TSaveDialog;
    Label8: TLabel;
    Label9: TLabel;
    Edit10: TEdit;
    Label10: TLabel;
    Edit2: TEdit;
    procedure Button1Click(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure NextButtonClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure TabSheet3Show(Sender: TObject);
    procedure TabSheet2Show(Sender: TObject);
    procedure Button8Click(Sender: TObject);
    procedure PrevButtonClick(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button7Click(Sender: TObject);
    procedure TabSheet4Show(Sender: TObject);
    procedure BackupRadioButtonClick(Sender: TObject);
    procedure RestoreRadioButtonClick(Sender: TObject);
    procedure ConnectionComboBoxSelect(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
  private
    { Private declarations }
        PhoneID                        : integer;
        Connected                      : boolean;
        PhoneCallBackPointer           : PPhoneCallBackProc;
        SecurityCallBackPointer        : PSecurityCallBackProc;
        SMSCallBackPointer             : PSMSCallBackProc;
        BackupInfo,BackupInfo0         : GSM_Backup_Info;
        CancelThread                   : boolean;
        StartupDevice                  : string;
        StartupConnection              : string;
  public
    { Public declarations }
  end;

type
  TMyThread = class(TThread)
  private
    { Private declarations }
  protected
    procedure Execute; override;
  end;
  TMyThread2 = class(TThread)
  private
    { Private declarations }
  protected
    procedure Execute; override;
  end;

var
  Form1: TForm1;
  BackupThread:TMyThread;
  CheckThread:TMyThread2;

  implementation

{$R *.dfm}

//called, when phone is connected or disconnected
procedure ChangePhoneState1(x:integer;ID:integer;status:boolean);stdcall;
var
  error: GSM_Error;
  buffer : array[1..100] of char;
  buffer2: array[1..100] of char;
  ver:Double;
begin
	if status then
	begin
    error:=GSM_GetModelName(Form1.PhoneID,@buffer);
    if (error = ERR_NONE) then
    begin
      error:=GSM_GetManufacturer(Form1.PhoneID,@buffer2);
      if (error = ERR_NONE) then
      begin
        Form1.ModelEdit.Text:=String(buffer2);
        Form1.ModelEdit.Text:=Form1.ModelEdit.Text+' '+buffer;
      end else
      begin
        Form1.ModelEdit.Text:=buffer;
      end;
      Form1.StatusBar1.Panels.Items[0].Text:=Form1.ModelEdit.Text;
    end;
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get model: error '+inttostr(integer(error))),'',0);

		error:=GSM_GetIMEI(Form1.PhoneID,@buffer);
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get IMEI: error '+inttostr(integer(error))),'',0);
    if (error = ERR_NONE) then Form1.IMEIEdit.Text:=buffer;

    error:=GSM_GetFirmwareVersion(Form1.PhoneID,@ver);
    if (error = ERR_NONE) then Form1.Edit2.Text:=floattostr(ver);

    if Form1.PageControl1.ActivePage=Form1.TabSheet2 then Form1.NextButton.Enabled:=true;

//    error:=GSM_EndConnection(Form1.PhoneID);
//    if (error <> ERR_NONE) then application.MessageBox(pchar('End connection: error '+inttostr(integer(error))),'',0);
  end else begin
    Form1.StatusBar1.Panels.Items[0].Text:='Phone DISCONECTED';
	end;
  Form1.connected:=status;
end;

procedure TForm1.Button1Click(Sender: TObject);
var
   Device: PChar;
   Connection: PChar;
   error: GSM_Error;
   buffer : array[1..100] of char;
   Con:string;
begin
   GetMem(Device,Length(DeviceComboBox.Text) + 1);
   StrCopy(Device, PChar(DeviceComboBox.Text));

   case ConnectionComboBox.ItemIndex of
     0: Con:='irdaphonet';
     1: Con:='fbusdku5';
     2: con:='fbus';
     3: con:='dlr3';
   end;
   GetMem(Connection,Length(Con) + 1);
   StrCopy(Connection, PChar(Con));

   PhoneCallBackPointer    := @ChangePhoneState1;
   SecurityCallBackPointer := nil;
   SMSCallBackPointer      := nil;

   error:=GSM_StartConnection(@PhoneID,Device,Connection,'','','',@PhoneCallBackPointer,@SecurityCallBackPointer,@SMSCallBackPointer);
   if (error<>ERR_NONE) then application.MessageBox(pchar('Start: error '+inttostr(integer(error))),'',0);

   FreeMem(Device);
   FreeMem(Connection);
end;

procedure TForm1.Button6Click(Sender: TObject);
var
  error:GSM_Error;
  buff:array[1..200] of char;
  DT:GSM_DateTime;
begin
  if BackupRadioButton.Checked then
  begin
    if SaveBackupDialog.Execute then
    begin
      if SaveBackupDialog.FileName<>'' then
      begin
        FileNameEdit.Text:=SaveBackupDialog.FileName;
        GSM_GetBackupFormatFeatures(PAnsiString(FileNameEdit.Text),@BackupInfo0);
      end;
    end;
  end else
  begin
    if OpenRestoreDialog.Execute then
    begin
      if OpenRestoreDialog.FileName<>'' then
      begin
        FileNameEdit.Text:=OpenRestoreDialog.FileName;

        error:=GSM_ReadBackupFile(PAnsiString(FileNameEdit.Text),@BackupInfo0);
        if error <> ERR_NONE then exit;

        error:=GSM_GetBackupFileIMEI(@buff);
        if error <> ERR_NONE then exit;
        Edit4.Text:=String(buff);

        error:=GSM_GetBackupFileModel(@buff);
        if error <> ERR_NONE then exit;
        Edit1.Text:=String(buff);

        error:=GSM_GetBackupFileDateTime(@DT);
        if error <> ERR_NONE then exit;
        Edit7.Text:=inttostr(DT.Day)+'-'+inttostr(DT.Month)+'-'+inttostr(DT.Year)+' '+inttostr(DT.Hour)+':'+inttostr(DT.Minute);

        error:=GSM_GetBackupFileCreator(@buff);
        if error <> ERR_NONE then exit;
        Edit10.Text:=String(buff);
      end;
    end;
  end;
  NextButton.Enabled:=true;
end;

procedure TForm1.NextButtonClick(Sender: TObject);
begin
  if PageControl1.ActivePage=TabSheet0 then
  begin
    PageControl1.ActivePage:=TabSheet1;
    PrevButton.Enabled:=true;
    NextButton.Enabled:=false;
    if FileNameEdit.Text<>'' then NextButton.Enabled:=true;
    exit;
  end;
  if PageControl1.ActivePage=TabSheet1 then
  begin
    if StartupConnection = '' then
    begin
      PageControl1.ActivePage:=TabSheet2;
      PrevButton.Enabled:=true;
      exit;
    end else
    begin
      PageControl1.ActivePage:=TabSheet3;
      NextButton.Enabled:=false;
      PrevButton.Enabled:=false;
      exit;
    end;
  end;
  if PageControl1.ActivePage=TabSheet2 then
  begin
    PageControl1.ActivePage:=TabSheet3;
    NextButton.Enabled:=false;
    PrevButton.Enabled:=false;
    exit;
  end;
  if PageControl1.ActivePage=TabSheet3 then
  begin
    PageControl1.ActivePage:=TabSheet4;
    PrevButton.Enabled:=false;
    NextButton.Caption:='Cancel';
    exit;
  end;
  if PageControl1.ActivePage=TabSheet4 then
  begin
    if NextButton.Caption = 'Cancel' then
    begin
      BackupThread.Suspend;
      if MessageDlg('Do you want to cancel process ?',
         mtConfirmation, [mbYes, mbNo], 0) = mrYes then
      begin
        CancelThread:=true;
        Form1.NextButton.Caption:='Quit';
      end;
      BackupThread.Resume;
    end else begin
      halt;
    end;
  end;
end;

procedure TForm1.FormCreate(Sender: TObject);
var
   i,level:integer;
   Device: PChar;
   Connection: PChar;
   error: GSM_Error;
   buffer : array[1..100] of char;
   Con:string;
   Style : Integer;
begin
  StartupDevice:='';
  StartupConnection:='';
  level:=0;
  for i := 1 to ParamCount do
  begin
    case level of
    0:
      begin
        if LeftBStr(ParamStr(i),11) = '-connection' then level:=1;
        if LeftBStr(ParamStr(i),7) = '-device' then level:=2;
      end;
    1:
      begin
        StartupConnection:=ParamStr(i);
        level:=0;
      end;
    2:
      begin
        StartupDevice:=ParamStr(i);
        level:=0;
      end;
    end;
  end;

  Connected:=false;
  for i:=1 to PageControl1.PageCount do
  begin
    PageControl1.Pages[i-1].TabVisible:=false;
  end;
  PageControl1.ActivePage:=TabSheet0;
  BackupThread:=TMyThread.Create(True);
  BackupThread.Priority:=tpIdle;

  if StartupConnection<>'' then
  begin
    GetMem(Device,Length(StartupDevice) + 1);
    StrCopy(Device, PChar(StartupDevice));

    GetMem(Connection,Length(StartupConnection) + 1);
    StrCopy(Connection, PChar(StartupConnection));

    PhoneCallBackPointer    := @ChangePhoneState1;
    SecurityCallBackPointer := nil;
    SMSCallBackPointer      := nil;

    error:=GSM_StartConnection(@PhoneID,Device,Connection,'','','',@PhoneCallBackPointer,@SecurityCallBackPointer,@SMSCallBackPointer);
    if (error<>ERR_NONE) then application.MessageBox(pchar('Start: error '+inttostr(integer(error))),'',0);

    FreeMem(Device);
    FreeMem(Connection);

//    Style := GetWindowLong(Application.Handle, GWL_EXSTYLE);
//    SetWindowLong(Application.Handle, GWL_EXSTYLE, Style OR WS_EX_TOOLWINDOW AND NOT WS_EX_APPWINDOW);
  end;
//  Form1.Show();
end;

procedure TForm1.TabSheet3Show(Sender: TObject);
begin
  CheckThread:=TMyThread2.Create(True);
  CheckThread.Priority:=tpIdle;
  CheckThread.Resume;
end;

procedure TForm1.TabSheet2Show(Sender: TObject);
begin
  NextButton.Enabled:=connected;
  ConnectionComboBox.onSelect(Sender);
end;

procedure TForm1.Button8Click(Sender: TObject);
begin
  Application.MessageBox('(c) 2004 by Marcin Wiacek. Created in cooperation with MatrixFlasher team','',0);
end;

procedure TForm1.PrevButtonClick(Sender: TObject);
begin
  if PageControl1.ActivePage=TabSheet4 then
  begin
    PageControl1.ActivePage:=TabSheet3;
    exit;
  end;
  if PageControl1.ActivePage=TabSheet3 then
  begin
    if StartupConnection = '' then
    begin
      PageControl1.ActivePage:=TabSheet2;
      exit;
    end else
    begin
      PageControl1.ActivePage:=TabSheet1;
      if FileNameEdit.Text<>'' then NextButton.Enabled:=true;
      exit;
    end;
  end;
  if PageControl1.ActivePage=TabSheet2 then
  begin
    PageControl1.ActivePage:=TabSheet1;
    if FileNameEdit.Text<>'' then NextButton.Enabled:=true;
    exit;
  end;
  if PageControl1.ActivePage=TabSheet1 then
  begin
    PageControl1.ActivePage:=TabSheet0;
    NextButton.Enabled:=true;
    PrevButton.Enabled:=false;
    exit;
  end;
end;

procedure TForm1.Button5Click(Sender: TObject);
begin
  if CheckBox1.Enabled then CheckBox1.Checked:=true;
  if CheckBox2.Enabled then CheckBox2.Checked:=true;
  if CheckBox3.Enabled then CheckBox3.Checked:=true;
  if CheckBox4.Enabled then CheckBox4.Checked:=true;
end;

procedure TForm1.Button7Click(Sender: TObject);
begin
  if CheckBox1.Enabled then CheckBox1.Checked:=false;
  if CheckBox2.Enabled then CheckBox2.Checked:=false;
  if CheckBox3.Enabled then CheckBox3.Checked:=false;
  if CheckBox4.Enabled then CheckBox4.Checked:=false;
end;

procedure TForm1.TabSheet4Show(Sender: TObject);
begin
  CancelThread:=false;
  BackupThread.Resume;
end;

procedure TMyThread.Execute;
var
  error:GSM_Error;
  percent:integer;
  num:integer;
begin
  if Form1.BackupRadioButton.Checked then
  begin
    GSM_StartBackup(Form1.PhoneID,@Form1.BackupInfo);
    if (Form1.CheckBox1.Enabled) and (Form1.CheckBox1.Checked) then
    begin
      percent:=200;
      num:=0;
      Form1.Memo1.Lines.Add('Making backup of phone phonebook');
      while percent<>100 do
      begin
        if FOrm1.CancelThread then exit;
        error:=GSM_BackupPhonePBK(Form1.PhoneID,@percent);
        Form1.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup phone pbk: error '+inttostr(integer(error))),'',0);
      end;
      Form1.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (Form1.CheckBox2.Enabled) and (Form1.CheckBox2.Checked) then
    begin
      percent:=200;
      num:=0;
      Form1.Memo1.Lines.Add('Making backup of SIM phonebook');
      while percent<>100 do
      begin
        if FOrm1.CancelThread then exit;
        error:=GSM_BackupSIMPBK(Form1.PhoneID,@percent);
        Form1.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
      Form1.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (Form1.CheckBox3.Enabled) and (Form1.CheckBox3.Checked) then
    begin
      percent:=200;
      num:=0;
      Form1.Memo1.Lines.Add('Making backup of calendar');
      Form1.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if FOrm1.CancelThread then exit;
        error:=GSM_BackupCalendar(Form1.PhoneID,@percent);
        if percent = 0 then
        begin
          Form1.ProgressBar1.Max:=500;
          Form1.ProgressBar1.Position:=Form1.ProgressBar1.Position+1;
        end else
        begin
          Form1.ProgressBar1.Position:=percent;
        end;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      Form1.ProgressBar1.Max:=100;
      Form1.ProgressBar1.Position:=100;
      Form1.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (Form1.CheckBox4.Enabled) and (Form1.CheckBox4.Checked) then
    begin
      percent:=200;
      num:=0;
      Form1.Memo1.Lines.Add('Making backup of ToDo');
      while percent<>100 do
      begin
        if FOrm1.CancelThread then exit;
        error:=GSM_BackupToDo(Form1.PhoneID,@percent);
        Form1.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup ToDo: error '+inttostr(integer(error))),'',0);
      end;
      Form1.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    Form1.Memo1.Lines.Add('Saving to file '+Form1.FileNameEdit.Text);
    GSM_EndBackup(PChar(Form1.FileNameEdit.Text),true);
  end else begin
    if (Form1.CheckBox1.Enabled) and (Form1.CheckBox1.Checked) then
    begin
      percent:=200;
      Form1.Memo1.Lines.Add('Restoring phone phonebook');
      while percent<>100 do
      begin
        if FOrm1.CancelThread then exit;
        error:=GSM_RestorePhonePBK(Form1.PhoneID,@percent);
        Form1.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup phone pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (Form1.CheckBox2.Enabled) and (Form1.CheckBox2.Checked) then
    begin
      percent:=200;
      Form1.Memo1.Lines.Add('Restoring SIM phonebook');
      while percent<>100 do
      begin
        if FOrm1.CancelThread then exit;
        error:=GSM_RestoreSIMPBK(Form1.PhoneID,@percent);
        Form1.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
  end;
  Form1.NextButton.Caption:='Quit';
end;

procedure TForm1.BackupRadioButtonClick(Sender: TObject);
begin
  Form1.Caption:='Backup from phone to file';
  FileNameEdit.Text:='';
  Edit1.Text:='';
  Edit4.Text:='';
  Edit7.Text:='';
  Edit10.Text:='';
  Label2.Enabled:=RestoreRadioButton.Checked;
  Label3.Enabled:=RestoreRadioButton.Checked;
  Label4.Enabled:=RestoreRadioButton.Checked;
  Label9.Enabled:=RestoreRadioButton.Checked;
end;

procedure TForm1.RestoreRadioButtonClick(Sender: TObject);
begin
  Form1.Caption:='Restore from file to phone';
  FileNameEdit.Text:='';
  Edit1.Text:='';
  Edit4.Text:='';
  Edit7.Text:='';
  Edit10.Text:='';
  Label2.Enabled:=RestoreRadioButton.Checked;
  Label3.Enabled:=RestoreRadioButton.Checked;
  Label4.Enabled:=RestoreRadioButton.Checked;
  Label9.Enabled:=RestoreRadioButton.Checked;
end;

procedure TMyThread2.Execute;
begin
  with Form1 do
  begin
//    Form1.Cursor:=crHourGlass;
    CheckBox1.Enabled:=false;
    CheckBox2.Enabled:=false;
    CheckBox3.Enabled:=false;
    CheckBox4.Enabled:=false;
    BackupInfo:=BackupInfo0;
    GSM_GetBackupFeaturesForBackup(PhoneID,PChar(FileNameEdit.Text),@BackupInfo);
    CheckBox1.Enabled:=BackupInfo.PhonePhonebook;
    CheckBox2.Enabled:=BackupInfo.SIMPhonebook;
    CheckBox3.Enabled:=BackupInfo.Calendar;
    CheckBox4.Enabled:=BackupInfo.ToDo;
    NextButton.Enabled:=true;
    PrevButton.Enabled:=true;
//    Form1.Cursor:=crDefault;
  end;
end;

procedure TForm1.ConnectionComboBoxSelect(Sender: TObject);
begin
  DeviceComboBox.Enabled:=true;
  Label8.Enabled:=true;
  If ConnectionComboBox.ItemIndex < 2 then
  begin
    DeviceComboBox.Enabled:=false;
    Label8.Enabled:=false;
  end;
end;

procedure TForm1.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
    if NextButton.Caption = 'Cancel' then
    begin
      BackupThread.Suspend;
      if MessageDlg('Do you want to cancel process ?',
         mtConfirmation, [mbYes, mbNo], 0) = mrYes then
      begin
        CancelThread:=true;
        Form1.NextButton.Caption:='Quit';
      end;
      BackupThread.Resume;
      CanClose:=false;
    end else begin
      halt;
    end;
end;

end.
