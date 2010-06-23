/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for test utility.

*/

/* Prototypes */

int monitormode(int argc, char *argv[]);

#ifdef SECURITY
  int entersecuritycode(char *type);
  int getsecuritycodestatus(void);
  int getsecuritycode(char *type);
#endif

#ifdef DEBUG
  int sniff(int nargc, char *nargv[]);
  int decodefile(int nargc, char *nargv[]); 
  int getbinringfromfile(int nargc, char *nargv[]); 
#endif

int getmemory(int argc, char *argv[]);
int writephonebook(int argc, char *argv[]);
int getspeeddial(char *number);
int setspeeddial(char *argv[]);
int getsms(int argc, char *argv[]);
int renamesmsc(int argc, char *argv[]);
int deletesms(int argc, char *argv[]);
int sendsms(int argc, char *argv[]);
int savesms(int argc, char *argv[]);
int sendlogo(int argc, char *argv[]);
int savelogo(int argc, char *argv[]);
int sendringtone(int argc, char *argv[]);
int saveringtone(int argc, char *argv[]);
int getsmsc(char *mcn);
int setdatetime(int argc, char *argv[]);
int getdatetime(void);
int setalarm(char *argv[]);
int getalarm(void);
int dialvoice(char *number);
int cancelcall(void);
int getcalendarnote(int argc, char *argv[]);
int writecalendarnote(char *argv[]);
int deletecalendarnote(char *index);
int getdisplaystatus();
int netmonitor(char *_mode);
int identify(void);
int senddtmf(char *String);
int foogle(char *argv[]);
int pmon(void);
int setlogo(int argc, char *argv[]);
int getlogo(int argc, char *argv[]);
int binringtoneconvert(int argc, char *argv[]);
int setringtone(int argc, char *argv[]);
int getringtone(int argc, char *argv[]);
int reset(int argc, char *argv[]);
int getprofile(int argc, char *argv[]);
int setprofile(int argc, char *argv[]);
int allringtones();
int displayoutput();
int presskeysequence(char *argv[]);
int restoresettings(char *argv[]);
int backupsettings(char *argv[]);
int playringtone(int argc, char *argv[]);
int ringtoneconvert(int argc, char *argv[]);
int bitmapconvert(int argc, char *argv[]);
int composer(int argc, char *argv[]);
int getphoneprofile();
int setphoneprofile(int argc, char *argv[]);
int getoperatorname();
int setoperatorname(int argc, char *argv[]);
int getvoicemailbox();
int phonetests();
int simlockinfo();
int geteeprom ();
int setsimlock ();
int getsmsfolders ();
int resetphonesettings ();
int sendprofile(int argc, char *argv[]);
int showbitmap(int argc, char *argv[]);
int nm_collect(int argc, char *argv[]);
int netmonitordata(int argc, char *argv[]);
int getwapbookmark(int argc, char *argv[]);
int setwapbookmark(int argc, char *argv[]);
int savewapbookmark(int argc, char *argv[]);
int sendwapbookmark(int argc, char *argv[]);
int getwapsettings(int argc, char *argv[]);
int savewapsettings(int argc, char *argv[]);
int sendwapsettings(int argc, char *argv[]);
int savecalendarnote(int argc, char *argv[]);
int sendcalendarnote(int argc, char *argv[]);
int receivesms(int argc, char *argv[]);
int getsmsstatus(int argc, char *argv[]);
int divert(int argc, char *argv[]);
int savephonebookentry(int argc, char *argv[]);
int sendphonebookentry(int argc, char *argv[]);
GSM_Error sendsmsfromfile(char * outbox, char * logname);

