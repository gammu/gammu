object AboutForm: TAboutForm
  Left = 336
  Top = 267
  BorderStyle = bsDialog
  Caption = 'About...'
  ClientHeight = 114
  ClientWidth = 247
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
    Left = 80
    Top = 80
    Width = 97
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 233
    Height = 65
    TabOrder = 1
    object Label1: TLabel
      Left = 8
      Top = 16
      Width = 179
      Height = 13
      Alignment = taCenter
      Caption = 'Created by Marcin Wi'#261'cek and others'
    end
    object Label3: TLabel
      Left = 8
      Top = 40
      Width = 35
      Height = 13
      Alignment = taCenter
      Caption = 'Version'
    end
  end
end
