program EXE;


uses
  Forms,
  Unit1 in '..\Unit1.pas' {BackupForm},
  StrUtils,
  Gammu in '..\..\gammu.pas',
  about in '..\about.pas' {AboutForm};

{$R *.res}

var
   i,level:integer;

begin
  Application.Initialize;
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
  Application.CreateForm(TBackupForm, BackupForm);
  Application.CreateForm(TAboutForm, AboutForm);
  Application.Run;
end.
