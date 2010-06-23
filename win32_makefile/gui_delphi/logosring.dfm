object LogosRingForm: TLogosRingForm
  Left = 192
  Top = 108
  BorderStyle = bsDialog
  Caption = 'LogosRingForm'
  ClientHeight = 106
  ClientWidth = 202
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 32
    Height = 13
    Caption = 'Label1'
  end
  object Button1: TButton
    Left = 8
    Top = 72
    Width = 73
    Height = 25
    Caption = '&Yes'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 120
    Top = 72
    Width = 73
    Height = 25
    Caption = '&No'
    TabOrder = 1
    OnClick = Button2Click
  end
  object ComboBox3: TComboBox
    Left = 8
    Top = 32
    Width = 185
    Height = 21
    ItemHeight = 13
    TabOrder = 2
    Text = 'ComboBox3'
  end
end
