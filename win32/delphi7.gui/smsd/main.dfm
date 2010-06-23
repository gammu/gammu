object MainForm: TMainForm
  Left = 219
  Top = 110
  Width = 554
  Height = 422
  HelpType = htKeyword
  HelpKeyword = '0'
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Gammu Gateway'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 0
    Top = 111
    Width = 546
    Height = 4
    Cursor = crVSplit
    Align = alTop
  end
  object Splitter2: TSplitter
    Left = 0
    Top = 269
    Width = 546
    Height = 3
    Cursor = crVSplit
    Align = alBottom
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 351
    Width = 546
    Height = 25
    Panels = <
      item
        Alignment = taCenter
        Width = 150
      end
      item
        Alignment = taCenter
        Width = 150
      end
      item
        Alignment = taCenter
        Width = 150
      end
      item
        Alignment = taCenter
        Width = 150
      end>
    OnDblClick = Getinfoaboutdevices1Click
  end
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 546
    Height = 111
    Align = alTop
    Caption = 'Incoming messages'
    TabOrder = 0
    object IncomingSMSListView: TListView
      Left = 2
      Top = 15
      Width = 542
      Height = 94
      Align = alClient
      Columns = <
        item
          Caption = 'Text'
          MaxWidth = 50
        end
        item
          Caption = 'Device'
        end
        item
          Caption = 'Date/Time'
          Width = 150
        end
        item
          Caption = 'Sender'
          Width = 150
        end>
      GridLines = True
      ReadOnly = True
      RowSelect = True
      SortType = stBoth
      TabOrder = 0
      ViewStyle = vsReport
      OnColumnClick = IncomingSMSListViewColumnClick
      OnCompare = IncomingSMSListViewCompare
      OnKeyDown = FormKeyDown
    end
  end
  object GroupBox2: TGroupBox
    Left = 0
    Top = 115
    Width = 546
    Height = 154
    Align = alClient
    Caption = 'Outgoing messages'
    TabOrder = 1
    object OutgoingSMSListView: TListView
      Left = 2
      Top = 15
      Width = 542
      Height = 137
      Align = alClient
      Columns = <
        item
          Caption = 'Text'
          MaxWidth = 50
        end
        item
          Caption = 'Device'
        end
        item
          Caption = 'Date/Time'
          Width = 150
        end
        item
          Caption = 'Receiver'
          Width = 150
        end
        item
          Caption = 'Status'
          Width = 45
        end>
      GridLines = True
      ReadOnly = True
      RowSelect = True
      SortType = stBoth
      TabOrder = 0
      ViewStyle = vsReport
      OnColumnClick = OutgoingSMSListViewColumnClick
      OnCompare = OutgoingSMSListViewCompare
      OnDblClick = SendSMS1Click
      OnKeyDown = FormKeyDown
    end
  end
  object GroupBox3: TGroupBox
    Left = 0
    Top = 272
    Width = 546
    Height = 79
    Align = alBottom
    Caption = 'General gateway log'
    TabOrder = 2
    object LogListView: TListView
      Left = 2
      Top = 15
      Width = 542
      Height = 62
      Align = alClient
      Columns = <
        item
          Caption = 'Device'
        end
        item
          Caption = 'Date/Time'
          Width = 150
        end
        item
          Caption = 'Event'
          Width = 290
        end>
      GridLines = True
      ReadOnly = True
      RowSelect = True
      SortType = stBoth
      TabOrder = 0
      ViewStyle = vsReport
      OnColumnClick = LogListViewColumnClick
      OnCompare = LogListViewCompare
      OnKeyDown = FormKeyDown
    end
  end
  object MainMenu1: TMainMenu
    Left = 280
    Top = 56
    object File1: TMenuItem
      Caption = 'File'
      object Exit1: TMenuItem
        Caption = 'Exit'
        OnClick = Exit1Click
      end
    end
    object ools1: TMenuItem
      Caption = 'Tools'
      object Getinfoaboutdevices1: TMenuItem
        Caption = 'Info about devices'
        Enabled = False
        OnClick = Getinfoaboutdevices1Click
      end
      object SendSMS1: TMenuItem
        Caption = 'Send SMS'
        Enabled = False
        OnClick = SendSMS1Click
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object Restartallconnections1: TMenuItem
        Caption = 'Restart all connections'
        OnClick = Restartallconnections1Click
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object Options1: TMenuItem
        Caption = 'Options'
        OnClick = Options1Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object Readme1: TMenuItem
        Caption = 'Readme'
        OnClick = Readme1Click
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object About1: TMenuItem
        Caption = 'About'
        OnClick = About1Click
      end
    end
  end
  object SendSMSTimer: TTimer
    Enabled = False
    OnTimer = SendSMSTimerTimer
    Left = 312
    Top = 56
  end
end
