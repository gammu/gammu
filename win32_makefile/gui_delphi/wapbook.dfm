object WAPBookForm: TWAPBookForm
  Left = 215
  Top = 152
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Edit WAP Bookmark'
  ClientHeight = 78
  ClientWidth = 369
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
    Top = 16
    Width = 31
    Height = 13
    Caption = 'Name:'
  end
  object Label2: TLabel
    Left = 8
    Top = 48
    Width = 25
    Height = 13
    Caption = 'URL:'
  end
  object Button1: TButton
    Left = 280
    Top = 8
    Width = 81
    Height = 25
    Caption = '&OK'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 280
    Top = 40
    Width = 81
    Height = 25
    Caption = '&Cancel'
    TabOrder = 1
    OnClick = Button2Click
  end
  object Edit1: TEdit
    Left = 56
    Top = 8
    Width = 193
    Height = 21
    TabOrder = 2
  end
  object Edit2: TEdit
    Left = 56
    Top = 40
    Width = 193
    Height = 21
    TabOrder = 3
  end
end
