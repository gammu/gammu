unit backup;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TBackupForm = class(TForm)
    GroupBox1: TGroupBox;
    CheckBox1: TCheckBox;
    CheckBox2: TCheckBox;
    CheckBox3: TCheckBox;
    CheckBox4: TCheckBox;
    CheckBox5: TCheckBox;
    Button1: TButton;
    Button2: TButton;
    GroupBox2: TGroupBox;
    ListBox1: TListBox;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure FormActivate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  BackupForm: TBackupForm;

implementation

uses MainUnit;

{$R *.dfm}

procedure TBackupForm.Button2Click(Sender: TObject);
begin
  PhoneBackupAvailable:=false;
  BackupForm.Close;
end;

procedure TBackupForm.Button1Click(Sender: TObject);
begin
  BackupForm.Close;
end;

procedure TBackupForm.FormActivate(Sender: TObject);
begin
  CheckBox1.Checked:=false;
  CheckBox2.Checked:=false;
  CheckBox3.Checked:=false;
  CheckBox4.Checked:=false;
  CheckBox5.Checked:=false;
end;

end.
