object DeviceInfoForm: TDeviceInfoForm
  Left = 361
  Top = 227
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Info about connected devices'
  ClientHeight = 232
  ClientWidth = 314
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object ListView: TListView
    Left = 0
    Top = 0
    Width = 314
    Height = 193
    Align = alTop
    Columns = <
      item
        Caption = 'Device'
      end
      item
        Caption = 'Parameter'
        Width = 140
      end
      item
        Caption = 'Value'
        Width = 120
      end>
    ColumnClick = False
    ReadOnly = True
    RowSelect = True
    TabOrder = 1
    ViewStyle = vsReport
  end
  object Button1: TButton
    Left = 112
    Top = 200
    Width = 89
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
  end
end
