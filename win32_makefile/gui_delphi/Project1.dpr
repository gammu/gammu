program Project1;

uses
  Forms,
  MainUnit in 'mainunit.pas' {MainForm},
  GnokiiAPI in 'GnokiiAPI.pas',
  LogosRing in 'LogosRing.pas' {LogosRingForm},
  Netmon in 'netmon.pas' {NetmonForm},
  Calendar in 'Calendar.pas' {CalendarForm},
  SMSEdit in 'smsedit.pas' {SMSEditForm},
  smscedit in 'smscedit.pas' {SMSCEditForm},
  pbkedit in 'pbkedit.pas' {PbkForm},
  backup in 'backup.pas' {BackupForm},
  backup2 in 'backup2.pas' {BackupProgressForm},
  pbkedit2 in 'pbkedit2.pas' {PBKEditForm},
  wapbook in 'wapbook.pas' {WAPBookForm};

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'Mygnokii for WIN32';
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TLogosRingForm, LogosRingForm);
  Application.CreateForm(TNetmonForm, NetmonForm);
  Application.CreateForm(TCalendarForm, CalendarForm);
  Application.CreateForm(TSMSEditForm, SMSEditForm);
  Application.CreateForm(TSMSCEditForm, SMSCEditForm);
  Application.CreateForm(TPbkForm, PbkForm);
  Application.CreateForm(TNetmonForm, NetmonForm);
  Application.CreateForm(TBackupForm, BackupForm);
  Application.CreateForm(TBackupProgressForm, BackupProgressForm);
  Application.CreateForm(TPBKEditForm, PBKEditForm);
  Application.CreateForm(TWAPBookForm, WAPBookForm);
  Application.Run;
end.
