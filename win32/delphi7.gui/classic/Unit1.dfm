object Form1: TForm1
  Left = 217
  Top = 146
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Example'
  ClientHeight = 365
  ClientWidth = 498
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Pin: TLabel
    Left = 392
    Top = 312
    Width = 15
    Height = 13
    Caption = 'Pin'
  end
  object ConnectionLabel: TLabel
    Left = 392
    Top = 280
    Width = 89
    Height = 13
    Caption = '>>>connection<<<'
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 8
    Width = 377
    Height = 233
    Caption = 'Events'
    TabOrder = 0
    object InfoListBox: TListBox
      Left = 2
      Top = 15
      Width = 373
      Height = 216
      Align = alClient
      ItemHeight = 13
      TabOrder = 0
    end
  end
  object InitButton: TButton
    Left = 392
    Top = 16
    Width = 97
    Height = 25
    Caption = 'Init'
    TabOrder = 1
    OnClick = InitButtonClick
  end
  object GetNetInfoButton: TButton
    Left = 392
    Top = 80
    Width = 97
    Height = 25
    Caption = 'GetNetInfo'
    TabOrder = 2
    OnClick = GetNetInfoButtonClick
  end
  object GetAllInboxSMSButton: TButton
    Left = 392
    Top = 112
    Width = 97
    Height = 25
    Caption = 'GetAllInboxSMS'
    TabOrder = 3
    OnClick = GetAllInboxSMSButtonClick
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 340
    Width = 498
    Height = 25
    Panels = <
      item
        Width = 150
      end
      item
        Width = 100
      end>
  end
  object DeviceComboBox: TComboBox
    Left = 392
    Top = 256
    Width = 97
    Height = 21
    ItemHeight = 13
    ItemIndex = 1
    TabOrder = 5
    Text = 'com2:'
    Items.Strings = (
      'com1:'
      'com2:'
      'com3:'
      'com4:'
      'com5:'
      'com6:'
      'incorrect port name')
  end
  object PINEdit: TEdit
    Left = 424
    Top = 304
    Width = 65
    Height = 21
    TabOrder = 6
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 248
    Width = 377
    Height = 81
    Caption = 'Send sms'
    TabOrder = 7
    object Label2: TLabel
      Left = 8
      Top = 16
      Width = 53
      Height = 13
      Caption = 'Destination'
    end
    object Label3: TLabel
      Left = 8
      Top = 48
      Width = 21
      Height = 13
      Caption = 'Text'
    end
    object DestinationNumberEdit: TEdit
      Left = 80
      Top = 16
      Width = 153
      Height = 21
      TabOrder = 0
      Text = 'number'
    end
    object DeliveryReportCheckBox: TCheckBox
      Left = 256
      Top = 16
      Width = 105
      Height = 17
      Caption = 'Delivery report'
      TabOrder = 1
    end
    object SMSTextEdit: TEdit
      Left = 80
      Top = 48
      Width = 153
      Height = 21
      TabOrder = 2
      Text = 'test sms'
    end
    object SendButton: TButton
      Left = 256
      Top = 48
      Width = 73
      Height = 25
      Caption = 'Send'
      TabOrder = 3
      OnClick = SendButtonClick
    end
  end
  object GetInfoButton: TButton
    Left = 392
    Top = 144
    Width = 97
    Height = 25
    Caption = 'GetInfo'
    TabOrder = 8
    OnClick = GetInfoButtonClick
  end
  object ResetButton: TButton
    Left = 392
    Top = 176
    Width = 97
    Height = 25
    Caption = 'Reset'
    TabOrder = 9
    OnClick = ResetButtonClick
  end
  object TerminateButton: TButton
    Left = 392
    Top = 48
    Width = 97
    Height = 25
    Caption = 'Terminate'
    TabOrder = 10
    OnClick = TerminateButtonClick
  end
  object Button1: TButton
    Left = 392
    Top = 208
    Width = 97
    Height = 25
    Caption = 'ChangePointer'
    TabOrder = 11
    OnClick = Button1Click
  end
end
