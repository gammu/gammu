unit MainUnit;

interface

{$I-}

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, GnokiiAPI, ExtCtrls, Menus, StdCtrls, Spin, IniFiles;

type
  TMainForm = class(TForm)
    StatusBar1: TStatusBar;
    TreeView1: TTreeView;
    Splitter1: TSplitter;
    MainMenu1: TMainMenu;
    File1: TMenuItem;
    Quit1: TMenuItem;
    Help1: TMenuItem;
    About1: TMenuItem;
    Timer1: TTimer;
    OpenDialog1: TOpenDialog;
    SaveDialog1: TSaveDialog;
    PageControl1: TPageControl;
    NetmonitorTabSheet: TTabSheet;
    GroupBox1: TGroupBox;
    Label1: TLabel;
    NetmonGroupBox: TGroupBox;
    NetmonLabel: TLabel;
    SpinEdit1: TSpinEdit;
    Button11: TButton;
    PhonebookTabSheet: TTabSheet;
    GroupBox2: TGroupBox;
    Panel1: TPanel;
    Label2: TLabel;
    ComboBox1: TComboBox;
    ListView1: TListView;
    PhoneTabSheet: TTabSheet;
    GroupBox3: TGroupBox;
    GroupBox12: TGroupBox;
    Label4: TLabel;
    Label5: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Label6: TLabel;
    Button9: TButton;
    GroupBox14: TGroupBox;
    ListView8: TListView;
    GroupBox11: TGroupBox;
    CheckBox1: TCheckBox;
    LogosTabSheet: TTabSheet;
    GroupBox4: TGroupBox;
    LogosImage: TImage;
    Image1: TImage;
    Image2: TImage;
    ComboBox2: TComboBox;
    Button6: TButton;
    Button7: TButton;
    Button8: TButton;
    Button12: TButton;
    Button4: TButton;
    Button5: TButton;
    RingtonesTabSheet: TTabSheet;
    GroupBox5: TGroupBox;
    Label11: TLabel;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    SMSTabSheet: TTabSheet;
    GroupBox6: TGroupBox;
    Panel2: TPanel;
    Label10: TLabel;
    ComboBox3: TComboBox;
    ListView2: TListView;
    SpeedTabSheet: TTabSheet;
    GroupBox7: TGroupBox;
    ListView3: TListView;
    CalendarTabSheet: TTabSheet;
    GroupBox8: TGroupBox;
    ListView4: TListView;
    SMSCTabSheet: TTabSheet;
    GroupBox9: TGroupBox;
    ListView6: TListView;
    CallerTabSheet: TTabSheet;
    GroupBox10: TGroupBox;
    ListView5: TListView;
    ProfileTabSheet: TTabSheet;
    GroupBox13: TGroupBox;
    ListView7: TListView;
    Panel3: TPanel;
    Splitter2: TSplitter;
    GroupBox15: TGroupBox;
    GroupBox16: TGroupBox;
    Button13: TButton;
    Button14: TButton;
    Label15: TLabel;
    Edit1: TEdit;
    Edit2: TEdit;
    Button16: TButton;
    CalendarPopupMenu: TPopupMenu;
    New1: TMenuItem;
    Edit3: TMenuItem;
    Delete1: TMenuItem;
    SMSPopupMenu: TPopupMenu;
    Edit4: TMenuItem;
    Memo1: TMemo;
    Memo2: TMemo;
    New2: TMenuItem;
    Label14: TLabel;
    Label16: TLabel;
    WapBookTabSheet: TTabSheet;
    GroupBox17: TGroupBox;
    DateTimePicker1: TDateTimePicker;
    ConfigTabSheet: TTabSheet;
    GroupBox18: TGroupBox;
    Configuration1: TMenuItem;
    N1: TMenuItem;
    Delete2: TMenuItem;
    SMSCPopupMenu: TPopupMenu;
    Edit5: TMenuItem;
    Label3: TLabel;
    ComboBox4: TComboBox;
    Label12: TLabel;
    ComboBox5: TComboBox;
    Label13: TLabel;
    CheckBox2: TCheckBox;
    Label17: TLabel;
    Label18: TLabel;
    PBKPopupMenu: TPopupMenu;
    Edit7: TMenuItem;
    Button17: TButton;
    SMS1: TMenuItem;
    Button18: TButton;
    SMSSendSave1: TMenuItem;
    ComboBox6: TComboBox;
    WAPSettTabSheet: TTabSheet;
    DivertTabSheet: TTabSheet;
    GroupBox19: TGroupBox;
    GroupBox20: TGroupBox;
    Timer2: TTimer;
    ListView9: TListView;
    AboutTabSheet: TTabSheet;
    GroupBox21: TGroupBox;
    Label19: TLabel;
    Label20: TLabel;
    PCSMSTabSheet: TTabSheet;
    GroupBox22: TGroupBox;
    Panel4: TPanel;
    Splitter3: TSplitter;
    Panel5: TPanel;
    ListView10: TListView;
    CopytoPC1: TMenuItem;
    N2: TMenuItem;
    Memo4: TMemo;
    Memo5: TMemo;
    PCSMSPopupMenu: TPopupMenu;
    SendSave1: TMenuItem;
    N3: TMenuItem;
    Delete3: TMenuItem;
    Label21: TLabel;
    ComboBox7: TComboBox;
    Label22: TLabel;
    Edit6: TEdit;
    Label23: TLabel;
    Edit8: TEdit;
    Label24: TLabel;
    ComboBox8: TComboBox;
    Label25: TLabel;
    ComboBox9: TComboBox;
    Label26: TLabel;
    ComboBox10: TComboBox;
    Label27: TLabel;
    Edit9: TEdit;
    Label28: TLabel;
    Edit10: TEdit;
    Label29: TLabel;
    Edit11: TEdit;
    Label30: TLabel;
    Edit12: TEdit;
    Label31: TLabel;
    ComboBox11: TComboBox;
    Label32: TLabel;
    ComboBox12: TComboBox;
    Label33: TLabel;
    ComboBox13: TComboBox;
    Label34: TLabel;
    Edit13: TEdit;
    Label35: TLabel;
    Edit14: TEdit;
    Label36: TLabel;
    Edit15: TEdit;
    Label37: TLabel;
    ComboBox14: TComboBox;
    Memo3: TMemo;
    PageControl2: TPageControl;
    TabSheet1: TTabSheet;
    Button19: TButton;
    Label38: TLabel;
    Edit16: TEdit;
    Label39: TLabel;
    Edit17: TEdit;
    Label40: TLabel;
    ComboBox15: TComboBox;
    Label41: TLabel;
    Edit18: TEdit;
    Button20: TButton;
    PCCalendarTabSheet: TTabSheet;
    GroupBox23: TGroupBox;
    ListView11: TListView;
    N4: TMenuItem;
    CopytoPC2: TMenuItem;
    PCCalendarPopupMenu: TPopupMenu;
    Copytocalendar1: TMenuItem;
    N5: TMenuItem;
    Delete5: TMenuItem;
    PCOfflineTabSheet: TTabSheet;
    GroupBox24: TGroupBox;
    Label42: TLabel;
    Label43: TLabel;
    Label44: TLabel;
    Label45: TLabel;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    N6: TMenuItem;
    Savebackuptofile1: TMenuItem;
    Restorebackupfromfile1: TMenuItem;
    N7: TMenuItem;
    Savephonebooktofileanotherapplication1: TMenuItem;
    PCPBKTabSheet: TTabSheet;
    GroupBox25: TGroupBox;
    ListView12: TListView;
    Edit19: TMenuItem;
    N10: TMenuItem;
    Delete4: TMenuItem;
    Button22: TButton;
    WAPBookPopupMenu: TPopupMenu;
    New20: TMenuItem;
    SMSSendSave2: TMenuItem;
    N8: TMenuItem;
    Edit20: TMenuItem;
    Button23: TButton;
    CopyphonebooktoPC1: TMenuItem;
    GroupBox26: TGroupBox;
    CheckBox3: TCheckBox;
    TrackBar1: TTrackBar;
    Label46: TLabel;
    CheckBox4: TCheckBox;
    CheckBox5: TCheckBox;
    procedure TreeView1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormCreate(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure NetmonitorTabSheetShow(Sender: TObject);
    procedure NetmonitorTabSheetHide(Sender: TObject);
    procedure SpinEdit1Change(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure PhonebookTabSheetHide(Sender: TObject);
    procedure Quit1Click(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure About1Click(Sender: TObject);
    procedure LogosImageMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure Button1Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure PhoneTabSheetShow(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure LogosTabSheetShow(Sender: TObject);
    procedure ComboBox2Select(Sender: TObject);
    procedure Button7Click(Sender: TObject);
    procedure LogosImageMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure Button6Click(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button8Click(Sender: TObject);
    procedure SpeedTabSheetShow(Sender: TObject);
    procedure Button9Click(Sender: TObject);
    procedure SpeedTabSheetHide(Sender: TObject);
    procedure Button11Click(Sender: TObject);
    procedure Button12Click(Sender: TObject);
    procedure CalendarTabSheetShow(Sender: TObject);
    procedure CallerTabSheetShow(Sender: TObject);
    procedure SMSCTabSheetShow(Sender: TObject);
    procedure Button10Click(Sender: TObject);
    procedure ProfileTabSheetShow(Sender: TObject);
    procedure SMSCTabSheetHide(Sender: TObject);
    procedure CalendarTabSheetHide(Sender: TObject);
    procedure CallerTabSheetHide(Sender: TObject);
    procedure ProfileTabSheetHide(Sender: TObject);
    procedure PhonebookTabSheetShow(Sender: TObject);
    procedure SMSTabSheetShow(Sender: TObject);
    procedure ComboBox3Change(Sender: TObject);
    procedure ListView2SelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure New1Click(Sender: TObject);
    procedure Edit3Click(Sender: TObject);
    procedure ListView4SelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure Delete1Click(Sender: TObject);
    procedure Button16Click(Sender: TObject);
    procedure Edit4Click(Sender: TObject);
    procedure New2Click(Sender: TObject);
    procedure Configuration1Click(Sender: TObject);
    procedure TreeView1KeyUp(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure SMSTabSheetHide(Sender: TObject);
    procedure SMSPopupMenuPopup(Sender: TObject);
    procedure CalendarPopupMenuPopup(Sender: TObject);
    procedure Edit1Change(Sender: TObject);
    procedure Delete2Click(Sender: TObject);
    procedure Edit5Click(Sender: TObject);
    procedure Image1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure SMSCPopupMenuPopup(Sender: TObject);
    procedure ListView6SelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure ListView1SelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure Edit7Click(Sender: TObject);
    procedure Button18Click(Sender: TObject);
    procedure Button17Click(Sender: TObject);
    procedure SMS1Click(Sender: TObject);
    procedure SMSSendSave1Click(Sender: TObject);
    procedure Button13Click(Sender: TObject);
    procedure Button14Click(Sender: TObject);
    procedure ConfigTabSheetHide(Sender: TObject);
    procedure ConfigTabSheetShow(Sender: TObject);
    procedure Timer2Timer(Sender: TObject);
    procedure AboutTabSheetShow(Sender: TObject);
    procedure PCSMSTabSheetShow(Sender: TObject);
    procedure CopytoPC1Click(Sender: TObject);
    procedure ListView10SelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure PCSMSPopupMenuPopup(Sender: TObject);
    procedure Delete3Click(Sender: TObject);
    procedure SendSave1Click(Sender: TObject);
    procedure WapBookTabSheetHide(Sender: TObject);
    procedure WapBookTabSheetShow(Sender: TObject);
    procedure ComboBox7Change(Sender: TObject);
    procedure Button19Click(Sender: TObject);
    procedure Button20Click(Sender: TObject);
    procedure PCOfflineTabSheetShow(Sender: TObject);
    procedure PCCalendarTabSheetShow(Sender: TObject);
    procedure CopytoPC2Click(Sender: TObject);
    procedure Delete5Click(Sender: TObject);
    procedure Copytocalendar1Click(Sender: TObject);
    procedure PCCalendarPopupMenuPopup(Sender: TObject);
    procedure Savebackuptofile1Click(Sender: TObject);
    procedure Restorebackupfromfile1Click(Sender: TObject);
    procedure PBKPopupMenuPopup(Sender: TObject);
    procedure Savephonebooktofileanotherapplication1Click(Sender: TObject);
    procedure ListView4DblClick(Sender: TObject);
    procedure Edit19Click(Sender: TObject);
    procedure Delete4Click(Sender: TObject);
    procedure Button22Click(Sender: TObject);
    procedure New20Click(Sender: TObject);
    procedure ListView9SelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure Edit20Click(Sender: TObject);
    procedure WAPBookPopupMenuPopup(Sender: TObject);
    procedure SMSSendSave2Click(Sender: TObject);
    procedure Button23Click(Sender: TObject);
    procedure CopyphonebooktoPC1Click(Sender: TObject);
    procedure PCPBKTabSheetShow(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  MainForm: TMainForm;
  setcalendarnote:GSM_CalendarNote;
  setsms:GSM_MultiSMSMessage;
  setsmslocation:integer;
  smscselectitem:integer;
  bitma:GSM_Bitmap;
  rington:GSM_Ringtone;
  calenda:GSM_CalendarNote;
  pbk:GSM_PhonebookEntry;
  previewcolor:TColor;
  bookmarknum2:integer;
  bookma:GSM_WAPBookmark;
  wapset:GSM_WAPSettings;
  pbkofflinenumber:integer;
  
  PhoneBackup: GSM_Backup;
  PhoneBackupAvailable:boolean;

procedure AddCalendarNote(note:GSM_CalendarNote;myview:TListView);
function DayOFweek3(date:TDateTime):string;
procedure SetSMSC(num:integer);
procedure ReadPBKOffline;

implementation

uses Calendar, SMSEdit, LogosRing, Netmon, smscedit, pbkedit, backup,
  backup2, pbkedit2, wapbook;

{$R *.dfm}

type
  TNetmonThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TPhonebookThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TSpeedThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TSMSCThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TSMSThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TCalendarThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TCallerThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TProfileThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TPlayRingtoneThread = class(TThread)
    public
      procedure Execute; override;
  end;
  TWAPBookmarkThread = class(TThread)
    public
      procedure Execute; override;
  end;

var
  //phone sheet
  phoneinfo:boolean;

  //reading netmonitor variables
  netmonnum: integer; //number of netmon screen
  NetmonThread: TNetmonThread;

  //reading phonebook memories variables
  phonebookstatus:GSM_MemoryStatus;
  phonebookcur:integer;
  phonebookfound:integer;
  PhonebookThread: TPhonebookThread;
  pbknumberentry:integer;
  
  //logos
  pointsize:integer;
  x2,y2:byte;
  logofilename:string='xxx';

  //ringtones
  PlayRingtoneThread: TPlayRingtoneThread;
  ringindex:integer;
  binring:GSM_BinRingtone;

  //speeddials
  speednum:integer;
  SpeedThread: TSpeedThread;

  //caller groups
  caller:array[1..5] of GSM_Bitmap;
  callerinit:boolean;
  callernum:integer;
  CallerThread: TCallerThread;

  //smsc
  smscnum:integer;
  SMSCThread: TSMSCThread;
  smscinit: boolean;
  SMSC2:array[1..10] of GSM_MessageCenter;

  //calendar
  calendarnum:integer;
  CalendarThread: TCalendarThread;
  calendarselect:integer;
  readcalendar:boolean;

  CalendarOffline:array[1..300] of GSM_CalendarNote;
  CalendarOfflineNumber:integer;

  //profiles
  profilenum:integer;
  ProfileThread: TProfileThread;

  SMS:array[1..200] of GSM_SMSMessage;
  SMSnum:integer;
  SMSfolders:GSM_SMSFolders;
  SMSstatus:GSM_SMSStatus;
  smsnumindex:integer;
  SMSThread: TSMSThread;
  smsnum2:integer;

  SMSOffline:array[1..300] of GSM_SMSMessage;
  SMSOfflineNumber:integer;

  //wap bookmarks
  WAPBookmarkThread: TWAPBookmarkThread;
  bookmarknum:integer;

function Inttostr2(I: Longint;digits:integer): string;

{ Convert any integer type to a string }
var
  S: string[11];
  j:integer;
begin
  Str(I:digits, S);
  for  j:=1 to 11 do
  begin
    if s[j]<>' ' then break;
    s[j]:='0';
  end;
  Result:= S;
end;

procedure SaveMainConfig;
var
  MyIniFile: TIniFile;
begin
  MyIniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'\gnokiirc');
  MyIniFile.WriteString('win32gui', 'x', inttostr(MainForm.Left));
  MyIniFile.WriteString('win32gui', 'y', inttostr(MainForm.Top));
  MyIniFile.WriteString('win32gui', 'width', inttostr(MainForm.Width));
  MyIniFile.WriteString('win32gui', 'height', inttostr(MainForm.Height));
  MyIniFile.WriteString('win32gui', 'treewidth', inttostr(MainForm.Treeview1.Width));
  MyIniFile.Free;
end;

function DayOFweek3(date:TDateTime):string;
var
  days: array[1..7] of string;
begin
  days[1] := 'Sun';
  days[2] := 'Mon';
  days[3] := 'Tue';
  days[4] := 'Wed';
  days[5] := 'Thu';
  days[6] := 'Fri';
  days[7] := 'Sat';
  DayOfWeek3:= days[DayOfWeek(date)];
end;

function DayOFweek2(day:integer;month:integer;year:integer):string;
var
  date:TDateTime;
begin
  Date:=EncodeDate(Year,Month,Day);
  DayOfWeek2:= DayOfWeek3(Date);
end;

procedure SetSMSC(num:integer);
begin
  SMSEditForm.Edit1.Text:=SMSC2[num].Number;
  case SMSC2[num].Validity of
    GSMV_1_Hour   : SMSEditForm.ComboBox3.ItemIndex:=0;
    GSMV_6_Hours  : SMSEditForm.ComboBox3.ItemIndex:=1;
    GSMV_24_Hours : SMSEditForm.ComboBox3.ItemIndex:=2;
    GSMV_72_Hours : SMSEditForm.ComboBox3.ItemIndex:=3;
    GSMV_1_Week   : SMSEditForm.ComboBox3.ItemIndex:=4;
  else
    SMSEditForm.ComboBox3.ItemIndex:=5;

  end;
  case SMSC2[num].Format of
    GSMF_Text   : SMSEditForm.ComboBox4.ItemIndex:=0;
    GSMF_Fax    : SMSEditForm.ComboBox4.ItemIndex:=1;
    GSMF_Email  : SMSEditForm.ComboBox4.ItemIndex:=2;
  else
    SMSEditForm.ComboBox4.ItemIndex:=3;
  end;
end;

procedure ReadPBKOffline;
var F:file of GSM_PhonebookEntry;
begin
  AssignFile(F, 'gnokiipbk');
  Reset(F);
  if (ioresult=0) then
  begin
    phonebackup.PhonePhonebookUsed:=0;
    pbkofflinenumber:=0;
    while not eof(f) do
    begin
      pbkofflinenumber:=pbkofflinenumber+1;
      read(f,PhoneBackup.PhonePhonebook[pbkofflinenumber]);
    end;
    CloseFile(F);
  end;
end;

procedure SavePBKOffline;
var F:file of GSM_PhonebookEntry;
    i:integer;
begin
  AssignFile(F, 'gnokiipbk');
  Rewrite(F);
  for i:=1 to PBKOfflineNumber do
  begin
    Write(f,PhoneBackup.PhonePhonebook[i]);
  end;
  CloseFile(F);
end;

procedure AddSMSEditSenderList;
var i,j:integer;
begin
  ReadPBKOffline;
  SMSEditForm.ComboBox9.Items.Clear;
  SMSEditForm.ComboBox9.Sorted:=false;
  for i:=1 to PBKOfflineNumber do
  begin
    if (PhoneBackup.PhonePhonebook[i].Number[1]<>chr(0)) then
    begin
      SMSEditForm.ComboBox9.Items.Add(PhoneBackup.PhonePhonebook[i].Name);
    end else
    begin
      for j:=1 to PhoneBackup.PhonePhonebook[i].SubEntriesCount do
      begin
        if PhoneBackup.PhonePhonebook[i].SubEntries[j].EntryType=GSM_Number then
        begin
          SMSEditForm.ComboBox9.Items.Add(PhoneBackup.PhonePhonebook[i].Name);
          break;
        end;
      end;
    end;
  end;
  SMSEditForm.ComboBox9.Sorted:=true;
end;

procedure ReadCallerGroup();
var
  logo:GSM_Bitmap;
  name:Pchar;
  i:integer;
begin
  if callernum<6 then
  begin
    i:=1;
    while i<>0 do
    begin
      logo.type2:=GSM_CallerLogo;
      logo.number:=callernum-1;
      MainForm.Timer2.Enabled:=false;
      i:=GSM_GetBitmap(@logo);
      MainForm.Timer2.Enabled:=true;
    end;
    caller[callernum]:=logo;
    MainForm.ListView5.Items.Add;
    MainForm.ListView5.Items.Item[callernum-1].Caption:=inttostr(callernum);
    if logo.text[1]=chr(0) then
    begin
      name:='';
      case callernum of
        1:name:='Family';
        2:name:='VIP';
        3:name:='Friends';
        4:name:='Colleagues';
        5:name:='Other';
      end;
      for i:=1 to 255 do
      begin
        caller[callernum].text[i]:=chr(0);
      end;
      for i:=1 to strlen(name) do
      begin
        caller[callernum].text[i]:=name[i-1];
      end;
    end;
    MainForm.ListView5.Items.Item[callernum-1].Subitems.Add(caller[callernum].text);
    if logo.ringtone=16 then
      MainForm.ListView5.Items.Item[callernum-1].Subitems.Add('default')
    else
      MainForm.ListView5.Items.Item[callernum-1].Subitems.Add(inttostr(logo.ringtone));
    if logo.enabled<>0 then
      MainForm.ListView5.Items.Item[callernum-1].Subitems.Add('logo enabled')
    else
      MainForm.ListView5.Items.Item[callernum-1].Subitems.Add('logo disabled');
  end else callerinit:=true;
  callernum:=callernum+1;
end;

procedure ReadAllCallerGroups();
begin
  if (GSM_GetModelFeature(FN_CALLERGROUPS)<>0) then
  begin
    if callerinit=false then
    begin
      MainForm.ListView5.Items.Clear;
      callernum:=1;
      while callerinit=false do
      begin
        ReadCallerGroup();
        if callerinit then break;
      end;
    end;
  end else
  begin
  end;
end;

//logos sheet
procedure UpdatePointBitmap(i,j:integer);
begin
  if GSM_IsPointBitmap(@bitma,i,j)<>0 then
  begin
    MainForm.LogosImage.Canvas.Brush.Color := clBlack;
    MainForm.Image1.Canvas.Pen.Color := clBlack;
  end else
  begin
    MainForm.LogosImage.Canvas.Brush.Color := PreviewColor;//$0063cf63
    MainForm.Image1.Canvas.Pen.Color := PreviewColor;
  end;
  MainForm.LogosImage.Canvas.FillRect(Rect(i*pointsize+i,j*pointsize+j,(i+1)*pointsize+i,(j+1)*pointsize+j));
  MainForm.Image1.Canvas.Polyline([Point(28+i, 145+j), Point(29+i,146+j)]);
end;

procedure DrawBitmap();
var
  xsize:integer;
  ysize:integer;
  i,j:integer;
begin
  MainForm.StatusBar1.Panels[0].Text:=inttostr(bitma.width);
  xsize:=bitma.width;
  ysize:=bitma.heigh;

  MainForm.LogosImage.Canvas.Brush.Style := bsSolid;
  MainForm.LogosImage.Canvas.Brush.Color := clScrollBar;

  MainForm.LogosImage.Canvas.Pen.Mode := pmCopy;

  MainForm.LogosImage.Canvas.Pen.Color := clScrollBar;
  MainForm.LogosImage.Canvas.Rectangle(0,0,MainForm.LogosImage.Width,MainForm.LogosImage.Height);

  MainForm.LogosImage.Canvas.Pen.Color := PreviewColor;
  MainForm.LogosImage.Canvas.Rectangle(0,0,xsize*pointsize+xsize-1,ysize*pointsize+ysize-1);

  MainForm.LogosImage.Canvas.Brush.Color := PreviewColor;
  MainForm.LogosImage.Canvas.FloodFill(2,2,PreviewColor,fsBorder);

  MainForm.LogosImage.Canvas.Pen.Color := clBlack;

  for i:=0 to xsize do
  begin
    MainForm.LogosImage.Canvas.MoveTo(i*pointsize+i-1,0);
    MainForm.LogosImage.Canvas.LineTo(i*pointsize+i-1,ysize*pointsize+ysize-1);
  end;
  for i:=0 to ysize do
  begin
    MainForm.LogosImage.Canvas.MoveTo(0,i*pointsize+i-1);
    MainForm.LogosImage.Canvas.LineTo(xsize*pointsize+xsize-1,i*pointsize+i-1);
  end;

  MainForm.Image1.Picture.Assign(MainForm.Image2.Picture);
  MainForm.Image1.Canvas.Pen.Mode := pmCopy;
  for i:=0 to xsize-1 do
  begin
   for j:=0 to ysize-1 do
    begin
      UpdatePointBitmap(i,j);
    end;
  end;
end;

procedure TMainForm.Image1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  sr: TSearchRec;
  filefound:integer;
begin
  filefound:=0;
  if FindFirst(ExtractFilePath(Application.ExeName)+'\*bmp', faAnyFile, sr) = 0 then
  begin
    repeat
      if (sr.Attr and faAnyFile) = sr.Attr then
      begin
        if sr.Size=57814 then
        begin
          case filefound of
            1:begin
                Image2.Picture.LoadFromFile(ExtractFilePath(Application.ExeName)+'\'+sr.name);
                PreviewColor:=Image2.Canvas.Pixels[70,170];
                DrawBitmap;
                filefound:=2;
                logofilename:=sr.name;
              end;
            0:if logofilename=sr.Name then filefound:=1;
          end;
          if filefound=2 then break;
        end;
      end;
    until FindNext(sr) <> 0;
    FindClose(sr);
  end;
  if filefound<>2 then
  begin
    if FindFirst(ExtractFilePath(Application.ExeName)+'\*bmp', faAnyFile, sr) = 0 then
    begin
      Image2.Picture.LoadFromFile(ExtractFilePath(Application.ExeName)+'\'+sr.name);
      DrawBitmap;
      logofilename:=sr.name;
    end;
  end;
end;

procedure LogosMousePos(x,y:integer);
var
  i:integer;
begin
  i:=0;x2:=0;
  while i<=x do
  begin
    i:=i+pointsize+1;
    x2:=x2+1;
  end;
  i:=0;y2:=0;
  while i<=y do
  begin
    i:=i+pointsize+1;
    y2:=y2+1;
  end;
end;

procedure TMainForm.ComboBox2Select(Sender: TObject);
var
  bittype:integer;
begin
  case ComboBox2.ItemIndex of
    0:bittype:=GSM_CallerLogo;
    1:begin
        bittype:=GSM_OperatorLogo;
        if GSMPhoneConnected then
        begin
          case GSM_GetModelFeature(FN_STARTUP) of
            F_STA62:bittype:=GSM_7110OperatorLogo;
            F_STA71:bittype:=GSM_7110OperatorLogo;
          end;
        end;
      end;
    2:bittype:=GSM_PictureImage;
    3:begin
        bittype:=GSM_StartupLogo;
        if GSMPhoneConnected then
        begin
          case GSM_GetModelFeature(FN_STARTUP) of
            F_STA62:bittype:=GSM_6210StartupLogo;
            F_STA71:bittype:=GSM_7110StartupLogo;
          end;
        end;
      end;
  else
    bittype:=GSM_StartupLogo;
    if GSMPhoneConnected then
    begin
      case GSM_GetModelFeature(FN_STARTUP) of
        F_STA62:bittype:=GSM_6210StartupLogo;
        F_STA71:bittype:=GSM_7110StartupLogo;
      end;
    end;
  end;
  GSM_ResizeBitmap(@bitma,bittype);
  DrawBitmap();
  if (bitma.type2<>GSM_OperatorLogo) and
     (bitma.type2<>GSM_7110OperatorLogo) then
    Button8.Enabled:=false
  else
    Button8.Enabled:=true;
end;

procedure TMainForm.LogosImageMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
  LogosMousePos(x,y);
  if x2<=bitma.width then
  begin
    if y2<=bitma.heigh then
      MainForm.StatusBar1.Panels[0].Text:=inttostr(x2)+','+inttostr(y2)
    else
      MainForm.StatusBar1.Panels[0].Text:='';
  end else
    MainForm.StatusBar1.Panels[0].Text:='';
end;

procedure TMainForm.LogosTabSheetShow(Sender: TObject);
begin
  DrawBitmap();
end;

procedure TMainForm.Button7Click(Sender: TObject);
var
  i:integer;
begin
  LogosRingForm.Label1.Caption:='What logo do you want to get ?';
  LogosRingForm.ComboBox3.Items.Clear;
  case bitma.type2 of
    GSM_CallerLogo:
      begin
        LogosRingForm.ComboBox3.Visible:=true;
        ReadAllCallerGroups();
        for i:=1 to 5 do
        begin
          LogosRingForm.ComboBox3.Items.Add(caller[i].text);
        end;
        LogosRingForm.ComboBox3.ItemIndex:=0;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
        begin
          bitma.number:=LogosRingForm.ComboBox3.ItemIndex;
          MainForm.Timer2.Enabled:=false;
          GSM_GetBitmap(@bitma);
          MainForm.Timer2.Enabled:=true;
          DrawBitmap();
        end;
      end;
    GSM_PictureImage:
      begin
        LogosRingForm.ComboBox3.Visible:=true;
        for i:=1 to 20 do
        begin
          LogosRingForm.ComboBox3.Items.Add('Location '+inttostr(i));
        end;
        LogosRingForm.ComboBox3.ItemIndex:=0;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
        begin
          bitma.number:=LogosRingForm.ComboBox3.ItemIndex;
          MainForm.Timer2.Enabled:=false;
          GSM_GetBitmap(@bitma);
          MainForm.Timer2.Enabled:=true;
          DrawBitmap();
        end;
      end;
    else
    begin
      MainForm.Timer2.Enabled:=false;
      GSM_GetBitmap(@bitma);
      MainForm.Timer2.Enabled:=true;
      DrawBitmap();
    end;
  end;
end;

procedure TMainForm.Button4Click(Sender: TObject);
var
  bit:GSM_Bitmap;
begin
  if OpenDialog1.Execute then
  begin
    if GSM_ReadBitmapFile(PChar(OpenDialog1.FileName),@bit)=0 then
    begin
      bitma:=bit;
      DrawBitmap();
      case bitma.type2 of
        GSM_CallerLogo  :ComboBox2.ItemIndex:=0;
        GSM_OperatorLogo:ComboBox2.ItemIndex:=1;
        GSM_7110OperatorLogo:ComboBox2.ItemIndex:=1;
        GSM_PictureImage:ComboBox2.ItemIndex:=2;
        GSM_StartupLogo :ComboBox2.ItemIndex:=3;
        GSM_7110StartupLogo :ComboBox2.ItemIndex:=3;
        GSM_6210StartupLogo :ComboBox2.ItemIndex:=3;
      end;
    end;
  end;
end;

procedure TMainForm.LogosImageMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  LogosMousePos(x,y);
  if x2<=bitma.width then
  begin
    if y2<=bitma.heigh then
    begin
      if ssLeft in Shift then
        GSM_SetPointBitmap(@bitma,x2-1,y2-1)
      else
        GSM_ClearPointBitmap(@bitma,x2-1,y2-1);

      UpdatePointBitmap(x2-1,y2-1);
    end;
  end;
end;

procedure TMainForm.Button6Click(Sender: TObject);
var
  i,j:integer;
  Name,Data:array[1..255] of char;
  oldbitma:GSM_Bitmap;
begin
  LogosRingForm.Label1.Caption:='Do you want to set logo ?';
  LogosRingForm.ComboBox3.Items.Clear;
  case bitma.type2 of
    GSM_StartupLogo:
      begin
        LogosRingForm.ComboBox3.Visible:=false;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
          GSM_SetBitmap(@bitma);
      end;
    GSM_7110OperatorLogo:
      begin
        LogosRingForm.ComboBox3.Visible:=true;
        LogosRingForm.ComboBox3.Items.Add('Preview');
        //preparing network names
        i:=0;
        while true do
        begin
          GSM_GetNetworkData(i,@Name,@Data);

          // conversion problems
          Label14.Caption:=Name;
          if strcomp(PChar(Label14.Caption),PChar('unknown'))=0 then break;

          // conversion problems
          Label14.Caption:=' - '+Label14.Caption;
          for j:=6 downto 1 do
          begin
            Label14.Caption:=Data[j]+Label14.Caption;
          end;

          LogosRingForm.ComboBox3.Items.Add(Pchar(Label14.Caption));
          i:=i+1;
        end;
        //looking for current network
        if GSMPhoneConnected then
        begin
          // conversion problems
          Label14.Caption:=MainForm.StatusBar1.Panels[1].Text;
          Label14.Caption:=' - '+Label14.Caption;
          for j:=6 downto 1 do
          begin
            Label14.Caption:=GSMNetworkInfo.NetworkCode[j]+Label14.Caption;
          end;

          i:=1;
          while true do
          begin
            if strcomp(pchar(LogosRingForm.ComboBox3.Items[i]),pchar(Label14.Caption))=0
              then break;
            i:=i+1;
          end;
          LogosRingForm.ComboBox3.ItemIndex:=i;
        end;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
        begin
          if LogosRingForm.ComboBox3.ItemIndex=0 then
          begin
            bitma.number:=255;
            for i:=1 to 6 do
            begin
              bitma.netcode[i]:=GSMNetworkInfo.NetworkCode[i];
            end;
            bitma.netcode[7]:=chr(0);
          end else
          begin
            GSM_GetNetworkData(LogosRingForm.ComboBox3.ItemIndex-1,@Name,@bitma.netcode);
          end;
          GSM_SetBitmap(@bitma);
        end;
      end;
    GSM_OperatorLogo:
      begin
        LogosRingForm.ComboBox3.Visible:=true;
        LogosRingForm.ComboBox3.Items.Add('Preview');
        //preparing network names
        i:=0;
        while true do
        begin
          GSM_GetNetworkData(i,@Name,@Data);

          // conversion problems
          Label14.Caption:=Name;
          if strcomp(PChar(Label14.Caption),PChar('unknown'))=0 then break;

          // conversion problems
          Label14.Caption:=' - '+Label14.Caption;
          for j:=6 downto 1 do
          begin
            Label14.Caption:=Data[j]+Label14.Caption;
          end;

          LogosRingForm.ComboBox3.Items.Add(Pchar(Label14.Caption));
          i:=i+1;
        end;
        //looking for current network
        if GSMPhoneConnected then
        begin
          // conversion problems
          Label14.Caption:=MainForm.StatusBar1.Panels[1].Text;
          Label14.Caption:=' - '+Label14.Caption;
          for j:=6 downto 1 do
          begin
            Label14.Caption:=GSMNetworkInfo.NetworkCode[j]+Label14.Caption;
          end;

          i:=1;
          while true do
          begin
            if strcomp(pchar(LogosRingForm.ComboBox3.Items[i]),pchar(Label14.Caption))=0
              then break;
            i:=i+1;
          end;
          LogosRingForm.ComboBox3.ItemIndex:=i;
        end;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
        begin
          if LogosRingForm.ComboBox3.ItemIndex=0 then
          begin
            bitma.number:=255;
            for i:=1 to 6 do
            begin
              bitma.netcode[i]:=GSMNetworkInfo.NetworkCode[i];
            end;
            bitma.netcode[7]:=chr(0);
          end else
          begin
            GSM_GetNetworkData(LogosRingForm.ComboBox3.ItemIndex-1,@Name,@bitma.netcode);
          end;
          GSM_SetBitmap(@bitma);
        end;
      end;
    GSM_PictureImage:
      begin
        LogosRingForm.ComboBox3.Visible:=true;
        for i:=1 to 20 do
        begin
          LogosRingForm.ComboBox3.Items.Add('Location '+inttostr(i));
        end;
        LogosRingForm.ComboBox3.ItemIndex:=0;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
        begin
          if LogosRingForm.ComboBox3.ItemIndex=0 then
          begin
            bitma.number:=255;
          end else
          begin
            bitma.number:=LogosRingForm.ComboBox3.ItemIndex;
          end;
          GSM_SetBitmap(@bitma);
        end;
      end;
    GSM_CallerLogo:
      begin
        LogosRingForm.ComboBox3.Visible:=true;
        LogosRingForm.ComboBox3.Items.Add('Preview');
        ReadAllCallerGroups();
        for i:=1 to 5 do
        begin
          LogosRingForm.ComboBox3.Items.Add(caller[i].text);
        end;
        LogosRingForm.ComboBox3.ItemIndex:=1;
        LogosRingForm.ShowModal();
        if (LogosRingForm.ComboBox3.Visible) then
        begin
          if LogosRingForm.ComboBox3.ItemIndex=0 then
          begin
            bitma.number:=255;
          end else
          begin
            bitma.number:=LogosRingForm.ComboBox3.ItemIndex-1;
            //we need to get old caller group datas
            oldbitma.type2:=bitma.type2;
            oldbitma.number:=bitma.number;
            GSM_GetBitmap(@oldbitma);
            bitma.ringtone:=oldbitma.ringtone;
            bitma.text:=oldbitma.text;
          end;
          GSM_SetBitmap(@bitma);
        end;
      end;
  end;
end;

procedure TMainForm.Button5Click(Sender: TObject);
begin
  if SaveDialog1.Execute then
  begin
    GSM_SaveBitmapFile(PChar(SaveDialog1.FileName),@bitma);
  end;
end;

procedure TMainForm.Button8Click(Sender: TObject);
var
  bitma2:GSM_Bitmap;
begin
  if MessageDlg('Restore logo to default ?',
    mtConfirmation, [mbYes, mbNo], 0) = mrYes then
  begin
    bitma2:=bitma;
    bitma2.netcode[1]:='0';
    bitma2.netcode[2]:='0';
    bitma2.netcode[3]:='0';
    bitma2.netcode[4]:=' ';
    bitma2.netcode[5]:='0';
    bitma2.netcode[6]:='0';
    bitma2.netcode[7]:=chr(0);
    GSM_ClearBitmap(@bitma2);
    GSM_SetBitmap(@bitma2);
  end;
end;

procedure TMainForm.Button12Click(Sender: TObject);
begin
  GSM_ClearBitmap(@bitma);
  DrawBitmap();
end;

//MainForm main functions
procedure TMainForm.TreeView1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  MainForm.TreeView1.FullExpand;
  case MainForm.TreeView1.Selected.ImageIndex of
    0: MainForm.PageControl1.ActivePage:=PhoneTabSheet;
    1: MainForm.PageControl1.ActivePage:=CalendarTabSheet;
    2: MainForm.PageControl1.ActivePage:=DivertTabSheet;
    3: MainForm.PageControl1.ActivePage:=CallerTabSheet;
    4: MainForm.PageControl1.ActivePage:=LogosTabSheet;
    5: MainForm.PageControl1.ActivePage:=NetmonitorTabSheet;
    6: MainForm.PageControl1.ActivePage:=PhonebookTabSheet;
    7: MainForm.PageControl1.ActivePage:=ProfileTabSheet;
    8: MainForm.PageControl1.ActivePage:=RingtonesTabSheet;
    9: MainForm.PageControl1.ActivePage:=SMSTabSheet;
    10: MainForm.PageControl1.ActivePage:=SMSCTabSheet;
    11: MainForm.PageControl1.ActivePage:=SpeedTabSheet;
    12: MainForm.PageControl1.ActivePage:=WAPBookTabSheet;
    13: MainForm.PageControl1.ActivePage:=WAPSettTabSheet;
    14: MainForm.PageControl1.ActivePage:=PCOfflineTabSheet;
    15: MainForm.PageControl1.ActivePage:=PCCalendarTabSheet;
    16: MainForm.PageControl1.ActivePage:=PCPBKTabSheet;
    17: MainForm.PageControl1.ActivePage:=PCSMSTabSheet;
  end;
end;

procedure TMainForm.Configuration1Click(Sender: TObject);
begin
  MainForm.PageControl1.ActivePage:=ConfigTabSheet;
end;

procedure TMainForm.TreeView1KeyUp(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  MainForm.TreeView1.FullExpand;
end;

procedure TMainForm.FormCreate(Sender: TObject);
begin
  MainForm.PhonebookTabSheet.TabVisible:=false;
  MainForm.NetmonitorTabSheet.TabVisible:=false;
  MainForm.LogosTabSheet.TabVisible:=false;
  MainForm.PhoneTabSheet.TabVisible:=false;
  MainForm.RingtonesTabSheet.TabVisible:=false;
  MainForm.SMSTabSheet.TabVisible:=false;
  MainForm.SMSCTabSheet.TabVisible:=false;
  MainForm.SpeedTabSheet.TabVisible:=false;
  MainForm.CalendarTabSheet.TabVisible:=false;
  MainForm.CallerTabSheet.TabVisible:=false;
  MainForm.ProfileTabSheet.TabVisible:=false;
  MainForm.WAPBookTabSheet.TabVisible:=false;
  MainForm.WAPSettTabSheet.TabVisible:=false;
  MainForm.DivertTabSheet.TabVisible:=false;
  MainForm.ConfigTabSheet.TabVisible:=false;
  MainForm.AboutTabSheet.TabVisible:=false;
  MainForm.PCSMSTabSheet.TabVisible:=false;
  MainForm.PCCalendarTabSheet.TabVisible:=false;
  MainForm.PCOfflineTabSheet.TabVisible:=false;
  MainForm.PCPBKTabSheet.TabVisible:=false;
  MainForm.PageControl1.ActivePage:=PhoneTabSheet;

  PreviewColor:=Image2.Canvas.Pixels[70,170];

  bitma.size:=126;
  bitma.width:=72;
  bitma.heigh:=14;
  bitma.type2:=GSM_OperatorLogo;
  GSM_ClearBitmap(@bitma);

  pointsize:=3;
  MainForm.LogosImage.Width:=96*pointsize+96-1;
  MainForm.LogosImage.Height:=65*pointsize+65-1;

  netmonnum:=1;

  MainForm.Width:=660;
  MainForm.Height:=470;

  callerinit:=false;
  phoneinfo:=false;
  smscinit:=false;

  smsofflinenumber:=0;
  calendarofflinenumber:=0;
  pbkofflinenumber:=0;
  
  readcalendar:=false;
end;

procedure TMainForm.Timer1Timer(Sender: TObject);
var
  NetworkName:array[1..255] of char;
begin
  if GSMPhoneConnected then
  begin
    // Here we should enable interface
    GSM_GetNetworkName(@NetworkName,@GSMNetworkInfo.NetworkCode);
    MainForm.StatusBar1.Panels[1].Text:=NetworkName;
    if PhonebookThread=nil then
      ComboBox1.Enabled:=true;

    SaveBackupTofile1.Enabled:=true;
    Restorebackupfromfile1.Enabled:=true;

    ListView1.Enabled:=true;
    NetmonGroupBox.Enabled:=true;
    SpinEdit1.Enabled:=true;
    Button2.Enabled:=true;
    Button3.Enabled:=true;
    Button6.Enabled:=true;
    Button7.Enabled:=true;
    Button9.Enabled:=true;
    Button11.Enabled:=true;

    //ringtones
    Button16.Enabled:=true;
    if Edit1.Text<>'' then Edit1.Enabled:=true;
    if Edit2.Text<>'' then Edit2.Enabled:=true;
    Button22.Enabled:=true;

    If MainForm.PageControl1.ActivePage=MainForm.NetmonitorTabSheet then
      NetmonitorTabSheetShow(Sender);

    If MainForm.PageControl1.ActivePage=MainForm.PhoneTabSheet then
    begin
      if not phoneinfo then
        PhoneTabSheetShow(Sender);
    end;

    If MainForm.PageControl1.ActivePage=MainForm.SpeedTabSheet then
    begin
      if speednum<>10 then
        SpeedTabSheetShow(Sender);
    end;

    if (bitma.type2<>GSM_OperatorLogo) and
       (bitma.type2<>GSM_7110OperatorLogo) then
      Button8.Enabled:=false
    else
      Button8.Enabled:=true;

    if Edit2.Text<>'' then
      Button14.Enabled:=true
    else
      Button14.Enabled:=false;

    if Edit1.Text<>'' then
    begin
      Button2.Enabled:=true;
      Button17.Enabled:=true;
      if PlayRingtoneThread<>nil then
      begin
        Button3.Enabled:=false;
        Button16.Enabled:=true;
      end
      else
      begin
        Button3.Enabled:=true;
        Button16.Enabled:=false;
      end;
    end else
    begin
      Button2.Enabled:=false;
      Button17.Enabled:=false;
      Button16.Enabled:=false;
      Button3.Enabled:=false;
    end;

    Button18.Enabled:=false;
    if bitma.type2=GSM_PictureImage then
      Button18.Enabled:=true;
    if (bitma.type2=GSM_OperatorLogo) or
       (bitma.type2=GSM_7110OperatorLogo) then
      Button18.Enabled:=true;
    if bitma.type2=GSM_CallerLogo then
      Button18.Enabled:=true;

    //wap settings
    WAPSettTabSheet.Enabled:=true;
    if GSM_GetModelFeature(FN_WAP)=0 then
    begin
      Button23.Enabled:=false;
      GroupBox19.Enabled:=false;
      groupbox19.Ctl3D:=false;
      ComboBox7.enabled:=false;
      ComboBox8.enabled:=false;
      ComboBox9.enabled:=false;
      ComboBox10.enabled:=false;
      ComboBox11.enabled:=false;
      ComboBox12.enabled:=false;
      ComboBox13.enabled:=false;
      ComboBox14.enabled:=false;
      Edit6.Enabled:=false;
      Edit8.Enabled:=false;
      Edit9.Enabled:=false;
      Edit10.Enabled:=false;
      Edit11.Enabled:=false;
      Edit12.Enabled:=false;
      Edit13.Enabled:=false;
      Edit14.Enabled:=false;
      Edit15.Enabled:=false;
    end else
    begin
      GroupBox19.Enabled:=true;
      groupbox19.Ctl3D:=true;
      ComboBox7.enabled:=true;
      ComboBox8.enabled:=true;
      ComboBox9.enabled:=true;
      ComboBox10.enabled:=true;
      ComboBox11.enabled:=true;
      ComboBox12.enabled:=true;
      ComboBox13.enabled:=true;
      ComboBox14.enabled:=true;
      Edit6.Enabled:=true;
      Edit8.Enabled:=true;
      Edit9.Enabled:=true;
      Edit10.Enabled:=true;
      Edit11.Enabled:=true;
      Edit12.Enabled:=true;
      Edit13.Enabled:=true;
      Edit14.Enabled:=true;
      Edit15.Enabled:=true;
      Button23.Enabled:=false;
      if ComboBox7.ItemIndex<>0 then Button23.Enabled:=true;
    end;

    button19.Enabled:=true;
    button20.Enabled:=true;
  end else
  begin
    // Here we should disabled interface
    MainForm.StatusBar1.Panels[1].Text:='Not connected';

    SaveBackupTofile1.Enabled:=false;
    Restorebackupfromfile1.Enabled:=false;

    callerinit:=false;

    MainForm.ComboBox1.Enabled:=false;
    MainForm.ListView1.Enabled:=false;
    MainForm.NetmonGroupBox.Enabled:=false;
    MainForm.SpinEdit1.Enabled:=false;
    MainForm.Button2.Enabled:=false;
    MainForm.Button3.Enabled:=false;
    MainForm.Button6.Enabled:=false;
    MainForm.Button7.Enabled:=false;
    MainForm.Button8.Enabled:=false;
    MainForm.Button9.Enabled:=false;
    MainForm.Button11.Enabled:=false;
    Button18.Enabled:=false;

    //ringtones
    Button16.Enabled:=false;
    Button14.Enabled:=false;
    Button17.Enabled:=false;
    Edit1.Enabled:=false;
    Edit2.Enabled:=false;
    Button22.Enabled:=false;

    MainForm.NetmonitorTabSheetHide(Sender);
    MainForm.PhonebookTabSheetHide(Sender);
    MainForm.SpeedTabSheetHide(Sender);

    //WAP settings
    ComboBox7.enabled:=false;
    ComboBox8.enabled:=false;
    ComboBox9.enabled:=false;
    ComboBox10.enabled:=false;
    ComboBox11.enabled:=false;
    ComboBox12.enabled:=false;
    ComboBox13.enabled:=false;
    ComboBox14.enabled:=false;
    Edit6.Enabled:=false;
    Edit8.Enabled:=false;
    Edit9.Enabled:=false;
    Edit10.Enabled:=false;
    Edit11.Enabled:=false;
    Edit12.Enabled:=false;
    Edit13.Enabled:=false;
    Edit14.Enabled:=false;
    Edit15.Enabled:=false;
    Button23.Enabled:=false;

    button19.Enabled:=false;
    button20.Enabled:=false;

    phoneinfo:=false;
  end;
end;

procedure TMainForm.Quit1Click(Sender: TObject);
begin
  if GSMPhoneConnected then GSM_Terminate();
  SaveMainConfig;
  Halt;
end;

procedure TMainForm.FormResize(Sender: TObject);
begin
  MainForm.StatusBar1.Panels[0].Width:=MainForm.Width-150;
end;

procedure TMainForm.About1Click(Sender: TObject);
begin
  AboutTabSheet.Show;
end;

//netmonitor sheet functions

procedure TNetmonThread.Execute;
var
  netmon:array[1..255] of char;
begin
  while not Terminated do
  begin
    GSM_Netmonitor(netmonnum,@netmon);
    MainForm.NetmonLabel.Caption:=netmon;
    MainForm.NetmonGroupBox.Caption:=' Screen ' + IntToStr(netmonnum) + ' ';
  end;
end;

procedure TMainForm.NetmonitorTabSheetShow(Sender: TObject);
begin
  if NetmonThread<>nil then
  else begin
    if GSMPhoneConnected then
    begin
      MainForm.Timer2.Enabled:=false;
      NetmonThread:=TNetmonThread.Create(false);
    end;
  end;
end;

procedure TMainForm.NetmonitorTabSheetHide(Sender: TObject);
begin
  if (NetmonThread<>nil) then
  begin
    NetmonThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    NetmonThread:=nil;
  end;
end;

procedure TMainForm.SpinEdit1Change(Sender: TObject);
begin
  netmonnum:=SpinEdit1.Value;
end;

procedure TMainForm.Button11Click(Sender: TObject);
var
  netmon:array[1..255] of char;
begin
  NetmonForm.ShowModal;
  if NetmonForm.RadioGroup1.ItemIndex<>-1 then
  begin
    GSM_Netmonitor(241+NetmonForm.RadioGroup1.ItemIndex,@netmon);
  end;
end;

//phonebook sheet functions

procedure EndPhonebookThread;
begin
  if (PhonebookThread<>nil) then
  begin
    PhonebookThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    PhonebookThread:=nil;
    MainForm.ComboBox1.Enabled:=true;
  end;
end;

procedure AddPBKEntry(MyListView:TListView;x:GSM_PhonebookEntry;number:integer);
var j:integer;
begin
  MyListView.Items.Item[number].Subitems.Add(x.Name);

  if (x.Number[1]<>chr(0)) then
    MyListView.Items.Item[number].Subitems.Add(x.Number)
  else begin
    for j:=1 to x.SubEntriesCount do
    begin
      if x.SubEntries[j].EntryType=GSM_Number then
      begin
        MyListView.Items.Item[number].Subitems.Add(x.subentries[j].Number);
        break;
      end;
    end;
  end;

  case x.Group of
    0:MyListView.Items.Item[number].Subitems.Add(caller[x.Group+1].text);
    1:MyListView.Items.Item[number].Subitems.Add(caller[x.Group+1].text);
    2:MyListView.Items.Item[number].Subitems.Add(caller[x.Group+1].text);
    3:MyListView.Items.Item[number].Subitems.Add(caller[x.Group+1].text);
    4:MyListView.Items.Item[number].Subitems.Add(caller[x.Group+1].text);
  else
    MyListView.Items.Item[number].Subitems.Add('not set');
  end;

  for j:=1 to x.SubEntriesCount do
  begin
    if x.SubEntries[j].EntryType=GSM_Date then
    begin
      MyListView.Items.Item[number].Subitems.Add(
        DayOfweek2(x.SubEntries[j].Date.Day,x.SubEntries[j].Date.Month,x.SubEntries[j].Date.Year)+' '+
        inttostr2(x.SubEntries[j].Date.Day,2)+'-'+inttostr2(x.SubEntries[j].Date.Month,2)+'-'+
        inttostr(x.SubEntries[j].Date.Year));
      MyListView.Items.Item[number].Subitems.Add(
        inttostr2(x.SubEntries[j].Date.Hour,2)+':'+inttostr2(x.SubEntries[j].Date.Minute,2)+':'+
        inttostr2(x.SubEntries[j].Date.Second,2));
      break;
    end;
  end;
end;

procedure TPhonebookThread.Execute;
var
  x:GSM_PhonebookEntry;
begin
  while not Terminated do
  begin
    if (phonebookcur<phonebookstatus.Used+phonebookstatus.Free) then
    begin
      phonebookcur:=phonebookcur+1;
      MainForm.StatusBar1.Panels[0].Text:='Reading '+inttostr(phonebookcur)+'/'+inttostr(phonebookstatus.Free+phonebookstatus.Used);
      MainForm.ListView1.Items.Add;
      MainForm.ListView1.Items.Item[phonebookcur-1].Caption:=inttostr(phonebookcur);
      if (phonebookfound<phonebookstatus.Used) then
      begin
        x.MemoryType:=phonebookstatus.MemoryType;
        x.Location:=phonebookcur;
        if (x.MemoryType=GMT_ON) then x.Location:=x.Location-1;
        if (x.MemoryType=GMT_DC) then x.Location:=x.Location-1;
        if (x.MemoryType=GMT_MC) then x.Location:=x.Location-1;
        if (x.MemoryType=GMT_RC) then x.Location:=x.Location-1;
        if GSM_GetMemoryLocation(@x)=0 then
        begin

          if (x.MemoryType=GMT_ME) then
          begin
            PhoneBackup.PhonePhonebookUsed:=PhoneBackup.PhonePhonebookUsed+1;
          end;

          PBKOfflineNumber:=PBKOfflineNumber+1;
          PhoneBackup.PhonePhonebook[PBKOfflineNumber]:=x;

          if (x.MemoryType=GMT_SM) then
          begin
            PhoneBackup.SIMPhonebookUsed:=PhoneBackup.SIMPhonebookUsed+1;
            PhoneBackup.SIMPhonebook[PhoneBackup.SIMPhonebookUsed]:=x;
          end;
          AddPBKEntry(MainForm.ListView1,x,phonebookcur-1);
          if (x.Name[1]<>chr(0)) then phonebookfound:=phonebookfound+1;
        end;
      end;
    end else
    begin
      MainForm.StatusBar1.Panels[0].Text:=inttostr(phonebookstatus.Used)+'/'+inttostr(phonebookstatus.Free+phonebookstatus.Used)+' entries';
      EndPhonebookThread;
    end;
  end;
end;

procedure TMainForm.PhonebookTabSheetShow(Sender: TObject);
begin
  if GSMPhoneConnected then ComboBox1.Enabled:=true;
end;

procedure TMainForm.Button10Click(Sender: TObject);
var
  i:integer;
  FileHandle:integer;
begin
  if SaveDialog1.Execute then
  begin
    FileHandle := FileCreate(SaveDialog1.FileName);
    for i:=1 to ListView1.Items.Count do
    begin
      FileWrite(FileHandle,ListView1.Items.Item[i].Caption, SizeOf(ListView1.Items.Item[i].Caption));
    end;
    FileClose(FileHandle);
  end;
end;

procedure TMainForm.ComboBox1Change(Sender: TObject);
begin
  if (ComboBox1.ItemIndex<>0) then
  begin
    case ComboBox1.ItemIndex of
      1:phonebookstatus.MemoryType:=GMT_DC;
      2:phonebookstatus.MemoryType:=GMT_MC;
      3:phonebookstatus.MemoryType:=GMT_ON;
      4:phonebookstatus.MemoryType:=GMT_ME;
      5:phonebookstatus.MemoryType:=GMT_SM;
      6:phonebookstatus.MemoryType:=GMT_RC;
    end;
    if GSM_GetMemoryStatus(@phonebookstatus)<>0 then
    begin
      phonebookstatus.Free:=0;
      phonebookstatus.Used:=30;
    end;
    phonebookcur:=0;
    phonebookfound:=0;

    PBKOfflineNumber:=0;
    PhoneBackup.PhonePhonebookUsed:=0;
    PhoneBackup.SIMPhonebookUsed:=0;
    if (phonebookstatus.MemoryType=GMT_ME) then
    begin
      PhoneBackup.PhonePhonebookSize:=phonebookstatus.Free+phonebookstatus.Used;
    end;
    if (phonebookstatus.MemoryType=GMT_SM) then
    begin
      PhoneBackup.SIMPhonebookSize:=phonebookstatus.Free+phonebookstatus.Used;
    end;

    ComboBox1.Enabled:=false;

    ReadAllCallerGroups();

    MainForm.ListView1.Items.Clear;

    MainForm.Timer2.Enabled:=false;
    PhonebookThread:=TPhonebookThread.Create(false);
  end;
end;

procedure TMainForm.PhonebookTabSheetHide(Sender: TObject);
begin
  EndPhonebookThread;
end;

//ringtones
procedure TMainForm.Button1Click(Sender: TObject);
var
  ring:GSM_Ringtone;
begin
  if OpenDialog1.Execute then
  begin
    if GSM_ReadRingtoneFile(PChar(OpenDialog1.FileName),@ring)=0 then
    begin
      Edit1.Text:=ring.Name;
      rington:=ring;
    end;
  end;
end;

procedure EndPlayRingtoneThread;
begin
  if (PlayRingtoneThread<>nil) then
  begin
    PlayRingtoneThread.Terminate();
    GSM_PlayTone(255*255,0);
    MainForm.Timer2.Enabled:=true;
    PlayRingtoneThread:=nil;
  end;
end;

procedure TPlayRingtoneThread.Execute;
begin
  while not Terminated do
  begin
    GSM_PlayNote(rington.notes[ringindex]);
    ringindex:=ringindex+1;
    if ringindex>rington.NrNotes then
      EndPlayRingtoneThread;
  end;
end;

procedure TMainForm.Button16Click(Sender: TObject);
begin
  EndPlayRingtoneThread;
end;

procedure TMainForm.Button3Click(Sender: TObject);
begin
  ringindex:=1;
  MainForm.Timer2.Enabled:=false;
  PlayRingtoneThread:=TPlayRingtoneThread.Create(false);
end;

procedure TMainForm.Button2Click(Sender: TObject);
var
  i:integer;
  mybinring:GSM_BinRingtone;
  myring:GSM_Ringtone;
begin
  LogosRingForm.Label1.Caption:='Do you want to set ringtone ?';
  LogosRingForm.ComboBox3.Visible:=true;
  LogosRingForm.ComboBox3.Items.Clear();
  LogosRingForm.ComboBox3.Items.Add('Preview');
  MainForm.Timer2.Enabled:=false;
  for i:=1 to GSM_GetModelFeature(FN_RINGNUMBER) do
  begin
    mybinring.location:=i;
    if (GSM_GetPhoneRingtone(@mybinring,@myring)=0) then
    begin
      if GSM_GetModelFeature(FN_RINGTONES)=F_RING_SM then
      begin
        LogosRingForm.ComboBox3.Items.Add(myring.name);
      end else
      begin
        LogosRingForm.ComboBox3.Items.Add(mybinring.name);
      end;
    end else
      LogosRingForm.ComboBox3.Items.Add('Location '+inttostr(i));
  end;
  MainForm.Timer2.Enabled:=true;
  LogosRingForm.ComboBox3.ItemIndex:=1;
  LogosRingForm.ShowModal;
  if LogosRingForm.ComboBox3.Visible then
  begin
    if LogosRingForm.ComboBox3.ItemIndex=0 then
      rington.location:=255
    else
      rington.location:=LogosRingForm.ComboBox3.ItemIndex;
    MainForm.Timer2.Enabled:=false;
    GSM_SetRingtone(@rington,@i);
    MainForm.Timer2.Enabled:=true;
  end;
end;

//phone sheet
procedure TMainForm.PhoneTabSheetShow(Sender: TObject);
var
  xxxx,xxx:array[1..256] of char;
  time:GSM_DateTime;
  siml:GSM_AllSimlocks;
  i:integer;
  code:GSM_SecurityCode;
begin
  if GSMPhoneConnected then
  begin
    GSM_GetRevision(@xxx);
    Label9.Caption:=xxx;
    GSM_GetImei(@xxx);
    Label7.Caption:=xxx;
    GSM_GetModel(@xxx);
    GSM_GetModelNumber(@xxx,@xxxx);
    Label5.Caption:=xxx;
    Label5.Caption:=Label5.Caption+' ('+xxxx;
    Label5.Caption:=Label5.Caption+')';
    GSM_GetAlarm(1,@time);
    if time.isSet<>0 then
      CheckBox1.Checked:=true
    else
      CheckBox1.Checked:=false;
    DateTimePicker1.Time:=EncodeTime(time.Hour,time.Minute,0,0);
    ListView8.Items.Clear();
    GSM_GetSimlock(@siml);
    for i:=1 to 4 do
    begin
      MainForm.ListView8.Items.Add;
      MainForm.ListView8.Items.Item[i-1].Caption:=inttostr(i);
      MainForm.ListView8.Items.Item[i-1].Subitems.Add(siml.simlocks[i].data);
      if siml.simlocks[i].enabled=0 then
        MainForm.ListView8.Items.Item[i-1].Subitems.Add('disabled')
      else
      begin
        if siml.simlocks[i].factory<>0 then
          MainForm.ListView8.Items.Item[i-1].Subitems.Add('factory')
        else
          MainForm.ListView8.Items.Item[i-1].Subitems.Add('user');
      end;
      MainForm.ListView8.Items.Item[i-1].Subitems.Add(inttostr(siml.simlocks[i].counter));
    end;
    code.Type2:=GSCT_SecurityCode;
    GSM_GetSecurityCode(@code);
    Label18.Caption:=code.Code;

    phoneinfo:=true;
  end;
end;

procedure TMainForm.Button9Click(Sender: TObject);
begin
  GSM_Reset(GSM_SoftReset);
end;

//speed dials
procedure TMainForm.SpeedTabSheetShow(Sender: TObject);
begin
  if SpeedThread<>nil then
  else begin
    if GSMPhoneConnected then
    begin
      MainForm.ListView3.Items.Clear;
      speednum:=1;
      MainForm.Timer2.Enabled:=false;
      SpeedThread:=TSpeedThread.Create(false);
    end;
  end;
end;

procedure EndSpeedThread();
begin
  if (SpeedThread<>nil) then
  begin
    SpeedThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    SpeedThread:=nil;
  end;
end;

procedure TMainForm.SpeedTabSheetHide(Sender: TObject);
begin
  EndSpeedThread();
end;

procedure TSpeedThread.Execute;
var
  entry1:GSM_SpeedDial;
  entry2:GSM_PhonebookEntry;
  i:integer;
begin
  while not Terminated do
  begin
    entry1.Number:=speednum;
    if GSM_GetSpeedDial(@entry1)=0 then
    begin
      entry2.Location:=entry1.Location;
      entry2.MemoryType:=entry1.MemoryType;
      if entry2.Location=0 then
        entry2.Location:=speednum;
      GSM_GetMemoryLocation(@entry2);
      MainForm.ListView3.Items.Add;
      MainForm.ListView3.Items.Item[speednum-1].Caption:=inttostr(speednum);
      MainForm.ListView3.Items.Item[speednum-1].Subitems.Add(entry2.Name);
      if entry2.Number[1]<>chr(0) then
        MainForm.ListView3.Items.Item[speednum-1].Subitems.Add(entry2.Number)
      else begin
        for i:=1 to entry2.SubEntriesCount do
        begin
          if entry2.SubEntries[i].EntryType=GSM_Number then
          begin
            MainForm.ListView3.Items.Item[speednum-1].Subitems.Add(entry2.SubEntries[i].Number);
            break;
          end;
        end;
      end;
      speednum:=speednum+1;
    end;
    if speednum=10 then
      EndSpeedThread();
  end;
end;

//calendar
procedure EndCalendarThread();
begin
  if (CalendarThread<>nil) then
  begin
    CalendarThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    CalendarThread:=nil;
  end;
end;

procedure AddCalendarNote(note:GSM_CalendarNote;myview:TListView);
var
  j,z:integer;
  found:boolean;
begin
  myview.Items.Add;
  myview.Items.Item[myview.Items.Count-1].Caption:=inttostr(myview.Items.Count);
  case note.Type2 of
    GCN_Reminder:myview.Items.Item[myview.Items.Count-1].Subitems.Add('Reminder');
    GCN_Call    :myview.Items.Item[myview.Items.Count-1].Subitems.Add('Call');
    GCN_Meeting :myview.Items.Item[myview.Items.Count-1].Subitems.Add('Meeting');
    GCN_Birthday:myview.Items.Item[myview.Items.Count-1].Subitems.Add('Birthday');
  end;
  myview.Items.Item[myview.Items.Count-1].Subitems.Add(note.Text);

  found:=false;
  if note.Type2=GCN_Call then
  begin
    MainForm.Label14.Caption:=note.Phone;
    for z:=1 to PBKOfflineNumber do
    begin
      if (PhoneBackup.PhonePhonebook[z].Number[1]<>chr(0)) then
      begin
        MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[z].Number;
        if (MainForm.Label14.Caption=MainForm.Label16.Caption) then
        begin
          myview.Items.Item[myview.Items.Count-1].Subitems.Add(PhoneBackup.PhonePhonebook[z].Name);
          found:=true;
          break;
        end;
      end;
      for j:=1 to PhoneBackup.PhonePhonebook[z].SubEntriesCount do
      begin
        MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[z].SubEntries[j].Number;
        if (MainForm.Label14.Caption=MainForm.Label16.Caption) then
        begin
          myview.Items.Item[myview.Items.Count-1].Subitems.Add(PhoneBackup.PhonePhonebook[z].Name);
          found:=true;
          break;
        end;
      end;
    end;
  end;
  if not found then
    myview.Items.Item[myview.Items.Count-1].Subitems.Add(note.Phone);

  myview.Items.Item[myview.Items.Count-1].Subitems.Add(DayOfweek2(note.Time.Day,note.Time.Month,note.Time.Year)+' '+inttostr2(note.Time.Day,2)+'-'+inttostr2(note.Time.Month,2)+'-'+inttostr(note.Time.Year)+' '+inttostr2(note.Time.Hour,2)+':'+inttostr2(note.Time.Minute,2)+':'+inttostr2(note.Time.Second,2));
  if note.Alarm.Year<>0 then
     myview.Items.Item[myview.Items.Count-1].Subitems.Add(DayOfweek2(note.Alarm.Day,note.Alarm.Month,note.Alarm.Year)+' '+inttostr2(note.Alarm.Day,2)+'-'+inttostr2(note.Alarm.Month,2)+'-'+inttostr(note.Alarm.Year)+' '+inttostr2(note.Alarm.Hour,2)+':'+inttostr2(note.Alarm.Minute,2)+':'+inttostr2(note.Alarm.Second,2))
  else
     myview.Items.Item[myview.Items.Count-1].Subitems.Add('');
end;

procedure TCalendarThread.Execute;
var
  note:GSM_CalendarNote;
begin
  while not Terminated do
  begin
    note.Location:=calendarnum;
    if GSM_GetCalendarNote(@note)=0 then
    begin
      AddCalendarNote(note,MainForm.ListView4);
      calendarnum:=calendarnum+1;
    end else begin
      readcalendar:=true;
      EndCalendarThread();
    end;
  end;
end;

procedure TMainForm.CalendarTabSheetShow(Sender: TObject);
begin
  if CalendarThread=nil then
  begin
    if GSMPhoneConnected then
    begin
      if (GSM_GetModelFeature(FN_Calendar)<>0) then
      begin
        ListView4.Enabled:=true;
        ListView4.Ctl3D:=true;
        GroupBox8.Ctl3D:=true;

        MainForm.ListView4.Items.Clear;
        CalendarNum:=1;
        MainForm.Timer2.Enabled:=false;
        readcalendar:=false;

        ReadPBKOffline;

        CalendarThread:=TCalendarThread.Create(false);
      end else
      begin
        ListView4.Enabled:=false;
        ListView4.Ctl3D:=false;
        GroupBox8.Ctl3D:=false;
      end;
    end;
  end;
end;

procedure TMainForm.CalendarTabSheetHide(Sender: TObject);
begin
  EndCalendarThread();
end;

procedure TMainForm.New1Click(Sender: TObject);
var i,j:integer;
begin
  setcalendarnote.Location:=MainForm.ListView4.Items.Count+1;
  CalendarForm.ComboBox1.ItemIndex:=0;
  CalendarForm.ComboBox1.OnSelect(nil);
  CalendarForm.Edit2.Text:='';
  CalendarForm.ComboBox3.Text:='';
  CalendarForm.DateTimePicker1.DateTime:=Date;
  CalendarForm.DateTimePicker3.Time:=Time;
  CalendarForm.DateTimePicker2.DateTime:=Date;
  CalendarForm.DateTimePicker4.Time:=Time;

  ReadPBKOffline;
  CalendarForm.ComboBox3.Items.Clear;
  CalendarForm.ComboBox3.Sorted:=false;
  for i:=1 to PBKOfflineNumber do
  begin
    if (PhoneBackup.PhonePhonebook[i].Number[1]<>chr(0)) then
    begin
      CalendarForm.ComboBox3.Items.Add(PhoneBackup.PhonePhonebook[i].Name);
    end else
    begin
      for j:=1 to PhoneBackup.PhonePhonebook[i].SubEntriesCount do
      begin
        if PhoneBackup.PhonePhonebook[i].SubEntries[j].EntryType=GSM_Number then
        begin
          CalendarForm.ComboBox3.Items.Add(PhoneBackup.PhonePhonebook[i].Name);
          break;
        end;
      end;
    end;
  end;
  CalendarForm.ComboBox3.Sorted:=true;

  CalendarForm.ShowModal;
end;

procedure EditCalendarNote;
var i,j,z:integer;
begin
  i:=0;

  CalendarForm.DateTimePicker1.Date:=EncodeDate(setcalendarnote.Time.Year,
                                                setcalendarnote.Time.Month,
                                                setcalendarnote.Time.Day);
  CalendarForm.DateTimePicker3.Time:=EncodeTime(setcalendarnote.Time.Hour,
                                                setcalendarnote.Time.Minute,
                                                setcalendarnote.Time.Second,i);
  case setcalendarnote.Type2  of
    GCN_BIRTHDAY:    CalendarForm.ComboBox1.ItemIndex:=0;
    GCN_CALL:        CalendarForm.ComboBox1.ItemIndex:=1;
    GCN_MEETING:     CalendarForm.ComboBox1.ItemIndex:=2;
    GCN_REMINDER:    CalendarForm.ComboBox1.ItemIndex:=3;
  end;

  CalendarForm.Edit2.Text:=setcalendarnote.Text;
  CalendarForm.ComboBox3.Text:=setcalendarnote.Phone;

  if setcalendarnote.Alarm.Year<>0 then
  begin
    CalendarForm.CheckBox1.checked:=true;
    CalendarForm.DateTimePicker2.Date:=EncodeDate(setcalendarnote.Alarm.Year,
                                                  setcalendarnote.Alarm.Month,
                                                  setcalendarnote.Alarm.Day);
    CalendarForm.DateTimePicker4.Time:=EncodeTime(setcalendarnote.Alarm.Hour,
                                                  setcalendarnote.Alarm.Minute,
                                                  setcalendarnote.Alarm.Second,i);
  end else
  begin
    CalendarForm.CheckBox1.checked:=false;
    CalendarForm.DateTimePicker2.Date:=CalendarForm.DateTimePicker1.Date;
    CalendarForm.DateTimePicker4.Time:=CalendarForm.DateTimePicker3.Time;
  end;

  if GSM_GetModelFeature(FN_CALENDAR)=F_CAL71 then
  begin
    if setcalendarnote.Type2<>GCN_BIRTHDAY then
    begin
      CalendarForm.CheckBox2.Checked:=True;
      case setcalendarnote.Recurrance of
        0     :begin
                 CalendarForm.CheckBox2.Checked:=False;
                 CalendarForm.ComboBox2.Enabled:=False;
               end;
        1*24  :CalendarForm.ComboBox2.ItemIndex:=0;
        7*24  :CalendarForm.ComboBox2.ItemIndex:=1;
        14*24 :CalendarForm.ComboBox2.ItemIndex:=2;
        365*24:CalendarForm.ComboBox2.ItemIndex:=3;
      end;
      CalendarForm.RadioButton1.Enabled:=False;
      CalendarForm.RadioButton2.Enabled:=False;
    end else begin
      CalendarForm.ComboBox2.ItemIndex:=0;
      CalendarForm.CheckBox2.Checked:=False;
      CalendarForm.ComboBox2.Enabled:=False;
      CalendarForm.RadioButton1.Enabled:=True;
      CalendarForm.RadioButton2.Enabled:=True;
      if setcalendarnote.AlarmType=1 then
        CalendarForm.RadioButton1.Checked:=True
      else
        CalendarForm.RadioButton2.Checked:=True;
    end;
  end;
  CalendarForm.ComboBox1.OnSelect(nil);

  ReadPBKOffline;
  CalendarForm.ComboBox3.Items.Clear;
  CalendarForm.ComboBox3.Sorted:=false;
  for i:=1 to PBKOfflineNumber do
  begin
    if (PhoneBackup.PhonePhonebook[i].Number[1]<>chr(0)) then
    begin
      CalendarForm.ComboBox3.Items.Add(PhoneBackup.PhonePhonebook[i].Name);
    end else
    begin
      for j:=1 to PhoneBackup.PhonePhonebook[i].SubEntriesCount do
      begin
        if PhoneBackup.PhonePhonebook[i].SubEntries[j].EntryType=GSM_Number then
        begin
          CalendarForm.ComboBox3.Items.Add(PhoneBackup.PhonePhonebook[i].Name);
          break;
        end;
      end;
    end;
  end;
  CalendarForm.ComboBox3.Sorted:=true;

  if setcalendarnote.Type2=GCN_Call then
  begin
    MainForm.Label14.Caption:=setcalendarnote.Phone;
    for i:=1 to PBKOfflineNumber do
    begin
      if (PhoneBackup.PhonePhonebook[i].Number[1]<>chr(0)) then
      begin
        MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[i].Number;
        if (MainForm.Label14.Caption=MainForm.Label16.Caption) then
        begin
          MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[i].Name;
          for z:=0 to CalendarForm.ComboBox3.Items.Count do
          begin
            if (MainForm.Label14.Caption=CalendarForm.ComboBox3.Items[z]) then
            begin
              CalendarForm.ComboBox3.ItemIndex:=z;
              break;
            end;
          end;
          break;
        end;
      end;
      for j:=1 to PhoneBackup.PhonePhonebook[i].SubEntriesCount do
      begin
        MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[i].SubEntries[j].Number;
        if (MainForm.Label14.Caption=MainForm.Label16.Caption) then
        begin
          MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[i].Name;
          for z:=0 to CalendarForm.ComboBox3.Items.Count do
          begin
            if (MainForm.Label14.Caption=CalendarForm.ComboBox3.Items[z]) then
            begin
              CalendarForm.ComboBox3.ItemIndex:=z;
              break;
            end;
          end;
          break;
        end;
      end;
    end;
  end;

  CalendarForm.ShowModal;
end;

procedure TMainForm.Edit3Click(Sender: TObject);
begin
  setcalendarnote.Location:=calendarselect;

  mainform.Timer2.Enabled:=false;
  GSM_GetCalendarNote(@setcalendarnote);
  mainform.Timer2.Enabled:=true;

  EditCalendarNote;
end;

procedure TMainForm.ListView4SelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
  if Selected then
  begin
    calendarselect:=Item.Index+1;
  end;
end;

procedure TMainForm.Delete1Click(Sender: TObject);
begin
//  if listview4.selected<>nil then
//  begin
    if MessageDlg('Do you want to delete this calendar note ?',
       mtConfirmation, [mbYes, mbNo], 0) = mrYes then
    begin
      setcalendarnote.Location:=calendarselect;
      GSM_DeleteCalendarNote(@setcalendarnote);
      MainForm.CalendarTabSheetShow(nil);
    end;
//  end;
end;

//caller groups
procedure EndCallerThread();
begin
  if (CallerThread<>nil) then
  begin
    CallerThread.Terminate();
    CallerThread:=nil;
  end;
end;

procedure TCallerThread.Execute;
begin
  while not Terminated do
  begin
    ReadCallerGroup();
    if callerinit then EndCallerThread();
  end;
end;

procedure TMainForm.CallerTabSheetShow(Sender: TObject);
begin
  if CallerThread=nil then
  begin
    if GSMPhoneConnected then
    begin
      MainForm.ListView5.Items.Clear;
      callerinit:=false;
      callernum:=1;
      CallerThread:=TCallerThread.Create(false);
    end;
  end;
end;

procedure TMainForm.CallerTabSheetHide(Sender: TObject);
begin
  EndCallerThread();
end;

//smsc
procedure TMainForm.SMSCTabSheetShow(Sender: TObject);
begin
  if SMSCThread<>nil then
  else begin
    if GSMPhoneConnected then
    begin
      MainForm.ListView6.Items.Clear;
      smscinit:=false;
      smscnum:=1;
      MainForm.Timer2.Enabled:=false;
      SMSCThread:=TSMSCThread.Create(false);
    end;
  end;
end;

procedure SMSCTabHide();
begin
  if (SMSCThread<>nil) then
  begin
    SMSCThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    SMSCThread:=nil;
  end;
end;

procedure TMainForm.SMSCTabSheetHide(Sender: TObject);
begin
  SMSCTabHide();
end;

procedure ReadSMSC();
var
  smsc:GSM_MessageCenter;
begin
    smsc.No:=smscnum;
    MainForm.StatusBar1.Panels[0].Text:='Reading SMSC '+inttostr(smscnum);
    if GSM_GetSMSCenter(@smsc)=0 then
    begin
      SMSC2[smscnum]:=smsc;
      MainForm.ListView6.Items.Add;
      MainForm.ListView6.Items.Item[smscnum-1].Caption:=inttostr(smscnum);
      MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add(smsc.name);
      MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add(smsc.number);
      case smsc.Validity of
        GSMV_1_Hour   : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('1 hour');
        GSMV_6_Hours  : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('6 hours');
        GSMV_24_Hours : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('1 day');
        GSMV_72_Hours : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('3 days');
        GSMV_1_Week   : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('1 week');
        GSMV_Max_Time : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('max. time');
      else
        MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('unknown');
      end;
      case smsc.Format of
        GSMF_Text   : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('text');
        GSMF_Fax    : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('fax');
        GSMF_Voice  : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('voice');
        GSMF_ERMES  : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('ERMES');
        GSMF_Paging : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('paging');
        GSMF_UCI    : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('UCI');
        GSMF_Email  : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('email');
        GSMF_X400   : MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('X400');
      else
        MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add('unknown');
      end;
      MainForm.ListView6.Items.Item[smscnum-1].Subitems.Add(smsc.defaultrecipient);
      smscnum:=smscnum+1;
    end else
    begin
      MainForm.StatusBar1.Panels[0].Text:='';
      smscinit:=true;
    end;
end;

procedure TSMSCThread.Execute;
begin
  while not Terminated do
  begin
    ReadSMSC();
    if smscinit then SMSCTabHide();
  end;
end;

//profiles
procedure ProfileTabHide();
begin
  if (ProfileThread<>nil) then
  begin
    ProfileThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    ProfileThread:=nil;
  end;
end;

procedure TProfileThread.Execute;
var
  profile:GSM_Profile;
begin
  while not Terminated do
  begin
    profile.number:=profilenum-1;
    GSM_GetProfile(@profile);
    MainForm.ListView7.Items.Add;
    MainForm.ListView7.Items.Item[profilenum-1].Caption:=inttostr(profilenum);
    MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add(profile.name);
    case profile.CallAlert of
      PROFILE_CALLALERT_RINGING  :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('ringing');
      PROFILE_CALLALERT_BEEPONCE :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('beep once');
      PROFILE_CALLALERT_OFF      :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('off');
      PROFILE_CALLALERT_RINGONCE :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('ring once');
      PROFILE_CALLALERT_ASCENDING:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('ascending');
      PROFILE_CALLALERT_CALLERGROUPS:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('caller groups');
    end;
    MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add(inttostr(profile.Ringtone));
    case profile.Volume of
      PROFILE_VOLUME_LEVEL1:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 1');
      PROFILE_VOLUME_LEVEL2:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 2');
      PROFILE_VOLUME_LEVEL3:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 3');
      PROFILE_VOLUME_LEVEL4:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 4');
      PROFILE_VOLUME_LEVEL5:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 5');
    end;
    case profile.Vibration of
      PROFILE_VIBRATION_OFF  :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('off');
      PROFILE_VIBRATION_ON   :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('on');
      PROFILE_VIBRATION_FIRST:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('vibrate first');
    end;
    case profile.MessageTone of
      PROFILE_MESSAGE_NOTONE   :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('no tone');
      PROFILE_MESSAGE_STANDARD :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('standard');
      PROFILE_MESSAGE_SPECIAL  :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('special');
      PROFILE_MESSAGE_BEEPONCE :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('beep once');
      PROFILE_MESSAGE_ASCENDING:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('ascending');
    end;
    case profile.KeypadTone of
      PROFILE_KEYPAD_OFF   :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('off');
      PROFILE_KEYPAD_LEVEL1:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 1');
      PROFILE_KEYPAD_LEVEL2:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 2');
      PROFILE_KEYPAD_LEVEL3:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('volume 3');
    end;
    case profile.WarningTone of
     PROFILE_WARNING_OFF:MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('off');
     PROFILE_WARNING_ON :MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add('on');
    end;
    MainForm.ListView7.Items.Item[profilenum-1].Subitems.Add(inttostr(profile.CallerGroups));
    profilenum:=profilenum+1;
    case GSM_GetModelFeature(FN_PROFILES) of
      F_PROF61:if profilenum=8 then ProfileTabHide();
      F_PROF51:if profilenum=3 then ProfileTabHide();
      F_PROF33:if profilenum=7 then ProfileTabHide();
    end;
  end;
end;

procedure TMainForm.ProfileTabSheetShow(Sender: TObject);
begin
  if ProfileThread<>nil then
  else begin
    if GSMPhoneConnected then
    begin
      MainForm.ListView7.Items.Clear;
      profilenum:=1;
      MainForm.Timer2.Enabled:=false;
      ProfileThread:=TProfileThread.Create(false);
    end;
  end;
end;

procedure TMainForm.ProfileTabSheetHide(Sender: TObject);
begin
  ProfileTabHide();
end;

//sms
procedure AddSMSInfo(myobject:TListView;mysms:GSM_SMSMessage);
var number,i,z:integer;
    found:boolean;
begin
  number:=myobject.Items.Count;

  myobject.Items.Add;
  myobject.Items.Item[number].Caption:=inttostr(number+1);
  myobject.Items.Item[number].Subitems.Add(mysms.MessageText);

  found:=false;
  for z:=1 to PBKOfflineNumber do
  begin
    mainform.Label14.Caption:=mysms.Sender;
    mainform.Label16.Caption:=PhoneBackup.PhonePhonebook[z].Number;
    if (PhoneBackup.PhonePhonebook[z].Number[1]<>chr(0)) then
    begin
      if (mainform.Label14.Caption=mainform.Label16.Caption) then
      begin
        myobject.Items.Item[number].Subitems.Add(PhoneBackup.PhonePhonebook[z].Name);
        found:=true;
        break;
      end;
    end;
    for i:=1 to PhoneBackup.PhonePhonebook[z].SubEntriesCount do
    begin
      mainform.Label16.Caption:=PhoneBackup.PhonePhonebook[z].SubEntries[i].Number;
      if (PhoneBackup.PhonePhonebook[z].SubEntries[i].EntryType=GSM_Number)
         and (mainform.Label14.Caption=mainform.Label16.Caption) then
      begin
        myobject.Items.Item[number].Subitems.Add(PhoneBackup.PhonePhonebook[z].Name);
        found:=true;
        break;
      end;
    end;
    if found then break;
  end;
  if not found then myobject.Items.Item[number].Subitems.Add(mysms.Sender);

  if mysms.SMSData=1 then
  begin
    myobject.Items.Item[number].Subitems.Add(DayOfweek2(mysms.Time.Day,mysms.Time.Month,mysms.Time.Year)+' '+inttostr2(mysms.Time.Day,2)+'-'+inttostr2(mysms.Time.Month,2)+'-'+inttostr(mysms.Time.Year));
    myobject.Items.Item[number].Subitems.Add(inttostr2(mysms.Time.Hour,2)+':'+inttostr2(mysms.Time.Minute,2)+':'+inttostr2(mysms.Time.Second,2));
  end;
end;

procedure DisplaySMSfolder();
var
  i:integer;
begin
  MainForm.ListView2.Items.Clear();
  i:=0;
  while i<>SMSstatus.Number do
  begin
    i:=i+1;
    if (SMSfolders.Folder[SMS[i].folder].Name=
        SMSfolders.Folder[MainForm.ComboBox3.ItemIndex].Name) then
    begin
      AddSMSInfo(MainForm.ListView2,SMS[i]);
    end;
  end;
end;

procedure SendSaveSMS(mysms:GSM_SMSMessage);
var i,j,z:integer;
begin
  //text
  SMSEditForm.Memo1.Text:=mysms.MessageText;

  //number
  SMSEditForm.ComboBox9.Text:=mysms.sender;
  AddSMSEditSenderList;
  MainForm.Label14.Caption:=mysms.sender;
  for i:=1 to PBKOfflineNumber do
  begin
    if (PhoneBackup.PhonePhonebook[i].Number[1]<>chr(0)) then
    begin
      MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[i].Number;
      if (MainForm.Label14.Caption=MainForm.Label16.Caption) then
      begin
        MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[i].Name;
        for z:=0 to SMSEditForm.ComboBox9.Items.Count do
        begin
          if (MainForm.Label14.Caption=SMSEditForm.ComboBox9.Items[z]) then
          begin
            SMSEditForm.ComboBox9.ItemIndex:=z;
            break;
          end;
        end;
        break;
      end;
    end;
    for j:=1 to PhoneBackup.PhonePhonebook[i].SubEntriesCount do
    begin
      MainForm.Label16.Caption:=PhoneBackup.PhonePhonebook[i].SubEntries[j].Number;
      if (MainForm.Label14.Caption=MainForm.Label16.Caption) then
      begin
        MainForm.Label14.Caption:=PhoneBackup.PhonePhonebook[i].Name;
        for z:=0 to SMSEditForm.ComboBox9.Items.Count do
        begin
          if (MainForm.Label14.Caption=SMSEditForm.ComboBox9.Items[z]) then
          begin
            SMSEditForm.ComboBox9.ItemIndex:=z;
            break;
          end;
        end;
        break;
      end;
    end;
  end;

  //sms folder
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=0 to MainForm.ComboBox3.items.count-1 do
  begin
    SMSEditForm.ComboBox1.Items.Add(MainForm.ComboBox3.Items.Strings[i]);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=MainForm.ComboBox3.ItemIndex;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=0;

  if mysms.ReplyViaSameSMSC<>0 then
    SMSEditForm.CheckBox3.Checked:=true
  else
    SMSEditForm.CheckBox3.Checked:=false;

  if mysms.Class2=1 then
    SMSEditForm.CheckBox1.Checked:=true
  else
    SMSEditForm.CheckBox1.Checked:=false;

  if mysms.status=GSS_SENTREAD then
    SMSEditForm.CheckBox4.Checked:=true
  else
    SMSEditForm.CheckBox4.Checked:=false;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PbkSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.TextSMSTabSheet;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.Edit4Click(Sender: TObject);
begin
  setsmslocation:=SMS[smsnumindex].Location;
  SendSaveSMS(SMS[smsnumindex]);
end;

procedure TMainForm.New2Click(Sender: TObject);
var i:integer;
begin
  setsmslocation:=0;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;

  //folders
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=0 to ComboBox3.items.count-1 do
  begin
    SMSEditForm.ComboBox1.Items.Add(ComboBox3.Items.Strings[i]);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=ComboBox3.ItemIndex;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.TextSMSTabSheet;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure EndSMSThread;
begin
  if (SMSThread<>nil) then
  begin
    SMSThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    SMSThread:=nil;
  end;
end;

procedure TSMSThread.Execute;
var
  error:integer;
begin
  while not Terminated do
  begin
    if SMSnum<>SMSstatus.Number then
    begin
      if GSM_GetModelFeature(FN_SMS)=F_SMS71 then
        SMS[SMSnum+1].Location:=0
      else
        SMS[SMSnum+1].Location:=smsnum2;
      error:=GSM_GetSMSMessage(@SMS[SMSnum+1]);
      if error=0 then
      begin
        if SMS[SMSnum+1].Time.year<1900 then
        begin
          SMS[SMSnum+1].Time.year:=SMS[SMSnum+1].Time.year+2000;
        end;
        if (SMSfolders.Folder[SMS[SMSnum+1].folder].Name=
            SMSfolders.Folder[MainForm.ComboBox3.ItemIndex].Name) then
        begin
          AddSMSInfo(MainForm.ListView2,SMS[SMSnum+1]);
        end;
        SMSnum:=SMSnum+1;
      end;
      if error=GE_SMSTOOLONG then
      begin
        SMSstatus.Number:=SMSstatus.Number-1;
      end;

      smsnum2:=smsnum2+1;
    end else
    begin
      MainForm.ComboBox3.Enabled:=true;
      EndSMSThread;
    end;
  end;
end;

procedure TMainForm.SMSTabSheetShow(Sender: TObject);
var
  i:integer;
begin
  MainForm.ComboBox3.Enabled:=false;
  if GSMPhoneConnected then
  begin
    if smscinit=false then
    begin
      MainForm.ListView6.Items.Clear;
      smscnum:=1;
      while true do
      begin
        ReadSMSC;
        if smscinit then break;
      end;
    end;

    MainForm.Memo1.Lines.Clear;
    MainForm.Memo2.Lines.Clear;
    GSM_GetSMSFolders(@SMSfolders);
    MainForm.ComboBox3.Items.Clear();
    for i:=1 to SMSfolders.number do
    begin
      MainForm.ComboBox3.Items.Add(SMSfolders.folder[i].Name);
    end;
    MainForm.ComboBox3.ItemIndex:=0;

    SMSnum:=0;
    GSM_GetSMSStatus(@SMSstatus);
    smsnum2:=1;
    MainForm.ListView2.Items.Clear;
    MainForm.Timer2.Enabled:=false;
    ReadPBKOffline;
    SMSThread:=TSMSThread.Create(false);
  end;
end;

procedure TMainForm.ComboBox3Change(Sender: TObject);
begin
  ReadPBKOffline;
  DisplaySMSfolder();
end;

procedure AddSMSDescription(mysms:GSM_SMSMessage;firstmemo:TMemo;secondmemo:TMemo);
var
  ch:tcaption;
  z:integer;
begin
  firstmemo.Lines.Clear;
  firstmemo.Lines.Add(pchar('Sender: '+mysms.Sender));

  if mysms.SMSData=1 then
  begin
    ch:=mysms.MessageCenter.Number;
    MainForm.Label14.Caption:=mysms.MessageCenter.Number;
    if (smscinit) and (GSMPhoneConnected) then
    begin
      for z:=1 to smscnum-1 do
      begin
        MainForm.Label16.Caption:=SMSC2[z].Number;
        if MainForm.Label14.Caption=MainForm.Label16.caption then
          ch:=SMSC2[z].Name;
        end;
      end;
      if mysms.ReplyViaSameSMSC<>0 then
         ch:=' (reply via the same SMSC) '+ch;
         firstmemo.Lines.Add('SMSC: '+ch);

        if (mysms.type2=GST_DR) and (mysms.MessageText[1]='D') then
        begin
          firstmemo.Lines.Add(pchar('Date & time: '+
            DayOfweek2(mysms.Time.Day,mysms.Time.Month,mysms.Time.Year)+' '+inttostr2(mysms.Time.Day,2)+'-'+inttostr2(mysms.Time.Month,2)+'-'+inttostr(mysms.Time.Year)+' '+
            inttostr2(mysms.Time.Hour,2)+':'+inttostr2(mysms.Time.Minute,2)+':'+inttostr2(mysms.Time.Second,2))+
            '   SMSC response date & time: '+
            DayOfweek2(mysms.SMSCTime.Day,mysms.SMSCTime.Month,mysms.SMSCTime.Year)+' '+inttostr2(mysms.SMSCTime.Day,2)+'-'+inttostr2(mysms.SMSCTime.Month,2)+'-'+inttostr(mysms.SMSCTime.Year)+' '+
            inttostr2(mysms.SMSCTime.Hour,2)+':'+inttostr2(mysms.SMSCTime.Minute,2)+':'+inttostr2(mysms.SMSCTime.Second,2));
        end else
        begin
          firstmemo.Lines.Add(pchar('Date & time: '+
            DayOfweek2(mysms.Time.Day,mysms.Time.Month,mysms.Time.Year)+' '+inttostr2(mysms.Time.Day,2)+'-'+inttostr2(mysms.Time.Month,2)+'-'+inttostr(mysms.Time.Year)+' '+
            inttostr2(mysms.Time.Hour,2)+':'+inttostr2(mysms.Time.Minute,2)+':'+inttostr2(mysms.Time.Second,2)));
        end;
      end;

  secondmemo.Lines.Clear;
  secondmemo.Lines.Add(pchar(''+mysms.MessageText));
end;

procedure TMainForm.ListView2SelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
var
  i:integer;
  j:integer;
begin
  j:=0;
  i:=0;
  while i<>SMSstatus.Number do
  begin
    i:=i+1;
    if (SMSfolders.Folder[SMS[i].folder].Name=
        SMSfolders.Folder[MainForm.ComboBox3.ItemIndex].Name) then
    begin
      if (j=MainForm.ListView2.ItemIndex) then
      begin
        smsnumindex:=i;
        AddSMSDescription(SMS[i],MainForm.Memo1,MainForm.Memo2);
      end;
      j:=j+1;
    end;
  end;
end;

procedure TMainForm.SMSTabSheetHide(Sender: TObject);
begin
  EndSMSThread;
end;

procedure TMainForm.SMSPopupMenuPopup(Sender: TObject);
begin
  New2.Enabled:=false;
  Edit4.Enabled:=false;
  CopyToPC1.Enabled:=false;
  Delete2.Enabled:=false;
  if GSMPhoneConnected then
  begin
    New2.Enabled:=true;
    if listview2.selected<>nil then
    begin
      Edit4.Enabled:=true;
      CopyToPC1.Enabled:=true;
      Delete2.Enabled:=true;
    end;
  end;
end;

procedure TMainForm.CalendarPopupMenuPopup(Sender: TObject);
begin
  if GSMPhoneConnected then
  begin
    New1.Enabled:=true;
    if listview4.selected<>nil then
    begin
      Edit3.Enabled:=true;
      Delete1.Enabled:=true;
      SMS1.Enabled:=true;
      CopyToPC2.Enabled:=true;
    end else
    begin
      Edit3.Enabled:=false;
      Delete1.Enabled:=false;
      SMS1.Enabled:=false;
      CopyToPC2.Enabled:=false;
    end;
  end else
  begin
    New1.Enabled:=false;
    Delete1.Enabled:=false;
    Edit3.Enabled:=false;
    SMS1.Enabled:=false;
    CopyToPC2.Enabled:=false;
  end;
end;

procedure TMainForm.Edit1Change(Sender: TObject);
var
  i:integer;
begin
  for i:=1 to 20 do
    rington.name[i]:=chr(0);
  for i:=1 to strlen(pchar(edit1.text)) do
    rington.name[i]:=edit1.text[i];
end;

procedure TMainForm.Delete2Click(Sender: TObject);
var
  mysms:GSM_SMSMessage;
begin
  if MessageDlg('Do you want to delete this SMS ?',
      mtConfirmation, [mbYes, mbNo], 0) = mrYes then
  begin
    mysms.Location:=SMS[smsnumindex].Location;
    MainForm.Timer2.Enabled:=false;
    GSM_DeleteSMSMessage(@mysms);
    MainForm.Timer2.Enabled:=true;
    MainForm.SMSTabSheetShow(nil);
  end;
end;

procedure TMainForm.Edit5Click(Sender: TObject);
begin
//  if listview6.selected<>nil then
//  begin
      smsceditform.edit1.Text:=smsc2[smscselectitem].name;
      smsceditform.edit2.Text:=smsc2[smscselectitem].number;
      smsceditform.edit3.Text:=smsc2[smscselectitem].DefaultRecipient;
      case smsc2[smscselectitem].Validity of
        GSMV_1_Hour   : SMSCEditForm.combobox1.itemindex:=0;
        GSMV_6_Hours  : SMSCEditForm.combobox1.itemindex:=1;
        GSMV_24_Hours : SMSCEditForm.combobox1.itemindex:=2;
        GSMV_72_Hours : SMSCEditForm.combobox1.itemindex:=3;
        GSMV_1_Week   : SMSCEditForm.combobox1.itemindex:=4;
      else
        SMSCEditForm.combobox1.itemindex:=5;
      end;
      case smsc2[smscselectitem].Format of
        GSMF_Text   : SMSCEditForm.combobox2.itemindex:=0;
        GSMF_Fax    : SMSCEditForm.combobox2.itemindex:=1;
        GSMF_Email  : SMSCEditForm.combobox2.itemindex:=2;
      else
        SMSCEditForm.combobox2.itemindex:=3;
      end;
  SMSCEditForm.ShowModal;
//  end;
end;

procedure TMainForm.SMSCPopupMenuPopup(Sender: TObject);
begin
  if GSMPhoneConnected then
  begin
    if listview6.selected<>nil then
      Edit5.Enabled:=true
    else
      Edit5.Enabled:=false;
  end else
  begin
    Edit5.Enabled:=false;
  end;

end;

procedure TMainForm.ListView6SelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
  if Selected then
  begin
    smscselectitem:=Item.Index+1;
  end;
end;

procedure TMainForm.ListView1SelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
 if Selected then
  begin
    pbknumberentry:=Item.Index+1;
  end;
end;

procedure TMainForm.Edit7Click(Sender: TObject);
var
  current:integer;
  i:integer;
begin
//  if listview1.Selected<>nil then
//  begin
    pbk.Location:=pbknumberentry;
    case ComboBox1.ItemIndex of
      1:begin pbk.MemoryType:=GMT_DC;pbk.Location:=pbk.Location-1;end;
      2:begin pbk.MemoryType:=GMT_MC;pbk.Location:=pbk.Location-1;end;
      3:begin pbk.MemoryType:=GMT_ON;pbk.Location:=pbk.Location-1;end;
      4:pbk.MemoryType:=GMT_ME;
      5:pbk.MemoryType:=GMT_SM;
      6:begin pbk.MemoryType:=GMT_RC;pbk.Location:=pbk.Location-1;end;
    end;
    GSM_GetMemoryLocation(@pbk);
    current:=0;
    pbkform.ListView1.Items.Clear;
    pbkform.ListView1.Items.Add;
    pbkform.ListView1.Items.Item[0].Caption:='Name';
    pbkform.ListView1.Items.Item[0].SubItems.Add(pbk.name);
    if (pbk.Number[1]<>chr(0)) then
    begin
      current:=current+1;
      pbkform.ListView1.Items.Add;
      pbkform.ListView1.Items.Item[current].Caption:='Number';
      pbkform.ListView1.Items.Item[current].SubItems.Add(pbk.number);
    end;
    for i:=1 to pbk.SubEntriesCount do
    begin
      current:=current+1;
      pbkform.ListView1.Items.Add;
      case pbk.SubEntries[i].EntryType of
        GSM_Number:case pbk.SubEntries[i].NumberType of
                     GSM_General :pbkform.ListView1.Items.Item[current].Caption:='General number';
                     GSM_General2:pbkform.ListView1.Items.Item[current].Caption:='General number';
                     GSM_Mobile  :pbkform.ListView1.Items.Item[current].Caption:='Mobile';
                     GSM_Work    :pbkform.ListView1.Items.Item[current].Caption:='Work';
                     GSM_Fax     :pbkform.ListView1.Items.Item[current].Caption:='Fax';
                     GSM_Home    :pbkform.ListView1.Items.Item[current].Caption:='Home';
                   else
                     pbkform.ListView1.Items.Item[current].Caption:=inttostr(pbk.SubEntries[i].NumberType);
                   end;
        GSM_Note  :pbkform.ListView1.Items.Item[current].Caption:='Note';
        GSM_Postal:pbkform.ListView1.Items.Item[current].Caption:='Address';
        GSM_Email :pbkform.ListView1.Items.Item[current].Caption:='Email';
        GSM_Date  :begin
                     pbkform.ListView1.Items.Item[current].Caption:='Date';
                     pbkform.ListView1.Items.Item[current].SubItems.Add(DayOfweek2(pbk.SubEntries[i].Date.Day,pbk.SubEntries[i].Date.Month,pbk.SubEntries[i].Date.Year)+' '+inttostr2(pbk.SubEntries[i].Date.Day,2)+'-'+inttostr2(pbk.SubEntries[i].Date.Month,2)+'-'+inttostr(pbk.SubEntries[i].Date.Year)+' '+inttostr2(pbk.SubEntries[i].Date.Hour,2)+':'+inttostr2(pbk.SubEntries[i].Date.Minute,2)+':'+inttostr2(pbk.SubEntries[i].Date.Second,2));
                   end;
      end;
      if (pbk.SubEntries[i].EntryType<>GSM_Date) then
         pbkform.ListView1.Items.Item[current].SubItems.Add(pbk.SubEntries[i].Number);
    end;

    if (pbk.Group>=0) and (pbk.Group<=4) then
    begin
      current:=current+1;
      pbkform.ListView1.Items.Add;
      pbkform.ListView1.Items.Item[current].Caption:='Caller group';
      pbkform.ListView1.Items.Item[current].SubItems.Add(caller[pbk.Group+1].text);
    end;
    pbkform.showmodal;
//  end;
end;

procedure TMainForm.Button18Click(Sender: TObject);
var
  i,j:integer;
  Name,Data:array[1..255] of char;
begin
  if smscinit=false then
  begin
    MainForm.ListView6.Items.Clear;
    smscnum:=1;
    while true do
    begin
      ReadSMSC;
      if smscinit then break;
    end;
  end;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;
  
  //folders
  GSM_GetSMSFolders(@SMSfolders);
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=1 to SMSFolders.number do
  begin
    SMSEditForm.ComboBox1.Items.Add(SMSfolders.folder[i].Name);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=0;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.BitmapSMSTabSheet;

  SMSEditForm.Image1.Picture:=Image1.Picture;

  SMSEditForm.Memo2.Text:='';
  SMSEditForm.CheckBox5.Checked:=false;
  if bitma.type2<>GSM_PictureImage then
  begin
    SMSEditForm.Memo2.Visible:=false;
    SMSEditForm.Label10.Visible:=false;
    SMSEditForm.CheckBox5.Visible:=false;
    SMSEditForm.CheckBox6.Visible:=false;
  end else
  begin
    SMSEditForm.Memo2.Visible:=true;
    SMSEditForm.Label10.Visible:=true;
    SMSEditForm.CheckBox5.Visible:=true;
    SMSEditForm.CheckBox6.Visible:=true;
  end;

  SMSEditForm.Label9.Visible:=false;
  SMSEditForm.ComboBox7.Visible:=false;
  if bitma.type2=GSM_OperatorLogo then
  begin
    SMSEditForm.Label9.Visible:=true;
    SMSEditForm.ComboBox7.Visible:=true;
    //preparing network names
    i:=0;
    while true do
    begin
      GSM_GetNetworkData(i,@Name,@Data);

      // conversion problems
      Label14.Caption:=Name;
      if strcomp(PChar(Label14.Caption),PChar('unknown'))=0 then break;

      // conversion problems
      Label14.Caption:=' - '+Label14.Caption;
      for j:=6 downto 1 do
      begin
        Label14.Caption:=Data[j]+Label14.Caption;
      end;

      SMSEditForm.ComboBox7.Items.Add(Pchar(Label14.Caption));
      i:=i+1;
    end;
    //looking for current network
    if GSMPhoneConnected then
    begin
      // conversion problems
      Label14.Caption:=MainForm.StatusBar1.Panels[1].Text;
      Label14.Caption:=' - '+Label14.Caption;
      for j:=6 downto 1 do
      begin
        Label14.Caption:=GSMNetworkInfo.NetworkCode[j]+Label14.Caption;
      end;

      i:=1;
      while true do
      begin
        if strcomp(pchar(SMSEditForm.ComboBox7.Items[i]),pchar(Label14.Caption))=0
          then break;
        i:=i+1;
      end;
      SMSEditForm.ComboBox7.ItemIndex:=i;
    end;
  end;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.Button17Click(Sender: TObject);
var
  i:integer;
begin
  if smscinit=false then
  begin
    MainForm.ListView6.Items.Clear;
    smscnum:=1;
    while true do
    begin
      ReadSMSC;
      if smscinit then break;
    end;
  end;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;
  
  //folders
  GSM_GetSMSFolders(@SMSfolders);
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=1 to SMSFolders.number do
  begin
    SMSEditForm.ComboBox1.Items.Add(SMSfolders.folder[i].Name);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=0;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.RingtoneSMSTabSheet;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.SMS1Click(Sender: TObject);
var
  i:integer;
begin
  if smscinit=false then
  begin
    MainForm.ListView6.Items.Clear;
    smscnum:=1;
    while true do
    begin
      ReadSMSC;
      if smscinit then break;
    end;
  end;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;
  
  //folders
  GSM_GetSMSFolders(@SMSfolders);
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=1 to SMSFolders.number do
  begin
    SMSEditForm.ComboBox1.Items.Add(SMSfolders.folder[i].Name);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=0;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.CalendarSMSTabSheet;

  calenda.Location:=calendarselect;
  MainForm.Timer2.Enabled:=false;
  GSM_GetCalendarNote(@calenda);
  MainForm.Timer2.Enabled:=true;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.SMSSendSave1Click(Sender: TObject);
var
  i:integer;
begin
  if smscinit=false then
  begin
    MainForm.ListView6.Items.Clear;
    smscnum:=1;
    while true do
    begin
      ReadSMSC;
      if smscinit then break;
    end;
  end;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;
  
  //folders
  GSM_GetSMSFolders(@SMSfolders);
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=1 to SMSFolders.number do
  begin
    SMSEditForm.ComboBox1.Items.Add(SMSfolders.folder[i].Name);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=0;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.PBKSMSTabSheet;

  pbk.Location:=pbknumberentry-1;
  case ComboBox1.ItemIndex of
    1:pbk.MemoryType:=GMT_DC;
    2:pbk.MemoryType:=GMT_MC;
    3:pbk.MemoryType:=GMT_ON;
    4:pbk.MemoryType:=GMT_ME;
    5:pbk.MemoryType:=GMT_SM;
    6:pbk.MemoryType:=GMT_RC;
  end;
  MainForm.Timer2.Enabled:=false;
  GSM_GetMemoryLocation(@pbk);
  MainForm.Timer2.Enabled:=true;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.Button13Click(Sender: TObject);
var
  ring:GSM_BinRingtone;
begin
  if OpenDialog1.Execute then
  begin
    if GSM_ReadBinRingtoneFile(PChar(OpenDialog1.FileName),@ring)=0 then
    begin
      Edit2.Text:=ring.Name;
      binring:=ring;
    end;
  end;

end;

procedure TMainForm.Button14Click(Sender: TObject);
var
  i:integer;
  mybinring:GSM_BinRingtone;
  myring:GSM_Ringtone;
begin
  LogosRingForm.Label1.Caption:='Do you want to set ringtone ?';
  LogosRingForm.ComboBox3.Visible:=true;
  LogosRingForm.ComboBox3.Items.Clear();
  MainForm.Timer2.Enabled:=false;
  for i:=1 to GSM_GetModelFeature(FN_RINGNUMBER) do
  begin
    mybinring.location:=i;
    if (GSM_GetPhoneRingtone(@mybinring,@myring)=0) then
    begin
      if GSM_GetModelFeature(FN_RINGTONES)=F_RING_SM then
      begin
        LogosRingForm.ComboBox3.Items.Add(myring.name);
      end else
      begin
        LogosRingForm.ComboBox3.Items.Add(mybinring.name);
      end;
    end else
      LogosRingForm.ComboBox3.Items.Add('Location '+inttostr(i));
  end;
  MainForm.Timer2.Enabled:=true;
  LogosRingForm.ComboBox3.ItemIndex:=0;
  LogosRingForm.ShowModal;
  if LogosRingForm.ComboBox3.Visible then
  begin
    binring.location:=LogosRingForm.ComboBox3.ItemIndex+1;
    MainForm.Timer2.Enabled:=false;
    GSM_SetBinRingtone(@binring);
    MainForm.Timer2.Enabled:=true;
  end;
end;

procedure TMainForm.ConfigTabSheetHide(Sender: TObject);
var
  MyIniFile: TIniFile;
begin
  if MessageDlg('Do you want to save config ?',
    mtConfirmation, [mbYes, mbNo], 0) = mrYes then
  begin
    MyIniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'\gnokiirc');
    case MainForm.ComboBox4.ItemIndex of
      0:MyIniFile.WriteString('global', 'connection', 'mbus');
      1:MyIniFile.WriteString('global', 'connection', 'fbus');
      2:MyIniFile.WriteString('global', 'connection', 'dlr3');
      3:MyIniFile.WriteString('global', 'connection', 'infrared');
    end;
    MyIniFile.WriteString('global', 'model', MainForm.ComboBox6.Text);
    MyIniFile.WriteString('global', 'port', MainForm.ComboBox5.Text);
    if MainForm.CheckBox2.Checked then
      MyIniFile.WriteString('global', 'synchronizetime', 'yes')
    else
      MyIniFile.WriteString('global', 'synchronizetime', 'no');
    MyIniFile.Free;
  end;
end;

procedure TMainForm.ConfigTabSheetShow(Sender: TObject);
var
  MyIniFile: TIniFile;
  x:string;
  i:integer;
begin
  MyIniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'\gnokiirc');

  x:=MyInifile.ReadString('global', 'connection', 'fbus');
  MainForm.ComboBox4.ItemIndex:=0;
  if x='fbus' then MainForm.ComboBox4.ItemIndex:=1;
  if x='dlr3' then MainForm.ComboBox4.ItemIndex:=2;
  if x='infrared' then MainForm.ComboBox4.ItemIndex:=3;

  x:=MyInifile.ReadString('global', 'port', 'com2:');
  MainForm.ComboBox5.ItemIndex:=0;
  for i:=1 to MainForm.ComboBox5.Items.Count do
  begin
    if MainForm.ComboBox5.Items.Strings[i-1]=x then
      MainForm.ComboBox5.ItemIndex:=i-1;
  end;

  x:=MyInifile.ReadString('global', 'model', 'auto');
  MainForm.ComboBox6.ItemIndex:=0;
  for i:=1 to MainForm.ComboBox6.Items.Count do
  begin
    if MainForm.ComboBox6.Items.Strings[i-1]=x then
      MainForm.ComboBox6.ItemIndex:=i-1;
  end;

  x:=MyInifile.ReadString('global', 'synchronizetime', 'no');
  if x='yes' then
    MainForm.CheckBox2.Checked:=True
  else
    MainForm.CheckBox2.Checked:=False;
  MyIniFile.Free;
end;

procedure TMainForm.Timer2Timer(Sender: TObject);
begin
  if SparkyThread.Suspended then SparkyThread.Resume;
  Timer2.Interval:=2000;
end;

procedure TMainForm.AboutTabSheetShow(Sender: TObject);
var
  xxx:array[1..256] of char;
begin
  GSM_GetDLLVersion(@xxx);
  Label20.Caption:=xxx;
end;

procedure SaveSMSOffline;
var F:file of GSM_SMSMessage;
    i:integer;
begin
  AssignFile(F, 'gnokiisms');
  Rewrite(F);
  for i:=1 to SMSOfflineNumber do
  begin
    Write(f,SMSOffline[i]);
  end;
  CloseFile(F);
end;

procedure ReadSMSOffline;
var F:file of GSM_SMSMessage;
begin
  MainForm.ListView10.items.Clear;
  AssignFile(F, 'gnokiisms');
  Reset(F);
  if (ioresult=0) then
  begin
    ReadPBKOffline;
    smsofflinenumber:=0;
    while not eof(f) do
    begin
      smsofflinenumber:=smsofflinenumber+1;
      read(f,smsoffline[smsofflinenumber]);
      AddSMSInfo(MainForm.ListView10,smsoffline[smsofflinenumber]);
    end;
    CloseFile(F);
  end;
end;

procedure TMainForm.PCSMSTabSheetShow(Sender: TObject);
begin
  ReadSMSOffline;
end;

procedure TMainForm.CopytoPC1Click(Sender: TObject);
begin
  SMSofflinenumber:=smsofflinenumber+1;
  SMSoffline[SMSofflinenumber]:=SMS[smsnumindex];
  SaveSMSOffline;
end;

procedure TMainForm.ListView10SelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
  AddSMSDescription(SMSOffline[Item.Index+1],MainForm.Memo4,MainForm.Memo5);
end;

procedure TMainForm.PCSMSPopupMenuPopup(Sender: TObject);
begin
  SendSave1.Enabled:=False;
  if GSMPhoneConnected then SendSave1.Enabled:=True;

  Delete3.Enabled:=false;
  if listview10.selected<>nil then Delete3.Enabled:=True;
end;

procedure TMainForm.Delete3Click(Sender: TObject);
var
  i:integer;
begin
  if Application.MessageBox('Do you want to delete this SMS ?','Question',MB_YESNO)=IDYES then
  begin
    for i:=MainForm.ListView10.ItemIndex to smsofflinenumber-1 do
    begin
      SMSOffline[i]:=SMSOffline[i+1];
    end;
    Smsofflinenumber:=smsofflinenumber-1;
    SaveSMSOffline;
    ReadSMSOffline;
  end;
end;

procedure TMainForm.SendSave1Click(Sender: TObject);
var i:integer;
begin
  GSM_GetSMSFolders(@SMSfolders);
  MainForm.ComboBox3.Items.Clear();
  for i:=1 to SMSfolders.number do
  begin
    MainForm.ComboBox3.Items.Add(SMSfolders.folder[i].Name);
  end;
  MainForm.ComboBox3.ItemIndex:=0;
  setsmslocation:=0;
  SendSaveSMS(SMSOffline[MainForm.ListView10.ItemIndex+1]);
end;


procedure EndWAPBookmarkThread;
begin
  if (WAPBookmarkThread<>nil) then
  begin
    WAPBookmarkThread.Terminate();
    MainForm.Timer2.Enabled:=true;
    WAPBookmarkThread:=nil;
  end;
end;

procedure TWAPBookmarkThread.Execute;
var
  bookmark:GSM_WAPBookmark;
  i:integer;
begin
  while not Terminated do
  begin
    bookmark.location:=bookmarknum;
    i:=GSM_GetWAPBookmark(@bookmark);
    MainForm.ListView9.Items.Add;
    MainForm.ListView9.Items.Item[bookmarknum-1].Caption:=inttostr(bookmarknum);
    if (i=0) then
    begin
      MainForm.ListView9.Items.Item[bookmarknum-1].Subitems.Add(bookmark.title);
      MainForm.ListView9.Items.Item[bookmarknum-1].Subitems.Add(bookmark.adress);
    end else
    begin
      MainForm.ListView9.Items.Item[bookmarknum-1].Subitems.Add('');
    end;
    bookmarknum:=bookmarknum+1;
    if bookmarknum=15 then EndWAPBookmarkThread;
  end;
end;

procedure TMainForm.WapBookTabSheetHide(Sender: TObject);
begin
  EndWAPBookmarkThread;
end;

procedure TMainForm.WapBookTabSheetShow(Sender: TObject);
begin
  if GSMPhoneConnected then
  begin
    if GSM_GetModelFeature(FN_WAP)<>0 then
    begin
      GroupBox17.Ctl3D:=true;
      ListView9.Ctl3D:=true;
      ListView9.Enabled:=true;
      bookmarknum:=1;
      MainForm.ListView9.Items.Clear;
      MainForm.Timer2.Enabled:=false;
      WAPBookmarkThread:=TWAPBookmarkThread.Create(false);
    end else
    begin
      GroupBox17.Ctl3D:=false;
      ListView9.Ctl3D:=false;
      ListView9.Enabled:=false;
    end;
  end;
end;

procedure TMainForm.ComboBox7Change(Sender: TObject);
var
  settings:GSM_WAPSettings;
begin
  if combobox7.ItemIndex>0 then
  begin
    settings.location:=combobox7.ItemIndex;
    MainForm.Timer2.Enabled:=false;
    GSM_GetWAPSettings(@settings);
    MainForm.Timer2.Enabled:=true;

    if (settings.title[1]<>chr(0)) then
      Edit6.Text:=settings.title
    else
      edit6.Text:='set '+inttostr(combobox7.ItemIndex);

    edit8.Text:=settings.homepage;

    combobox10.itemindex:=1;
    if settings.iscontinuous<>0 then combobox10.Itemindex:=0;

    combobox9.itemindex:=1;
    if settings.issecurity<>0 then combobox9.Itemindex:=0;

    if (settings.bearer>=0) and (settings.bearer<3) then
    begin
      combobox8.ItemIndex:=settings.bearer;

      Label27.enabled:=false;
      Label28.enabled:=false;
      Label29.enabled:=false;
      label30.enabled:=false;
      label31.enabled:=false;
      label32.enabled:=false;
      label33.enabled:=false;
      label34.enabled:=false;
      label35.enabled:=false;
      label36.enabled:=false;
      label37.enabled:=false;
      edit9.enabled:=false;
      edit10.enabled:=false;
      edit11.enabled:=false;
      edit12.enabled:=false;
      edit13.enabled:=false;
      edit14.enabled:=false;
      edit15.enabled:=false;
      combobox11.enabled:=false;
      combobox12.enabled:=false;
      combobox13.enabled:=false;
      combobox14.enabled:=false;

      case settings.bearer of
        WAPSETTINGS_BEARER_SMS:
          begin
            label27.enabled:=true;
            edit9.text:=settings.server;
            edit9.enabled:=true;

            label28.enabled:=true;
            edit10.text:=settings.service;
            edit10.enabled:=true;
          end;
        WAPSETTINGS_BEARER_DATA:
          begin
            label29.enabled:=true;
            edit11.text:=settings.dialup;
            edit11.enabled:=true;

            label34.enabled:=true;
            edit13.text:=settings.user;
            edit13.enabled:=true;

            label35.enabled:=true;
            edit14.text:=settings.password;
            edit14.enabled:=true;

            label30.enabled:=true;
            edit12.text:=settings.ipaddress;
            edit12.enabled:=true;

            label31.enabled:=true;
            combobox11.enabled:=true;
            combobox11.ItemIndex:=0;
            if settings.isnormalauthentication=0 then
              combobox11.ItemIndex:=1;

            label32.enabled:=true;
            combobox12.enabled:=true;
            combobox12.ItemIndex:=0;
            if settings.isISDNcall=0 then
              combobox12.ItemIndex:=1;

            label33.enabled:=true;
            combobox13.enabled:=true;
            combobox13.ItemIndex:=1;
            if settings.isspeed14400=0 then
              combobox13.ItemIndex:=0;
          end;
        WAPSETTINGS_BEARER_USSD:
          begin
            label36.enabled:=true;
            edit15.text:=settings.code;
            edit15.enabled:=true;

            label37.enabled:=true;
            combobox14.Enabled:=true;
            combobox14.ItemIndex:=1;
            if settings.isIP=0 then
            begin
              combobox14.ItemIndex:=1;

              label28.enabled:=true;
              edit10.text:=settings.service;
              edit10.enabled:=true;
            end else
            begin
              label30.enabled:=true;
              edit12.text:=settings.ipaddress;
              edit12.enabled:=true;
            end;
          end;
      end;
    end;
  end;
end;

procedure TMainForm.Button19Click(Sender: TObject);
var
  Divert:GSM_CallDivert;
begin
  MainForm.Timer2.Enabled:=false;

  divert.DType:=GSM_CDV_Busy;
  divert.ctype:=GSM_CDV_VoiceCalls;
  divert.Operation:=GSM_CDV_Query;
  if gsm_calldivertoperation(@divert)=0 then
  begin
    if divert.enabled=0 then
    begin
      Label38.enabled:=false;
      edit16.enabled:=false;
      edit16.Text:='Not active';
    end else
    begin
      Label38.enabled:=true;
      edit16.enabled:=true;
      edit16.Text:=divert.Number;
    end;
  end;

  divert.DType:=GSM_CDV_NoAnswer;
  divert.ctype:=GSM_CDV_VoiceCalls;
  divert.Operation:=GSM_CDV_Query;
  if gsm_calldivertoperation(@divert)=0 then
  begin
    if divert.enabled=0 then
    begin
      Label39.enabled:=false;
      edit17.enabled:=false;
      edit17.Text:='Not active';
      label40.Enabled:=false;
      combobox15.Enabled:=false;
    end else
    begin
      Label39.enabled:=true;
      edit17.enabled:=true;
      edit17.Text:=divert.Number;
      label40.Enabled:=true;
      combobox15.Enabled:=true;
      case divert.timeout of
         5:combobox15.ItemIndex:=0;
        10:combobox15.ItemIndex:=1;
        15:combobox15.ItemIndex:=2;
        20:combobox15.ItemIndex:=3;
        25:combobox15.ItemIndex:=4;
        30:combobox15.ItemIndex:=5;
      end;
    end;
  end;

  divert.DType:=GSM_CDV_OutOfreach;
  divert.ctype:=GSM_CDV_VoiceCalls;
  divert.Operation:=GSM_CDV_Query;
  if gsm_calldivertoperation(@divert)=0 then
  begin
    if divert.enabled=0 then
    begin
      Label41.enabled:=false;
      edit18.enabled:=false;
      edit18.Text:='Not active';
    end else
    begin
      Label41.enabled:=true;
      edit18.enabled:=true;
      edit18.Text:=divert.Number;
    end;
  end;

  MainForm.Timer2.Enabled:=true;
end;

procedure TMainForm.Button20Click(Sender: TObject);
var
  Divert:GSM_CallDivert;
begin
  MainForm.Timer2.Enabled:=false;

  divert.DType:=GSM_CDV_AllTypes;
  divert.ctype:=GSM_CDV_AllCalls;
  divert.Operation:=GSM_CDV_Erasure;
  gsm_calldivertoperation(@divert);

  MainForm.Timer2.Enabled:=true;

end;

procedure ReadCalendarOffline;
var F:file of GSM_calendarnote;
begin
  MainForm.ListView11.items.Clear;
  AssignFile(F, 'gnokiicalendar');
  Reset(F);
  if (ioresult=0) then
  begin
    calendarofflinenumber:=0;
    while not eof(f) do
    begin
      calendarofflinenumber:=calendarofflinenumber+1;
      read(f,calendaroffline[calendarofflinenumber]);
      AddCalendarNote(calendaroffline[calendarofflinenumber],mainform.ListView11);
    end;
    CloseFile(F);
  end;
end;

procedure TMainForm.PCOfflineTabSheetShow(Sender: TObject);
begin
  ReadSMSOffline;
  Label43.Caption:=inttostr(smsofflinenumber);
  ReadCalendarOffline;
  Label45.Caption:=inttostr(calendarofflinenumber);
end;

procedure SaveCalendarOffline;
var F:file of GSM_CalendarNote;
    i:integer;
begin
  AssignFile(F, 'gnokiicalendar');
  Rewrite(F);
  for i:=1 to CalendarOfflineNumber do
  begin
    Write(f,CalendarOffline[i]);
  end;
  CloseFile(F);
end;

procedure TMainForm.PCCalendarTabSheetShow(Sender: TObject);
begin
  ReadCalendarOffline;
end;

procedure TMainForm.CopytoPC2Click(Sender: TObject);
begin
  setcalendarnote.Location:=calendarselect;
  mainform.Timer2.Enabled:=false;
  GSM_GetCalendarNote(@setcalendarnote);
  mainform.Timer2.Enabled:=true;
  Calendarofflinenumber:=calendarofflinenumber+1;
  Calendaroffline[Calendarofflinenumber]:=setcalendarnote;
  SaveCalendarOffline;
end;

procedure TMainForm.Delete5Click(Sender: TObject);
var
  i:integer;
begin
  if Application.MessageBox('Do you want to delete this note ?','Question',MB_YESNO)=IDYES then
  begin
    for i:=MainForm.ListView11.ItemIndex to calendarofflinenumber-1 do
    begin
      CalendarOffline[i]:=CalendarOffline[i+1];
    end;
    Calendarofflinenumber:=calendarofflinenumber-1;
    SaveCalendarOffline;
    ReadCalendarOffline;
  end;
end;

procedure TMainForm.Copytocalendar1Click(Sender: TObject);
begin
  if not readcalendar then
  begin
    application.MessageBox('Please read first calendar notes from phone','info',mb_ok);
  end else begin
    setcalendarnote:=calendaroffline[mainform.listview11.itemindex+1];
    setcalendarnote.Location:=MainForm.ListView4.Items.Count+1;

    EditCalendarNote;
  end;
end;

procedure TMainForm.PCCalendarPopupMenuPopup(Sender: TObject);
begin
  CopyTocalendar1.Enabled:=False;
  if (GSMPhoneConnected) and (GSM_GetModelFeature(FN_Calendar)<>0)
                         and (listview11.selected<>nil) then copytocalendar1.Enabled:=True;

  Delete5.Enabled:=false;
  if listview11.selected<>nil then Delete5.Enabled:=True;
end;

procedure TMainForm.Savebackuptofile1Click(Sender: TObject);
begin
  PhoneBackup.PhonePhonebookUsed:=0;
  PhoneBackup.SIMPhonebookUsed:=0;
  PhoneBackup.CallerAvailable:=0;
  PhoneBackup.StartupLogoAvailable:=0;
  PhoneBackup.StartupText.text[1]:=chr(0);
  PhoneBackup.OperatorLogoAvailable:=0;
  PhoneBackup.SpeedAvailable:=0;

  BackupForm.CheckBox3.Enabled:=true;
  BackupForm.CheckBox4.Enabled:=true;
  BackupForm.CheckBox2.Enabled:=true;
  BackupForm.CheckBox1.Enabled:=true;
  BackupForm.CheckBox5.Enabled:=true;

  BackupForm.GroupBox1.Caption:=' What do you want to backup ? ';
  PhoneBackupAvailable:=True;
  BackupForm.ListBox1.ItemIndex:=0;
  BackupForm.GroupBox2.Visible:=true;
  BackupForm.ShowModal;
  if PhoneBackupAvailable then
  begin
    BackupProgressForm.ShowModal;
  end;
end;

procedure TMainForm.Restorebackupfromfile1Click(Sender: TObject);
begin
  if OpenDialog1.Execute then
  begin
    if GSM_ReadBackupFile(PChar(OpenDialog1.FileName),@phonebackup)=0 then
    begin
      BackupForm.CheckBox3.Enabled:=true;
      if PhoneBackup.CallerAvailable=0 then
        BackupForm.CheckBox3.Enabled:=false;

      BackupForm.CheckBox4.Enabled:=true;
      if PhoneBackup.OperatorLogoAvailable=0 then
        BackupForm.CheckBox4.Enabled:=false;

      BackupForm.CheckBox2.Enabled:=true;
      if PhoneBackup.PhonePhonebookUsed=0 then
        BackupForm.CheckBox2.Enabled:=false;

      BackupForm.CheckBox1.Enabled:=true;
      if PhoneBackup.SIMPhonebookUsed=0 then
        BackupForm.CheckBox1.Enabled:=false;

      BackupForm.CheckBox5.Enabled:=true;

      BackupForm.GroupBox1.Caption:=' What do you want to restore ? ';
      PhoneBackupAvailable:=true;
      BackupForm.GroupBox2.Visible:=false;
      BackupForm.ShowModal;
      if PhoneBackupAvailable then
      begin
        PhoneBackupAvailable:=false;
        BackupProgressForm.ShowModal;
      end;
    end;
  end;
end;

procedure TMainForm.PBKPopupMenuPopup(Sender: TObject);
begin
  Savephonebooktofileanotherapplication1.enabled:=false;
  Delete4.enabled:=false;
  Edit19.Enabled:=false;
  Edit7.Enabled:=false;
  SMSSendSave1.Enabled:=false;
  copyphonebooktopc1.Enabled:=false;
  if GSMPhoneConnected then
  begin
    if (pbkofflinenumber<>0) then copyphonebooktopc1.Enabled:=true;
    if (ComboBox1.ItemIndex=4) or (ComboBox1.ItemIndex=5) then
    begin
      Savephonebooktofileanotherapplication1.enabled:=true;
      if listview1.Selected<>nil then
      begin
        Delete4.enabled:=true;
        Edit19.Enabled:=true;
      end;
    end;
    if listview1.Selected<>nil then
    begin
      SMSSendSave1.Enabled:=true;
      Edit7.Enabled:=true;
    end;
  end;
end;

procedure TMainForm.Savephonebooktofileanotherapplication1Click(
  Sender: TObject);
begin
  PhoneBackup.CallerAvailable:=0;
  PhoneBackup.StartupLogoAvailable:=0;
  PhoneBackup.StartupText.text[1]:=chr(0);
  PhoneBackup.OperatorLogoAvailable:=0;

  BackupForm.CheckBox3.Enabled:=false;
  BackupForm.CheckBox4.Enabled:=false;

  BackupForm.CheckBox2.Enabled:=true;
  if PhoneBackup.PhonePhonebookUsed=0 then
    BackupForm.CheckBox2.Enabled:=false;

  BackupForm.CheckBox1.Enabled:=true;
  if PhoneBackup.SIMPhonebookUsed=0 then
    BackupForm.CheckBox1.Enabled:=false;

  BackupForm.CheckBox5.Enabled:=false;

  BackupForm.GroupBox1.Caption:=' What do you want to backup ? ';
  PhoneBackupAvailable:=True;
  BackupForm.ListBox1.ItemIndex:=0;
  BackupForm.GroupBox2.Visible:=true;
  BackupForm.ShowModal;
  if PhoneBackupAvailable then
  begin
    if MainForm.SaveDialog1.Execute then
    begin
      GSM_SaveBackupFile(PChar(MainForm.SaveDialog1.FileName),@phonebackup);
    end;
  end;
end;

procedure TMainForm.ListView4DblClick(Sender: TObject);
begin
//  if (GSMPhoneConnected) and (ListView4.Selected<>nil) then
//    Edit3Click(Sender);
end;

procedure TMainForm.Edit19Click(Sender: TObject);
var
  i,j:integer;
  num_num,text_num:integer;
begin
  if listview1.Selected<>nil then
  begin
    PBKEditForm.ComboBox1.ItemIndex:=1;
    PBKEditForm.COmboBox2.Items:=PBKEditForm.COmboBox1.Items;
    PBKEditForm.ComboBox2.ItemIndex:=1;
    PBKEditForm.COmboBox3.Items:=PBKEditForm.COmboBox1.Items;
    PBKEditForm.ComboBox3.ItemIndex:=1;
    PBKEditForm.COmboBox4.Items:=PBKEditForm.COmboBox1.Items;
    PBKEditForm.ComboBox4.ItemIndex:=1;
    PBKEditForm.COmboBox5.Items:=PBKEditForm.COmboBox1.Items;
    PBKEditForm.ComboBox5.ItemIndex:=1;
    PBKEditForm.ComboBox6.ItemIndex:=0;
    PBKEditForm.COmboBox7.Items:=PBKEditForm.COmboBox6.Items;
    PBKEditForm.ComboBox7.ItemIndex:=0;
    PbkEditForm.Edit2.Text:='';

    num_num:=1;
    text_num:=1;
    pbk.Location:=pbknumberentry;
    case ComboBox1.ItemIndex of
      4:pbk.MemoryType:=GMT_ME;
      5:pbk.MemoryType:=GMT_SM;
    end;
    GSM_GetMemoryLocation(@pbk);

    pbkeditform.ComboBox1.enabled:=false;
    pbkeditform.ComboBox2.enabled:=false;
    pbkeditform.ComboBox3.enabled:=false;
    pbkeditform.ComboBox4.enabled:=false;
    pbkeditform.ComboBox5.enabled:=false;
    pbkeditform.ComboBox6.enabled:=false;
    pbkeditform.ComboBox7.enabled:=false;
    pbkeditform.Edit3.enabled:=false;
    pbkeditform.Edit4.enabled:=false;
    pbkeditform.Edit5.enabled:=false;
    pbkeditform.Edit6.enabled:=false;
    pbkeditform.Edit7.enabled:=false;
    pbkeditform.Edit8.enabled:=false;
    PbkEditForm.Edit3.Text:='Not available';
    PbkEditForm.Edit4.Text:='Not available';
    PbkEditForm.Edit5.Text:='Not available';
    PbkEditForm.Edit6.Text:='Not available';
    PbkEditForm.Edit7.Text:='Not available';
    PbkEditForm.Edit8.Text:='Not available';
    if (pbk.MemoryType=GMT_ME) and
       (GSM_GetModelFeature(FN_PHONEBOOK)=F_PBK71INT) then
    begin
      pbkeditform.ComboBox1.enabled:=true;
      pbkeditform.ComboBox2.enabled:=true;
      pbkeditform.ComboBox3.enabled:=true;
      pbkeditform.ComboBox4.enabled:=true;
      pbkeditform.ComboBox5.enabled:=true;
      pbkeditform.ComboBox6.enabled:=true;
      pbkeditform.ComboBox7.enabled:=true;
      pbkeditform.Edit3.enabled:=true;
      pbkeditform.Edit4.enabled:=true;
      pbkeditform.Edit5.enabled:=true;
      pbkeditform.Edit6.enabled:=true;
      pbkeditform.Edit7.enabled:=true;
      pbkeditform.Edit8.enabled:=true;
      PbkEditForm.Edit3.Text:='';
      PbkEditForm.Edit4.Text:='';
      PbkEditForm.Edit5.Text:='';
      PbkEditForm.Edit6.Text:='';
      PbkEditForm.Edit7.Text:='';
      PbkEditForm.Edit8.Text:='';
    end;

    pbkeditform.Label2.Enabled:=true;
    pbkeditform.ComboBox8.Enabled:=true;
    if (GSM_GetModelFeature(FN_CALLERGROUPS)=0) then
    begin
      pbkeditform.Label2.Enabled:=false;
      pbkeditform.ComboBox8.Enabled:=false;
    end;

    pbkeditform.Edit1.Text:=pbk.name;
    if (pbk.Number[1]<>chr(0)) then
    begin
      pbkeditform.ComboBox1.ItemIndex:=1;
      pbkeditform.Edit2.Text:=pbk.number;
      num_num:=num_num+1;
    end;
    for i:=1 to pbk.SubEntriesCount do
    begin
      case pbk.SubEntries[i].EntryType of
        GSM_Number:
          begin
            j:=1;
            case pbk.SubEntries[i].NumberType of
              GSM_General :j:=1;
              GSM_General2:j:=1;
              GSM_Mobile  :j:=3;
              GSM_Work    :j:=4;
              GSM_Fax     :j:=0;
              GSM_Home    :j:=2;
            end;
            case num_num of
              1:begin
                  pbkeditform.ComboBox1.ItemIndex:=j;
                  pbkeditform.edit2.text:=pbk.SubEntries[i].Number;
                end;
              2:begin
                  pbkeditform.ComboBox2.ItemIndex:=j;
                  pbkeditform.edit3.text:=pbk.SubEntries[i].Number;
                end;
              3:begin
                  pbkeditform.ComboBox3.ItemIndex:=j;
                  pbkeditform.edit4.text:=pbk.SubEntries[i].Number;
                end;
              4:begin
                  pbkeditform.ComboBox4.ItemIndex:=j;
                  pbkeditform.edit5.text:=pbk.SubEntries[i].Number;
                end;
              5:begin
                  pbkeditform.ComboBox5.ItemIndex:=j;
                  pbkeditform.edit6.text:=pbk.SubEntries[i].Number;
                end;
            end;
            num_num:=num_num+1;
          end;
        GSM_Note  :
          begin
            j:=1;
            case text_num of
              1:begin
                  pbkeditform.ComboBox6.ItemIndex:=j;
                  pbkeditform.edit7.text:=pbk.SubEntries[i].Number;
                end;
              2:begin
                  pbkeditform.ComboBox7.ItemIndex:=j;
                  pbkeditform.edit8.text:=pbk.SubEntries[i].Number;
                end;
            end;
            text_num:=text_num+1;
          end;
        GSM_Postal:
          begin
            j:=2;
            case text_num of
              1:begin
                  pbkeditform.ComboBox6.ItemIndex:=j;
                  pbkeditform.edit7.text:=pbk.SubEntries[i].Number;
                end;
              2:begin
                  pbkeditform.ComboBox7.ItemIndex:=j;
                  pbkeditform.edit8.text:=pbk.SubEntries[i].Number;
                end;
            end;
            text_num:=text_num+1;
          end;
        GSM_Email :
          begin
            j:=0;
            case text_num of
              1:begin
                  pbkeditform.ComboBox6.ItemIndex:=j;
                  pbkeditform.edit7.text:=pbk.SubEntries[i].Number;
                end;
              2:begin
                  pbkeditform.ComboBox7.ItemIndex:=j;
                  pbkeditform.edit8.text:=pbk.SubEntries[i].Number;
                end;
            end;
            text_num:=text_num+1;
          end;

      end;
    end;

    pbkeditform.ComboBox8.items.clear;
    pbkeditform.ComboBox8.items.add('Not assigned');
    for i:=1 to 5 do
    begin
      pbkeditform.ComboBox8.items.add(caller[i].text);
    end;
    pbkeditform.ComboBox8.itemindex:=0;
    if (GSM_GetModelFeature(FN_CALLERGROUPS)<>0) then
    begin
      if (pbk.Group>=0) and (pbk.Group<=4) then
      begin
        pbkeditform.ComboBox8.itemindex:=pbk.Group+1;
      end;
    end;
    
    PbkEditForm.ShowModal;
  end;

end;

procedure TMainForm.Delete4Click(Sender: TObject);
begin
  if Application.MessageBox('Do you want to delete this phonebook entry ?','Question',MB_YESNO)=IDYES then
  begin

    pbk.Location:=pbknumberentry;
    case ComboBox1.ItemIndex of
      4:pbk.MemoryType:=GMT_ME;
      5:pbk.MemoryType:=GMT_SM;
    end;

    MainForm.Timer2.Enabled:=false;
    GSM_GetMemoryLocation(@pbk);

    pbk.SubEntriesCount:=0;
    pbk.Name[1]:=chr(0);
    pbk.Number[1]:=chr(0);
    GSM_WritePhonebookLocation(@pbk);
    MainForm.Timer2.Enabled:=true;
    MainForm.ComboBox1Change(nil);
  end;
end;


procedure TMainForm.Button22Click(Sender: TObject);
var
  i:integer;
  mybinring:GSM_BinRingtone;
  myring:GSM_Ringtone;
begin
  LogosRingForm.Label1.Caption:='Do you want to get ringtone ?';
  LogosRingForm.ComboBox3.Visible:=true;
  LogosRingForm.ComboBox3.Items.Clear();
  MainForm.Timer2.Enabled:=false;
  for i:=1 to GSM_GetModelFeature(FN_RINGNUMBER) do
  begin
    mybinring.location:=i;
    if (GSM_GetPhoneRingtone(@mybinring,@myring)=0) then
    begin
      if GSM_GetModelFeature(FN_RINGTONES)=F_RING_SM then
      begin
        LogosRingForm.ComboBox3.Items.Add(myring.name);
      end else
      begin
        LogosRingForm.ComboBox3.Items.Add(mybinring.name);
      end;
    end else
      LogosRingForm.ComboBox3.Items.Add('Location '+inttostr(i));
  end;
  MainForm.Timer2.Enabled:=true;
  LogosRingForm.ComboBox3.ItemIndex:=0;
  LogosRingForm.ShowModal;
  if LogosRingForm.ComboBox3.Visible then
  begin
    mybinring.location:=LogosRingForm.ComboBox3.ItemIndex+1;
    MainForm.Timer2.Enabled:=false;
    if (GSM_GetPhoneRingtone(@mybinring,@myring)=0) then
    begin
      MainForm.Timer2.Enabled:=true;
      if GSM_GetModelFeature(FN_RINGTONES)=F_RING_SM then
      begin
        application.MessageBox('Smart Messaging format','',0);
        if SaveDialog1.Execute then
        begin
          GSM_SaveRingtoneFile(PChar(SaveDialog1.FileName),@myring);
        end;
      end else
      begin
        application.MessageBox('Binary format','',0);
        if SaveDialog1.Execute then
        begin
          GSM_SaveBinRingtoneFile(PChar(SaveDialog1.FileName),@mybinring);
        end;
      end;
    end;

    MainForm.Timer2.Enabled:=true;
  end;

end;

procedure TMainForm.New20Click(Sender: TObject);
begin
  bookmarknum:=65535;
  WAPBookForm.Edit1.Text:='';
  WAPBookForm.Edit2.Text:='';
  WAPBookForm.ShowModal;
end;

procedure TMainForm.ListView9SelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
begin
  if Selected then bookmarknum2:=item.Index+1;
end;

procedure TMainForm.Edit20Click(Sender: TObject);
var
  bookmark:GSM_WAPBookmark;
begin
  bookmark.location:=bookmarknum2;
  mainform.Timer2.Enabled:=false;
  GSM_GetWAPBookmark(@bookmark);
  mainform.Timer2.Enabled:=true;
  WAPBookForm.Edit1.Text:=bookmark.title;
  WAPBookForm.Edit2.Text:=bookmark.adress;
  WAPBookForm.ShowModal;
end;

procedure TMainForm.WAPBookPopupMenuPopup(Sender: TObject);
begin
  New20.Enabled:=false;
  Edit20.Enabled:=false;
  SMSSendSave2.Enabled:=false;
  if (WAPBookmarkThread=nil) and (GSMPhoneConnected) then
  begin
    New20.Enabled:=true;
    if (ListView9.Selected<>nil) and
       (ListView9.Selected.SubItems[0]<>'') then
    begin
      Edit20.Enabled:=true;
      SMSSendSave2.Enabled:=true;
    end;
  end;
end;

procedure TMainForm.SMSSendSave2Click(Sender: TObject);
var
  i:integer;
begin
  if smscinit=false then
  begin
    MainForm.ListView6.Items.Clear;
    smscnum:=1;
    while true do
    begin
      ReadSMSC;
      if smscinit then break;
    end;
  end;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;
  
  //folders
  GSM_GetSMSFolders(@SMSfolders);
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=1 to SMSFolders.number do
  begin
    SMSEditForm.ComboBox1.Items.Add(SMSfolders.folder[i].Name);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=0;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.WAPBookTabSheet;

  bookma.location:=bookmarknum2;
  mainform.Timer2.Enabled:=false;
  GSM_GetWAPBookmark(@bookma);
  mainform.Timer2.Enabled:=true;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.Button23Click(Sender: TObject);
var
  i:integer;
begin
  if smscinit=false then
  begin
    MainForm.ListView6.Items.Clear;
    smscnum:=1;
    while true do
    begin
      ReadSMSC;
      if smscinit then break;
    end;
  end;

  //sms text
  SMSEditForm.Memo1.Text:='';

  //number
  SMSEditForm.ComboBox9.Text:='';
  AddSMSEditSenderList;
  
  //folders
  GSM_GetSMSFolders(@SMSfolders);
  SMSEditForm.ComboBox1.Items.Clear;
  for i:=1 to SMSFolders.number do
  begin
    SMSEditForm.ComboBox1.Items.Add(SMSfolders.folder[i].Name);
  end;
  SMSEditForm.ComboBox1.ItemIndex:=0;

  //smsc
  SetSMSC(1);
  SMSEditForm.ComboBox2.Items.Clear;
  SMSEditForm.ComboBox2.Items.Add('custom');
  for i:=1 to smscnum-1 do
  begin
    if SMSC2[i].Name[1]<>chr(0) then
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i)+' - '+SMSC2[i].Name)
    else
      SMSEditForm.ComboBox2.Items.Add('set '+inttostr(i));
  end;
  SMSEditForm.ComboBox2.ItemIndex:=1;

  SMSEditForm.TextSMSTabSheet.TabVisible:=false;
  SMSEditForm.BitmapSMSTabSheet.TabVisible:=false;
  SMSEditForm.RingtoneSMSTabSheet.TabVisible:=false;
  SMSEditForm.CalendarSMSTabSheet.TabVisible:=false;
  SMSEditForm.WAPBookTabSheet.TabVisible:=false;
  SMSEditForm.WAPSettTabSheet.TabVisible:=false;
  SMSEditForm.PBKSMSTabSheet.TabVisible:=false;
  SMSEditForm.SMSEditPageControl.ActivePage:=Smseditform.WAPSettTabSheet;

  Wapset.location:=combobox7.ItemIndex;
  MainForm.Timer2.Enabled:=false;
  GSM_GetWAPSettings(@wapset);
  MainForm.Timer2.Enabled:=true;

  if (GSM_GetModelFeature(FN_SMS)=F_SMS71) then
  begin
    SMSEditForm.edit3.Visible:=true;
    SMSEditForm.label14.Visible:=true;
  end else
  begin
    SMSEditForm.edit3.Visible:=false;
    SMSEditForm.label14.Visible:=false;
  end;

  SMSEditForm.ShowModal;
end;

procedure TMainForm.CopyphonebooktoPC1Click(Sender: TObject);
begin
  SavePBKOffline;
end;

procedure TMainForm.PCPBKTabSheetShow(Sender: TObject);
var i:integer;
begin
  ReadPBKOffline;
  ListView12.Items.Clear;
  for i:=1 to PBKOfflineNumber do
  begin
    MainForm.ListView12.Items.Add;
    MainForm.ListView12.Items.Item[i-1].Caption:=inttostr(PhoneBackup.PhonePhonebook[i].Location);
    AddPBKEntry(ListView12,PhoneBackup.PhonePhonebook[i],i-1);
  end;
end;

procedure TMainForm.FormShow(Sender: TObject);
var
  MyIniFile: TIniFile;
  x:string;
begin
  MyIniFile := TIniFile.Create(ExtractFilePath(Application.ExeName)+'\gnokiirc');

  x:=MyInifile.ReadString('win32gui', 'x', '100');
  MainForm.Left:=strtoint(x);

  x:=MyInifile.ReadString('win32gui', 'y', '100');
  MainForm.Top:=strtoint(x);

  x:=MyInifile.ReadString('win32gui', 'treewidth', '93');
  MainForm.TreeView1.Width:=strtoint(x);

  x:=MyInifile.ReadString('win32gui', 'width', '655');
  MainForm.Width:=strtoint(x);

  x:=MyInifile.ReadString('win32gui', 'height', '431');
  MainForm.Height:=strtoint(x);

  MyIniFile.Free;

end;

procedure TMainForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  SaveMainConfig;
end;

end.
