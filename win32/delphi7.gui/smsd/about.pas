unit about;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Gammu, ComCtrls, ExtCtrls;

type
  TAboutForm = class(TForm)
    Button1: TButton;
    AboutListView: TListView;
    Label1: TLabel;
    Panel2: TPanel;
    procedure Button1Click(Sender: TObject);
    procedure FormShow(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  AboutForm: TAboutForm;

implementation

{$R *.dfm}

procedure TAboutForm.Button1Click(Sender: TObject);
begin
  Close();
end;

procedure TAboutForm.FormShow(Sender: TObject);
var
  buffer : array[1..100] of char;
begin
  GSM_GetGammuVersion(@buffer);
  with AboutListView do
  begin
    Items.Clear;
    Items.Add;
    Items.Item[Items.Count-1].Caption:='Gammu DLL version';
    Items.Item[Items.Count-1].SubItems.Add(buffer);
  end;
end;

end.
