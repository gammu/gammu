object BackupForm: TBackupForm
  Left = 414
  Top = 99
  Width = 290
  Height = 290
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = 'Backup/Restore settings'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 265
    Height = 129
    Caption = ' '
    TabOrder = 0
    object CheckBox5: TCheckBox
      Left = 8
      Top = 88
      Width = 185
      Height = 17
      Caption = 'Startup text and logo'
      TabOrder = 4
    end
    object CheckBox1: TCheckBox
      Left = 8
      Top = 72
      Width = 169
      Height = 17
      Caption = 'Phonebook from SIM card'
      TabOrder = 0
    end
    object CheckBox2: TCheckBox
      Left = 8
      Top = 56
      Width = 153
      Height = 17
      Caption = 'Phonebook from phone'
      TabOrder = 1
    end
    object CheckBox4: TCheckBox
      Left = 8
      Top = 40
      Width = 169
      Height = 17
      Caption = 'Operator logo'
      TabOrder = 3
    end
    object CheckBox3: TCheckBox
      Left = 8
      Top = 24
      Width = 153
      Height = 17
      Caption = 'Caller groups'
      TabOrder = 2
    end
  end
  object Button1: TButton
    Left = 8
    Top = 232
    Width = 81
    Height = 25
    Caption = 'OK'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 192
    Top = 232
    Width = 81
    Height = 25
    Caption = 'Cancel'
    TabOrder = 2
    OnClick = Button2Click
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 144
    Width = 265
    Height = 81
    Caption = 'Where do you want to export your settings ?'
    TabOrder = 3
    object ListBox1: TListBox
      Left = 2
      Top = 15
      Width = 261
      Height = 64
      Align = alClient
      ItemHeight = 13
      Items.Strings = (
        'Save to Logo Manager Backup (LMB) file')
      TabOrder = 0
    end
  end
end
