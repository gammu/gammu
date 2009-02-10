object SendSMSForm: TSendSMSForm
  Left = 310
  Top = 169
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Send SMS'
  ClientHeight = 333
  ClientWidth = 289
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object PriceLabel: TLabel
    Left = 8
    Top = 288
    Width = 3
    Height = 13
  end
  object GroupBox1: TGroupBox
    Left = 0
    Top = 160
    Width = 289
    Height = 121
    Align = alTop
    BiDiMode = bdRightToLeft
    Caption = 'Text'
    ParentBiDiMode = False
    TabOrder = 0
    object Memo: TMemo
      Left = 2
      Top = 15
      Width = 285
      Height = 104
      Align = alClient
      TabOrder = 0
      OnChange = MemoChange
    end
  end
  object GroupBox3: TGroupBox
    Left = 0
    Top = 0
    Width = 289
    Height = 160
    Align = alTop
    Caption = 'Numbers of recipients'
    TabOrder = 1
    object RecipientsListBox: TListBox
      Left = 2
      Top = 15
      Width = 191
      Height = 143
      Align = alLeft
      ItemHeight = 13
      MultiSelect = True
      TabOrder = 0
    end
    object Button3: TButton
      Left = 200
      Top = 16
      Width = 81
      Height = 25
      Caption = '&Add'
      TabOrder = 1
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 200
      Top = 48
      Width = 81
      Height = 25
      Caption = '&Delete'
      TabOrder = 2
      OnClick = Button4Click
    end
  end
  object OKButton: TButton
    Left = 112
    Top = 304
    Width = 81
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 2
  end
  object Button2: TButton
    Left = 200
    Top = 304
    Width = 81
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 3
  end
end
