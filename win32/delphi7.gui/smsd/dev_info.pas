unit dev_info;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls;

type
  TDeviceInfoForm = class(TForm)
    ListView: TListView;
    Button1: TButton;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  DeviceInfoForm: TDeviceInfoForm;

implementation

{$R *.dfm}

end.
