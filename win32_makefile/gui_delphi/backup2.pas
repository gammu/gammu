unit backup2;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls, GnokiiAPI;

type
  TBackupThread = class(TThread)
    public
      procedure Execute; override;
  end;

type
  TBackupProgressForm = class(TForm)
    GroupBox1: TGroupBox;
    Button1: TButton;
    Label1: TLabel;
    ProgressBar1: TProgressBar;
    ProgressBar2: TProgressBar;
    Label2: TLabel;
    Label3: TLabel;
    ProgressBar3: TProgressBar;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    procedure FormActivate(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  BackupProgressForm: TBackupProgressForm;
  BackupThread: TBackupThread;

implementation

uses MainUnit, backup;

{$R *.dfm}

procedure TBackupThread.Execute;
var
  pbkstatus:GSM_MemoryStatus;
  bitmap:GSM_Bitmap;
  pbkentry:GSM_PhonebookEntry;
  i,j:integer;
begin
  if PhoneBackupAvailable then //backup
  begin
    if backupform.checkbox3.checked then
    begin
      PhoneBackup.CallerAvailable:=1;
      for i:=1 to 5 do
      begin
        bitmap.type2:=GSM_CallerLogo;
        bitmap.number:=i-1;
        GSM_GetBitmap(@bitmap);
        PhoneBackup.CallerGroups[i]:=bitmap;
        backupprogressform.ProgressBar3.StepIt;
        if Terminated then exit;
      end;
    end;
    if backupform.checkbox4.checked then
    begin
      bitmap.type2:=GSM_7110OperatorLogo;
      if (GSM_GetBitmap(@bitmap)<>0) then
      begin
        if Terminated then exit;
        bitmap.type2:=GSM_OperatorLogo;
        if (GSM_GetBitmap(@bitmap)=0) then
        begin
          PhoneBackup.OperatorLogoAvailable:=1;
        end
      end else PhoneBackup.OperatorLogoAvailable:=1;
      if Terminated then exit;
      if PhoneBackup.OperatorLogoAvailable=1 then
      begin
        PhoneBackup.OperatorLogo:=Bitmap;
        BackupProgressForm.Label5.Caption:='Done';
      end else
      begin
        BackupProgressForm.Label5.Caption:='Failed';
      end;
    end;
    if backupform.checkbox2.checked then
    begin
      pbkstatus.MemoryType:=GMT_ME;
      GSM_GetMemoryStatus(@pbkstatus);
      if Terminated then exit;
      backupprogressform.ProgressBar1.Max:=pbkstatus.Used;
      PhoneBackup.PhonePhonebookSize:=pbkstatus.Used+pbkstatus.Free;
      i:=1;j:=0;
      while (i<>pbkstatus.Used+pbkstatus.Free) do
      begin
        pbkentry.MemoryType:=GMT_ME;
        pbkentry.Location:=i;
        GSM_GetMemoryLocation(@pbkentry);
        if Terminated then exit;
        if (pbkentry.Name[1]<>chr(0)) then
        begin
          PhoneBackup.PhonePhonebookUsed:=PhoneBackup.PhonePhonebookUsed+1;
          PhoneBackup.PhonePhonebook[PhoneBackup.PhonePhonebookUsed]:=pbkentry;
          backupprogressform.ProgressBar1.StepIt;
          j:=j+1;
        end;
        if j=pbkstatus.Used then break;
        i:=i+1;
      end;
    end;
    if backupform.checkbox1.checked then
    begin
      pbkstatus.MemoryType:=GMT_SM;
      GSM_GetMemoryStatus(@pbkstatus);
      if Terminated then exit;
      backupprogressform.ProgressBar2.Max:=pbkstatus.Used;
      PhoneBackup.SIMPhonebookSize:=pbkstatus.Used+pbkstatus.Free;
      i:=1;j:=0;
      while (i<>pbkstatus.Used+pbkstatus.Free) do
      begin
        pbkentry.MemoryType:=GMT_SM;
        pbkentry.Location:=i;
        GSM_GetMemoryLocation(@pbkentry);
        if Terminated then exit;
        if (pbkentry.Name[1]<>chr(0)) then
        begin
          PhoneBackup.SIMPhonebookUsed:=PhoneBackup.SIMPhonebookUsed+1;
          PhoneBackup.SIMPhonebook[PhoneBackup.SIMPhonebookUsed]:=pbkentry;
          backupprogressform.ProgressBar2.StepIt;
          j:=j+1;
        end;
        if j=pbkstatus.Used then break;
        i:=i+1;
      end;
    end;
    if backupform.checkbox5.checked then
    begin
      case GSM_GetModelFeature(FN_STARTUP) of
        F_STANIM  :PhoneBackup.StartupLogo.type2:=GSM_StartupLogo;
        F_STA     :PhoneBackup.StartupLogo.type2:=GSM_StartupLogo;
        F_STA62   :PhoneBackup.StartupLogo.type2:=GSM_6210StartupLogo;
        F_STA71   :PhoneBackup.StartupLogo.type2:=GSM_7110StartupLogo;
      end;
      if GSM_GetModelFeature(FN_STARTUP)<>0 then
      begin
        PhoneBackup.StartupLogoAvailable:=1;
        GSM_GetBitmap(@PhoneBackup.StartupLogo);
        if Terminated then exit;
      end;
      PhoneBackup.StartupText.type2:=GSM_WelcomeNoteText;
      GSM_GetBitmap(@PhoneBackup.StartupText);
      BackupProgressForm.Label7.Caption:='Done';
      if Terminated then exit;
    end;
    if not Terminated then
    begin
      if MainForm.SaveDialog1.Execute then
      begin
        GSM_SaveBackupFile(PChar(MainForm.SaveDialog1.FileName),@phonebackup);
      end;
      BackupProgressForm.Close;
    end;
  end else //restore
  begin
    if backupform.checkbox3.checked then
    begin
      for i:=1 to 5 do
      begin
        GSM_SetBitmap(@PhoneBackup.CallerGroups[i]);
        backupprogressform.ProgressBar3.StepIt;
        if Terminated then exit;
      end;
    end;
    if backupform.checkbox4.checked then
    begin
      GSM_SetBitmap(@PhoneBackup.OperatorLogo);
      if Terminated then exit;
      BackupProgressForm.Label5.Caption:='Done';
    end;
    if backupform.checkbox2.checked then
    begin
      pbkstatus.MemoryType:=GMT_ME;
      GSM_GetMemoryStatus(@pbkstatus);
      if Terminated then exit;
      backupprogressform.ProgressBar1.Max:=pbkstatus.Used+pbkstatus.Free;
      i:=1;j:=1;
      while (i<>pbkstatus.Used+pbkstatus.Free) do
      begin
        pbkentry.MemoryType:=GMT_ME;
        pbkentry.Location:=i;
        pbkentry.Name[1]:=chr(0);
        pbkentry.Number[1]:=chr(0);
        pbkentry.SubEntriesCount:=0;
        if j<=PhoneBackup.PhonePhonebookUsed then
        begin
          if PhoneBackup.PhonePhonebook[j].Location=pbkentry.Location then
          begin
            pbkentry:=PhoneBackup.PhonePhonebook[j];
            j:=j+1;
          end;
        end;
        GSM_WritePhonebookLocation(@pbkentry);
        if Terminated then exit;
        backupprogressform.ProgressBar1.StepIt;
        i:=i+1;
      end;
    end;
    if backupform.checkbox1.checked then
    begin
      pbkstatus.MemoryType:=GMT_SM;
      GSM_GetMemoryStatus(@pbkstatus);
      if Terminated then exit;
      backupprogressform.ProgressBar2.Max:=pbkstatus.Used+pbkstatus.Free;
      i:=1;j:=1;
      while (i<>pbkstatus.Used+pbkstatus.Free) do
      begin
        pbkentry.MemoryType:=GMT_SM;
        pbkentry.Location:=i;
        pbkentry.Name[1]:=chr(0);
        pbkentry.Number[1]:=chr(0);
        pbkentry.SubEntriesCount:=0;
        if j<=PhoneBackup.SIMPhonebookUsed then
        begin
          if PhoneBackup.SIMPhonebook[j].Location=pbkentry.Location then
          begin
            pbkentry:=PhoneBackup.SIMPhonebook[j];
            j:=j+1;
          end;
        end;
        GSM_WritePhonebookLocation(@pbkentry);
        if Terminated then exit;
        backupprogressform.ProgressBar2.StepIt;
        i:=i+1;
      end;
    end;
    if backupform.checkbox5.checked then
    begin
      if GSM_GetModelFeature(FN_STARTUP)<>0 then
      begin
        if PhoneBackup.StartupLogoAvailable<>0 then
        begin
          GSM_SetBitmap(@PhoneBackup.StartupLogo);
          if Terminated then exit;
        end;
      end;
      GSM_SetBitmap(@PhoneBackup.StartupText);
    end;
    if not Terminated then
    begin
      BackupProgressForm.Close;
    end;
  end;
end;

procedure TBackupProgressForm.FormActivate(Sender: TObject);
begin
  MainForm.Timer2.Enabled:=false;

  BackupProgressForm.Label5.Caption:='';
  BackupProgressForm.Label7.Caption:='';

  ProgressBar1.Position:=0;
  ProgressBar2.Position:=0;
  ProgressBar3.Position:=0;

  Label3.Enabled:=backupform.checkbox3.checked;
  Label4.Enabled:=backupform.checkbox4.checked;
  Label1.Enabled:=backupform.checkbox2.checked;
  Label2.Enabled:=backupform.checkbox1.checked;
  Label6.Enabled:=backupform.checkbox5.checked;

  BackupThread:=TBackupThread.Create(false);
end;

procedure TBackupProgressForm.Button1Click(Sender: TObject);
begin
  BackupProgressForm.Close;
end;

procedure TBackupProgressForm.FormCloseQuery(Sender: TObject;
  var CanClose: Boolean);
begin
  BackupThread.Terminate();
  BackupProgressForm.Close;
  MainForm.Timer2.Enabled:=true;
  CanClose:=true;
end;

end.
