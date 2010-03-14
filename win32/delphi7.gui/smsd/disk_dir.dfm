object DirectoryForm: TDirectoryForm
  Left = 272
  Top = 194
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Please select directory'
  ClientHeight = 253
  ClientWidth = 293
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
  object OKButton: TButton
    Left = 128
    Top = 224
    Width = 81
    Height = 25
    Caption = 'OK'
    ModalResult = 1
    TabOrder = 0
  end
  object CancelButton: TButton
    Left = 216
    Top = 224
    Width = 73
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object ShellTreeView1: TShellTreeView
    Left = 0
    Top = 0
    Width = 293
    Height = 217
    AutoContextMenus = False
    ObjectTypes = [otFolders]
    Root = 'C:\'
    UseShellImages = True
    Align = alTop
    AutoRefresh = False
    HideSelection = False
    Indent = 19
    ParentColor = False
    ParentShowHint = False
    RightClickSelect = True
    ShowHint = True
    ShowRoot = False
    TabOrder = 2
    OnEditing = ShellTreeView1Editing
  end
end
