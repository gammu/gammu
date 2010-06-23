unit SMSEdit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls,MainUnit,GnokiiAPI, ExtCtrls, ComCtrls;

type
  TSMSEditForm = class(TForm)
    GroupBox1: TGroupBox;
    ComboBox1: TComboBox;
    Label1: TLabel;
    Memo1: TMemo;
    Label2: TLabel;
    Label3: TLabel;
    CheckBox1: TCheckBox;
    Button1: TButton;
    CheckBox2: TCheckBox;
    Button3: TButton;
    Edit1: TEdit;
    GroupBox2: TGroupBox;
    ComboBox2: TComboBox;
    Label4: TLabel;
    ComboBox3: TComboBox;
    Label5: TLabel;
    Label6: TLabel;
    ComboBox4: TComboBox;
    RadioGroup1: TRadioGroup;
    CheckBox3: TCheckBox;
    CheckBox4: TCheckBox;
    ComboBox5: TComboBox;
    SMSEditPageControl: TPageControl;
    TextSMSTabSheet: TTabSheet;
    Label7: TLabel;
    BitmapSMSTabSheet: TTabSheet;
    CalendarSMSTabSheet: TTabSheet;
    RingtoneSMSTabSheet: TTabSheet;
    Label8: TLabel;
    ComboBox6: TComboBox;
    GroupBox3: TGroupBox;
    Label9: TLabel;
    ComboBox7: TComboBox;
    Label10: TLabel;
    CheckBox5: TCheckBox;
    ScrollBox1: TScrollBox;
    Image1: TImage;
    GroupBox4: TGroupBox;
    Memo2: TMemo;
    CheckBox6: TCheckBox;
    GroupBox5: TGroupBox;
    PBKSMSTabSheet: TTabSheet;
    GroupBox6: TGroupBox;
    Label11: TLabel;
    Label12: TLabel;
    Label13: TLabel;
    ComboBox8: TComboBox;
    Label14: TLabel;
    Edit3: TEdit;
    WAPBookTabSheet: TTabSheet;
    WAPSettTabSheet: TTabSheet;
    GroupBox7: TGroupBox;
    GroupBox8: TGroupBox;
    ComboBox9: TComboBox;
    procedure Button3Click(Sender: TObject);
    procedure Memo1Change(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure RadioGroup1Click(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure ComboBox2Change(Sender: TObject);
    procedure ComboBox5Change(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  SMSEditForm: TSMSEditForm;
  maxlength:integer;

implementation

{$R *.dfm}

procedure TSMSEditForm.Button3Click(Sender: TObject);
begin
  SMSEditForm.Close;
end;

procedure TSMSEditForm.Memo1Change(Sender: TObject);
begin
  GroupBox1.Caption:='Text ' + inttostr(Memo1.GetTextLen)+'/'+inttostr(maxlength);
end;

procedure TSMSEditForm.Button1Click(Sender: TObject);
var
  p,i,j:integer;
  z:integer;
  w:word;
  unicode,screensaver:integer;
  smstext:array[1..500] of char;
begin
  if (SMSEditPageControl.ActivePage=TextSMSTabSheet) then
  begin
    z:=GSM_Coding_Default;
    case combobox5.ItemIndex of
      0: z:=GSM_Coding_Default;
      1: z:=GSM_Coding_Unicode;
    end;

    for i:=1 to 500 do
      smstext[i]:=chr(0);
    w:=strlen(pchar(SMSEditForm.Memo1.Text));
    for i:=1 to w do
      smstext[i]:=SMSEditForm.Memo1.Text[i];

    GSM_MakeMultiPartSMS2(@setsms,@smstext,w,GSM_NoUDH,z);

    for i:=1 to setsms.number do
    begin
      if CheckBox1.Checked then
        setsms.sms[i].Class2 := 0
      else
       setsms.sms[i].Class2 := -1;
    end;

    setsms.sms[1].location:=setsmslocation;
  end;
  if (SMSEditPageControl.ActivePage=BitmapSMSTabSheet) then
  begin
    unicode:=0;
    screensaver:=0;
    if bitma.type2=GSM_PictureImage then
    begin
      for i:=1 to 256 do
        bitma.text[i]:=chr(0);
      for i:=1 to strlen(pchar(Memo2.text)) do
        bitma.text[i]:=Memo2.Text[i];
    end;
    GSM_SaveBitmapToSMS(@setsms,@bitma,unicode,screensaver);
  end;
  if (SMSEditPageControl.ActivePage=RingtoneSMSTabSheet) then
  begin
    GSM_SaveRingtoneToSMS(@setsms,@rington,1);
  end;
  if (SMSEditPageControl.ActivePage=WAPBookTabSheet) then
  begin
    GSM_SaveWAPBookmarkToSMS(@setsms,@bookma);
  end;
  if (SMSEditPageControl.ActivePage=WAPSettTabSheet) then
  begin
    GSM_SaveWAPSettingsToSMS(@setsms,@wapset);
  end;
  if (SMSEditPageControl.ActivePage=CalendarSMSTabSheet) then
  begin
    GSM_SaveCalendarNoteToSMS(@setsms,@calenda);
  end;
  if (SMSEditPageControl.ActivePage=PBKSMSTabSheet) then
  begin
    i:=10;
    case ComboBox8.ItemIndex of
      0:i:=10;
      1:i:=21;
    end;
    GSM_SavePhonebookEntryToSMS(@setsms,@pbk,i);
  end;

  for z:=1 to setsms.number do
  begin
    for i:=1 to 40 do
      setsms.sms[z].MessageCenter.number[i]:=chr(0);
    for i:=1 to strlen(pchar(Edit1.text)) do
      setsms.sms[z].MessageCenter.number[i]:=Edit1.Text[i];
    setsms.sms[z].MessageCenter.No := ComboBox2.ItemIndex;
    setsms.sms[z].Validity := 4320;  //do it

    for i:=1 to 41 do
      setsms.sms[z].destination[i]:=chr(0);
    if (ComboBox9.ItemIndex=-1) then
    begin
      for i:=1 to strlen(pchar(ComboBox9.text)) do
        setsms.sms[z].destination[i]:=ComboBox9.Text[i];
    end else
    begin
      ReadPBKOffline;
      for p:=1 to PBKOfflineNumber do
      begin
        MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[p].Name;
        if (MainForm.Label16.Caption=ComboBox9.Text) then
        begin
          if (PhoneBackup.PhonePhonebook[p].Number[1]<>chr(0)) then
          begin
            MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[p].Number;
            for i:=1 to strlen(pchar(MainForm.Label14.Caption)) do
              setsms.sms[z].destination[i]:=PhoneBackup.PhonePhonebook[p].Number[i];
          end else
          begin
            for j:=1 to PhoneBackup.PhonePhonebook[p].SubEntriesCount do
            begin
              if PhoneBackup.PhonePhonebook[p].SubEntries[j].EntryType=GSM_Number then
              begin
                MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[p].SubEntries[j].Number;
                for i:=1 to strlen(pchar(MainForm.Label14.Caption)) do
                  setsms.sms[z].destination[i]:=PhoneBackup.PhonePhonebook[p].SubEntries[j].Number[i];
                break;
              end;
            end;
          end;
        end;
      end;
    end;

    setsms.sms[z].folder:=combobox1.ItemIndex;

    setsms.sms[z].Type2 := GST_SMS;
    //delivery report
    if CheckBox2.Checked then
    begin
      if RadioGroup1.ItemIndex=1 then setsms.sms[z].Type2:= GST_DR;
    end;

    for i:=1 to 26 do
      setsms.sms[z].name[i]:=chr(0);
    for i:=1 to strlen(pchar(SMSEditForm.edit3.Text)) do
      setsms.sms[z].name[i]:=SMSEditForm.edit3.Text[i];

    //read/unread
    if Checkbox4.Checked then
      setsms.sms[z].status:=GSS_SENTREAD
    else
      setsms.sms[z].status:=GSS_NOTSENTREAD;

    if CheckBox3.Checked then
      setsms.sms[z].ReplyViaSameSMSC := 1
    else
      setsms.sms[z].ReplyViaSameSMSC := 0;
  end;

  if Application.MessageBox(pchar('Do you want to save/send '+inttostr(setsms.number)+' sms ?'),'question',MB_YESNO)=IDYES then
  begin
    if (RadioGroup1.itemindex=0) then
    begin
      for z:=1 to setsms.number do
      begin
        GSM_SaveSMSMessage(@setsms.sms[z]);
      end;
    end else
    begin
      for z:=1 to setsms.number do
      begin
        GSM_SendSMSMessage(@setsms.sms[z]);
      end;
    end;
  end;

  MainForm.SMSTabSheetShow(nil);

  SMSEditForm.Close;
end;

procedure TSMSEditForm.RadioGroup1Click(Sender: TObject);
begin
  case RadioGroup1.ItemIndex of
    0:begin
        ComboBox1.Enabled:=true;
        CheckBox2.Enabled:=false;
        CheckBox4.Enabled:=true;
        ComboBox3.Enabled:=false;
        ComboBox4.Enabled:=false;
        Label14.Enabled:=true;
        edit3.Enabled:=true;
      end;
    1:begin
        ComboBox1.Enabled:=false;
        CheckBox2.Enabled:=true;
        CheckBox4.Enabled:=false;
        Label14.Enabled:=false;
        edit3.Enabled:=false;
        if ComboBox2.ItemIndex<>0 then
        begin
          ComboBox3.Enabled:=false;
          ComboBox4.Enabled:=false;
        end else
        begin
          ComboBox3.Enabled:=true;
          ComboBox4.Enabled:=true;
        end;
      end;
  end;
  if ComboBox2.ItemIndex<>0 then
  begin
    Edit1.Enabled:=false;
  end else
  begin
    Edit1.Enabled:=true;
  end;
end;

procedure TSMSEditForm.FormShow(Sender: TObject);
begin
  RadioGroup1Click(nil);
  ComboBox5Change(nil);
//  ScrollBox1.VertScrollBar.Visible:=true;
  ScrollBox1.VertScrollBar.Position:=80;
//  ScrollBox1.VertScrollBar.Visible:=false;
end;

procedure TSMSEditForm.ComboBox2Change(Sender: TObject);
begin
  if ComboBox2.ItemIndex<>0 then
  begin
    SetSMSC(ComboBox2.ItemIndex);
  end;
  RadioGroup1Click(nil);
end;

procedure TSMSEditForm.ComboBox5Change(Sender: TObject);
begin
  case ComboBox5.ItemIndex of
    0:maxlength:=160;
    1:maxlength:=70;
  end;
  Memo1.MaxLength:=maxlength;
  Memo1Change(nil);
end;

end.
