unit pbkedit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls, MainUnit, GnokiiAPI, Menus;

type
  TPbkForm = class(TForm)
    ListView1: TListView;
    Button1: TButton;
    Button2: TButton;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  PbkForm: TPbkForm;

implementation

{$R *.dfm}

procedure TPbkForm.Button1Click(Sender: TObject);
begin
  Pbkform.Close;
end;

procedure TPbkForm.Button2Click(Sender: TObject);
begin
  Pbkform.Close;
end;

end.
