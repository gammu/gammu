unit LogosRing;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls,GnokiiAPI,MainUnit;

type
  TLogosRingForm = class(TForm)
    Button1: TButton;
    Button2: TButton;
    ComboBox3: TComboBox;
    Label1: TLabel;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  LogosRingForm: TLogosRingForm;

implementation

{$R *.dfm}

procedure TLogosRingForm.Button1Click(Sender: TObject);
begin
  ComboBox3.Visible:=true;
  LogosRingForm.Close;
end;

procedure TLogosRingForm.Button2Click(Sender: TObject);
begin
  ComboBox3.Visible:=false;
  LogosRingForm.Close;
end;

end.
