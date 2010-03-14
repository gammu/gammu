unit wapbook;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, GnokiiAPI;

type
  TWAPBookForm = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Label1: TLabel;
    Edit1: TEdit;
    Label2: TLabel;
    Edit2: TEdit;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  WAPBookForm: TWAPBookForm;

implementation

uses MainUnit;

{$R *.dfm}

procedure TWAPBookForm.Button2Click(Sender: TObject);
begin
  WAPBookForm.Close;
end;

procedure TWAPBookForm.Button1Click(Sender: TObject);
var i:integer;
    book:GSM_WAPBookmark;
begin
  for i:=1 to 256 do book.adress[i]:=chr(0);
  for i:=1 to 51 do book.title[i]:=chr(0);

  for i:=1 to strlen(pchar(Edit1.text)) do
    book.title[i]:=Edit1.Text[i];
  for i:=1 to strlen(pchar(Edit2.text)) do
    book.adress[i]:=Edit2.Text[i];

  book.location:=bookmarknum2;
  
  MainForm.Timer2.enabled:=false;
  GSM_SetWAPBookmark(@book);
  MainForm.Timer2.enabled:=true;

  MainForm.WAPBookTabSheetShow(nil);

  WAPBookForm.Close;
end;

end.
