unit pbk;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TPhonebookForm = class(TForm)
    GroupBox1: TGroupBox;
    Label1: TLabel;
    Label2: TLabel;
    NameEdit: TEdit;
    NumberEdit: TEdit;
    OKButton: TButton;
    Button2: TButton;
    procedure NameEditChange(Sender: TObject);
    procedure NumberEditChange(Sender: TObject);
    procedure FormShow(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  PhonebookForm: TPhonebookForm;

implementation

{$R *.dfm}

procedure EnableOK;
begin
  with PhonebookForm do
  begin
    OKButton.Enabled:=true;
    if (strlen(Pchar(NameEdit.Text)) = 0) or (strlen(PChar(NumberEdit.Text)) = 0) then OKButton.Enabled:=false;
  end;
end;

procedure TPhonebookForm.NameEditChange(Sender: TObject);
begin
  EnableOK;
end;

procedure TPhonebookForm.NumberEditChange(Sender: TObject);
begin
  EnableOK;
end;

procedure TPhonebookForm.FormShow(Sender: TObject);
begin
  EnableOK;
end;

end.
