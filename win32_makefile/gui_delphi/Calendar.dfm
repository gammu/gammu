object CalendarForm: TCalendarForm
  Left = 290
  Top = 105
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsDialog
  Caption = 'Edit/Add calendar'
  ClientHeight = 287
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
    Left = 16
    Top = 16
    Width = 63
    Height = 13
    Caption = 'Type of note:'
  end
  object Label2: TLabel
    Left = 16
    Top = 112
    Width = 26
    Height = 13
    Caption = 'Date:'
  end
  object Label3: TLabel
    Left = 16
    Top = 48
    Width = 60
    Height = 13
    Caption = 'Text of note:'
  end
  object Label4: TLabel
    Left = 16
    Top = 80
    Width = 34
    Height = 13
    Caption = 'Phone:'
  end
  object Label5: TLabel
    Left = 88
    Top = 112
    Width = 32
    Height = 13
    Caption = 'Label5'
  end
  object ComboBox1: TComboBox
    Left = 88
    Top = 16
    Width = 201
    Height = 21
    ItemHeight = 13
    TabOrder = 0
    Text = 'Birthday'
    OnSelect = ComboBox1Select
    Items.Strings = (
      'Birthday'
      'Call'
      'Meeting'
      'Reminder (Memo)')
  end
  object DateTimePicker1: TDateTimePicker
    Left = 120
    Top = 112
    Width = 89
    Height = 25
    CalAlignment = dtaLeft
    Date = 37172.5632527778
    Time = 37172.5632527778
    DateFormat = dfShort
    DateMode = dmComboBox
    Kind = dtkDate
    ParseInput = False
    TabOrder = 2
    OnChange = DateTimePicker1Change
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 144
    Width = 289
    Height = 73
    Caption = 'Alarm'
    TabOrder = 4
    object Label6: TLabel
      Left = 80
      Top = 16
      Width = 32
      Height = 13
      Caption = 'Label6'
    end
    object CheckBox1: TCheckBox
      Left = 8
      Top = 16
      Width = 65
      Height = 17
      Caption = 'Enabled'
      Checked = True
      State = cbChecked
      TabOrder = 0
      OnClick = CheckBox1Click
    end
    object DateTimePicker2: TDateTimePicker
      Left = 112
      Top = 16
      Width = 89
      Height = 25
      CalAlignment = dtaLeft
      Date = 37172.5661734144
      Time = 37172.5661734144
      DateFormat = dfShort
      DateMode = dmComboBox
      Kind = dtkDate
      ParseInput = False
      TabOrder = 1
      OnChange = DateTimePicker2Change
    end
    object DateTimePicker4: TDateTimePicker
      Left = 208
      Top = 16
      Width = 73
      Height = 25
      CalAlignment = dtaLeft
      Date = 37172.5977914468
      Time = 37172.5977914468
      DateFormat = dfShort
      DateMode = dmComboBox
      Kind = dtkTime
      ParseInput = False
      TabOrder = 2
    end
    object RadioButton1: TRadioButton
      Left = 112
      Top = 40
      Width = 57
      Height = 25
      Caption = 'Silent'
      Checked = True
      TabOrder = 3
      TabStop = True
    end
    object RadioButton2: TRadioButton
      Left = 208
      Top = 48
      Width = 73
      Height = 17
      Caption = 'With Tone'
      TabOrder = 4
    end
  end
  object Edit2: TEdit
    Left = 88
    Top = 48
    Width = 201
    Height = 21
    TabOrder = 1
    Text = 'Edit2'
  end
  object Button1: TButton
    Left = 8
    Top = 256
    Width = 81
    Height = 25
    Caption = '&OK'
    TabOrder = 5
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 208
    Top = 256
    Width = 89
    Height = 25
    Caption = '&Cancel'
    TabOrder = 6
    OnClick = Button2Click
  end
  object DateTimePicker3: TDateTimePicker
    Left = 216
    Top = 112
    Width = 73
    Height = 25
    CalAlignment = dtaLeft
    Date = 37172.5971435185
    Time = 37172.5971435185
    DateFormat = dfShort
    DateMode = dmComboBox
    Kind = dtkTime
    ParseInput = False
    TabOrder = 3
  end
  object CheckBox2: TCheckBox
    Left = 16
    Top = 224
    Width = 81
    Height = 17
    Caption = 'Recurrance'
    TabOrder = 7
    OnClick = CheckBox2Click
  end
  object ComboBox2: TComboBox
    Left = 120
    Top = 224
    Width = 169
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 8
    Text = '1 day'
    Items.Strings = (
      '1 day'
      '1 week (7 days)'
      '2 weeks (14 days)'
      '1 year (365 days)')
  end
  object ComboBox3: TComboBox
    Left = 88
    Top = 80
    Width = 201
    Height = 21
    ItemHeight = 13
    TabOrder = 9
  end
end