typedef enum {
  OPT_HELP,
  OPT_VERSION,
  OPT_MONITOR,
  OPT_ENTERSECURITYCODE,
  OPT_GETSECURITYCODESTATUS,
  OPT_GETSECURITYCODE,
  OPT_SETDATETIME,
  OPT_GETDATETIME,
  OPT_SETALARM,
  OPT_GETALARM,
  OPT_DIALVOICE,
  OPT_CANCELCALL,
  OPT_GETCALENDARNOTE,
  OPT_WRITECALENDARNOTE,
  OPT_DELCALENDARNOTE,
  OPT_GETDISPLAYSTATUS,
  OPT_GETMEMORY,
  OPT_WRITEPHONEBOOK,
  OPT_GETSPEEDDIAL,
  OPT_SETSPEEDDIAL,
  OPT_GETSMS,
  OPT_DELETESMS,
  OPT_SENDSMS,
  OPT_SAVESMS,
  OPT_GETSMSC,
  OPT_RENAMESMSC,
  OPT_PMON,
  OPT_NETMONITOR,
  OPT_NM_COLLECT,
  OPT_NETMONITORDATA,
  OPT_IDENTIFY,
  OPT_SENDDTMF,
  OPT_RESET,
  OPT_SETLOGO,
  OPT_GETLOGO,
  OPT_SENDLOGO,
  OPT_SAVELOGO,
  OPT_BITMAPCONVERT,
  OPT_SENDRINGTONE,
  OPT_SAVERINGTONE,
  OPT_SETRINGTONE,
  OPT_PLAYRINGTONE,
  OPT_RINGTONECONVERT,
  OPT_COMPOSER,
  OPT_ALLRINGTONES,
  OPT_GETRINGTONE,
  OPT_SNIFFER,
  OPT_GETPROFILE,
  OPT_SETPROFILE,
  OPT_DISPLAYOUTPUT,
  OPT_PRESSKEYSEQUENCE,
  OPT_RESTORESETTINGS,
  OPT_BACKUPSETTINGS,
  OPT_GETPHONEPROFILE,
  OPT_SETPHONEPROFILE,
  OPT_GETOPERATORNAME,
  OPT_SETOPERATORNAME,
  OPT_GETVOICEMAILBOX,
  OPT_PHONETESTS,
  OPT_FOOGLE,
  OPT_SIMLOCKINFO,
  OPT_SETSIMLOCK,
  OPT_GETEEPROM=128,
  OPT_GETSMSFOLDERS=129,
  OPT_RESETPHONESETTINGS=130,
  OPT_SENDPROFILE=131,
  OPT_SHOWBITMAP=132,
  OPT_BINRINGTONECONVERT=133,
  OPT_GETWAPBOOKMARK=134,
  OPT_SAVEWAPBOOKMARK=135,
  OPT_SENDWAPBOOKMARK=136,
  OPT_GETWAPSETTINGS=137,
  OPT_SAVEWAPSETTINGS=138,
  OPT_SAVECALENDARNOTE=139,
  OPT_SENDCALENDARNOTE=140,
  OPT_SAVEPHONEBOOKENTRY=141,
  OPT_SENDPHONEBOOKENTRY=142,
  OPT_DECODEFILE=143,
  OPT_RECEIVESMS=144,
  OPT_GETSMSSTATUS=145,
  OPT_DIVERT=146,
  OPT_SENDWAPSETTINGS=147,
  OPT_SETWAPBOOKMARK=148,
  OPT_GETBINRINGFROMFILE=149
} opt_index;

struct gnokii_arg_len {
  int gal_opt;
  int gal_min;
  int gal_max;
  int gal_flags;
};

/* This is used for checking correct argument count. If it is used then if
   the user specifies some argument, their count should be equivalent to the
   count the programmer expects. */

#define GAL_XOR 0x01

/* for --presskeysequence */
struct keys_table_position {
  char whatchar;
  int whatcode;
};

/* for --presskeysequence */
struct keys_table_position Keys[] = {
{'m',PHONEKEY_MENU},
{'M',PHONEKEY_MENU},
{'n',PHONEKEY_NAMES},
{'N',PHONEKEY_NAMES},
{'p',PHONEKEY_POWER},
{'P',PHONEKEY_POWER},
{'u',PHONEKEY_UP},
{'U',PHONEKEY_UP},
{'d',PHONEKEY_DOWN},
{'D',PHONEKEY_DOWN},
{'+',PHONEKEY_INCREASEVOLUME},
{'-',PHONEKEY_DECREASEVOLUME},
{'1',PHONEKEY_1},
{'2',PHONEKEY_2},
{'3',PHONEKEY_3},
{'4',PHONEKEY_4},
{'5',PHONEKEY_5},
{'6',PHONEKEY_6},
{'7',PHONEKEY_7},
{'8',PHONEKEY_8},
{'9',PHONEKEY_9},
{'*',PHONEKEY_ASTERISK},
{'0',PHONEKEY_0},
{'#',PHONEKEY_HASH},
{'g',PHONEKEY_GREEN},
{'G',PHONEKEY_GREEN},
{'r',PHONEKEY_RED},
{'R',PHONEKEY_RED},
{' ',0}
};

/* for NETMONITOR */

//maximal number of screens for --nm_collect
#define MAX_NM_COLLECT 20

/* 255 used internally by functions */
#define NM_MAX_SCREEN 254

/* in 6150 v4.13 effectively used was 296 FIELDS + date + time ... */
#define NM_MAX_FIELDS 512 	

typedef struct {
	char *name;
	int x;
	int y;
	int len;
	char *mname;
	int s_nr;
	char type;
	char value[NM_MAX_SCREEN_WIDTH];
	void *next;
} PARAM_INFO_MON;

// info for make ouput, filled in by parse_process()
typedef struct {
	int req_header;
	unsigned long req_wait;
	int req_n;
	char req_fs[11];
	char req_ls[11];
	PARAM_INFO_MON *data[NM_MAX_FIELDS];
	char out_f[NM_MAX_FIELDS];
	unsigned char req_screen[NM_MAX_SCREEN];
} OUT_INFO_MON;

int semicolon_pipe_substitution( GSM_PhonebookEntry *pentry, unsigned int direction );
int str_substch( char *str, const char toric, const char sost );
