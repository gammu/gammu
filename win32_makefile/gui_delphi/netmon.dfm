object NetmonForm: TNetmonForm
  Left = 302
  Top = 272
  Width = 202
  Height = 185
  Caption = 'Set Netmonitor'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 8
    Top = 128
    Width = 81
    Height = 25
    Caption = 'OK'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 104
    Top = 128
    Width = 81
    Height = 25
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = Button2Click
  end
  object RadioGroup1: TRadioGroup
    Left = 8
    Top = 16
    Width = 177
    Height = 97
    Caption = ' Netmonitor state '
    ItemIndex = 2
    Items.Strings = (
      'OFF'
      'Simple'
      'Extended')
    TabOrder = 2
  end
end
