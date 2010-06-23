program smsd;

uses
  Forms,
  main in 'main.pas' {MainForm},
  config in 'config.pas' {ConfigForm},
  Gammu in '..\gammu.pas',
  about in 'about.pas' {AboutForm},
  disk_dir in 'disk_dir.pas' {DirectoryForm},
  send_sms in 'send_sms.pas' {SendSMSForm},
  dev_info in 'dev_info.pas' {DeviceInfoForm},
  pbk in 'pbk.pas' {PhonebookForm},
  phonenum in 'phonenum.pas' {AddNumberForm};

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'SMS daemon';
  Application.HelpFile := '';
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TConfigForm, ConfigForm);
  Application.CreateForm(TAboutForm, AboutForm);
  Application.CreateForm(TDirectoryForm, DirectoryForm);
  Application.CreateForm(TSendSMSForm, SendSMSForm);
  Application.CreateForm(TDeviceInfoForm, DeviceInfoForm);
  Application.CreateForm(TPhonebookForm, PhonebookForm);
  Application.CreateForm(TAddNumberForm, AddNumberForm);
  Application.Run;
end.
