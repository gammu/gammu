object Form1: TForm1
  Left = 217
  Top = 146
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Example'
  ClientHeight = 145
  ClientWidth = 410
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
    Top = 16
    Width = 22
    Height = 13
    Caption = 'IMEI'
  end
  object Label2: TLabel
    Left = 8
    Top = 48
    Width = 78
    Height = 13
    Caption = 'Simlock network'
  end
  object Label3: TLabel
    Left = 232
    Top = 16
    Width = 57
    Height = 13
    Caption = 'Connection:'
  end
  object Label4: TLabel
    Left = 232
    Top = 48
    Width = 37
    Height = 13
    Caption = 'Device:'
  end
  object Label5: TLabel
    Left = 8
    Top = 80
    Width = 32
    Height = 13
    Caption = 'Model:'
  end
  object Label6: TLabel
    Left = 8
    Top = 112
    Width = 68
    Height = 13
    Caption = 'Security code:'
  end
  object Edit1: TEdit
    Left = 296
    Top = 16
    Width = 97
    Height = 21
    TabOrder = 0
    Text = 'irdaphonet'
  end
  object Edit2: TEdit
    Left = 96
    Top = 16
    Width = 121
    Height = 21
    TabOrder = 1
  end
  object Edit3: TEdit
    Left = 96
    Top = 48
    Width = 121
    Height = 21
    TabOrder = 2
  end
  object Edit4: TEdit
    Left = 296
    Top = 48
    Width = 97
    Height = 21
    TabOrder = 3
    Text = 'com2:'
  end
  object Button1: TButton
    Left = 296
    Top = 104
    Width = 97
    Height = 25
    Caption = 'Make job'
    TabOrder = 4
    OnClick = Button1Click
  end
  object Edit5: TEdit
    Left = 96
    Top = 80
    Width = 121
    Height = 21
    TabOrder = 5
  end
  object Edit6: TEdit
    Left = 96
    Top = 112
    Width = 121
    Height = 21
    TabOrder = 6
  end
end
