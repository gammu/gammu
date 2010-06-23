unit smscedit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, GnokiiAPI,MainUnit;

type
  TSMSCEditForm = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Label1: TLabel;
    Edit1: TEdit;
    Label3: TLabel;
    ComboBox1: TComboBox;
    Label4: TLabel;
    ComboBox2: TComboBox;
    Label5: TLabel;
    Edit3: TEdit;
    Edit2: TEdit;
    Label2: TLabel;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  SMSCEditForm: TSMSCEditForm;

implementation

{$R *.dfm}

procedure TSMSCEditForm.Button2Click(Sender: TObject);
begin
  SMSCEditForm.Close;
end;

procedure TSMSCEditForm.Button1Click(Sender: TObject);
var
  smsc:GSM_MessageCenter;
  i:integer;
begin
  smsc.no:=smscselectitem;

  for i:=1 to 20 do
    smsc.name[i]:=chr(0);
  for i:=1 to strlen(pchar(Edit1.text)) do
    smsc.name[i]:=Edit1.Text[i];

  for i:=1 to 40 do
    smsc.number[i]:=chr(0);
  for i:=1 to strlen(pchar(Edit2.text)) do
    smsc.number[i]:=Edit2.Text[i];

  for i:=1 to 40 do
    smsc.defaultrecipient[i]:=chr(0);
  for i:=1 to strlen(pchar(Edit3.text)) do
    smsc.defaultrecipient[i]:=Edit3.Text[i];

  case ComboBox1.ItemIndex of
    0:smsc.Validity:=    GSMV_1_Hour  ;
    1:smsc.Validity:=    GSMV_6_Hours ;
    2:smsc.Validity:=    GSMV_24_Hours;
    3:smsc.Validity:=    GSMV_72_Hours;
    4:smsc.Validity:=    GSMV_1_Week  ;
    5:smsc.Validity:=    GSMV_Max_Time;
  end;

  case combobox2.ItemIndex of
    0:smsc.Format:=       GSMF_Text;
    1:smsc.Format:=     GSMF_Fax   ;
    3:smsc.Format:=     GSMF_Paging;
  else
    smsc.format:=GSMF_Email;
  end;

  GSM_SetSMSCenter(@smsc);
  MainForm.SMSCTabSheetShow(nil);
  SMSCEditForm.Close;
end;

end.
