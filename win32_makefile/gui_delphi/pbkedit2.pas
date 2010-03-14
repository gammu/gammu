unit pbkedit2;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, MainUnit, GnokiiAPI;

type
  TPBKEditForm = class(TForm)
    GroupBox1: TGroupBox;
    Label1: TLabel;
    Edit1: TEdit;
    ComboBox1: TComboBox;
    ComboBox2: TComboBox;
    ComboBox3: TComboBox;
    ComboBox4: TComboBox;
    ComboBox5: TComboBox;
    ComboBox6: TComboBox;
    ComboBox7: TComboBox;
    Edit2: TEdit;
    Edit3: TEdit;
    Edit4: TEdit;
    Edit5: TEdit;
    Edit6: TEdit;
    Edit7: TEdit;
    Edit8: TEdit;
    Button1: TButton;
    Button2: TButton;
    Label2: TLabel;
    ComboBox8: TComboBox;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  PBKEditForm: TPBKEditForm;

implementation

{$R *.dfm}

procedure TPBKEditForm.Button2Click(Sender: TObject);
begin
  PBKEditForm.Close;
end;

procedure AddOneNumber(Edit: TEdit; ComboBox:TComboBox);
var i:integer;
begin
  if Edit.Text<>'' then
  begin
    if not ComboBox.Enabled then
    begin
      for i:=1 to 49 do
        pbk.Number[i]:=chr(0);
      for i:=1 to strlen(pchar(Edit.text)) do
        pbk.Number[i]:=Edit.Text[i];
    end else
    begin
      pbk.SubEntriesCount:=pbk.SubEntriesCount+1;
      case combobox.ItemIndex of
        0:pbk.SubEntries[pbk.SubEntriesCount].NumberType:=GSM_Fax;
        1:pbk.SubEntries[pbk.SubEntriesCount].NumberType:=GSM_General;
        2:pbk.SubEntries[pbk.SubEntriesCount].NumberType:=GSM_Home;
        3:pbk.SubEntries[pbk.SubEntriesCount].NumberType:=GSM_Mobile;
        4:pbk.SubEntries[pbk.SubEntriesCount].NumberType:=GSM_Work;
      end;
      pbk.SubEntries[pbk.SubEntriesCount].EntryType:=GSM_Number;
      for i:=1 to 61 do
        pbk.SubEntries[pbk.SubEntriesCount].Number[i]:=chr(0);
      for i:=1 to strlen(pchar(Edit.text)) do
        pbk.SubEntries[pbk.SubEntriesCount].Number[i]:=Edit.Text[i];
    end;
  end;
end;

procedure AddOneText(Edit: TEdit; ComboBox:TComboBox);
var i:integer;
begin
  if Edit.Text<>'' then
  begin
    pbk.SubEntriesCount:=pbk.SubEntriesCount+1;
    case combobox.ItemIndex of
      0:pbk.SubEntries[pbk.SubEntriesCount].EntryType:=GSM_Email;
      1:pbk.SubEntries[pbk.SubEntriesCount].EntryType:=GSM_Note;
      2:pbk.SubEntries[pbk.SubEntriesCount].EntryType:=GSM_Postal;
    end;
    for i:=1 to 61 do
      pbk.SubEntries[pbk.SubEntriesCount].Number[i]:=chr(0);
    for i:=1 to strlen(pchar(Edit.text)) do
      pbk.SubEntries[pbk.SubEntriesCount].Number[i]:=Edit.Text[i];
  end;
end;

procedure TPBKEditForm.Button1Click(Sender: TObject);
var i:integer;
begin

  pbk.SubEntriesCount:=0;

  for i:=1 to 51 do
    pbk.Name[i]:=chr(0);
  for i:=1 to strlen(pchar(Edit1.text)) do
    pbk.Name[i]:=Edit1.Text[i];

  pbk.Number[1]:=chr(0);
  AddOneNumber(Edit2,ComboBox1);
  AddOneNumber(Edit3,ComboBox2);
  AddOneNumber(Edit4,ComboBox3);
  AddOneNumber(Edit5,ComboBox4);
  AddOneNumber(Edit6,ComboBox5);

  AddOneText(Edit7,ComboBox6);
  AddOneText(Edit8,ComboBox7);

  if (ComboBox8.ItemIndex=0) then
  begin
    pbk.Group:=5;
  end else
  begin
    pbk.Group:=ComboBox8.ItemIndex-1;
  end;

  MainForm.Timer2.Enabled:=false;
  GSM_WritePhonebookLocation(@pbk);
  MainForm.Timer2.Enabled:=true;

  MainForm.ComboBox1Change(nil);

  PBKEditForm.Close;
end;

end.
