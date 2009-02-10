object BackupForm: TBackupForm
  Left = 247
  Top = 143
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Backup from phone to file'
  ClientHeight = 311
  ClientWidth = 395
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 0
    Top = 0
    Width = 395
    Height = 265
    ActivePage = TabSheet3
    Align = alTop
    Style = tsFlatButtons
    TabOrder = 0
    object TabSheet1: TTabSheet
      Caption = 'Step 1/5'
      ImageIndex = 4
      OnShow = TabSheet1Show
      object GroupBox2: TGroupBox
        Left = 0
        Top = 0
        Width = 387
        Height = 73
        Align = alTop
        Caption = 'Please select operation'
        TabOrder = 0
        object BackupRadioButton: TRadioButton
          Left = 8
          Top = 24
          Width = 169
          Height = 17
          Caption = 'Backup data from phone to file'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = BackupRadioButtonClick
        end
        object RestoreRadioButton: TRadioButton
          Left = 8
          Top = 48
          Width = 169
          Height = 17
          Caption = 'Restore data from file to phone'
          TabOrder = 1
          OnClick = RestoreRadioButtonClick
        end
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'Step 2/4'
      ImageIndex = 1
      OnShow = TabSheet2Show
      object Button1: TButton
        Left = 256
        Top = 176
        Width = 121
        Height = 25
        Caption = 'Try to connect'
        TabOrder = 1
        OnClick = Button1Click
      end
      object GroupBox3: TGroupBox
        Left = 0
        Top = 97
        Width = 387
        Height = 72
        Align = alTop
        Caption = 'Connection parameters'
        TabOrder = 0
        object Label7: TLabel
          Left = 8
          Top = 16
          Width = 80
          Height = 13
          Caption = 'Connection type:'
        end
        object Label8: TLabel
          Left = 8
          Top = 40
          Width = 37
          Height = 13
          Caption = 'Device:'
        end
        object ConnectionComboBox: TComboBox
          Left = 144
          Top = 16
          Width = 233
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          ItemIndex = 0
          TabOrder = 0
          Text = 'Nokia infrared'
          OnSelect = ConnectionComboBoxSelect
          Items.Strings = (
            'Nokia infrared'
            'Nokia DKU5 cable'
            'Nokia FBUS cable'
            'Nokia DLR3 cable')
        end
        object DeviceComboBox: TComboBox
          Left = 144
          Top = 40
          Width = 233
          Height = 21
          ItemHeight = 13
          ItemIndex = 1
          TabOrder = 1
          Text = 'COM2:'
          Items.Strings = (
            'COM1:'
            'COM2:'
            'COM3:'
            'COM4:'
            'COM5:'
            'COM6:'
            'COM7:'
            'COM8:'
            'COM9:')
        end
      end
      object GroupBox1: TGroupBox
        Left = 0
        Top = 0
        Width = 387
        Height = 97
        Align = alTop
        Caption = 'Phone details'
        TabOrder = 2
        object Label5: TLabel
          Left = 8
          Top = 16
          Width = 32
          Height = 13
          Caption = 'Model:'
        end
        object Label6: TLabel
          Left = 8
          Top = 40
          Width = 25
          Height = 13
          Caption = 'IMEI:'
        end
        object Label10: TLabel
          Left = 8
          Top = 64
          Width = 45
          Height = 13
          Caption = 'Firmware:'
        end
        object ModelEdit: TEdit
          Left = 144
          Top = 16
          Width = 233
          Height = 21
          TabStop = False
          ReadOnly = True
          TabOrder = 2
        end
        object IMEIEdit: TEdit
          Left = 144
          Top = 40
          Width = 233
          Height = 21
          TabStop = False
          ReadOnly = True
          TabOrder = 1
        end
        object Edit2: TEdit
          Left = 144
          Top = 64
          Width = 233
          Height = 21
          TabStop = False
          ReadOnly = True
          TabOrder = 0
        end
      end
    end
    object TabSheet3: TTabSheet
      Caption = 'Step 3/4'
      ImageIndex = 2
      OnShow = TabSheet3Show
      object GroupBox5: TGroupBox
        Left = 0
        Top = 0
        Width = 387
        Height = 233
        Align = alTop
        Caption = 'File and features'
        TabOrder = 0
        object Label11: TLabel
          Left = 8
          Top = 16
          Width = 48
          Height = 13
          Caption = 'File name:'
        end
        object Label2: TLabel
          Left = 8
          Top = 32
          Width = 70
          Height = 13
          Caption = 'Source phone:'
          Enabled = False
        end
        object Label3: TLabel
          Left = 8
          Top = 48
          Width = 95
          Height = 13
          Caption = 'Source phone IMEI:'
          Enabled = False
        end
        object Label4: TLabel
          Left = 8
          Top = 64
          Width = 69
          Height = 13
          Caption = 'Date and time:'
          Enabled = False
        end
        object Label9: TLabel
          Left = 8
          Top = 80
          Width = 37
          Height = 13
          Caption = 'Creator:'
          Enabled = False
        end
        object CheckBox1: TCheckBox
          Left = 8
          Top = 120
          Width = 201
          Height = 17
          Caption = 'Phone phonebook'
          TabOrder = 0
          OnClick = CheckBox1Click
        end
        object CheckBox2: TCheckBox
          Left = 8
          Top = 136
          Width = 217
          Height = 17
          Caption = 'SIM card phonebook'
          TabOrder = 1
          OnClick = CheckBox2Click
        end
        object CheckBox3: TCheckBox
          Left = 8
          Top = 152
          Width = 241
          Height = 17
          Caption = 'Calendar/reminder notes'
          TabOrder = 2
          OnClick = CheckBox3Click
        end
        object CheckBox4: TCheckBox
          Left = 8
          Top = 168
          Width = 201
          Height = 17
          Caption = 'ToDo'
          TabOrder = 3
          OnClick = CheckBox4Click
        end
        object Button5: TButton
          Left = 312
          Top = 120
          Width = 65
          Height = 17
          Caption = 'Select all'
          TabOrder = 4
          OnClick = Button5Click
        end
        object Button7: TButton
          Left = 312
          Top = 136
          Width = 65
          Height = 17
          Caption = 'Unselect all'
          TabOrder = 5
          OnClick = Button7Click
        end
        object CheckBox5: TCheckBox
          Left = 168
          Top = 120
          Width = 129
          Height = 17
          Caption = 'WAP bookmarks'
          TabOrder = 6
          OnClick = CheckBox5Click
        end
        object CheckBox6: TCheckBox
          Left = 168
          Top = 136
          Width = 129
          Height = 17
          Caption = 'WAP settings'
          TabOrder = 7
          OnClick = CheckBox6Click
        end
        object CheckBox7: TCheckBox
          Left = 8
          Top = 184
          Width = 169
          Height = 17
          Caption = 'MMS settings'
          TabOrder = 8
          OnClick = CheckBox7Click
        end
        object FileNameEdit: TEdit
          Left = 112
          Top = 16
          Width = 193
          Height = 21
          ReadOnly = True
          TabOrder = 9
        end
        object Button2: TButton
          Left = 312
          Top = 16
          Width = 65
          Height = 17
          Caption = 'Select'
          TabOrder = 10
          OnClick = Button2Click
        end
        object Edit1: TEdit
          Left = 112
          Top = 32
          Width = 193
          Height = 21
          ReadOnly = True
          TabOrder = 11
        end
        object Edit4: TEdit
          Left = 112
          Top = 48
          Width = 193
          Height = 21
          ReadOnly = True
          TabOrder = 12
        end
        object Edit7: TEdit
          Left = 112
          Top = 64
          Width = 193
          Height = 21
          ReadOnly = True
          TabOrder = 13
        end
        object Edit10: TEdit
          Left = 112
          Top = 80
          Width = 193
          Height = 21
          ReadOnly = True
          TabOrder = 14
        end
        object Panel1: TPanel
          Left = 24
          Top = 112
          Width = 337
          Height = 2
          TabOrder = 15
        end
        object CheckBox8: TCheckBox
          Left = 168
          Top = 152
          Width = 121
          Height = 17
          Caption = 'FM radio'
          TabOrder = 16
          OnClick = CheckBox8Click
        end
        object CheckBox9: TCheckBox
          Left = 168
          Top = 168
          Width = 137
          Height = 17
          Caption = 'GPRS access points'
          TabOrder = 17
          OnClick = CheckBox9Click
        end
      end
    end
    object TabSheet4: TTabSheet
      Caption = 'Step 4/4'
      ImageIndex = 3
      OnShow = TabSheet4Show
      object Memo1: TMemo
        Left = 0
        Top = 0
        Width = 387
        Height = 213
        Align = alClient
        TabOrder = 0
      end
      object ProgressBar1: TProgressBar
        Left = 0
        Top = 213
        Width = 387
        Height = 21
        Align = alBottom
        Smooth = True
        Step = 1
        TabOrder = 1
      end
    end
  end
  object NextButton: TButton
    Left = 304
    Top = 264
    Width = 81
    Height = 25
    Caption = 'Next'
    TabOrder = 4
    OnClick = NextButtonClick
  end
  object PrevButton: TButton
    Left = 216
    Top = 264
    Width = 81
    Height = 25
    Caption = 'Previous'
    Enabled = False
    TabOrder = 2
    OnClick = PrevButtonClick
  end
  object Button8: TButton
    Left = 8
    Top = 264
    Width = 81
    Height = 25
    Caption = 'About'
    TabOrder = 1
    OnClick = Button8Click
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 293
    Width = 395
    Height = 18
    Panels = <
      item
        Alignment = taCenter
        Text = 'Please select operation and click Next'
        Width = 50
      end
      item
        Alignment = taCenter
        Text = 'DISCONECTED'
        Width = 150
      end>
  end
  object OpenRestoreDialog: TOpenDialog
    Options = [ofPathMustExist, ofFileMustExist, ofEnableSizing, ofForceShowHidden]
    Left = 116
    Top = 264
  end
  object SaveBackupDialog: TSaveDialog
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofCreatePrompt, ofNoReadOnlyReturn, ofEnableSizing]
    Left = 152
    Top = 264
  end
end
