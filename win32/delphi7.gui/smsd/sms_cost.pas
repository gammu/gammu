unit sms_cost;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TSMSPriceForm = class(TForm)
    GroupBox1: TGroupBox;
    Button1: TButton;
    Button2: TButton;
    Label1: TLabel;
    NumberEdit: TEdit;
    PriceEdit: TEdit;
    Label2: TLabel;
    CurrencyLabel: TLabel;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  SMSPriceForm: TSMSPriceForm;

implementation

{$R *.dfm}

end.
