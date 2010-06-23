program smsd;

uses
  Forms,
  main in 'MAIN.PAS' {MainForm},
  config in 'CONFIG.PAS' {ConfigForm},
  Gammu in '..\gammu.pas',
  about in 'about.pas' {AboutForm},
  disk_dir in 'disk_dir.pas' {DirectoryForm},
  send_sms in 'send_sms.pas' {SendSMSForm},
  dev_info in 'dev_info.pas' {DeviceInfoForm},
  pbk in 'pbk.pas' {PhonebookForm},
  phonenum in 'phonenum.pas' {AddNumberForm},
  sms_cost in 'sms_cost.pas' {SMSPriceForm};

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'Gammu Gateway';
  Application.HelpFile := '';
  Application.ShowMainForm := false;
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TConfigForm, ConfigForm);
  Application.CreateForm(TAboutForm, AboutForm);
  Application.CreateForm(TDirectoryForm, DirectoryForm);
  Application.CreateForm(TSendSMSForm, SendSMSForm);
  Application.CreateForm(TDeviceInfoForm, DeviceInfoForm);
  Application.CreateForm(TPhonebookForm, PhonebookForm);
  Application.CreateForm(TAddNumberForm, AddNumberForm);
  Application.CreateForm(TSMSPriceForm, SMSPriceForm);
  Application.Run;
end.
