unit send_sms;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Gammu, ComCtrls, ExtCtrls, PhoneNum;

type
  TSendSMSForm = class(TForm)
    GroupBox1: TGroupBox;
    GroupBox3: TGroupBox;
    OKButton: TButton;
    Button2: TButton;
    RecipientsListBox: TListBox;
    Button3: TButton;
    Button4: TButton;
    Memo: TMemo;
    PriceLabel: TLabel;
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure MemoChange(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  SendSMSForm: TSendSMSForm;
  OldSMSText : String;
  Price      : Extended;

implementation

uses Main;

{$R *.dfm}

procedure TSendSMSForm.Button3Click(Sender: TObject);
var i:integer;
begin
  AddNumberForm.ComboBox1.Items.Clear;
  i:=1;
  while true do
  begin
    if GatewayINIFile.ReadString('general', 'PhonebookNumber'+inttostr(i),'') = '' then break;
    if GatewayINIFile.ReadString('general', 'PhonebookName'+inttostr(i),'') = '' then break;
    AddNumberForm.ComboBox1.Items.Add(GatewayINIFile.ReadString('general', 'PhonebookName'+inttostr(i),''));
    i:=i+1;
  end;
  AddNumberForm.ComboBox1.Text:='';
  if AddNumberForm.ShowModal = mrOK then
  begin
    RecipientsListBox.Items.Add(AddNumberForm.ComboBox1.Text);
    Price:=Price+FindPrice(AddNumberForm.ComboBox1.Text);
  end;
  OKButton.Enabled:=false;
  if RecipientsListBox.Items.Count <> 0 then OKButton.Enabled:=true;
  PriceLabel.Caption:='Sending price: '+FloatToStr(Price)+' '+GatewayINIFile.ReadString('general', 'SMSCurrency','');
end;

procedure TSendSMSForm.Button4Click(Sender: TObject);
var i:integer;
begin
  i:=1;
  while true do
  begin
    if i>RecipientsListBox.Items.Count then break;
    if RecipientsListBox.Selected[i-1] then
    begin
      Price:=Price-FindPrice(RecipientsListBox.Items.Strings[i-1]);
      RecipientsListBox.Items.Delete(i-1);
      i:=1;
    end else
    begin
      i:=i+1;
    end;
  end;
  OKButton.Enabled:=false;
  if RecipientsListBox.Items.Count <> 0 then OKButton.Enabled:=true;
  PriceLabel.Caption:='Sending price: '+FloatToStr(Price)+' '+GatewayINIFile.ReadString('general', 'SMSCurrency','');
end;

procedure TSendSMSForm.MemoChange(Sender: TObject);
var
  SMSText,Temp     : array[1..10000] of char;
  i,c              : integer;
  SMSNum,CharsLeft : integer;
  MultiSMS         : GSM_MultiSMSMessage;
  tmp              : string;
begin
  //first create Unicode string
  StringToWideChar(Memo.Text, @Temp, 5000);
  i:=0;
  while i<strlen(PChar(Memo.Text)) do
  begin
    SMSText[i*2+1]:=Temp[i*2+2];
    SMSText[i*2+2]:=Temp[i*2+1];
    i:=i+1;
  end;
  SMSText[i*2+1]:=chr(0);
  SMSText[i*2+2]:=chr(0);

  //I don't know, where is the problem. It's required
  tmp:=pchar(inttostr(ord(smstext[1]))+' '+inttostr(ord(smstext[2]))+' '+inttostr(ord(smstext[3]))+' '+inttostr(ord(smstext[4])));

  //now initialy split messages
  //if message needs too many sms, remove chars
  C:=0;
  while true do
  begin
    MultiSMS.Number:=chr(0);
    GSM_MakeMultiPartSMS(@SMSText,i,UDH_NoUDH,GSM_Coding_Default,-1,0,@MultiSMS);
    if Ord(MultiSMS.Number) > 1 then
    begin
      MultiSMS.Number:=chr(0);
      GSM_MakeMultiPartSMS(@SMSText,i,UDH_ConcatenatedMessages,GSM_Coding_Default,-1,0,@MultiSMS);
    end;
    if Ord(MultiSMS.Number) <= StrToInt(GatewayINIFile.ReadString('general', 'SMSNumber','1')) then break;
    c:=c+1;
    i:=i-1;
    SMSText[i*2+1]:=chr(0);
    SMSText[i*2+2]:=chr(0);
  end;
  //now calculate sms counter
  GSM_SMSCounter(i,@SMSText,MultiSMS.SMS[1].UDH.UDHType,GSM_Coding_Default,@SMSNum,@CharsLeft);
  //delete chars from Memo, if required
  Memo.SelStart:=i;
  Memo.SelLength:=C;
  Memo.ClearSelection;
  //display counter
  GroupBox1.Caption:='Text ('+IntToStr(CharsLeft)+' chars left/'+IntToStr(SMSNum)+' SMS)';
end;

procedure TSendSMSForm.FormShow(Sender: TObject);
begin
  PriceLabel.Caption:='';
  Memo.OnChange(Sender);
  OKButton.Enabled:=false;
end;

procedure TSendSMSForm.FormCreate(Sender: TObject);
begin
  Price:=0;
end;

end.
