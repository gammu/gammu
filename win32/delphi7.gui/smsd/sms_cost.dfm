object SMSPriceForm: TSMSPriceForm
  Left = 420
  Top = 190
  BorderStyle = bsDialog
  Caption = 'SMSPriceForm'
  ClientHeight = 124
  ClientWidth = 327
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 313
    Height = 73
    Caption = 'SMS sending price settings'
    TabOrder = 0
    object Label1: TLabel
      Left = 16
      Top = 16
      Width = 100
      Height = 13
      HelpType = htKeyword
      Caption = 'Number starting from:'
    end
    object Label2: TLabel
      Left = 16
      Top = 40
      Width = 68
      Height = 13
      Caption = 'Sending price:'
    end
    object CurrencyLabel: TLabel
      Left = 248
      Top = 40
      Width = 68
      Height = 13
      Caption = 'CurrencyLabel'
    end
    object NumberEdit: TEdit
      Left = 136
      Top = 16
      Width = 169
      Height = 21
      TabOrder = 0
      Text = 'Edit'
    end
    object PriceEdit: TEdit
      Left = 136
      Top = 40
      Width = 105
      Height = 21
      TabOrder = 1
      Text = 'PriceEdit'
    end
  end
  object Button1: TButton
    Left = 168
    Top = 88
    Width = 73
    Height = 25
    Caption = 'OK'
    ModalResult = 1
    TabOrder = 1
  end
  object Button2: TButton
    Left = 248
    Top = 88
    Width = 73
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
