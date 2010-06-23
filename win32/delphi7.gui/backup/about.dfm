object AboutForm: TAboutForm
  Left = 321
  Top = 222
  BorderStyle = bsDialog
  Caption = 'About...'
  ClientHeight = 159
  ClientWidth = 247
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
    Left = 80
    Top = 128
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
    Height = 105
    TabOrder = 1
    object Label1: TLabel
      Left = 8
      Top = 16
      Width = 217
      Height = 13
      Alignment = taCenter
      Caption = 'Created by Marcin Wi'#261'cek and others'
    end
    object Label2: TLabel
      Left = 8
      Top = 32
      Width = 217
      Height = 13
      Alignment = taCenter
      Caption = 'Sponsored by MatrixFlasher team'
    end
    object Label3: TLabel
      Left = 8
      Top = 48
      Width = 217
      Height = 13
      Alignment = taCenter
      Caption = 'Version'
    end
    object Label4: TLabel
      Left = 8
      Top = 80
      Width = 217
      Height = 13
      Alignment = taCenter
      Caption = 'Homepage'
    end
    object Panel1: TPanel
      Left = 8
      Top = 72
      Width = 217
      Height = 2
      TabOrder = 0
    end
  end
end
