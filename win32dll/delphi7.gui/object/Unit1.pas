unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Gammu, StdCtrls;

type
  TForm1 = class(TForm)
    Button1: TButton;
    GroupBox1: TGroupBox;
    ListBox1: TListBox;
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

type
  TGammuGSMDevice = class(TObject)
  private
    fDeviceID                 : integer;
    fDevice                   : PChar;
    fConnection               : PChar;
    fPhoneCallBackPointer     : PPhoneCallBackProcClass;
    fSecurityCallBackPointer  : PSecurityCallBackProcClass;
    fSMSCallBackPointer       : PSMSCallBackProcClass;
  public
    function StartConnection(DeviceString:PChar;ConnectionString:PChar): GSM_Error;

    property Device:PChar read fDevice;
    property Connection:PChar read fConnection;
  published
    procedure SecurityCallBack(ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
    procedure SMSCallBack(ID:integer);stdcall;
    procedure PhoneCallBack(ID:integer;connected:boolean);stdcall;
  end;

var
  Form1           : TForm1;
  GammuGSMDevice  : TGammuGSMDevice;
//  GammuGSMDevice1 : TGammuGSMDevice;

implementation

{$R *.dfm}

function TGammuGSMDevice.StartConnection(DeviceString:PChar;ConnectionString:PChar): GSM_Error;
begin
   GetMem(fDevice,Length(DeviceString) + 1);
   StrCopy(fDevice,DeviceString);
   GetMem(fConnection,50);
   StrCopy(fConnection,ConnectionString);

   fPhoneCallBackPointer    := self.MethodAddress('phonecallback');
   if (fPhoneCallBackPointer = nil) then halt;
   fSecurityCallBackPointer := self.MethodAddress('securitycallback');
   if (fSecurityCallBackPointer = nil) then halt;
   fSMSCallBackPointer      := self.MethodAddress('smscallback');
   if (fSMSCallBackPointer = nil) then halt;

   Form1.ListBox1.Items.Add('Starting on device '+fDevice);

   StartConnection:=GSM_StartConnection(@fDeviceID,fDevice,fConnection,'','logfile','text',@fPhoneCallBackPointer,@fSecurityCallBackPointer,@fSMSCallBackPointer);
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  GammuGSMDevice := TGammuGSMDevice.Create;
  GammuGSMDevice.StartConnection('com2:','');
//  GammuGSMDevice1 := TGammuGSMDevice.Create;
//  GammuGSMDevice1.StartConnection('com1:','');
end;

procedure TGammuGSMDevice.PhoneCallBack(ID:integer;connected:boolean);stdcall;
begin
  Form1.ListBox1.Items.Add('phone callback from phone '+inttostr(ID));
  if (connected) then
  begin
    Form1.ListBox1.Items.Add('connected');
  end else begin
    Form1.ListBox1.Items.Add('not connected');
  end;
end;

procedure TGammuGSMDevice.SecurityCallBack(ID:integer;SecurityState:GSM_SecurityCodeType);stdcall;
begin
  Form1.ListBox1.Items.Add('security callback from phone '+inttostr(ID));
  case SecurityState of
      GSCT_SecurityCode:Form1.ListBox1.Items.Add('Security code');
      GSCT_Pin         :Form1.ListBox1.Items.Add('PIN');
      GSCT_Pin2        :Form1.ListBox1.Items.Add('PIN2');
      GSCT_Puk         :Form1.ListBox1.Items.Add('PUK');
      GSCT_Puk2        :Form1.ListBox1.Items.Add('PUK2');
      GSCT_None        :Form1.ListBox1.Items.Add('');
  end;
end;

procedure TGammuGSMDevice.SMSCallBack(ID:integer);stdcall;
begin
  Form1.ListBox1.Items.Add('sms callback from phone '+inttostr(ID));
end;

end.
