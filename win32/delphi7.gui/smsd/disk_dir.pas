unit disk_dir;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, FileCtrl, ComCtrls, ShellCtrls;

{$I-}

type
  TDirectoryForm = class(TForm)
    OKButton: TButton;
    CancelButton: TButton;
    ShellTreeView1: TShellTreeView;
    procedure ShellTreeView1Editing(Sender: TObject; Node: TTreeNode;
      var AllowEdit: Boolean);
    procedure FormShow(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  DirectoryForm: TDirectoryForm;

implementation

{$R *.dfm}

procedure TDirectoryForm.ShellTreeView1Editing(Sender: TObject;
  Node: TTreeNode; var AllowEdit: Boolean);
begin
  AllowEdit:=false;
end;

procedure TDirectoryForm.FormShow(Sender: TObject);
begin
  ShellTreeView1.SetFocus;
end;

end.
