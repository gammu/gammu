Attribute VB_Name = "Module1"
Type GSM_DateTime
    IsSet As Integer
    Year As Integer
    Month As Integer
    Day As Integer
    Hour As Integer
    Minute As Integer
    Second As Integer
    Timezone As Integer
End Type

Type GSM_MessageCenter
    No As Integer
    Name As String * 20             'array[1..20] of char;
    Format As Integer 'GSMF_Text
    Validity As Integer 'GSMV_Max_Time or 4320 [minutes] (ie. 72 hours)
    number As String * 40           'array[1..40] of char;
    DefaultRecipient As String * 40     'array[1..40] of char;
End Type

Type GSM_SMSMessage
    Coding As Integer '=GSM_Coding_Default
    Time As GSM_DateTime
    SMSCTime As GSM_DateTime
    Length As Integer '= 8
    Validity As Integer
    UDHType As Integer '=GSM_NoUDH
    UDH As String * 140             '[1..140] of char
    MessageText As String * 161         'array[1..161] of char
    MessageCenter As GSM_MessageCenter
    Sender As String * 41           'array[1..41] of char
    Destination As String * 41 '=chr(0)         'array[1..41] of char
    MessageNumber As Integer
    MemoryType As Integer
    Type2 As Integer 'GST_SMS or GST_DR (DR=Delivery Report)
    Status As Integer '=GSS_SENTREAD or GSS_NOTSENTREAD
    Class2 As Integer '=1
    Compression As Integer
    Location As Integer '=setsmslocation
    ReplyViaSameSMSC As Integer '=1 or 0
    folder As Integer '=GST_OUTBOX
    SMSData As Integer
    Name As String * 26
End Type

Type GSM_MultiSMSMessage
  number As Integer
  SMS(6) As GSM_SMSMessage
End Type

Public Declare Function GSM_SendSMSMessage Lib "gnokiiapi.dll" Alias "mysendsmsmessage" (ByRef SMSMSG As GSM_SMSMessage) As Integer
Public Declare Function GSM_SaveSMSMessage Lib "gnokiiapi.dll" Alias "mysavesmsmessage" (ByRef SMSMSG As GSM_SMSMessage) As Integer
Public Declare Function GSM_Initialize Lib "gnokiiapi.dll" Alias "myinitialize" (ByVal port As String, ByVal model As String, ByVal connectiontype As Integer) As Integer
Public Declare Function GSM_Terminate Lib "gnokiiapi.dll" Alias "myterminate" () As Integer
Public Declare Function GSM_MakeMultiPartSMS2 Lib "gnokiiapi.dll" Alias "mymakemultipartsms2" (ByRef SMS As GSM_MultiSMSMessage, ByVal MessageBuffer As String, ByVal MessageLength As Integer, ByVal UDHType As Integer, ByVal Coding As Integer) As Integer


