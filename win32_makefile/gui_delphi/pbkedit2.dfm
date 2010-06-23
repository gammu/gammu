object PBKEditForm: TPBKEditForm
  Left = 209
  Top = 107
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Edit phonebook entry'
  ClientHeight = 380
  ClientWidth = 347
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 16
    Width = 329
    Height = 305
    Caption = 'Phonebook entry details'
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 24
      Width = 31
      Height = 13
      Caption = 'Name:'
    end
    object Label2: TLabel
      Left = 8
      Top = 280
      Width = 59
      Height = 13
      Caption = 'Caller group:'
    end
    object Edit1: TEdit
      Left = 144
      Top = 24
      Width = 177
      Height = 21
      TabOrder = 0
    end
    object ComboBox1: TComboBox
      Left = 8
      Top = 64
      Width = 129
      Height = 21
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 1
      Text = 'Fax number'
      Items.Strings = (
        'Fax number'
        'General number'
        'Home number'
        'Mobile number'
        'Work number')
    end
    object ComboBox2: TComboBox
      Left = 8
      Top = 88
      Width = 129
      Height = 21
      ItemHeight = 13
      TabOrder = 2
      Text = 'ComboBox2'
    end
    object ComboBox3: TComboBox
      Left = 8
      Top = 112
      Width = 129
      Height = 21
      ItemHeight = 13
      TabOrder = 3
      Text = 'ComboBox3'
    end
    object ComboBox4: TComboBox
      Left = 8
      Top = 136
      Width = 129
      Height = 21
      ItemHeight = 13
      TabOrder = 4
      Text = 'ComboBox4'
    end
    object ComboBox5: TComboBox
      Left = 8
      Top = 160
      Width = 129
      Height = 21
      ItemHeight = 13
      TabOrder = 5
      Text = 'ComboBox5'
    end
    object ComboBox6: TComboBox
      Left = 8
      Top = 208
      Width = 129
      Height = 21
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 6
      Text = 'Email'
      Items.Strings = (
        'Email'
        'Note'
        'Post address')
    end
    object ComboBox7: TComboBox
      Left = 8
      Top = 232
      Width = 129
      Height = 21
      ItemHeight = 13
      TabOrder = 7
      Text = 'ComboBox7'
    end
    object Edit2: TEdit
      Left = 144
      Top = 64
      Width = 177
      Height = 21
      TabOrder = 8
    end
    object Edit3: TEdit
      Left = 144
      Top = 88
      Width = 177
      Height = 21
      TabOrder = 9
    end
    object Edit4: TEdit
      Left = 144
      Top = 112
      Width = 177
      Height = 21
      TabOrder = 10
    end
    object Edit5: TEdit
      Left = 144
      Top = 136
      Width = 177
      Height = 21
      TabOrder = 11
    end
    object Edit6: TEdit
      Left = 144
      Top = 160
      Width = 177
      Height = 21
      TabOrder = 12
    end
    object Edit7: TEdit
      Left = 144
      Top = 208
      Width = 177
      Height = 21
      TabOrder = 13
    end
    object Edit8: TEdit
      Left = 144
      Top = 232
      Width = 177
      Height = 21
      TabOrder = 14
    end
    object ComboBox8: TComboBox
      Left = 144
      Top = 272
      Width = 177
      Height = 21
      ItemHeight = 13
      TabOrder = 15
      Text = 'ComboBox8'
    end
  end
  object Button1: TButton
    Left = 8
    Top = 336
    Width = 89
    Height = 25
    Caption = '&OK'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 248
    Top = 336
    Width = 89
    Height = 25
    Caption = '&Cancel'
    TabOrder = 2
    OnClick = Button2Click
  end
end
