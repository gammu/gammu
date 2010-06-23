unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Gammu, ExtCtrls, ComCtrls, StrUtils, About;

type
  TBackupForm = class(TForm)
    Button1: TButton;
    PageControl1: TPageControl;
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
    TabSheet1: TTabSheet;
    GroupBox2: TGroupBox;
    BackupRadioButton: TRadioButton;
    RestoreRadioButton: TRadioButton;
    SaveBackupDialog: TSaveDialog;
    Label8: TLabel;
    Label10: TLabel;
    Edit2: TEdit;
    CheckBox5: TCheckBox;
    CheckBox6: TCheckBox;
    CheckBox7: TCheckBox;
    Label11: TLabel;
    FileNameEdit: TEdit;
    Button2: TButton;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label9: TLabel;
    Edit1: TEdit;
    Edit4: TEdit;
    Edit7: TEdit;
    Edit10: TEdit;
    Panel1: TPanel;
    CheckBox8: TCheckBox;
    CheckBox9: TCheckBox;
    procedure Button1Click(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure NextButtonClick(Sender: TObject);
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
    procedure Button2Click(Sender: TObject);
    procedure CheckBox1Click(Sender: TObject);
    procedure CheckBox2Click(Sender: TObject);
    procedure CheckBox3Click(Sender: TObject);
    procedure CheckBox4Click(Sender: TObject);
    procedure CheckBox5Click(Sender: TObject);
    procedure CheckBox6Click(Sender: TObject);
    procedure CheckBox7Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure TabSheet1Show(Sender: TObject);
    procedure TabSheet3Show(Sender: TObject);
    procedure CheckBox8Click(Sender: TObject);
    procedure CheckBox10Click(Sender: TObject);
    procedure CheckBox9Click(Sender: TObject);
  private
    { Private declarations }
        PhoneID                        : integer;
        Connected                      : boolean;
        PhoneCallBackPointer           : PPhoneCallBackProc;
        SecurityCallBackPointer        : PSecurityCallBackProc;
        SMSCallBackPointer             : PSMSCallBackProc;
        BackupInfo,BackupInfo0         : GSM_Backup_Info;
        CancelThread                   : boolean;
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
  TMyThread3 = class(TThread)
  private
    { Private declarations }
  protected
    procedure Execute; override;
  end;

var
  BackupForm: TBackupForm;
  BackupThread:TMyThread;
  CheckThread:TMyThread2;
  ConnectThread:TMyThread3;
  StartupDevice                  : string;
  StartupConnection              : string;

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
    error:=GSM_GetModelName(BackupForm.PhoneID,@buffer);
    if (error = ERR_NONE) then
    begin
      error:=GSM_GetManufacturer(BackupForm.PhoneID,@buffer2);
      if (error = ERR_NONE) then
      begin
        BackupForm.ModelEdit.Text:=String(buffer2);
        BackupForm.ModelEdit.Text:=BackupForm.ModelEdit.Text+' '+buffer;
      end else
      begin
        BackupForm.ModelEdit.Text:=buffer;
      end;
      BackupForm.StatusBar1.Panels.Items[1].Text:=BackupForm.ModelEdit.Text;
    end;
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get model: error '+inttostr(integer(error))),'',0);

		error:=GSM_GetIMEI(BackupForm.PhoneID,@buffer);
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get IMEI: error '+inttostr(integer(error))),'',0);
    if (error = ERR_NONE) then BackupForm.IMEIEdit.Text:=buffer;

    error:=GSM_GetFirmwareVersion(BackupForm.PhoneID,@ver);
    if (error = ERR_NONE) then BackupForm.Edit2.Text:=floattostr(ver);

    if BackupForm.PageControl1.ActivePage=BackupForm.TabSheet2 then BackupForm.NextButton.Enabled:=true;

//    error:=GSM_EndConnection(BackupForm.PhoneID);
//    if (error <> ERR_NONE) then application.MessageBox(pchar('End connection: error '+inttostr(integer(error))),'',0);
  end else begin
    BackupForm.StatusBar1.Panels.Items[1].Text:='DISCONECTED';
	end;
  BackupForm.connected:=status;
end;

procedure TMyThread3.Execute;
var
   Device: PChar;
   Connection: PChar;
   error: GSM_Error;
   Con:string;
begin
   with BackupForm do
   begin
     GetMem(Device,Length(DeviceComboBox.Text) + 1);
     StrCopy(Device, PChar(DeviceComboBox.Text));

     case ConnectionComboBox.ItemIndex of
       0: Con:='irdaphonet';
       1: Con:='fbusdku5';
       2: con:='fbus';
       3: con:='fbusdlr3';
     end;
     GetMem(Connection,Length(Con) + 1);
     StrCopy(Connection, PChar(Con));

     PhoneCallBackPointer    := @ChangePhoneState1;
     SecurityCallBackPointer := nil;
     SMSCallBackPointer      := nil;

     error:=GSM_StartConnection(@PhoneID,Device,Connection,'','','',false,@PhoneCallBackPointer,@SecurityCallBackPointer,@SMSCallBackPointer);
     if (error<>ERR_NONE) then
     begin
        application.MessageBox(pchar('Start: error '+inttostr(integer(error))),'',0);
        StatusBar1.Panels.Items[0].Text:='Select parameters and click "Try to connect"';
     end else
     begin
        StatusBar1.Panels.Items[0].Text:='Click "Next"';
     end;

     FreeMem(Device);
     FreeMem(Connection);

     Button1.Enabled:=true;
   end;
end;

procedure TBackupForm.Button1Click(Sender: TObject);
begin
   Button1.Enabled:=false;
   StatusBar1.Panels.Items[0].Text:='Checking connection. Please wait';
   if Connected then GSM_EndConnection(PhoneID);
   ConnectThread:=TMyThread3.Create(True);
   ConnectThread.Priority:=tpTimeCritical;
   ConnectThread.Resume;
end;

procedure TBackupForm.Button6Click(Sender: TObject);
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

function CheckSelected:boolean;
begin
  with BackupForm do
  begin
    CheckSelected:=false;
    if CheckBox1.Enabled and CheckBox1.Checked then CheckSelected:=true;
    if CheckBox2.Enabled and CheckBox2.Checked then CheckSelected:=true;
    if CheckBox3.Enabled and CheckBox3.Checked then CheckSelected:=true;
    if CheckBox4.Enabled and CheckBox4.Checked then CheckSelected:=true;
    if CheckBox5.Enabled and CheckBox5.Checked then CheckSelected:=true;
    if CheckBox6.Enabled and CheckBox6.Checked then CheckSelected:=true;
    if CheckBox7.Enabled and CheckBox7.Checked then CheckSelected:=true;
    if CheckBox8.Enabled and CheckBox8.Checked then CheckSelected:=true;
    if CheckBox9.Enabled and CheckBox9.Checked then CheckSelected:=true;
  end;
end;

procedure TBackupForm.NextButtonClick(Sender: TObject);
begin
  if PageControl1.ActivePage=TabSheet1 then
  begin
    if StartupConnection = '' then
    begin
      PageControl1.ActivePage:=TabSheet2;
      PrevButton.Enabled:=true;
      exit;
    end else
    begin
      PrevButton.Enabled:=true;
      PageControl1.ActivePage:=TabSheet3;
      if FileNameEdit.Text<>'' then
      begin
        NextButton.Enabled:=CheckSelected;
      end else
      begin
        Button5.Enabled:=false;
        Button7.Enabled:=false;
        CheckBox1.Enabled:=false;
        CheckBox2.Enabled:=false;
        CheckBox3.Enabled:=false;
        CheckBox4.Enabled:=false;
        CheckBox5.Enabled:=false;
        CheckBox6.Enabled:=false;
        CheckBox7.Enabled:=false;
        CheckBox8.Enabled:=false;
        CheckBox9.Enabled:=false;
        NextButton.Enabled:=false;
      end;
      exit;
    end;
  end;
  if PageControl1.ActivePage=TabSheet2 then
  begin
    PageControl1.ActivePage:=TabSheet3;
    if FileNameEdit.Text<>'' then
    begin
      NextButton.Enabled:=CheckSelected;
    end else
    begin
      Button5.Enabled:=false;
      Button7.Enabled:=false;
      CheckBox1.Enabled:=false;
      CheckBox2.Enabled:=false;
      CheckBox3.Enabled:=false;
      CheckBox4.Enabled:=false;
      CheckBox5.Enabled:=false;
      CheckBox6.Enabled:=false;
      CheckBox7.Enabled:=false;
      CheckBox8.Enabled:=false;
      CheckBox9.Enabled:=false;
      NextButton.Enabled:=false;
    end;
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
        BackupForm.NextButton.Caption:='Quit';
      end;
      BackupThread.Resume;
    end else begin
{$IFDEF DLL}
      modalresult:=mrnone;
      close;
{$ELSE}
      halt;
{$ENDIF}
    end;
  end;
end;

procedure TBackupForm.TabSheet2Show(Sender: TObject);
begin
  NextButton.Enabled:=connected;
  if connected then
  begin
    StatusBar1.Panels.Items[0].Text:='Click "Next"';
  end else
  begin
    StatusBar1.Panels.Items[0].Text:='Select parameters and click "Try to connect"';
  end;
  ConnectionComboBox.onSelect(Sender);
end;

procedure TBackupForm.Button8Click(Sender: TObject);
begin
  AboutForm.ShowModal();
end;

procedure TBackupForm.PrevButtonClick(Sender: TObject);
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
      NextButton.Enabled:=true;
      PrevButton.Enabled:=false;
      exit;
    end;
  end;
  if PageControl1.ActivePage=TabSheet2 then
  begin
    PageControl1.ActivePage:=TabSheet1;
    NextButton.Enabled:=true;
    PrevButton.Enabled:=false;
  end;
end;

procedure TBackupForm.Button5Click(Sender: TObject);
begin
  if CheckBox1.Enabled then CheckBox1.Checked:=true;
  if CheckBox2.Enabled then CheckBox2.Checked:=true;
  if CheckBox3.Enabled then CheckBox3.Checked:=true;
  if CheckBox4.Enabled then CheckBox4.Checked:=true;
  if CheckBox5.Enabled then CheckBox5.Checked:=true;
  if CheckBox6.Enabled then CheckBox6.Checked:=true;
  if CheckBox7.Enabled then CheckBox7.Checked:=true;
  if CheckBox8.Enabled then CheckBox8.Checked:=true;
  if CheckBox9.Enabled then CheckBox9.Checked:=true;
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.Button7Click(Sender: TObject);
begin
  if CheckBox1.Enabled then CheckBox1.Checked:=false;
  if CheckBox2.Enabled then CheckBox2.Checked:=false;
  if CheckBox3.Enabled then CheckBox3.Checked:=false;
  if CheckBox4.Enabled then CheckBox4.Checked:=false;
  if CheckBox5.Enabled then CheckBox5.Checked:=false;
  if CheckBox6.Enabled then CheckBox6.Checked:=false;
  if CheckBox7.Enabled then CheckBox7.Checked:=false;
  if CheckBox8.Enabled then CheckBox8.Checked:=false;
  if CheckBox9.Enabled then CheckBox9.Checked:=false;
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.TabSheet4Show(Sender: TObject);
begin
  StatusBar1.Panels.Items[0].Text:='Wait or click "Cancel"';
  CancelThread:=false;
  BackupThread.Resume;
end;

procedure TMyThread.Execute;
var
  error:GSM_Error;
  percent:integer;
  num:integer;
begin
  if BackupForm.BackupRadioButton.Checked then
  begin
    GSM_StartBackup(BackupForm.PhoneID,@BackupForm.BackupInfo);
    if (BackupForm.CheckBox1.Enabled) and (BackupForm.CheckBox1.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of phone phonebook');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupPhonePBK(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup phone pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox2.Enabled) and (BackupForm.CheckBox2.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of SIM phonebook');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupSIMPBK(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox3.Enabled) and (BackupForm.CheckBox3.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of calendar');
      BackupForm.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupCalendar(BackupForm.PhoneID,@percent);
        if percent = 0 then
        begin
          BackupForm.ProgressBar1.Max:=500;
          BackupForm.ProgressBar1.Position:=BackupForm.ProgressBar1.Position+1;
        end else
        begin
          BackupForm.ProgressBar1.Position:=percent;
        end;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.ProgressBar1.Max:=100;
      BackupForm.ProgressBar1.Position:=100;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox4.Enabled) and (BackupForm.CheckBox4.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of ToDo');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupToDo(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup ToDo: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox5.Enabled) and (BackupForm.CheckBox5.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of WAP bookmarks');
      BackupForm.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupWAPBookmark(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Max:=100;
        BackupForm.ProgressBar1.Position:=BackupForm.ProgressBar1.Position+1;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.ProgressBar1.Max:=100;
      BackupForm.ProgressBar1.Position:=100;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox6.Enabled) and (BackupForm.CheckBox6.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of WAP settings');
      BackupForm.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupWAPSettings(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Max:=40;
        BackupForm.ProgressBar1.Position:=BackupForm.ProgressBar1.Position+1;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.ProgressBar1.Max:=100;
      BackupForm.ProgressBar1.Position:=100;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox7.Enabled) and (BackupForm.CheckBox7.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of MMS settings');
      BackupForm.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupMMSSettings(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Max:=40;
        BackupForm.ProgressBar1.Position:=BackupForm.ProgressBar1.Position+1;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.ProgressBar1.Max:=100;
      BackupForm.ProgressBar1.Position:=100;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox8.Enabled) and (BackupForm.CheckBox8.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of FM radio stations');
      BackupForm.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupFMRadio(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Max:=40;
        BackupForm.ProgressBar1.Position:=BackupForm.ProgressBar1.Position+1;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.ProgressBar1.Max:=100;
      BackupForm.ProgressBar1.Position:=100;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    if (BackupForm.CheckBox7.Enabled) and (BackupForm.CheckBox7.Checked) then
    begin
      percent:=200;
      num:=0;
      BackupForm.Memo1.Lines.Add('Making backup of GPRS access points');
      BackupForm.ProgressBar1.Position:=0;
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_BackupGPRSPoint(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Max:=10;
        BackupForm.ProgressBar1.Position:=BackupForm.ProgressBar1.Position+1;
        if error = ERR_EMPTY then break;
        num:=num+1;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup sim pbk: error '+inttostr(integer(error))),'',0);
      end;
      BackupForm.ProgressBar1.Max:=100;
      BackupForm.ProgressBar1.Position:=100;
      BackupForm.Memo1.Lines.Add('   Read with success '+inttostr(num)+' entries');
    end;
    BackupForm.Memo1.Lines.Add('Saving to file '+BackupForm.FileNameEdit.Text);
    GSM_EndBackup(PChar(BackupForm.FileNameEdit.Text),true);
  end else begin
    if (BackupForm.CheckBox1.Enabled) and (BackupForm.CheckBox1.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring phone phonebook');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestorePhonePBK(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup phone pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox2.Enabled) and (BackupForm.CheckBox2.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring SIM phonebook');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreSIMPBK(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox3.Enabled) and (BackupForm.CheckBox3.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring calendar notes');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreCalendar(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox4.Enabled) and (BackupForm.CheckBox4.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring ToDo');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreToDo(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox5.Enabled) and (BackupForm.CheckBox5.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring WAP bookmarks');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreWAPBookmark(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox6.Enabled) and (BackupForm.CheckBox6.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring WAP settings');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreWAPSettings(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox7.Enabled) and (BackupForm.CheckBox7.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring MMS settings');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreMMSSettings(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox8.Enabled) and (BackupForm.CheckBox8.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring FM radio');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreFMRadio(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
    if (BackupForm.CheckBox9.Enabled) and (BackupForm.CheckBox9.Checked) then
    begin
      percent:=200;
      BackupForm.Memo1.Lines.Add('Restoring GPRS points');
      while percent<>100 do
      begin
        if BackupForm.CancelThread then exit;
        error:=GSM_RestoreGPRSPoint(BackupForm.PhoneID,@percent);
        BackupForm.ProgressBar1.Position:=percent;
        if error = ERR_EMPTY then break;
        if error<>ERR_NONE then application.MessageBox(pchar('Backup SIM pbk: error '+inttostr(integer(error))),'',0);
      end;
    end;
  end;
  BackupForm.StatusBar1.Panels.Items[0].Text:='Click "Quit". Thank you for using this software';
  BackupForm.NextButton.Caption:='Quit';
end;

procedure TBackupForm.BackupRadioButtonClick(Sender: TObject);
begin
  BackupForm.Caption:='Backup from phone to file';
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

procedure TBackupForm.RestoreRadioButtonClick(Sender: TObject);
begin
  BackupForm.Caption:='Restore from file to phone';
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
  with BackupForm do
  begin
    Button2.Enabled:=false;
    PrevButton.Enabled:=false;
    NextButton.Enabled:=false;
    Button5.Enabled:=false;
    Button7.Enabled:=false;
    CheckBox1.Enabled:=false;
    CheckBox2.Enabled:=false;
    CheckBox3.Enabled:=false;
    CheckBox4.Enabled:=false;
    CheckBox5.Enabled:=false;
    CheckBox6.Enabled:=false;
    CheckBox7.Enabled:=false;
    CheckBox8.Enabled:=false;
    CheckBox9.Enabled:=false;
    BackupInfo:=BackupInfo0;
    GSM_GetBackupFeaturesForBackup(PhoneID,PChar(FileNameEdit.Text),@BackupInfo);
    CheckBox1.Enabled:=BackupInfo.PhonePhonebook;
    CheckBox2.Enabled:=BackupInfo.SIMPhonebook;
    CheckBox3.Enabled:=BackupInfo.Calendar;
    CheckBox4.Enabled:=BackupInfo.ToDo;
    CheckBox5.Enabled:=BackupInfo.WAPBookmark;
    CheckBox6.Enabled:=BackupInfo.WAPSettings;
    CheckBox7.Enabled:=BackupInfo.MMSSettings;
    CheckBox8.Enabled:=BackupInfo.FMStation;
    CheckBox9.Enabled:=BackupInfo.GPRSPoint;
    Button2.Enabled:=true;
    Button5.Enabled:=true;
    Button7.Enabled:=true;
    NextButton.Enabled:=CheckSelected;
    PrevButton.Enabled:=true;
    StatusBar1.Panels.Items[0].Text:='Select features and click "Next"';
  end;
end;

procedure TBackupForm.ConnectionComboBoxSelect(Sender: TObject);
begin
  DeviceComboBox.Enabled:=true;
  Label8.Enabled:=true;
  If ConnectionComboBox.ItemIndex < 2 then
  begin
    DeviceComboBox.Enabled:=false;
    Label8.Enabled:=false;
  end;
end;

procedure TBackupForm.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
    if NextButton.Caption = 'Cancel' then
    begin
      BackupThread.Suspend;
      if MessageDlg('Do you want to cancel process ?',
         mtConfirmation, [mbYes, mbNo], 0) = mrYes then
      begin
        CancelThread:=true;
        BackupForm.StatusBar1.Panels.Items[0].Text:='Click "Quit". Thank you for using this software';
        BackupForm.NextButton.Caption:='Quit';
      end;
      BackupThread.Resume;
      CanClose:=false;
    end else begin
{$IFDEF DLL}
      if Connected then GSM_EndConnection(PhoneID);
      modalresult:=mrnone;
      close;
{$ELSE}
      halt;
{$ENDIF}
    end;
end;

procedure TBackupForm.Button2Click(Sender: TObject);
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
    StatusBar1.Panels.Items[0].Text:='Checking phone features. Please wait';
    CheckThread:=TMyThread2.Create(True);
    CheckThread.Priority:=tpTimeCritical;
    CheckThread.Resume;
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
      StatusBar1.Panels.Items[0].Text:='Checking phone features. Please wait';
      CheckThread:=TMyThread2.Create(True);
      CheckThread.Priority:=tpTimeCritical;
      CheckThread.Resume;
    end;
  end;
end;

procedure TBackupForm.CheckBox1Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox2Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox3Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox4Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox5Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox6Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox7Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.FormCreate(Sender: TObject);
var
   i:integer;
   Device: PChar;
   Connection: PChar;
   error: GSM_Error;
begin
  if Width - 100 > 0 then StatusBar1.Panels.Items[0].Width:=Width-100;
  Connected:=false;
  for i:=1 to PageControl1.PageCount do
  begin
    PageControl1.Pages[i-1].TabVisible:=false;
  end;
  PageControl1.ActivePage:=TabSheet1;
  BackupThread:=TMyThread.Create(True);
  BackupThread.Priority:=tpTimeCritical;
{$IFDEF DLL}
  BorderStyle:=bsDialog;
  FormStyle:=fsstayontop;
{$ENDIF}

  if StartupConnection<>'' then
  begin
    GetMem(Device,Length(StartupDevice) + 1);
    StrCopy(Device, PChar(StartupDevice));

    GetMem(Connection,Length(StartupConnection) + 1);
    StrCopy(Connection, PChar(StartupConnection));

    PhoneCallBackPointer    := @ChangePhoneState1;
    SecurityCallBackPointer := nil;
    SMSCallBackPointer      := nil;

    error:=GSM_StartConnection(@PhoneID,Device,Connection,'','','',false,@PhoneCallBackPointer,@SecurityCallBackPointer,@SMSCallBackPointer);

    FreeMem(Device);
    FreeMem(Connection);

    if error<>ERR_NONE then
    begin
{$IFDEF DLL}
      StartupCOnnection:='';
      StartupDevice:='';
//      BorderStyle:=bsDialog;
{$ELSE}
      halt;
{$ENDIF}
    end;
  end;
end;

procedure TBackupForm.TabSheet1Show(Sender: TObject);
begin
  StatusBar1.Panels.Items[0].Text:='Select operation and click "Next"';
end;

procedure TBackupForm.TabSheet3Show(Sender: TObject);
begin
  if FileNameEdit.Text<>'' then
  begin
    StatusBar1.Panels.Items[0].Text:='Select features and click "Next"'; 
  end else
  begin
    StatusBar1.Panels.Items[0].Text:='Select file for backup/restore';
  end;
end;

procedure TBackupForm.CheckBox8Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox10Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

procedure TBackupForm.CheckBox9Click(Sender: TObject);
begin
  NextButton.Enabled:=CheckSelected;
end;

end.
