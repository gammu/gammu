object PhonebookForm: TPhonebookForm
  Left = 296
  Top = 160
  BorderStyle = bsDialog
  Caption = 'PhonebookForm'
  ClientHeight = 117
  ClientWidth = 334
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
  object GroupBox1: TGroupBox
    Left = 0
    Top = 8
    Width = 329
    Height = 73
    Caption = 'Phonebook entry options'
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 16
      Width = 31
      Height = 13
      Caption = 'Name:'
    end
    object Label2: TLabel
      Left = 8
      Top = 40
      Width = 40
      Height = 13
      Caption = 'Number:'
    end
    object NameEdit: TEdit
      Left = 120
      Top = 16
      Width = 201
      Height = 21
      TabOrder = 0
      Text = 'NameEdit'
      OnChange = NameEditChange
    end
    object NumberEdit: TEdit
      Left = 120
      Top = 40
      Width = 201
      Height = 21
      TabOrder = 1
      Text = 'NumberEdit'
      OnChange = NumberEditChange
    end
  end
  object OKButton: TButton
    Left = 168
    Top = 88
    Width = 81
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
  end
  object Button2: TButton
    Left = 256
    Top = 88
    Width = 73
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
