object Form1: TForm1
  Left = 192
  Top = 107
  Width = 544
  Height = 375
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 16
    Top = 248
    Width = 97
    Height = 25
    Caption = 'Start'
    TabOrder = 0
    OnClick = Button1Click
  end
  object GroupBox1: TGroupBox
    Left = 24
    Top = 8
    Width = 409
    Height = 225
    Caption = 'Phone info'
    TabOrder = 1
    object ListBox1: TListBox
      Left = 2
      Top = 15
      Width = 405
      Height = 208
      Align = alClient
      ItemHeight = 13
      TabOrder = 0
    end
  end
end
