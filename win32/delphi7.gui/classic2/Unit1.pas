unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Gammu, ExtCtrls, ComCtrls;

type
  TForm1 = class(TForm)
    Edit1: TEdit;
    Edit2: TEdit;
    Label1: TLabel;
    Label2: TLabel;
    Edit3: TEdit;
    Label3: TLabel;
    Label4: TLabel;
    Edit4: TEdit;
    Button1: TButton;
    Label5: TLabel;
    Edit5: TEdit;
    Label6: TLabel;
    Edit6: TEdit;
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
        PhoneID                        : integer;
        PhoneCallBackPointer           : PPhoneCallBackProc;
        SecurityCallBackPointer        : PSecurityCallBackProc;
        SMSCallBackPointer             : PSMSCallBackProc;
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

  implementation

{$R *.dfm}

//called, when phone is connected or disconnected
procedure ChangePhoneState1(x:integer;ID:integer;status:boolean);stdcall;
var
  error: GSM_Error;
  buffer : array[1..100] of char;
begin
	if status then
	begin
		error:=GSM_GetDCT4SimlockNetwork(Form1.PhoneID,@buffer);
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get simlock: error '+inttostr(integer(error))),'',0);
    if (error = ERR_NONE) then
    begin
      buffer[6]:=chr(0);
      Form1.Edit3.Text:=buffer;
    end;

    error:=GSM_GetModelName(Form1.PhoneID,@buffer);
    if (error = ERR_NONE) then Form1.Edit5.Text:=buffer;
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get model: error '+inttostr(integer(error))),'',0);

		error:=GSM_GetIMEI(Form1.PhoneID,@buffer);
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get IMEI: error '+inttostr(integer(error))),'',0);
    if (error = ERR_NONE) then Form1.Edit2.Text:=buffer;

		error:=GSM_GetDCT4SecurityCode(Form1.PhoneID,@buffer);
    if (error <> ERR_NONE) then application.MessageBox(pchar('Get security code: error '+inttostr(integer(error))),'',0);
    if (error = ERR_NONE) then Form1.Edit6.Text:=buffer;

    error:=GSM_EndConnection(Form1.PhoneID);
    if (error <> ERR_NONE) then application.MessageBox(pchar('End connection: error '+inttostr(integer(error))),'',0);
	end;
end;

procedure TForm1.Button1Click(Sender: TObject);
var
   Device: PChar;
   Connection: PChar;
   error: GSM_Error;
   buffer : array[1..100] of char;
begin
   GetMem(Device,Length(Edit4.Text) + 1);
   StrCopy(Device, PChar(Edit4.Text));

   GetMem(Connection,Length(Edit1.Text) + 1);
   StrCopy(Connection, PChar(Edit1.Text));

   PhoneCallBackPointer    := @ChangePhoneState1;
   SecurityCallBackPointer := nil;
   SMSCallBackPointer      := nil;

   error:=GSM_StartConnection(@PhoneID,Device,Connection,'','','',false,@PhoneCallBackPointer,@SecurityCallBackPointer,@SMSCallBackPointer);
   if (error<>ERR_NONE) then application.MessageBox(pchar('Start: error '+inttostr(integer(error))),'',0);

   FreeMem(Device);
   FreeMem(Connection);
end;

end.
