object AddNumberForm: TAddNumberForm
  Left = 331
  Top = 241
  BorderStyle = bsDialog
  Caption = 'Add phone number'
  ClientHeight = 70
  ClientWidth = 299
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
    Top = 8
    Width = 100
    Height = 13
    Caption = 'Please enter number:'
  end
  object ComboBox1: TComboBox
    Left = 120
    Top = 8
    Width = 177
    Height = 21
    ItemHeight = 13
    TabOrder = 0
    Text = 'ComboBox1'
    OnChange = ComboBox1Change
  end
  object OKButton: TButton
    Left = 120
    Top = 40
    Width = 89
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
  end
  object CancelButton: TButton
    Left = 216
    Top = 40
    Width = 81
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
