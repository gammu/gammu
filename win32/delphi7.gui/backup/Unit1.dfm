object Form1: TForm1
  Left = 274
  Top = 156
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Backup from phone to file'
  ClientHeight = 313
  ClientWidth = 360
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
    Width = 360
    Height = 257
    ActivePage = TabSheet4
    Align = alTop
    Style = tsFlatButtons
    TabOrder = 0
    object TabSheet0: TTabSheet
      Caption = 'Step 1/5'
      ImageIndex = 4
      object GroupBox2: TGroupBox
        Left = 0
        Top = 0
        Width = 352
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
    object TabSheet1: TTabSheet
      Caption = 'Step 2/5'
      object GroupBox4: TGroupBox
        Left = 0
        Top = 0
        Width = 352
        Height = 145
        Align = alTop
        Caption = 'File details'
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
          Width = 70
          Height = 13
          Caption = 'Source phone:'
          Enabled = False
        end
        object Label3: TLabel
          Left = 8
          Top = 64
          Width = 95
          Height = 13
          Caption = 'Source phone IMEI:'
          Enabled = False
        end
        object Label4: TLabel
          Left = 8
          Top = 88
          Width = 66
          Height = 13
          Caption = 'Date and time'
          Enabled = False
        end
        object Label9: TLabel
          Left = 8
          Top = 112
          Width = 37
          Height = 13
          Caption = 'Creator:'
          Enabled = False
        end
        object FileNameEdit: TEdit
          Left = 112
          Top = 16
          Width = 161
          Height = 21
          ReadOnly = True
          TabOrder = 0
        end
        object Button6: TButton
          Left = 280
          Top = 16
          Width = 65
          Height = 25
          Caption = 'Select'
          TabOrder = 1
          OnClick = Button6Click
        end
        object Edit1: TEdit
          Left = 112
          Top = 40
          Width = 161
          Height = 21
          Enabled = False
          TabOrder = 2
        end
        object Edit4: TEdit
          Left = 112
          Top = 64
          Width = 161
          Height = 21
          Enabled = False
          TabOrder = 3
        end
        object Edit7: TEdit
          Left = 112
          Top = 88
          Width = 161
          Height = 21
          Enabled = False
          TabOrder = 4
        end
        object Edit10: TEdit
          Left = 112
          Top = 112
          Width = 161
          Height = 21
          ReadOnly = True
          TabOrder = 5
        end
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'Step 3/5'
      ImageIndex = 1
      OnShow = TabSheet2Show
      object Button1: TButton
        Left = 224
        Top = 176
        Width = 121
        Height = 25
        Caption = 'Try to connect'
        TabOrder = 0
        OnClick = Button1Click
      end
      object GroupBox3: TGroupBox
        Left = 0
        Top = 97
        Width = 352
        Height = 72
        Align = alTop
        Caption = 'Connection parameters'
        TabOrder = 1
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
          Left = 112
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
          Left = 112
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
        Width = 352
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
          Left = 112
          Top = 16
          Width = 233
          Height = 21
          ReadOnly = True
          TabOrder = 0
        end
        object IMEIEdit: TEdit
          Left = 112
          Top = 40
          Width = 233
          Height = 21
          ReadOnly = True
          TabOrder = 1
        end
        object Edit2: TEdit
          Left = 112
          Top = 64
          Width = 233
          Height = 21
          ReadOnly = True
          TabOrder = 2
        end
      end
    end
    object TabSheet3: TTabSheet
      Caption = 'Step 4/5'
      ImageIndex = 2
      OnShow = TabSheet3Show
      object GroupBox5: TGroupBox
        Left = 0
        Top = 0
        Width = 352
        Height = 161
        Align = alTop
        Caption = 'Please select features'
        TabOrder = 0
        object CheckBox1: TCheckBox
          Left = 8
          Top = 16
          Width = 201
          Height = 17
          Caption = 'Phone phonebook'
          TabOrder = 0
        end
        object CheckBox2: TCheckBox
          Left = 8
          Top = 32
          Width = 217
          Height = 17
          Caption = 'SIM card phonebook'
          TabOrder = 1
        end
        object CheckBox3: TCheckBox
          Left = 8
          Top = 48
          Width = 241
          Height = 17
          Caption = 'Calendar/reminder notes'
          TabOrder = 2
        end
        object CheckBox4: TCheckBox
          Left = 8
          Top = 64
          Width = 201
          Height = 17
          Caption = 'ToDo'
          TabOrder = 3
        end
        object Button5: TButton
          Left = 264
          Top = 16
          Width = 73
          Height = 25
          Caption = 'Select all'
          TabOrder = 4
          OnClick = Button5Click
        end
        object Button7: TButton
          Left = 264
          Top = 48
          Width = 73
          Height = 25
          Caption = 'Unselect all'
          TabOrder = 5
          OnClick = Button7Click
        end
      end
    end
    object TabSheet4: TTabSheet
      Caption = 'Step 5/5'
      ImageIndex = 3
      OnShow = TabSheet4Show
      object Memo1: TMemo
        Left = 0
        Top = 0
        Width = 352
        Height = 205
        Align = alClient
        TabOrder = 0
      end
      object ProgressBar1: TProgressBar
        Left = 0
        Top = 205
        Width = 352
        Height = 21
        Align = alBottom
        Smooth = True
        Step = 1
        TabOrder = 1
      end
    end
  end
  object NextButton: TButton
    Left = 272
    Top = 264
    Width = 81
    Height = 25
    Caption = 'Next'
    TabOrder = 1
    OnClick = NextButtonClick
  end
  object PrevButton: TButton
    Left = 184
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
    TabOrder = 3
    OnClick = Button8Click
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 295
    Width = 360
    Height = 18
    Panels = <
      item
        Text = 'Phone DISCONECTED'
        Width = 150
      end>
  end
  object OpenRestoreDialog: TOpenDialog
    Options = [ofPathMustExist, ofFileMustExist, ofEnableSizing, ofForceShowHidden]
    Left = 100
    Top = 264
  end
  object SaveBackupDialog: TSaveDialog
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofCreatePrompt, ofNoReadOnlyReturn, ofEnableSizing]
    Left = 128
    Top = 264
  end
end
