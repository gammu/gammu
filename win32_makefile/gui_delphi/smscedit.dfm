object SMSCEditForm: TSMSCEditForm
  Left = 316
  Top = 239
  BorderStyle = bsDialog
  Caption = 'Edit SMSC'
  ClientHeight = 203
  ClientWidth = 305
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
    Top = 8
    Width = 31
    Height = 13
    Caption = 'Name:'
  end
  object Label3: TLabel
    Left = 8
    Top = 72
    Width = 36
    Height = 13
    Caption = 'Validity:'
  end
  object Label4: TLabel
    Left = 8
    Top = 104
    Width = 35
    Height = 13
    Caption = 'Format:'
  end
  object Label5: TLabel
    Left = 8
    Top = 136
    Width = 80
    Height = 13
    Caption = 'Default recipient:'
  end
  object Label2: TLabel
    Left = 8
    Top = 40
    Width = 40
    Height = 13
    Caption = 'Number:'
  end
  object Button1: TButton
    Left = 8
    Top = 168
    Width = 81
    Height = 25
    Caption = '&OK'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 208
    Top = 168
    Width = 89
    Height = 25
    Caption = '&Cancel'
    TabOrder = 1
    OnClick = Button2Click
  end
  object Edit1: TEdit
    Left = 96
    Top = 8
    Width = 201
    Height = 21
    TabOrder = 2
    Text = 'Edit1'
  end
  object ComboBox1: TComboBox
    Left = 96
    Top = 72
    Width = 201
    Height = 21
    ItemHeight = 13
    TabOrder = 3
    Text = 'maximal time'
    Items.Strings = (
      '1 hour'
      '6 hours'
      '1 day (24 hours)'
      '3 days (72 hours)'
      '1 week'
      'maximal time')
  end
  object ComboBox2: TComboBox
    Left = 96
    Top = 104
    Width = 201
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 4
    Text = 'Text'
    Items.Strings = (
      'Text'
      'Fax'
      'Email'
      'Paging')
  end
  object Edit3: TEdit
    Left = 96
    Top = 136
    Width = 201
    Height = 21
    TabOrder = 5
    Text = 'Edit3'
  end
  object Edit2: TEdit
    Left = 96
    Top = 40
    Width = 201
    Height = 21
    TabOrder = 6
    Text = 'Edit2'
  end
end
