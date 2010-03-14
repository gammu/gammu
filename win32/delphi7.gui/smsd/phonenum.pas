unit phonenum;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TAddNumberForm = class(TForm)
    Label1: TLabel;
    ComboBox1: TComboBox;
    OKButton: TButton;
    CancelButton: TButton;
    procedure ComboBox1Change(Sender: TObject);
    procedure FormShow(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  AddNumberForm: TAddNumberForm;

implementation

{$R *.dfm}

procedure TAddNumberForm.ComboBox1Change(Sender: TObject);
begin
  OKButton.Enabled:=false;
  if (strlen(PChar(ComboBox1.Text))) <> 0 then OKButton.Enabled:=true;
end;

procedure TAddNumberForm.FormShow(Sender: TObject);
begin
  OKButton.Enabled:=false;
end;

end.
