object AboutForm: TAboutForm
  Left = 289
  Top = 261
  BorderStyle = bsDialog
  Caption = 'About'
  ClientHeight = 101
  ClientWidth = 273
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 48
    Width = 257
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'Copyright 2003 by Marcin Wi'#261'cek'
  end
  object Button1: TButton
    Left = 88
    Top = 72
    Width = 97
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = Button1Click
  end
  object AboutListView: TListView
    Left = 8
    Top = 16
    Width = 257
    Height = 17
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    Color = clScrollBar
    Columns = <
      item
        Caption = 'Parameter'
        Width = 128
      end
      item
        Alignment = taRightJustify
        Caption = 'Value'
        Width = 129
      end>
    ColumnClick = False
    ReadOnly = True
    RowSelect = True
    ShowColumnHeaders = False
    TabOrder = 1
    ViewStyle = vsReport
  end
  object Panel2: TPanel
    Left = 8
    Top = 40
    Width = 257
    Height = 2
    TabOrder = 2
  end
end
