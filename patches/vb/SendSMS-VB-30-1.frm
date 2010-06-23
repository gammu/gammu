VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4065
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   8925
   LinkTopic       =   "Form1"
   ScaleHeight     =   4065
   ScaleWidth      =   8925
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox Text3 
      Height          =   1455
      Left            =   5040
      TabIndex        =   5
      Top             =   2520
      Width           =   3255
   End
   Begin VB.CommandButton SendSMS 
      Caption         =   "Send SMS"
      Height          =   1815
      Left            =   4920
      TabIndex        =   4
      Top             =   240
      Width           =   3135
   End
   Begin VB.TextBox Text2 
      Height          =   495
      Left            =   960
      TabIndex        =   3
      Top             =   3360
      Width           =   3015
   End
   Begin VB.CommandButton Disconnect 
      Caption         =   "Disconnect"
      Height          =   615
      Left            =   840
      TabIndex        =   2
      Top             =   2400
      Width           =   3135
   End
   Begin VB.TextBox Text1 
      Height          =   615
      Left            =   720
      TabIndex        =   1
      Top             =   1440
      Width           =   3255
   End
   Begin VB.CommandButton Initialise 
      Caption         =   "Initalise"
      Height          =   1095
      Left            =   480
      TabIndex        =   0
      Top             =   120
      Width           =   3375
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Private Sub Disconnect_Click()
  dis = GSM_Terminate()
  Text2.Text = dis
End Sub

Private Sub Initialise_Click()
  result = GSM_Initialize("com2:", "6210", 0)
  Text1.Text = result
End Sub

Private Sub SendSMS_Click()
Dim Message As GSM_SMSMessage
Dim setsms As GSM_MultiSMSMessage
Dim smstext As String

w = 4 'length of message to send
z = 2 'encoding default = 2, unicode = 1
smstext = "test" & Chr(0)

result = GSM_MakeMultiPartSMS2(setsms, smstext, w, 0, z)
Text3.Text = result

setsms.SMS(1).Location = 0
setsms.SMS(1).Destination = "+61410509700" & Chr(0)
setsms.SMS(1).folder = 0
setsms.SMS(1).Type2 = 0 'GST_SMS (no delivery reports)
setsms.SMS(1).Name = Chr(0)
setsms.SMS(1).Status = 3 'GSS_NOTSENTREAD
setsms.SMS(1).ReplyViaSameSMSC = 0
setsms.SMS(1).Compression = 0

result = GSM_SaveSMSMessage(setsms.SMS(1))
Text3.Text = result

End Sub


