object ConfigForm: TConfigForm
  Left = 235
  Top = 103
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Configuration'
  ClientHeight = 438
  ClientWidth = 475
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 475
    Height = 401
    Align = alTop
    BevelOuter = bvNone
    Caption = 'Panel1'
    TabOrder = 0
    object TreeView1: TTreeView
      Left = 0
      Top = 0
      Width = 137
      Height = 401
      Align = alLeft
      Indent = 19
      ReadOnly = True
      TabOrder = 0
      OnMouseDown = TreeView1MouseDown
      Items.Data = {
        02000000200000000000000000000000FFFFFFFFFFFFFFFF0000000002000000
        0744657669636573200000000100000000000000FFFFFFFFFFFFFFFF00000000
        00000000074D6F64656D2031200000000200000000000000FFFFFFFFFFFFFFFF
        0000000000000000074D6F64656D20321E0000000600000000000000FFFFFFFF
        FFFFFFFF0000000000000000054F74686572}
    end
    object PageControl1: TPageControl
      Left = 137
      Top = 0
      Width = 338
      Height = 401
      ActivePage = OtherTabSheet
      Align = alClient
      TabOrder = 1
      object ModemTabSheet: TTabSheet
        Caption = 'Modem options'
        ImageIndex = 1
        object GroupBox6: TGroupBox
          Left = 0
          Top = 217
          Width = 330
          Height = 96
          Align = alTop
          Caption = 'Modem configuration'
          TabOrder = 0
          object Label4: TLabel
            Left = 8
            Top = 16
            Width = 50
            Height = 13
            Caption = 'Serial port:'
          end
          object Label5: TLabel
            Left = 8
            Top = 40
            Width = 57
            Height = 13
            Caption = 'Connection:'
          end
          object Label10: TLabel
            Left = 8
            Top = 64
            Width = 45
            Height = 13
            Caption = 'PIN code'
          end
          object PortComboBox: TComboBox
            Left = 72
            Top = 16
            Width = 121
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 0
            TabOrder = 0
            Text = 'COM1:'
            OnSelect = PortComboBoxSelect
            Items.Strings = (
              'COM1:'
              'COM2:'
              'COM3:'
              'COM4:')
          end
          object ConnectionComboBox: TComboBox
            Left = 72
            Top = 40
            Width = 121
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 0
            TabOrder = 1
            Text = 'Nokia FBUS'
            OnSelect = ConnectionComboBoxSelect
            Items.Strings = (
              'Nokia FBUS'
              'Nokia MBUS'
              'Nokia DLR3'
              'AT device (19200)'
              'AT device (115200)')
          end
          object PINEdit: TEdit
            Left = 72
            Top = 64
            Width = 121
            Height = 21
            TabOrder = 2
            Text = 'PINEdit'
            OnChange = PINEditChange
          end
        end
        object GroupBox1: TGroupBox
          Left = 0
          Top = 0
          Width = 330
          Height = 217
          Align = alTop
          Caption = 'Modem activity'
          TabOrder = 1
          object Label1: TLabel
            Left = 32
            Top = 48
            Width = 19
            Height = 13
            Caption = 'File:'
          end
          object Label2: TLabel
            Left = 32
            Top = 112
            Width = 19
            Height = 13
            Caption = 'File:'
          end
          object Label14: TLabel
            Left = 32
            Top = 160
            Width = 19
            Height = 13
            Caption = 'File:'
          end
          object Label15: TLabel
            Left = 32
            Top = 184
            Width = 35
            Height = 13
            Caption = 'Format:'
          end
          object IncomingLogCheckBox: TCheckBox
            Left = 8
            Top = 32
            Width = 193
            Height = 17
            Caption = 'Backup incoming messages'
            TabOrder = 0
            OnClick = IncomingLogCheckBoxClick
          end
          object ReceiveLogEdit: TEdit
            Left = 56
            Top = 48
            Width = 193
            Height = 21
            TabOrder = 1
            Text = 'ReceiveLogEdit'
            OnChange = ReceiveLogEditChange
          end
          object Button1: TButton
            Left = 256
            Top = 48
            Width = 65
            Height = 25
            Caption = 'Find'
            TabOrder = 2
            OnClick = Button1Click
          end
          object OutgoingLogCheckBox: TCheckBox
            Left = 8
            Top = 96
            Width = 153
            Height = 17
            Caption = 'Backup outgoing messages'
            TabOrder = 3
            OnClick = OutgoingLogCheckBoxClick
          end
          object SendLogEdit: TEdit
            Left = 56
            Top = 112
            Width = 193
            Height = 21
            TabOrder = 4
            Text = 'SendLogEdit'
            OnChange = SendLogEditChange
          end
          object Button2: TButton
            Left = 256
            Top = 112
            Width = 65
            Height = 25
            Caption = 'Find'
            TabOrder = 5
            OnClick = Button2Click
          end
          object TransmissionCheckBox: TCheckBox
            Left = 8
            Top = 144
            Width = 177
            Height = 17
            Caption = 'Log communication with device'
            TabOrder = 6
            OnClick = TransmissionCheckBoxClick
          end
          object TransmissionFileEdit: TEdit
            Left = 72
            Top = 160
            Width = 177
            Height = 21
            TabOrder = 7
            Text = 'TransmissionFileEdit'
            OnChange = TransmissionFileEditChange
          end
          object Button6: TButton
            Left = 256
            Top = 160
            Width = 65
            Height = 25
            Caption = 'Find'
            TabOrder = 8
            OnClick = Button6Click
          end
          object ReceiveCheckBox: TCheckBox
            Left = 8
            Top = 16
            Width = 169
            Height = 17
            Caption = 'Receive messages'
            TabOrder = 9
            OnClick = ReceiveCheckBoxClick
          end
          object SendCheckBox: TCheckBox
            Left = 8
            Top = 80
            Width = 105
            Height = 17
            Caption = 'Send messages'
            TabOrder = 10
            OnClick = SendCheckBoxClick
          end
          object Panel3: TPanel
            Left = 16
            Top = 76
            Width = 297
            Height = 2
            TabOrder = 11
          end
          object Panel4: TPanel
            Left = 16
            Top = 140
            Width = 297
            Height = 2
            TabOrder = 12
          end
          object TransmissionFormatComboBox: TComboBox
            Left = 72
            Top = 184
            Width = 177
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 3
            TabOrder = 13
            Text = 'Only errors'
            OnChange = TransmissionFormatComboBoxChange
            Items.Strings = (
              'Binary format'
              'Communication'
              'Full communication'
              'Only errors')
          end
        end
      end
      object OtherTabSheet: TTabSheet
        Caption = 'Other'
        ImageIndex = 2
        object GroupBox3: TGroupBox
          Left = 0
          Top = 121
          Width = 330
          Height = 120
          Align = alTop
          Caption = 'Sending SMS'
          TabOrder = 0
          object Label17: TLabel
            Left = 8
            Top = 16
            Width = 254
            Height = 13
            Caption = 'Maximal number of SMS for sending in one sequence:'
          end
          object TLabel1: TLabel
            Left = 8
            Top = 40
            Width = 220
            Height = 13
            Caption = 'Maximal number of retries during sending SMS:'
          end
          object Label18: TLabel
            Left = 8
            Top = 64
            Width = 211
            Height = 13
            Caption = 'How many second wait during sending SMS:'
          end
          object SMSNum: TEdit
            Left = 272
            Top = 16
            Width = 49
            Height = 21
            TabOrder = 0
            Text = 'SMSNum'
          end
          object CollectCheckBox: TCheckBox
            Left = 8
            Top = 88
            Width = 305
            Height = 25
            Caption = 'When sending is disabled for devices, collect SMS for later'
            TabOrder = 1
          end
          object SendingRetriesEdit: TEdit
            Left = 272
            Top = 40
            Width = 49
            Height = 21
            TabOrder = 2
            Text = 'SendingRetriesEdit'
          end
          object SendingTimeoutEdit: TEdit
            Left = 272
            Top = 64
            Width = 49
            Height = 21
            TabOrder = 3
            Text = 'SendingTimeoutEdit'
          end
        end
        object GroupBox5: TGroupBox
          Left = 0
          Top = 73
          Width = 330
          Height = 48
          Align = alTop
          Caption = 'Directories with temporary files'
          TabOrder = 1
          object Label20: TLabel
            Left = 8
            Top = 16
            Width = 68
            Height = 13
            Caption = 'Outgoing msg:'
          end
          object OutgoingEdit: TEdit
            Left = 80
            Top = 16
            Width = 185
            Height = 21
            TabOrder = 0
            Text = 'OutgoingEdit'
          end
          object Button12: TButton
            Left = 272
            Top = 16
            Width = 49
            Height = 25
            Caption = 'Find'
            TabOrder = 1
            OnClick = Button12Click
          end
        end
        object GroupBox7: TGroupBox
          Left = 0
          Top = 241
          Width = 330
          Height = 96
          Align = alTop
          Caption = 'Phonebook'
          TabOrder = 2
          object PhonebookListBox: TListBox
            Left = 8
            Top = 16
            Width = 201
            Height = 73
            ItemHeight = 13
            MultiSelect = True
            TabOrder = 0
            OnClick = PhonebookListBoxClick
          end
          object PhonebookAddButton: TButton
            Left = 224
            Top = 16
            Width = 97
            Height = 25
            Caption = 'Add'
            TabOrder = 1
            OnClick = PhonebookAddButtonClick
          end
          object PhonebookEditButton: TButton
            Left = 224
            Top = 40
            Width = 97
            Height = 25
            Caption = 'Edit'
            TabOrder = 2
            OnClick = PhonebookEditButtonClick
          end
          object PhonebookRemoveButton: TButton
            Left = 224
            Top = 64
            Width = 97
            Height = 25
            Caption = 'Remove'
            TabOrder = 3
            OnClick = PhonebookRemoveButtonClick
          end
        end
        object GroupBox4: TGroupBox
          Left = 0
          Top = 0
          Width = 330
          Height = 73
          Align = alTop
          Caption = 'Log'
          TabOrder = 3
          object Label16: TLabel
            Left = 32
            Top = 40
            Width = 19
            Height = 13
            Caption = 'File:'
          end
          object GatewayLogCheckBox: TCheckBox
            Left = 8
            Top = 16
            Width = 233
            Height = 17
            Caption = 'General Gateway issues (like resetting)'
            TabOrder = 0
          end
          object GatewayLogFileEdit: TEdit
            Left = 56
            Top = 40
            Width = 209
            Height = 21
            TabOrder = 1
            Text = 'GatewayLogFileEdit'
          end
          object Button5: TButton
            Left = 272
            Top = 40
            Width = 49
            Height = 25
            Caption = 'Find'
            TabOrder = 2
            OnClick = Button5Click
          end
        end
      end
    end
  end
  object Button3: TButton
    Left = 304
    Top = 408
    Width = 81
    Height = 25
    Caption = '&OK'
    TabOrder = 1
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 392
    Top = 408
    Width = 75
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object OpenDialog1: TOpenDialog
    Filter = 'All Files (*.*)|*.*|Text Files (*.txt)|*.txt'
    FilterIndex = 2
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofNoReadOnlyReturn, ofEnableSizing, ofDontAddToRecent]
    Title = 'Please select file'
    Left = 13
    Top = 281
  end
end
