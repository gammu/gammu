object BackupProgressForm: TBackupProgressForm
  Left = 234
  Top = 220
  BorderStyle = bsToolWindow
  Caption = 'Backup/Restore'
  ClientHeight = 219
  ClientWidth = 329
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  OnCloseQuery = FormCloseQuery
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 16
    Width = 313
    Height = 153
    Caption = 'Progress'
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 72
      Width = 114
      Height = 13
      Caption = 'Phonebook from phone:'
    end
    object Label2: TLabel
      Left = 8
      Top = 96
      Width = 127
      Height = 13
      Caption = 'Phonebook from SIM card:'
    end
    object Label3: TLabel
      Left = 8
      Top = 24
      Width = 64
      Height = 13
      Caption = 'Caller groups:'
    end
    object Label4: TLabel
      Left = 8
      Top = 48
      Width = 67
      Height = 13
      Caption = 'Operator logo:'
    end
    object Label5: TLabel
      Left = 144
      Top = 48
      Width = 57
      Height = 13
      Caption = '                   '
    end
    object Label6: TLabel
      Left = 8
      Top = 120
      Width = 101
      Height = 13
      Caption = 'Startup text and logo:'
    end
    object Label7: TLabel
      Left = 144
      Top = 120
      Width = 108
      Height = 13
      Caption = '                                    '
    end
    object ProgressBar1: TProgressBar
      Left = 144
      Top = 72
      Width = 153
      Height = 17
      Min = 0
      Max = 100
      Smooth = True
      Step = 1
      TabOrder = 0
    end
    object ProgressBar2: TProgressBar
      Left = 144
      Top = 96
      Width = 153
      Height = 17
      Min = 0
      Max = 100
      Smooth = True
      Step = 1
      TabOrder = 1
    end
    object ProgressBar3: TProgressBar
      Left = 144
      Top = 24
      Width = 153
      Height = 17
      Min = 0
      Max = 5
      Smooth = True
      Step = 1
      TabOrder = 2
    end
  end
  object Button1: TButton
    Left = 104
    Top = 184
    Width = 105
    Height = 25
    Caption = '&Cancel'
    TabOrder = 1
    OnClick = Button1Click
  end
end
