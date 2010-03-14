unit Netmon;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls;

type
  TNetmonForm = class(TForm)
    Button1: TButton;
    Button2: TButton;
    RadioGroup1: TRadioGroup;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  NetmonForm: TNetmonForm;

implementation

{$R *.dfm}

procedure TNetmonForm.Button1Click(Sender: TObject);
begin
  NetmonForm.close();
end;

procedure TNetmonForm.Button2Click(Sender: TObject);
begin
  RadioGroup1.ItemIndex:=-1;
  NetmonForm.close();
end;

procedure TNetmonForm.FormCreate(Sender: TObject);
begin
  RadioGroup1.ItemIndex:=2;
end;

end.
