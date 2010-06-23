unit Calendar;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls,GnokiiAPI,MainUnit;

type
  TCalendarForm = class(TForm)
    Label1: TLabel;
    ComboBox1: TComboBox;
    DateTimePicker1: TDateTimePicker;
    Label2: TLabel;
    GroupBox1: TGroupBox;
    CheckBox1: TCheckBox;
    DateTimePicker2: TDateTimePicker;
    Edit2: TEdit;
    Label3: TLabel;
    Label4: TLabel;
    Button1: TButton;
    Button2: TButton;
    DateTimePicker3: TDateTimePicker;
    DateTimePicker4: TDateTimePicker;
    Label5: TLabel;
    Label6: TLabel;
    CheckBox2: TCheckBox;
    ComboBox2: TComboBox;
    RadioButton1: TRadioButton;
    RadioButton2: TRadioButton;
    ComboBox3: TComboBox;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure CheckBox1Click(Sender: TObject);
    procedure DateTimePicker1Change(Sender: TObject);
    procedure DateTimePicker2Change(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure ComboBox1Select(Sender: TObject);
    procedure CheckBox2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  CalendarForm: TCalendarForm;

implementation

{$R *.dfm}

procedure TCalendarForm.Button1Click(Sender: TObject);
var
  i,j,z:integer;
  s:string;
begin
  case ComboBox1.ItemIndex of
    0:setcalendarnote.Type2:=GCN_BIRTHDAY;
    1:setcalendarnote.Type2:=GCN_CALL;
    2:setcalendarnote.Type2:=GCN_MEETING;
    3:setcalendarnote.Type2:=GCN_REMINDER;
  end;

  for i:=1 to 59 do
    setcalendarnote.Text[i]:=chr(0);
  for i:=1 to strlen(pchar(Edit2.text)) do
    setcalendarnote.Text[i]:=Edit2.Text[i];

  for i:=1 to 37 do
    setcalendarnote.Phone[i]:=chr(0);
  if (ComboBox3.ItemIndex=-1) then
  begin
    for i:=1 to strlen(pchar(ComboBox3.text)) do
      setcalendarnote.Phone[i]:=ComboBox3.Text[i];
  end else
  begin
    ReadPBKOffline;
    for z:=1 to PBKOfflineNumber do
    begin
      MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[z].Name;
      if (MainForm.Label16.Caption=ComboBox3.Text) then
      begin
        if (PhoneBackup.PhonePhonebook[z].Number[1]<>chr(0)) then
        begin
          MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[z].Number;
          for i:=1 to strlen(pchar(MainForm.Label14.Caption)) do
            setcalendarnote.Phone[i]:=PhoneBackup.PhonePhonebook[z].Number[i];
        end else
        begin
          for j:=1 to PhoneBackup.PhonePhonebook[z].SubEntriesCount do
          begin
            if PhoneBackup.PhonePhonebook[z].SubEntries[j].EntryType=GSM_Number then
            begin
              MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[z].SubEntries[j].Number;
              for i:=1 to strlen(pchar(MainForm.Label14.Caption)) do
                setcalendarnote.Phone[i]:=PhoneBackup.PhonePhonebook[z].SubEntries[j].Number[i];
              break;
            end;
          end;
        end;
      end;
    end;
  end;

  DateTimeToString(s, 'yyyy',CalendarForm.DateTimePicker1.Date);
  setcalendarnote.Time.Year:=strtoint(s);
  DateTimeToString(s, 'm',CalendarForm.DateTimePicker1.Date);
  setcalendarnote.Time.Month:=strtoint(s);
  DateTimeToString(s, 'd',CalendarForm.DateTimePicker1.Date);
  setcalendarnote.Time.Day:=strtoint(s);
  DateTimeToString(s, 'h',CalendarForm.DateTimePicker3.Time);
  setcalendarnote.Time.Hour:=strtoint(s);
  DateTimeToString(s, 'n',CalendarForm.DateTimePicker3.Time);
  setcalendarnote.Time.Minute:=strtoint(s);
  DateTimeToString(s, 's',CalendarForm.DateTimePicker3.Time);
  setcalendarnote.Time.Second:=strtoint(s);
  setcalendarnote.Time.IsSet:=1;

  DateTimeToString(s, 'yyyy',CalendarForm.DateTimePicker2.Date);
  setcalendarnote.Alarm.Year:=strtoint(s);
  DateTimeToString(s, 'm',CalendarForm.DateTimePicker2.Date);
  setcalendarnote.Alarm.Month:=strtoint(s);
  DateTimeToString(s, 'd',CalendarForm.DateTimePicker2.Date);
  setcalendarnote.Alarm.Day:=strtoint(s);
  DateTimeToString(s, 'h',CalendarForm.DateTimePicker4.Time);
  setcalendarnote.Alarm.Hour:=strtoint(s);
  DateTimeToString(s, 'n',CalendarForm.DateTimePicker4.Time);
  setcalendarnote.Alarm.Minute:=strtoint(s);
  DateTimeToString(s, 's',CalendarForm.DateTimePicker4.Time);
  setcalendarnote.Alarm.Second:=strtoint(s);

  if not CheckBox1.Checked then
    setcalendarnote.Alarm.Year:=0;

  if CheckBox2.Checked then
  begin
    case ComboBox2.ItemIndex of
      0:setcalendarnote.Recurrance:=1*24;
      1:setcalendarnote.Recurrance:=7*24;
      2:setcalendarnote.Recurrance:=14*24;
      3:setcalendarnote.Recurrance:=365*24;
    end;
  end else
  begin
    setcalendarnote.Recurrance:=0;
  end;

  if RadioButton1.Checked then
    setcalendarnote.AlarmType:=0
  else
    setcalendarnote.AlarmType:=1;
    
  GSM_DeleteCalendarNote(@setcalendarnote);
  GSM_WriteCalendarNote(@setcalendarnote);

  MainForm.CalendarTabSheetShow(nil);

  CalendarForm.Close;
end;

procedure TCalendarForm.Button2Click(Sender: TObject);
begin
  CalendarForm.Close;
end;

procedure TCalendarForm.CheckBox1Click(Sender: TObject);
begin
  if CheckBox1.Checked then
  begin
    DateTimePicker2.Enabled:=true;
    DateTimePicker4.Enabled:=true;
  end else
  begin
    DateTimePicker2.Enabled:=false;
    DateTimePicker4.Enabled:=false;
  end;
end;

procedure TCalendarForm.DateTimePicker1Change(Sender: TObject);
begin
  Label5.Caption:=DayOfWeek3(DateTimePicker1.Date);
end;

procedure TCalendarForm.DateTimePicker2Change(Sender: TObject);
begin
  Label6.Caption:=DayOfWeek3(DateTimePicker2.Date);
end;

procedure TCalendarForm.FormShow(Sender: TObject);
begin
  Label5.Caption:=DayOfWeek3(DateTimePicker1.Date);
  Label6.Caption:=DayOfWeek3(DateTimePicker2.Date);
end;

procedure TCalendarForm.ComboBox1Select(Sender: TObject);
begin
  if GSM_GetModelFeature(FN_CALENDAR)=F_CAL71 then
  begin
    CalendarForm.RadioButton1.Visible:=True;
    CalendarForm.RadioButton2.Visible:=True;
    CalendarForm.CheckBox2.Visible:=True;
    CalendarForm.ComboBox2.Visible:=True;
    if ComboBox1.ItemIndex=0 then //birthday
    begin
      CalendarForm.RadioButton1.Enabled:=True;
      CalendarForm.RadioButton2.Enabled:=True;
      CalendarForm.CheckBox2.Enabled:=False;
      CalendarForm.ComboBox2.Enabled:=False;
    end else
    begin
      CalendarForm.RadioButton1.Enabled:=False;
      CalendarForm.RadioButton2.Enabled:=False;
      CalendarForm.CheckBox2.Enabled:=True;
      CalendarForm.ComboBox2.Enabled:=CalendarForm.CheckBox2.Checked;
    end;
    if ComboBox1.ItemIndex=3 then //memo
    begin
      CalendarForm.CheckBox1.Enabled:=False;
      CalendarForm.Label6.Enabled:=False;
      CalendarForm.DateTimePicker2.Enabled:=False;
      CalendarForm.DateTimePicker4.Enabled:=False;
    end else begin
      CalendarForm.CheckBox1.Enabled:=True;
      CalendarForm.Label6.Enabled:=True;
      CalendarForm.DateTimePicker2.Enabled:=True;
      CalendarForm.DateTimePicker4.Enabled:=True;
    end;
  end else begin
    CalendarForm.RadioButton1.Visible:=False;
    CalendarForm.RadioButton2.Visible:=False;
    CalendarForm.CheckBox2.Visible:=False;
    CalendarForm.ComboBox2.Visible:=False;
  end;
  if ComboBox1.ItemIndex=1 then //call
  begin
    CalendarForm.ComboBox3.Enabled:=True;
    CalendarForm.Label4.Enabled:=True;
  end else
  begin
    CalendarForm.ComboBox3.Enabled:=False;
    CalendarForm.Label4.Enabled:=False;
  end;
end;

procedure TCalendarForm.CheckBox2Click(Sender: TObject);
begin
  CalendarForm.ComboBox2.Enabled:=CalendarForm.CheckBox2.Checked;
end;

end.
