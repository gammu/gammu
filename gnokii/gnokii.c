/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Mainline code for gnokii utility.  Handles command line parsing and
  reading/writing phonebook entries and other stuff.

*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#ifndef VC6
  #include <dirent.h>
  #if defined(__svr4__) || defined(__FreeBSD__)
  #  include <strings.h>	/* for bzero */
  #endif
#else
  /* for VC6 make scripts save VERSION constant in mversion.h file */
  #include "mversion.h"
#endif

#ifdef WIN32

  #include <windows.h>

  #include "misc_win32.h"
  #include "getopt.h"

  #define DEV_CONSOLE "CON:"

#else

  #include <unistd.h>
  #include <termios.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/time.h>
  #include <getopt.h>

  #define DEV_CONSOLE "/dev/tty"
#endif

#include "misc.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "gsm-networks.h"
#include "gsm-ringtones.h"
#include "gsm-bitmaps.h"
#include "gsm-wap.h"
#include "gsm-sms.h"
#include "gsm-datetime.h"
#include "gsm-phonebook.h"
#include "gsm-calendar.h"
#include "gsm-coding.h"
#include "newmodules/n6110.h"
#include "files/cfgreader.h"
#include "files/gsm-filetypes.h"
#include "gnokii.h"

#ifdef USE_NLS
  #include <locale.h>
#endif

char *model;           /* Model from .gnokiirc file. */
char *Port;            /* Port from .gnokiirc file */
char *Initlength;      /* Init length from .gnokiirc file */
char *Connection;      /* Connection type from .gnokiirc file */
char *SynchronizeTime; /* If we set date and time from computer to phone (from .gnokiirc file) */
char *BinDir;          /* Binaries directory from .gnokiirc file - not used here yet */

GSM_SMSMessage SMS[4];
  
char *GetProfileCallAlertString(int code) {

  switch (code) {
    case PROFILE_CALLALERT_RINGING     : return "Ringing";
    case PROFILE_CALLALERT_ASCENDING   : return "Ascending";
    case PROFILE_CALLALERT_RINGONCE    : return "Ring once";
    case PROFILE_CALLALERT_BEEPONCE    : return "Beep once";
    case PROFILE_CALLALERT_CALLERGROUPS: return "Caller groups";
    case PROFILE_CALLALERT_OFF         : return "Off";
    default                            : return "Unknown";
  }
}

char *GetProfileVolumeString(int code) {

  switch (code) {
    case PROFILE_VOLUME_LEVEL1 : return "Level 1";
    case PROFILE_VOLUME_LEVEL2 : return "Level 2";
    case PROFILE_VOLUME_LEVEL3 : return "Level 3";
    case PROFILE_VOLUME_LEVEL4 : return "Level 4";
    case PROFILE_VOLUME_LEVEL5 : return "Level 5";
    default                    : return "Unknown";
  }
}

char *GetProfileKeypadToneString(int code) {

  switch (code) {
    case PROFILE_KEYPAD_OFF    : return "Off";
    case PROFILE_KEYPAD_LEVEL1 : return "Level 1";
    case PROFILE_KEYPAD_LEVEL2 : return "Level 2";
    case PROFILE_KEYPAD_LEVEL3 : return "Level 3";
    default                    : return "Unknown";
  }
}

char *GetProfileMessageToneString(int code) {

  switch (code) {
    case PROFILE_MESSAGE_NOTONE    : return "No tone";
    case PROFILE_MESSAGE_STANDARD  : return "Standard";
    case PROFILE_MESSAGE_SPECIAL   : return "Special";
    case PROFILE_MESSAGE_BEEPONCE  : return "Beep once";
    case PROFILE_MESSAGE_ASCENDING : return "Ascending";
    default                        : return "Unknown";
  }
}

char *GetProfileWarningToneString(int code) {

  switch (code) {
    case PROFILE_WARNING_OFF : return "Off";
    case PROFILE_WARNING_ON  : return "On";
    default                  : return "Unknown";
  }
}

char *GetProfileOnOffString(int code) {

  switch (code) {
    case 0x00 : return "Off";
    case 0x01 : return "On";
    default   : return "Unknown";
  }
}

static char *GetProfileVibrationString(int code)
{
	switch (code) {
	case PROFILE_VIBRATION_OFF:	return "Off";
	case PROFILE_VIBRATION_ON:	return "On";
	case PROFILE_VIBRATION_FIRST:	return "Vibrate first";
	default:			return "Unknown";
	}
}

char BufferProfileGroups[90];

char *GetProfileCallerGroups(int code) 
{
  static char az_group_name[5][MAX_BITMAP_TEXT_LENGTH];
  static bool enteronce=false;
  register int i;
  GSM_Bitmap bitmap;

  if( code == PROFILE_CALLERGROUPS_ALL )
       return _("All calls alert");

  if( !enteronce ) {
    for(i=0;i<5;i++) az_group_name[i][0]='\0';
    enteronce=true;
  }

  BufferProfileGroups[0]=0;
    
  for(i=0;i<5;i++)
  {
    int code2test;
    char z_gtype[12];

    code2test=(i==0) ? 1 : 2<<(i-1);

    if( code & code2test )
    {
      if (!strcmp(az_group_name[i],"")) 
      {
        if (GetModelFeature (FN_CALLERGROUPS)!=0) {
          bitmap.type=GSM_CallerLogo;
          bitmap.number=i;
          strcpy(z_gtype,_("unknown"));
          if (GSM->GetBitmap(&bitmap)==GE_NONE)
            strcpy( az_group_name[i], bitmap.text );
        }
        if ((!strcmp(az_group_name[i],""))) {
            switch(i) {
            case 0:strcpy(az_group_name[i],_("Family"));break;
            case 1:strcpy(az_group_name[i],_("VIP"));break;
            case 2:strcpy(az_group_name[i],_("Friends"));break;
            case 3:strcpy(az_group_name[i],_("Colleagues"));break;
            case 4:strcpy(az_group_name[i],_("Other"));break;
            default:break;
          }
        }
      }
      strcpy(z_gtype,az_group_name[i]);

      if( strlen(BufferProfileGroups) ) strcat(BufferProfileGroups,"+");
      strcat(BufferProfileGroups, z_gtype);
    }
    
  }

  return BufferProfileGroups;
}

char *print_error(GSM_Error e)
{

//	case GE_DEVICEOPENFAILED:         return "Couldn't open specified serial device.";
//	case GE_UNKNOWNMODEL:             return "Model specified isn't known/supported.";
//	case GE_NOLINK:                   return "Couldn't establish link with phone.";
//	case GE_TRYAGAIN:                 return "Try again.";
//	case GE_INVALIDSMSLOCATION:       return "Invalid SMS location.";
//	case GE_INVALIDPHBOOKLOCATION:    return "Invalid phonebook location.";
//	case GE_INVALIDMEMORYTYPE:        return "Invalid type of memory.";
//	case GE_INVALIDSPEEDDIALLOCATION: return "Invalid speed dial location.";
//	case GE_INVALIDCALNOTELOCATION:   return "Invalid calendar note location.";
//	case GE_INVALIDDATETIME:          return "Invalid date, time or alarm specification.";
//	case GE_EMPTYSMSLOCATION:         return "SMS location is empty.";
//	case GE_PHBOOKNAMETOOLONG:        return "Phonebook name is too long.";
//	case GE_PHBOOKNUMBERTOOLONG:      return "Phonebook number is too long.";
//	case GE_PHBOOKWRITEFAILED:        return "Phonebook write failed.";
//	case GE_NONEWCBRECEIVED:          return "Attempt to read CB when no new CB received";
//	case GE_INTERNALERROR:            return "Problem occured internal to model specific code.";
//	case GE_NOTSUPPORTED:             return "Function not supported by the phone";
//	case GE_BUSY:                     return "Command is still being executed.";
//	case GE_USERCANCELED:             return "User has cancelled the action.";   
//	case GE_UNKNOWN:                  return "Unknown error - well better than nothing!!";
//	case GE_MEMORYFULL:               return "Memory is full";
//	case GE_LINEBUSY:                 return "Outgoing call requested reported line busy";
//	case GE_NOCARRIER:                return "No Carrier error during data call setup ?";

	switch (e) {
	case GE_NONE:                     return "No error, done OK";
	case GE_INVALIDSECURITYCODE:      return "Invalid Security code.";
	case GE_NOTIMPLEMENTED:           return "Called command is not implemented for the used model. Please contact marcin-wiacek@topnet.pl, if you want to help in implementing it";
	case GE_TIMEOUT:                  return "Command timed out.";
	case GE_CANTOPENFILE:             return "Can't open file with bitmap/ringtone";
	case GE_SUBFORMATNOTSUPPORTED:    return "Subformat of file not supported";
	case GE_WRONGNUMBEROFCOLORS:      return "Wrong number of colors in specified bitmap file (only 2 colors files supported)";
	case GE_WRONGCOLORS:              return "Wrong colors in bitmap file";
	case GE_INVALIDIMAGESIZE:         return "Invalid size of bitmap (in file, sms etc.)";
	case GE_INVALIDFILEFORMAT:        return "Invalid format of file";
        case GE_TOOSHORT:                 return "File too short";
        case GE_INSIDEBOOKMARKSMENU:      return "Inside WAP Bookmarks menu. Please leave it and try again";
        case GE_INVALIDBOOKMARKLOCATION:  return "Invalid or empty WAP bookmark location";
        case GE_INSIDESETTINGSMENU:       return "Inside WAP Settings menu. Please leave it and try again";
        case GE_INVALIDSETTINGSLOCATION:  return "Invalid or empty WAP settings location";
        case GE_EMPTYSMSC:                return "Empty SMSC number. Use --smsc";
	case GE_SMSSENDOK:                return "SMS was sent correctly.";
	case GE_SMSSENDFAILED:            return "SMS send fail.";
	case GE_SMSTOOLONG:               return "SMS message too long.";
	default:                          return "Unknown error.";
	}
}


GSM_Error GSM_ReadRingtoneFileOnConsole(char *FileName, GSM_Ringtone *ringtone)
{
  GSM_Error error;
  
  error=GSM_ReadRingtoneFile(FileName, ringtone);
  
  switch (error) {
    case GE_CANTOPENFILE:
    case GE_SUBFORMATNOTSUPPORTED:
      fprintf(stderr, _("File \"%s\"\nError: %s\n"),FileName,print_error(error));
      break;
    default:
      break;
  }
  
  return error;
}

GSM_Error GSM_SaveRingtoneFileOnConsole(char *FileName, GSM_Ringtone *ringtone)
{
  int confirm,confirm2;
  char ans[4];
  struct stat buf;
  GSM_Error error;

  /* Ask before overwriting */
  while (stat(FileName, &buf) == 0) {
  
    confirm=-1;
    confirm2=-1;
    
    while (confirm < 0) {
      fprintf(stderr, _("Saving ringtone. File \"%s\" exists. (O)verwrite, create (n)ew or (s)kip ? "),FileName);
      GetLine(stdin, ans, 4);
      if (!strcmp(ans, "O") || !strcmp(ans, "o")) confirm = 1;
      if (!strcmp(ans, "N") || !strcmp(ans, "n")) confirm = 2;
      if (!strcmp(ans, "S") || !strcmp(ans, "s")) return GE_USERCANCELED;
    }  
    if (confirm==1) break;
    if (confirm==2) {
      while (confirm2 < 0) {
        fprintf(stderr, _("Enter name of new file: "));
        GetLine(stdin, FileName, 50);
        if (&FileName[0]==0) return GE_USERCANCELED;
	confirm2=1;
      }  
    }
  }
  
  error=GSM_SaveRingtoneFile(FileName,ringtone);
  
  switch (error) {
    case GE_CANTOPENFILE:        fprintf(stderr, _("Failed to write file \"%s\"\n"),FileName);
                                 break;
    default:                     break;
  }
  
  return error;
}

GSM_Error GSM_ReadBitmapFileOnConsole(char *FileName, GSM_Bitmap *bitmap)
{
  GSM_Error error;
  
  error=GSM_ReadBitmapFile(FileName, bitmap);

  switch (error) {
    case GE_CANTOPENFILE:
    case GE_WRONGNUMBEROFCOLORS:
    case GE_WRONGCOLORS:        
    case GE_INVALIDFILEFORMAT:  
    case GE_SUBFORMATNOTSUPPORTED:
    case GE_TOOSHORT:
    case GE_INVALIDIMAGESIZE:
      fprintf(stderr, _("File \"%s\"\nError: %s\n"),FileName,print_error(error));
      break;
    default: 
      break;
  }
  
  return error;
}

GSM_Error GSM_SaveBitmapFileOnConsole(char *FileName, GSM_Bitmap *bitmap)
{
  int confirm,confirm2;
  char ans[4];
  struct stat buf;
  GSM_Error error;

  /* Ask before overwriting */
  while (stat(FileName, &buf) == 0) {
  
    confirm=-1;
    confirm2=-1;
    
    while (confirm < 0) {
      fprintf(stderr, _("Saving logo. File \"%s\" exists. (O)verwrite, create (n)ew or (s)kip ? "),FileName);
      GetLine(stdin, ans, 4);
      if (!strcmp(ans, "O") || !strcmp(ans, "o")) confirm = 1;
      if (!strcmp(ans, "N") || !strcmp(ans, "n")) confirm = 2;
      if (!strcmp(ans, "S") || !strcmp(ans, "s")) return GE_USERCANCELED;
    }  
    if (confirm==1) break;
    if (confirm==2) {
      while (confirm2 < 0) {
        fprintf(stderr, _("Enter name of new file: "));
        GetLine(stdin, FileName, 50);
        if (&FileName[0]==0) return GE_USERCANCELED;
	confirm2=1;
      }  
    }
  }
  
  error=GSM_SaveBitmapFile(FileName,bitmap);
  
  switch (error) {
    case GE_CANTOPENFILE:        fprintf(stderr, _("Failed to write file \"%s\"\n"),FileName);
                                 break;
    default:                     break;
  }
  
  return error;
}

/* mode == 0 -> overwrite
 * mode == 1 -> ask
 * mode == 2 -> append
 */
int GSM_SaveTextFileOnConsole(char *FileName, char *text, int mode)
{
  int confirm, confirm2;
  char ans[4];
  struct stat buf;
  int error;

  /* Ask before overwriting */
  if (mode==1) {
    while (stat(FileName, &buf) == 0 && mode==1) {
      
      confirm=-1;
      confirm2=-1;
      
      while (confirm < 0) {
        fprintf(stderr, _("File \"%s\" exists. (O)verwrite, (a)ppend, create (n)ew or (s)kip ? "),FileName);
        GetLine(stdin, ans, 4);
        if (!strcmp(ans, "O") || !strcmp(ans, "o")) {
          mode = 0;
          confirm = 1;
        }
        if (!strcmp(ans, "A") || !strcmp(ans, "a")) {
	  mode = 2;
          confirm = 1;
        }
        if (!strcmp(ans, "N") || !strcmp(ans, "n")) confirm=2;
        if (!strcmp(ans, "S") || !strcmp(ans, "s")) return -1;
      }
      
      if (confirm==2) {
        while (confirm2 < 0) {
          fprintf(stderr, _("Enter name of new file: "));
          GetLine(stdin, FileName, 50);
          if (&FileName[0]==0) return -1;
	  mode=1;
	  confirm2=1;
        }  
      }
      
    }  
  }
  
  error=GSM_SaveTextFile(FileName, text, mode);
  
  switch (error) {
    case -1: fprintf(stderr, _("Failed to write file \"%s\"\n"),  FileName);
             break;
    default: break;
  }

  return error;
}

int GSM_SendMultiPartSMSOnConsole(GSM_MultiSMSMessage *MultiSMS, int argnum, int argc, char *argv[],
                                  bool unicode, bool profile, bool scale) {

  int w,i;
  int smsd=0;
  
  struct option options[] = {
             { "smscno",       required_argument, NULL, '2'},
             { "smsc",         required_argument, NULL, '1'},
	     { "name",         required_argument, NULL, '3'},
             { "unicode",      no_argument,       NULL, '4'},
             { "profilestyle", no_argument,       NULL, '5'},
	     { "scale",        no_argument,       NULL, '6'},
             { NULL,           0,                 NULL,  0 }
  };

  GSM_Error error = GE_SMSSENDOK;

  if (argc < 0) {smsd = -1; argc = 0;}

  for (w=0;w<MultiSMS->number;w++) {

    if (argnum!=0) {
      optarg = NULL;
  
      /* We check optional parameters from ... */
      optind = argnum;

      while ((i = getopt_long(argc, argv, "v:ds", options, NULL)) != -1) {
        switch (i) {

          case '1': /* SMSC number */
            MultiSMS->SMS[w].MessageCenter.No = 0;
            strcpy(MultiSMS->SMS[w].MessageCenter.Number,optarg);
            break;

          case '2': /* SMSC number index in phone memory */
            MultiSMS->SMS[w].MessageCenter.No = atoi(optarg);

            if (MultiSMS->SMS[w].MessageCenter.No < 1 || MultiSMS->SMS[w].MessageCenter.No > 5) {
  	      fprintf(stderr, _("Incorrect SMSC number with \"smscno\" option (can't be <1 and >5) !\n"));
              GSM->Terminate();
              return -1;
	    }
            break;

	  case '3': /* Receiver/recipient */
	    strncpy(MultiSMS->SMS[w].Destination,optarg,11); break;

	  case '4': /* Unicode */
	    if (unicode) break;

	  case '5': /* Profile */
	    if (profile) break;

	  case '6': /* Scale */
	    if (scale) break;

          case 'v': /* Set validaty of SMS */
            MultiSMS->SMS[w].Validity = atoi(optarg);
            break;

          case 'd': /* delivery report */
            MultiSMS->SMS[w].Type=GST_DR;
            break;	

          case 's': /* Set replying via the same SMSC */
            MultiSMS->SMS[w].ReplyViaSameSMSC = true; break;

        default:
          fprintf(stderr,_("Unknown option number %i\n"),argc);
          GSM->Terminate();    
          return -1;

        }
      }
    }

    error=GSM->SendSMSMessage(&MultiSMS->SMS[w]);

    if (error == GE_SMSSENDOK) {
      fprintf(stdout, _("SMS %i/%i sent OK !\n"),w+1,MultiSMS->number);
    } else {
      fprintf(stdout, _("SMS %i/%i, sending failed (%d, %s)\n"),w+1,MultiSMS->number, error,print_error(error));
    }

  }

  if (smsd == 0) GSM->Terminate();

  return error;
}

int GSM_SaveMultiPartSMSOnConsole(GSM_MultiSMSMessage *MultiSMS, int argnum, int argc, char *argv[],
                                  bool inter, bool unicode, bool profile, bool scale) {

  int w,i;
  
  GSM_SMSMessage SMSold;

  struct option options[] = {
             { "smscno",       required_argument, NULL, '2'},
             { "smsc",         required_argument, NULL, '1'},
	     { "name",         required_argument, NULL, '3'},
             { "unicode",      no_argument,       NULL, '4'},
             { "profilestyle", no_argument,       NULL, '5'},
	     { "scale",        no_argument,       NULL, '6'},
             { NULL,           0,                 NULL,  0 }
  };

  int interactive;
  int confirm = -1;
  char ans[8];

  GSM_Error error;  

  interactive = inter;

  for (w=0;w<MultiSMS->number;w++) {

    if (argnum!=0) {
      optarg = NULL;
  
      /* We check optional parameters from ... */
      optind = argnum;

      while ((i = getopt_long(argc, argv, "risal:", options, NULL)) != -1) {
        switch (i) {

          case '1': /* SMSC number */
            MultiSMS->SMS[w].MessageCenter.No = 0;
            strcpy(MultiSMS->SMS[w].MessageCenter.Number,optarg);
            break;

          case '2': /* SMSC number index in phone memory */
            MultiSMS->SMS[w].MessageCenter.No = atoi(optarg);

            if (MultiSMS->SMS[w].MessageCenter.No < 1 || MultiSMS->SMS[w].MessageCenter.No > 5) {
  	      fprintf(stderr, _("Incorrect SMSC number with \"smscno\" option (can't be <1 and >5) !\n"));
              GSM->Terminate();
              return -1;
	    }
            break;

	  case '3': /* Receiver/recipient */
	    strncpy(MultiSMS->SMS[w].Destination,optarg,11); break;

	  case '4': /* Unicode */
	    if (unicode) break;

	  case '5': /* Profile */
	    if (profile) break;

	  case '6': /* Scale */
	    if (scale) break;

          case 'r': /* mark as read */
            MultiSMS->SMS[w].Status = GSS_SENTREAD; break;

	  case 'i': /* Save into Inbox */
            MultiSMS->SMS[w].folder = GST_INBOX; break;
	  
          case 's': /* Set replying via the same SMSC */
            MultiSMS->SMS[w].ReplyViaSameSMSC = true; break;

          case 'a': /* Ask before overwriting */
            interactive=true;break;     
	
          case 'l': /* Specify location */
            MultiSMS->SMS[0].Location = atoi(optarg); break;     

        default:
          fprintf(stderr,_("Unknown option number %i\n"),argc);
          GSM->Terminate();    
          return -1;
        }
      }
    }

    if (interactive && MultiSMS->SMS[0].Location!=0 && w==0) {
      SMSold.Location=MultiSMS->SMS[0].Location;
      error = GSM->GetSMSMessage(&SMSold);
      switch (error) {
        case GE_NONE:
          fprintf(stderr, _("Message at specified location exists. "));
          while (confirm < 0) {
            fprintf(stderr, _("Overwrite? (yes/no) "));
            GetLine(stdin, ans, 7);
            if (!strcmp(ans, "yes")) confirm = 1;
            if (!strcmp(ans, "no")) confirm = 0;
          }  
          if (!confirm) { GSM->Terminate(); return 0; }
          else break;
        case GE_INVALIDSMSLOCATION:
          fprintf(stderr, _("Invalid location\n"));
          GSM->Terminate();
          return -1;
        default:
/* FIXME: Remove this fprintf when the function is thoroughly tested */
#ifdef DEBUG
            fprintf(stderr, _("Location %d empty. Saving\n"), SMS[w].Location);
#endif
          break;
      }
    }

    error=GSM->SaveSMSMessage(&MultiSMS->SMS[w]);

    if (error == GE_NONE)
      fprintf(stdout, _("SMS %i/%i saved at location %i !\n"),w+1,MultiSMS->number,MultiSMS->SMS[w].MessageNumber);
    else
      fprintf(stdout, _("SMS %i/%i saving failed (%d, %s, location=%i)\n"), w+1, MultiSMS->number,error,print_error(error),MultiSMS->SMS[w].Location);
  }

  GSM->Terminate();  

  return 0;
}

void GSM_PlayRingtoneOnConsole(GSM_Ringtone *ringtone)
{
  int i;
#ifdef VC6
  char mychar;
#endif

  for (i=0;i<ringtone->NrNotes;i++) {
#ifdef VC6
    if (_kbhit()) {
      mychar=_getch();
      break;
    }
#endif
    GSM_PlayOneNote (ringtone->notes[i]);
  }
  GSM->PlayTone(255*255,0);
}

/* This function shows the copyright and some informations usefull for
   debugging. */
int version(void)
{

  fprintf(stdout, _("GNOKII Version %s\n"
"Copyright (C) Hugh Blemings <hugh@linuxcare.com>, 1999, 2000\n"
"Copyright (C) Pavel Janík ml. <Pavel.Janik@linux.cz>, 1999, 2000\n"
"Built %s %s for %s on %s \n"), VERSION, __TIME__, __DATE__, model, Port);

  return 0;
}

/* The function usage is only informative - it prints this program's usage and
   command-line options. */

int usage(void)
{

  fprintf(stdout, _("   usage: gnokii [--help|--monitor [-noloop|-nl]|--version]\n"
"          gnokii --getmemory memory_type [start [end]] [-short|-v30|-v21|-v]\n"
"          gnokii --writephonebook [-i]\n"
"          gnokii --sendphonebookentry destination memory_type location\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
"          gnokii --savephonebookentry memory_type location\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [--name name]\n"
"          gnokii --getvoicemailbox\n"
"          gnokii --getspeeddial number\n"
"          gnokii --setspeeddial number memory_type location\n\n"

"          gnokii --getsms memory_type start [end] [-f file]\n"
"          gnokii --getsmsstatus\n"
"          gnokii --getsmsfolders\n"
"          gnokii --deletesms memory_type start [end]\n"
"          gnokii --sendsms destination [--smsc message_center_number |\n"
"                 --smscno message_center_index] [--long n] [-s] [-C n]\n"
"                 [--enablevoice|--disablevoice|--enablefax|--disablefax|\n"
"                  --enableemail|--disableemail|--void][--unicode][-v n][-d]\n"
"                 [--text SMS_text]\n"
"          gnokii --savesms destination|\"\" [--smsc \n"
"                 message_center_number] [--smscno message_center_index]\n"
"                 [--long n] [-r] [-i] [-s][-C n][-a][-l][F n][--smsname name]\n"
"                 [--enablevoice|--disablevoice|--enablefax|--disablefax|\n"
"                  --enableemail|--disableemail|--void|--hang|--bug][--unicode]\n"
"          gnokii --receivesms\n"
"          gnokii --getsmsc message_center_number\n"
"          gnokii --renamesmsc number new_name\n\n"

"          gnokii --setdatetime [YYYY [MM [DD [HH [MM]]]]]\n"
"          gnokii --getdatetime\n"
"          gnokii --setalarm HH MM\n"
"          gnokii --getalarm\n\n"

"          gnokii --getcalendarnote { start end [-v30|-v10] | --short|-s }\n"
"          gnokii --writecalendarnote vcardfile number\n"
"          gnokii --deletecalendarnote index\n"
"          gnokii --sendcalendarnote destination vcardfile number\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
"          gnokii --savecalendarnote vcardfile number\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [--name name]\n\n"

"          gnokii --netmonitor {reset|off|field|devel|next|nr}\n"
"          gnokii --nm_collect screen1|-d [screen2|-d]...\n"
"          gnokii --netmonitordata [-S file] [-I file] [-h] [-n n] [-ts n][-tm n]\n"
"                 [-fs str] [-ls str] FLD1:FLD2:FLDn:... \n"
"                 (see files netmonitordata_????_??? for details)\n\n"

"          gnokii --bitmapconvert source destination\n"
"          gnokii --bitmapconvert source destination op|7110op [network code]\n"
"          gnokii --bitmapconvert source destination caller [caller group number]\n"
"          gnokii --bitmapconvert source destination\n"
"                   startup|7110startup|6210startup\n"
"          gnokii --bitmapconvert source destination picture\n"
"          gnokii --showbitmap logofile\n"
"          gnokii --sendlogo op destination logofile network_code\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
"          gnokii --sendlogo picture destination logofile text\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d] [--unicode]\n"
"          gnokii --sendlogo screensaver destination logofile\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
"          gnokii --sendlogo caller destination logofile\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
"          gnokii --savelogo op logofile network_code\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [-l] [--name name]\n"
"          gnokii --savelogo picture logofile text\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [-l] [--name name] [--unicode]\n"
"          gnokii --savelogo screensaver logofile\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [-l] [--name name]\n"
"          gnokii --savelogo caller logofile\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [-l] [--name name]\n"
"          gnokii --setlogo op|7110op [logofile] [network code]\n"
"          gnokii --setlogo startup|7110startup|6210startup [logofile]\n"
"          gnokii --setlogo startup 1|2|3\n"
"          gnokii --setlogo caller [logofile] [caller group number] [group name]\n"
"          gnokii --setlogo picture [logofile] [number] [text] [sender]\n"
"          gnokii --setlogo {dealer|text} [text]\n"
"          gnokii --getlogo op|7110op [logofile] [network code]\n"
"          gnokii --getlogo startup|7110startup|6210startup [logofile]\n"
"          gnokii --getlogo caller [logofile][caller group number]\n"
"          gnokii --getlogo picture [logofile][number]\n"
"          gnokii --getlogo {dealer|text}\n\n"

"          gnokii --sendringtone destination ringtonefile\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d] [--scale] [--profilestyle]\n"
"          gnokii --saveringtone ringtonefile\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [--name name] [--scale] [--profilestyle]\n"
"          gnokii --setringtone ringtonefile [location]\n"
"          gnokii --getringtone ringtonefile [location]\n"
"          gnokii --ringtoneconvert source destination\n"
"          gnokii --binringtoneconvert source destination\n"
"          gnokii --playringtone ringtonefile\n"
"          gnokii --composer ringtonefile\n"
"          gnokii --allringtones\n\n"

"          gnokii --getprofile [number]\n"
"          gnokii --setprofile number feature value\n"
"          gnokii --sendprofile destination profile_name ringtonefile\n"
"                 picturefile [--smsc message_center_number]\n"
"                 [--smscno message_center_index] [-s] [-v n] [-d] [--scale]\n\n"

"          gnokii --reset [soft|hard]\n"
"          gnokii --dialvoice number\n"
"          gnokii --cancelcall\n"
"          gnokii --displayoutput\n"
"          gnokii --presskeysequence sequence\n"
"          gnokii --backupsettings file\n"
"          gnokii --restoresettings file\n"
"          gnokii --getphoneprofile\n"
"          gnokii --setphoneprofile feature value\n"
"          gnokii --getoperatorname\n"
"          gnokii --setoperatorname code name\n"
"          gnokii --senddtmf string\n"
"          gnokii --divert register|enable|query|disable|erasure\n"
"                 all|busy|noans|outofreach all|voice|fax|data\n"
"                 [number timeout]\n\n"

"          gnokii --phonetests\n"
"          gnokii --simlock\n"
"          gnokii --getdisplaystatus\n"
"          gnokii --identify\n\n"

"          gnokii --getwapbookmark location\n"
"          gnokii --setwapbookmark title url [location]\n"
"          gnokii --sendwapbookmark location destination\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
"          gnokii --savewapbookmark location\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [-l] [--name name]\n"
"          gnokii --getwapsettings location\n"
"          gnokii --savewapsettings location\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-r] [-i] [-s] [-a] [-l] [--name name]\n"
"          gnokii --sendwapsettings location destination\n"
"                 [--smsc message_center_number] [--smscno message_center_index]\n"
"                 [-s] [-v n] [-d]\n"
  ));

#ifdef SECURITY
  fprintf(stdout, _(
"\n          gnokii --entersecuritycode PIN|PIN2|PUK|PUK2\n"
"          gnokii --getsecuritycodestatus\n"
"          gnokii --getsecuritycode PIN|PIN2|PUK|PUK2|SecurityCode\n"
"          gnokii --geteeprom\n"
"          gnokii --resetphonesettings\n"
  ));
#endif

#ifdef DEBUG
  fprintf(stdout, _(
"          gnokii --sniff [port]\n"
"          gnokii --decodefile file\n"
"          gnokii --getbinringfromfile file name offset file2\n"
  ));
#endif

  return 0;
}

/* fbusinit is the generic function which waits for the FBUS link. The limit
   is 10 seconds. After 10 seconds we quit. */

void fbusinit(void (*rlp_handler)(RLP_F96Frame *frame))
{

  int count=0;
  GSM_Error error;
#ifndef WIN32
  if (strcmp(GetMygnokiiVersion(),VERSION)!=0)
    fprintf(stderr,_("WARNING: version of installed libmygnokii.so (%s) is different to version of gnokii (%s)\n"),GetMygnokiiVersion(),VERSION);
#endif

  /* Initialise the code for the GSM interface. */     
  error = GSM_Initialise(model, Port, Initlength, GetConnectionTypeFromString(Connection), rlp_handler, SynchronizeTime);

  if (error != GE_NONE) {
    fprintf(stderr, _("GSM/FBUS init failed! (Unknown model ?). Quitting. (%d)\n"), error);
    exit(-1);
  }

  /* First (and important!) wait for GSM link to be active. We allow 10
     seconds... */
  while (count++ < 200 && *GSM_LinkOK == false)
    usleep(50000);

  if (*GSM_LinkOK == false) {
    fprintf (stderr, _("Hmmm... GSM_LinkOK never went true. Quitting.\n"));
    exit(-1);
  }  
}

/* This function checks that the argument count for a given options is withing
   an allowed range. */

int checkargs(int opt, struct gnokii_arg_len gals[], int argc)
{

  int i;

  /* Walk through the whole array with options requiring arguments. */

  for(i = 0;!(gals[i].gal_min == 0 && gals[i].gal_max == 0); i++) {

    /* Current option. */

    if(gals[i].gal_opt == opt) {

      /* Argument count checking. */

      if(gals[i].gal_flags == GAL_XOR) {
	if(gals[i].gal_min == argc || gals[i].gal_max == argc) return 0;
      }
      else {
	if(gals[i].gal_min <= argc && gals[i].gal_max >= argc) return 0;
      }

      return 1;

    }

  }

  /* We do not have options without arguments in the array, so check them. */

  if (argc==0) return 0;
          else return 1;
}

/* Main function - handles command line arguments, passes them to separate
   functions accordingly. */

int main(int argc, char *argv[])
{

  int c, i, rc = -1;
  int nargc = argc-2;
  char **nargv;

  /* Every option should be in this array. */

  static struct option long_options[] =
  {

    { "help",               no_argument,       NULL, OPT_HELP             },// Display usage.
    { "version",            no_argument,       NULL, OPT_VERSION          },// Display version and build information.
    { "getsmsfolders",      no_argument,       NULL, OPT_GETSMSFOLDERS    },// Gets SMS folders
    { "getsmsstatus",       no_argument,       NULL, OPT_GETSMSSTATUS     },// Get SMS Folder Status
    { "identify",           no_argument,       NULL, OPT_IDENTIFY         },// Identify
    { "pmon",               no_argument,       NULL, OPT_PMON             },// For development purposes: run in passive monitoring mode
    { "foogle",             no_argument,       NULL, OPT_FOOGLE           },// For development purposes: insert you function calls here
    { "getdatetime",        no_argument,       NULL, OPT_GETDATETIME      },// Get date and time mode    
    { "getalarm",           no_argument,       NULL, OPT_GETALARM         },// Get alarm    
    { "cancelcall",         no_argument,       NULL, OPT_CANCELCALL       },// Cancel Call    
    { "getdisplaystatus",   no_argument,       NULL, OPT_GETDISPLAYSTATUS },// Get display status mode    
    { "allringtones",       no_argument,       NULL, OPT_ALLRINGTONES     },/* Displays names of available ringtones */    
    { "displayoutput",      no_argument,       NULL, OPT_DISPLAYOUTPUT    },/* Show texts from phone's display */
    { "getphoneprofile",    no_argument,       NULL, OPT_GETPHONEPROFILE  },/* Get/Set phone profile settings */
    { "getoperatorname",    no_argument,       NULL, OPT_GETOPERATORNAME  },/* Get downloaded operator name */    
    { "getvoicemailbox",    no_argument,       NULL, OPT_GETVOICEMAILBOX  },/* Gets voice mailbox number */    
    { "phonetests",         no_argument,       NULL, OPT_PHONETESTS       },
    { "simlockinfo",        no_argument,       NULL, OPT_SIMLOCKINFO      },    
    { "receivesms",         no_argument,       NULL, OPT_RECEIVESMS       },    
    { "setoperatorname",    optional_argument, NULL, OPT_SETOPERATORNAME  },/* Set downloaded operator name */    
    { "setdatetime",        optional_argument, NULL, OPT_SETDATETIME      },// Set date and time    
    { "writephonebook",     optional_argument, NULL, OPT_WRITEPHONEBOOK   },// Write phonebook (memory) mode
    { "reset",              optional_argument, NULL, OPT_RESET            },// Resets the phone
    { "monitor",            optional_argument, NULL, OPT_MONITOR          },// Monitor mode
    { "setlogo",            optional_argument, NULL, OPT_SETLOGO          },// Set logo
    { "getprofile",         optional_argument, NULL, OPT_GETPROFILE       },// Show profile
    { "setalarm",           required_argument, NULL, OPT_SETALARM         },// Set alarm
    { "dialvoice",          required_argument, NULL, OPT_DIALVOICE        },// Voice call mode
    { "getcalendarnote",    required_argument, NULL, OPT_GETCALENDARNOTE  },// Get calendar note mode    
    { "writecalendarnote",  required_argument, NULL, OPT_WRITECALENDARNOTE},// Write calendar note mode
    { "sendcalendarnote",   required_argument, NULL, OPT_SENDCALENDARNOTE },
    { "savecalendarnote",   required_argument, NULL, OPT_SAVECALENDARNOTE },
    { "sendphonebookentry", required_argument, NULL, OPT_SENDPHONEBOOKENTRY},
    { "savephonebookentry", required_argument, NULL, OPT_SAVEPHONEBOOKENTRY},
    { "deletecalendarnote", required_argument, NULL, OPT_DELCALENDARNOTE  },// Delete calendar note mode    
    { "getmemory",          required_argument, NULL, OPT_GETMEMORY        },// Get memory mode
    { "getspeeddial",       required_argument, NULL, OPT_GETSPEEDDIAL     },// Get speed dial mode
    { "setspeeddial",       required_argument, NULL, OPT_SETSPEEDDIAL     },// Set speed dial mode
    { "getsms",             required_argument, NULL, OPT_GETSMS           },// Get SMS message mode
    { "deletesms",          required_argument, NULL, OPT_DELETESMS        },// Delete SMS message mode
    { "sendsms",            required_argument, NULL, OPT_SENDSMS          },// Send SMS message mode
    { "savesms",            required_argument, NULL, OPT_SAVESMS          },// Save SMS message mode
    { "sendlogo",           required_argument, NULL, OPT_SENDLOGO         },// Send logo as SMS message mode
    { "savelogo",           required_argument, NULL, OPT_SAVELOGO         },// Save logo on SIM
    { "sendringtone",       required_argument, NULL, OPT_SENDRINGTONE     },// Send ringtone as SMS message
    { "saveringtone",       required_argument, NULL, OPT_SAVERINGTONE     },// Saves ringtone on SIM
    { "setringtone",        required_argument, NULL, OPT_SETRINGTONE      },// Set ringtone    
    { "getringtone",        required_argument, NULL, OPT_GETRINGTONE      },// Get bin/normal ringtone    
    { "presskeysequence",   required_argument, NULL, OPT_PRESSKEYSEQUENCE },/* Presses keys in phone's display */
    { "getsmsc",            required_argument, NULL, OPT_GETSMSC          },// Get SMS center number mode
    { "renamesmsc",         required_argument, NULL, OPT_RENAMESMSC       },// Rename SMSC
    { "netmonitor",         required_argument, NULL, OPT_NETMONITOR       },// NetMonitor mode
    { "senddtmf",           required_argument, NULL, OPT_SENDDTMF         },// Send DTMF sequence
    { "getlogo",            required_argument, NULL, OPT_GETLOGO          },// Get logo
    { "setprofile",         required_argument, NULL, OPT_SETPROFILE       },// Set profile feature
    { "sendprofile",        required_argument, NULL, OPT_SENDPROFILE      },// Send profile via SMS
    { "setphoneprofile",    required_argument, NULL, OPT_SETPHONEPROFILE  },/* Get/Set phone profile settings */
    { "restoresettings",    required_argument, NULL, OPT_RESTORESETTINGS  },//Restore various settings from one file
    { "backupsettings",     required_argument, NULL, OPT_BACKUPSETTINGS   },//Backup various settings to one file
    { "playringtone",       required_argument, NULL, OPT_PLAYRINGTONE     },/* Plays ringtones */    
    { "composer",           required_argument, NULL, OPT_COMPOSER         },/* Shows ringtone like in Nokia Composer */    
    { "ringtoneconvert",    required_argument, NULL, OPT_RINGTONECONVERT  },/* Convert ringtone files */    
    { "binringtoneconvert", required_argument, NULL, OPT_BINRINGTONECONVERT},/* Convert binary ringtone files */    
    { "bitmapconvert",      required_argument, NULL, OPT_BITMAPCONVERT    },/* Convert bitmap files */    
    { "showbitmap",         required_argument, NULL, OPT_SHOWBITMAP       },    
    { "nm_collect",         required_argument, NULL, OPT_NM_COLLECT       },// NetMonitor periodical data collection mode (newbiee)
    { "netmonitordata",     required_argument, NULL, OPT_NETMONITORDATA   },// NetMonitor periodical data collection mode (advanced)
    { "getwapbookmark",     required_argument, NULL, OPT_GETWAPBOOKMARK   },    
    { "setwapbookmark",     required_argument, NULL, OPT_SETWAPBOOKMARK   },    
    { "savewapbookmark",    required_argument, NULL, OPT_SAVEWAPBOOKMARK  },    
    { "savewapsettings",    required_argument, NULL, OPT_SAVEWAPSETTINGS  },    
    { "sendwapsettings",    required_argument, NULL, OPT_SENDWAPSETTINGS  },    
    { "sendwapbookmark",    required_argument, NULL, OPT_SENDWAPBOOKMARK  },    
    { "getwapsettings",     required_argument, NULL, OPT_GETWAPSETTINGS   },    
    { "divert",             required_argument, NULL, OPT_DIVERT           },

#ifdef SECURITY
    { "entersecuritycode",  required_argument, NULL, OPT_ENTERSECURITYCODE    },// Enter Security Code mode
    { "getsecuritycode",    required_argument, NULL, OPT_GETSECURITYCODE      },// Get Security Code
  { "getsecuritycodestatus",no_argument,       NULL, OPT_GETSECURITYCODESTATUS},// Get Security Code status
    { "geteeprom",          no_argument,       NULL, OPT_GETEEPROM            },// Gets EEPROM
    { "resetphonesettings", no_argument,       NULL, OPT_RESETPHONESETTINGS   },// Reset phone settings
    { "setsimlock",         no_argument,       NULL, OPT_SETSIMLOCK           },// Sets simlock
#endif

#ifdef DEBUG
    { "sniff",              optional_argument, NULL, OPT_SNIFFER    },// Will show datas from port
    { "decodefile",         required_argument, NULL, OPT_DECODEFILE },//decode input file
    { "getbinringfromfile", required_argument, NULL, OPT_GETBINRINGFROMFILE },
#endif

    { 0, 0, 0, 0},
  };

  /* Every command which requires arguments should have an appropriate entry
     in this array. */
	
  struct gnokii_arg_len gals[] =
  {

    { OPT_MONITOR,           0, 1, 0 },

#ifdef SECURITY
    { OPT_ENTERSECURITYCODE, 1, 1, 0 },
    { OPT_GETSECURITYCODE,   1, 1, 0 },
#endif

#ifdef DEBUG
    { OPT_SNIFFER,           0, 1, 0 },
    { OPT_DECODEFILE,        1, 1, 0 },
    { OPT_GETBINRINGFROMFILE,4, 4, 0 },
#endif

    { OPT_SETDATETIME,       0, 5, 0 },
    { OPT_BACKUPSETTINGS,    1, 1, 0 },
    { OPT_RESTORESETTINGS,   1, 1, 0 },
    { OPT_SETALARM,          2, 2, 0 },
    { OPT_DIALVOICE,         1, 1, 0 },
    { OPT_GETCALENDARNOTE,   1, 3, 0 },
    { OPT_WRITECALENDARNOTE, 2, 2, 0 },
    { OPT_SAVECALENDARNOTE,  2, 9, 0 },
    { OPT_SENDCALENDARNOTE,  3, 9, 0 },
    { OPT_SAVEPHONEBOOKENTRY,2, 9, 0 },
    { OPT_SENDPHONEBOOKENTRY,3, 9, 0 },
    { OPT_DELCALENDARNOTE,   1, 1, 0 },
    { OPT_GETMEMORY,         2, 4, 0 },
    { OPT_GETSPEEDDIAL,      1, 1, 0 },
    { OPT_SETSPEEDDIAL,      3, 3, 0 },
    { OPT_GETSMS,            2, 5, 0 },
    { OPT_DELETESMS,         2, 3, 0 },
    { OPT_SENDSMS,           1,12, 0 },
    { OPT_SAVESMS,           1,11, 0 },
    { OPT_SENDLOGO,          3, 9, 0 },
    { OPT_SAVELOGO,          2,10, 0 },
    { OPT_SENDRINGTONE,      2, 7, 0 },
    { OPT_SAVERINGTONE,      1, 9, 0 },
    { OPT_GETSMSC,           1, 1, 0 },
    { OPT_RENAMESMSC,        2, 2, 0 },
    { OPT_NETMONITOR,        1, 1, 0 },
    { OPT_SENDDTMF,          1, 1, 0 },
    { OPT_SETLOGO,           1, 5, 0 },
    { OPT_GETLOGO,           1, 4, 0 },
    { OPT_SETRINGTONE,       1, 3, 0 },
    { OPT_GETRINGTONE,       1, 2, 0 },
    { OPT_PRESSKEYSEQUENCE,  1, 1, 0 },
    { OPT_RESET,             0, 1, 0 },
    { OPT_GETPROFILE,        0, 1, 0 },
    { OPT_SETPROFILE,        3, 3, 0 },
    { OPT_SENDPROFILE,       4,10, 0 },
    { OPT_WRITEPHONEBOOK,    0, 1, 0 },
    { OPT_PLAYRINGTONE,      1, 1, 0 },
    { OPT_COMPOSER,          1, 1, 0 },
    { OPT_RINGTONECONVERT,   2, 2, 0 },
    { OPT_BINRINGTONECONVERT,2, 2, 0 },
    { OPT_BITMAPCONVERT,     2, 4, 0 },
    { OPT_SHOWBITMAP,        1, 1, 0 },
    { OPT_SETOPERATORNAME,   0, 2, 0 },    
    { OPT_SETPHONEPROFILE,   2, 2, 0 },        
    { OPT_NM_COLLECT,        1, MAX_NM_COLLECT, 0 },
    { OPT_NETMONITORDATA,    0,99, 0 },
    { OPT_GETWAPBOOKMARK,    1, 1, 0 },
    { OPT_SETWAPBOOKMARK,    2, 3, 0 },
    { OPT_SAVEWAPBOOKMARK,   1, 9, 0 },
    { OPT_SENDWAPBOOKMARK,   2, 9, 0 },
    { OPT_GETWAPSETTINGS,    1, 1, 0 },
    { OPT_SAVEWAPSETTINGS,   1, 9, 0 },
    { OPT_SENDWAPSETTINGS,   2, 9, 0 },
    { OPT_DIVERT,            3, 5, 0 },    

    { 0, 0, 0, 0 },
  };

  opterr = 0;

  /* For GNU gettext */

#ifdef USE_NLS
#ifndef VC6
  textdomain("gnokii");
  setlocale(LC_ALL, "pl_PL"); //here is string for Polish localisation
#else
  setlocale(LC_ALL, ".852"); //Polish codepage for console, not "real" WIN CP
#endif

#endif

    /* Read config file */
    if (CFG_ReadConfig(&model, &Port, &Initlength, &Connection, &BinDir, &SynchronizeTime,false) < 0) {
	exit(-1);
    }

  /* Handle command line arguments. */

  c = getopt_long(argc, argv, "", long_options, NULL);

  if (c == -1) {


    /* No argument given - we should display usage. */
    usage();
    exit(-1);
  }

  /* We have to build an array of the arguments which will be passed to the
     functions.  Please note that every text after the --command will be
     passed as arguments.  A syntax like gnokii --cmd1 args --cmd2 args will
     not work as expected; instead args --cmd2 args is passed as a
     parameter. */

  if((nargv = malloc(sizeof(char *) * argc)) != NULL) {

    for(i = 2; i < argc; i++)
      nargv[i-2] = argv[i];
	
    if(checkargs(c, gals, nargc)) {

      free(nargv);

      /* Wrong number of arguments - we should display usage. */
      usage();
      exit(-1);
    }

#ifndef VC6
#if defined(__svr4__)
    /* have to ignore SIGALARM */
    sigignore(SIGALRM);
#endif
#endif

    switch(c) {

    // First, error conditions	
    case '?':
      fprintf(stderr, _("Use '%s --help' for usage informations.\n"), argv[0]);
      break;
	
    // Then, options with no arguments
    case OPT_HELP:                  rc = usage();                   break;
    case OPT_VERSION:               rc = version();                 break;
    case OPT_MONITOR:               rc = monitormode(nargc, nargv); break;
    case OPT_GETSMSFOLDERS:         rc = getsmsfolders();           break;
    case OPT_GETDATETIME:           rc = getdatetime();             break;
    case OPT_GETALARM:              rc = getalarm();                break;
    case OPT_GETDISPLAYSTATUS:      rc = getdisplaystatus();        break;
    case OPT_PMON:                  rc = pmon();                    break;
    case OPT_WRITEPHONEBOOK:        rc = writephonebook(nargc, nargv);break;

#ifdef SECURITY
    case OPT_ENTERSECURITYCODE:     rc = entersecuritycode(optarg); break;
    case OPT_GETSECURITYCODESTATUS: rc = getsecuritycodestatus();   break;
    case OPT_GETSECURITYCODE:       rc = getsecuritycode(optarg);   break;
    case OPT_GETEEPROM:             rc = geteeprom();               break;
    case OPT_RESETPHONESETTINGS:    rc = resetphonesettings();      break;
    case OPT_SETSIMLOCK:            rc = setsimlock();              break;
#endif

#ifdef DEBUG
    case OPT_SNIFFER:               rc = sniff(nargc, nargv);       break;
    case OPT_DECODEFILE:            rc = decodefile(nargc, nargv);  break;
    case OPT_GETBINRINGFROMFILE:    rc = getbinringfromfile(nargc, nargv);break;
#endif					
	
    // Now, options with arguments
    case OPT_SETDATETIME:           rc = setdatetime(nargc, nargv); break;
    case OPT_SETALARM:              rc = setalarm(nargv);           break;
    case OPT_DIALVOICE:             rc = dialvoice(optarg);         break;
    case OPT_CANCELCALL:            rc = cancelcall();              break;
    case OPT_GETCALENDARNOTE:       rc = getcalendarnote(nargc, nargv);break;
    case OPT_DELCALENDARNOTE:       rc = deletecalendarnote(optarg);break;
    case OPT_SAVECALENDARNOTE:      rc = savecalendarnote(nargc, nargv);break;
    case OPT_SENDCALENDARNOTE:      rc = sendcalendarnote(nargc, nargv);break;
    case OPT_SAVEPHONEBOOKENTRY:    rc = savephonebookentry(nargc, nargv);break;
    case OPT_SENDPHONEBOOKENTRY:    rc = sendphonebookentry(nargc, nargv);break;
    case OPT_WRITECALENDARNOTE:     rc = writecalendarnote(nargv);  break;
    case OPT_GETMEMORY:             rc = getmemory(nargc, nargv);   break;
    case OPT_GETSPEEDDIAL:          rc = getspeeddial(optarg);      break;
    case OPT_SETSPEEDDIAL:          rc = setspeeddial(nargv);       break;
    case OPT_GETSMS:                rc = getsms(argc, argv);        break;
    case OPT_GETSMSSTATUS:          rc = getsmsstatus(argc, argv);  break;
    case OPT_DELETESMS:             rc = deletesms(nargc, nargv);   break;
    case OPT_SENDSMS:               rc = sendsms(nargc, nargv);     break;
    case OPT_SAVESMS:               rc = savesms(nargc, nargv);     break;
    case OPT_DIVERT:                rc = divert(nargc, nargv);      break;
    case OPT_SENDLOGO:              rc = sendlogo(nargc, nargv);    break;
    case OPT_SAVELOGO:              rc = savelogo(nargc, nargv);    break;
    case OPT_GETSMSC:               rc = getsmsc(optarg);           break;
    case OPT_RENAMESMSC:            rc = renamesmsc(nargc,nargv);   break;
    case OPT_NETMONITOR:            rc = netmonitor(optarg);        break;
    case OPT_IDENTIFY:              rc = identify();                break;
    case OPT_SETLOGO:               rc = setlogo(nargc, nargv);     break;
    case OPT_GETLOGO:               rc = getlogo(nargc, nargv);     break;
    case OPT_RECEIVESMS:            rc = receivesms(nargc, nargv);  break;
    case OPT_SETRINGTONE:           rc = setringtone(nargc, nargv); break;
    case OPT_GETRINGTONE:           rc = getringtone(nargc, nargv); break;
    case OPT_PRESSKEYSEQUENCE:      rc = presskeysequence(nargv);   break;
    case OPT_SENDRINGTONE:          rc = sendringtone(nargc, nargv);break;
    case OPT_SAVERINGTONE:          rc = saveringtone(nargc, nargv);break;
    case OPT_GETPROFILE:            rc = getprofile(nargc, nargv);  break;
    case OPT_SETPROFILE:            rc = setprofile(nargc, nargv);  break;
    case OPT_SENDPROFILE:           rc = sendprofile(nargc, nargv); break;
    case OPT_DISPLAYOUTPUT:         rc = displayoutput();           break;
    case OPT_RESTORESETTINGS:       rc = restoresettings(nargv);    break;
    case OPT_BACKUPSETTINGS:        rc = backupsettings(nargv);     break;
    case OPT_RINGTONECONVERT:       rc = ringtoneconvert(nargc, nargv);break;
    case OPT_BINRINGTONECONVERT:    rc = binringtoneconvert(nargc, nargv);break;
    case OPT_BITMAPCONVERT:         rc = bitmapconvert(nargc, nargv);break;
    case OPT_SHOWBITMAP:            rc = showbitmap(nargc, nargv);  break;
    case OPT_PLAYRINGTONE:          rc = playringtone(nargc, nargv);break;
    case OPT_COMPOSER:              rc = composer(nargc, nargv);    break;
    case OPT_FOOGLE:                rc = foogle(nargv);             break;
    case OPT_PHONETESTS:            rc = phonetests();              break;
    case OPT_SIMLOCKINFO:           rc = simlockinfo();             break;
    case OPT_SENDDTMF:              rc = senddtmf(optarg);          break;
    case OPT_RESET:                 rc = reset(nargc,nargv);        break;
    case OPT_GETOPERATORNAME:       rc = getoperatorname();         break;
    case OPT_SETOPERATORNAME:       rc = setoperatorname(nargc,nargv);break;
    case OPT_GETWAPBOOKMARK:        rc = getwapbookmark(nargc,nargv);break;
    case OPT_SETWAPBOOKMARK:        rc = setwapbookmark(nargc,nargv);break;
    case OPT_SAVEWAPBOOKMARK:       rc = savewapbookmark(nargc,nargv);break;
    case OPT_SENDWAPBOOKMARK:       rc = sendwapbookmark(nargc,nargv);break;
    case OPT_GETWAPSETTINGS:        rc = getwapsettings(nargc,nargv);break;
    case OPT_SAVEWAPSETTINGS:       rc = savewapsettings(nargc,nargv);break;
    case OPT_SENDWAPSETTINGS:       rc = sendwapsettings(nargc,nargv);break;
    case OPT_ALLRINGTONES:          rc = allringtones();            break;
    case OPT_GETPHONEPROFILE:       rc = getphoneprofile();         break;
    case OPT_SETPHONEPROFILE:       rc = setphoneprofile(nargc,nargv);break;
    case OPT_GETVOICEMAILBOX:       rc = getvoicemailbox();         break;
    case OPT_NM_COLLECT:            rc = nm_collect(nargc, nargv);  break;
    case OPT_NETMONITORDATA:        rc = netmonitordata(nargc, nargv);break;

    default:         fprintf(stderr, _("Unknown option: %d\n"), c); break;

    }

    free(nargv);

    return(rc);
  }

  fprintf(stderr, _("Wrong number of arguments\n"));

  exit(-1);
}

/* Restores various phone settings from one file */
int restoresettings(char *argv[])
{
  GSM_Backup Backup;
  GSM_PhonebookEntry pbk;

  int confirm;
  char ans[4];

  int i,pos;

  GSM_MemoryStatus SIMMemoryStatus = {GMT_SM, 0, 0};
  GSM_MemoryStatus PhoneMemoryStatus = {GMT_ME, 0, 0};

  if (GSM_ReadBackupFile(argv[0], &Backup)!=GE_NONE) return 1;

  fbusinit(NULL);

  if (Backup.SIMPhonebookUsed!=0) {
    confirm=-1;    
    while (confirm < 0) {
      fprintf(stderr, _("Restore SIM phonebook ? "));
      GetLine(stdin, ans, 99);
      if (!strcmp(ans, "yes")) confirm = 1;
      if (!strcmp(ans, "no" )) confirm = 0;
    }
    if (confirm==1) {
      if (GSM->GetMemoryStatus(&SIMMemoryStatus) != GE_NONE) {
        fprintf(stderr,_("Error getting memory status !\n"));
        GSM->Terminate();
      }
      i=0;pos=1;

      while ((pos-1)!=SIMMemoryStatus.Used+SIMMemoryStatus.Free) {
        pbk.Location=pos;
        pbk.MemoryType=GMT_SM;
        pbk.Name[0]=0;
        pbk.Number[0]=0;
        pbk.SubEntriesCount = 0;
        if (i<Backup.SIMPhonebookUsed) {
          if (Backup.SIMPhonebook[i].Location==pbk.Location) {
            pbk=Backup.SIMPhonebook[i];
            i++;
//#ifdef DEBUG
//            fprintf(stdout,_("Copying from backup\n"));
//#endif
          }
        }
//#ifdef DEBUG
//        fprintf(stdout,_("Setting location %i\n"),pbk.Location);
//#endif
        GSM->WritePhonebookLocation(&pbk);
	fprintf(stderr,_("."));
        pos++;
      }
      fprintf(stderr,_("\n"));
    }
  }
  if (Backup.PhonePhonebookUsed!=0) {
    confirm=-1;    
    while (confirm < 0) {
      fprintf(stderr, _("Restore phone phonebook ? "));
      GetLine(stdin, ans, 99);
      if (!strcmp(ans, "yes")) confirm = 1;
      if (!strcmp(ans, "no" )) confirm = 0;
    }
    if (confirm==1) {
      if (GSM->GetMemoryStatus(&PhoneMemoryStatus) != GE_NONE) {
        fprintf(stderr,_("Error getting memory status !\n"));
        GSM->Terminate();
      }

      i=0;pos=1;

      while ((pos-1)!=PhoneMemoryStatus.Used+PhoneMemoryStatus.Free) {
        pbk.Location=pos;
        pbk.MemoryType=GMT_ME;
        pbk.Name[0]=0;
        pbk.Number[0]=0;
        pbk.SubEntriesCount = 0;
        if (i<Backup.PhonePhonebookUsed) {
          if (Backup.PhonePhonebook[i].Location==pbk.Location) {
            pbk=Backup.PhonePhonebook[i];
            i++;
//#ifdef DEBUG
//            fprintf(stdout,_("Copying from backup\n"));
//#endif
          }
        }
//#ifdef DEBUG
//        fprintf(stdout,_("Setting location %i\n"),pbk.Location);
//#endif
        GSM->WritePhonebookLocation(&pbk);
	fprintf(stderr,_("."));
        pos++;
      }
      fprintf(stderr,_("\n"));
    }
  }
  if (Backup.CallerAvailable==true) {
    confirm=-1;    
    while (confirm < 0) {
      fprintf(stderr, _("Restore caller groups ? "));
      GetLine(stdin, ans, 99);
      if (!strcmp(ans, "yes")) confirm = 1;
      if (!strcmp(ans, "no" )) confirm = 0;
    }
    if (confirm==1) {
      for (i=0;i<5;i++) GSM->SetBitmap(&Backup.CallerGroups[i]);
    }
  }
  if (Backup.OperatorLogoAvailable==true) {
    confirm=-1;    
    while (confirm < 0) {
      fprintf(stderr, _("Restore operator logo ? "));
      GetLine(stdin, ans, 99);
      if (!strcmp(ans, "yes")) confirm = 1;
      if (!strcmp(ans, "no" )) confirm = 0;
    }
    if (confirm==1) {
      GSM->SetBitmap(&Backup.OperatorLogo);
    }
  }
  if (Backup.StartupLogoAvailable==true) {
    confirm=-1;    
    while (confirm < 0) {
      fprintf(stderr, _("Restore startup logo ? "));
      GetLine(stdin, ans, 99);
      if (!strcmp(ans, "yes")) confirm = 1;
      if (!strcmp(ans, "no" )) confirm = 0;
    }
    if (confirm==1) {
      GSM->SetBitmap(&Backup.StartupLogo);
    }
  }

  GSM->Terminate();

  return 0;
}

/* Backup various phone settings from one file */
int backupsettings(char *argv[])
{
  GSM_PhonebookEntry PbkEntry;
  GSM_Error error;
  GSM_Backup Backup;
  int i;

  GSM_MemoryStatus SIMMemoryStatus = {GMT_SM, 0, 0};
  GSM_MemoryStatus PhoneMemoryStatus = {GMT_ME, 0, 0};

  fbusinit(NULL);

  fprintf(stderr,_("Backup phonebook from SIM..."));
  Backup.SIMPhonebookUsed=0;
  if (GSM->GetMemoryStatus(&SIMMemoryStatus) == GE_NONE) {
    Backup.SIMPhonebookSize=SIMMemoryStatus.Used+SIMMemoryStatus.Free;

    PbkEntry.MemoryType=GMT_SM;

    for (i=0;i<Backup.SIMPhonebookSize;i++)
    {
      if (SIMMemoryStatus.Used==Backup.SIMPhonebookUsed) break;

      PbkEntry.Location=i;
    
      error=GSM->GetMemoryLocation(&PbkEntry);
      switch (error) {
        case GE_NONE:
          Backup.SIMPhonebook[Backup.SIMPhonebookUsed]=PbkEntry;
          Backup.SIMPhonebookUsed++;
          fprintf(stderr,_("."));
          break;
        default:
          break;
      }
    }
    fprintf(stderr,_("Done\n"));
  } else fprintf(stderr,_("ERROR\n"));

  fprintf(stderr,_("Backup phonebook from phone..."));
  Backup.PhonePhonebookUsed=0;
  if (GSM->GetMemoryStatus(&PhoneMemoryStatus) == GE_NONE) {
    Backup.PhonePhonebookSize=PhoneMemoryStatus.Used+PhoneMemoryStatus.Free;

    PbkEntry.MemoryType=GMT_ME;

    for (i=0;i<Backup.PhonePhonebookSize;i++)
    {
      if (PhoneMemoryStatus.Used==Backup.PhonePhonebookUsed) break;

      PbkEntry.Location=i;
    
      error=GSM->GetMemoryLocation(&PbkEntry);
      switch (error) {
        case GE_NONE:
          Backup.PhonePhonebook[Backup.PhonePhonebookUsed]=PbkEntry;
          Backup.PhonePhonebookUsed++;
          fprintf(stderr,_("."));
          break;
        default:
          break;
      }
    }
    fprintf(stderr,_("Done\n"));
  } else fprintf(stderr,_("ERROR\n"));

  if( GetModelFeature (FN_CALLERGROUPS)!=0) {
    fprintf(stderr,_("Backup caller logos..."));
    Backup.CallerAvailable=true;
    for (i=0;i<5;i++) {
      Backup.CallerGroups[i].number=i;
      Backup.CallerGroups[i].type=GSM_CallerLogo;
      if (GSM->GetBitmap(&Backup.CallerGroups[i])!=GE_NONE) return 1;
    }
    fprintf(stderr,_("Done\n"));
  } else Backup.CallerAvailable=false;

//  fprintf(stderr,_("Backup speed dials..."));
  Backup.SpeedAvailable=false;
//  for (i=0;i<8;i++) {
//    Backup.SpeedDials[i].Number=i+1;
//    if (GSM->GetSpeedDial(&Backup.SpeedDials[i])!=GE_NONE) return 1;
//  }
//  fprintf(stderr,_("Done\n"));

  fprintf(stderr,_("Backup operator logo..."));
  Backup.OperatorLogoAvailable=true;
  Backup.OperatorLogo.type=GSM_7110OperatorLogo;
  if (GSM->GetBitmap(&Backup.OperatorLogo)!=GE_NONE) {
    Backup.OperatorLogo.type=GSM_OperatorLogo;
    if (GSM->GetBitmap(&Backup.OperatorLogo)!=GE_NONE) {
      Backup.OperatorLogoAvailable=false;
     fprintf(stderr,_("Error\n"));
    } else fprintf(stderr,_("Done\n"));
  } else fprintf(stderr,_("Done\n"));

  Backup.StartupLogoAvailable=false;
  if( GetModelFeature (FN_STARTUP)!=0) {
    fprintf(stderr,_("Backup startup logo..."));
    Backup.StartupLogoAvailable=true;
    switch (GetModelFeature (FN_STARTUP)) {
      case F_STA62: Backup.StartupLogo.type=GSM_6210StartupLogo;break;
      case F_STA71: Backup.StartupLogo.type=GSM_7110StartupLogo;break;
      default     : Backup.StartupLogo.type=GSM_StartupLogo;break;
    }
    if (GSM->GetBitmap(&Backup.StartupLogo)!=GE_NONE) {
      Backup.StartupLogoAvailable=false;
      fprintf(stderr,_("Error\n"));
    } else fprintf(stderr,_("Done\n"));
  }

  fprintf(stderr,_("Backup welcome note..."));
  Backup.StartupText.type=GSM_WelcomeNoteText;
  if (GSM->GetBitmap(&Backup.StartupText)!=GE_NONE) {
    fprintf(stderr,_("Error\n"));
  } else fprintf(stderr,_("Done\n"));

  GSM->Terminate();

  GSM_SaveBackupFile(argv[0], &Backup);

  return 0;
}

/* Presses keys on phone's keyboard */

int presskeysequence(char *argv[])
{
  int i,j;
  int keycode;
  char key;
  
  sleep(1);

  fbusinit(NULL);
  
  for (i=0;i<strlen(argv[0]);i++)
  {
    key=argv[0][i];
    keycode=0;
    j=0;
    
    if (key!='w' && key!='W')
    {
      while (Keys[j].whatchar!=' ') {
        if (Keys[j].whatchar==key) {    
          keycode=Keys[j].whatcode;
          break;
        }
        j++;
      }
    
      if (keycode==0) {
        fprintf(stderr,_("Unknown key: %c !\n"),key);
        GSM->Terminate();
        return -1;
      }
    
      if (GSM->PressKey(keycode,PRESSPHONEKEY)!=GE_NONE)
      {
        fprintf(stderr,_("Can't press key !\n"));
        GSM->Terminate();
        return -1;
      }
      if (GSM->PressKey(keycode,RELEASEPHONEKEY)!=GE_NONE)
      {
        fprintf(stderr,_("Can't release key !\n"));
        GSM->Terminate();
        return -1;
      }
    } else
    {
      sleep(2);
    }
  }
  
  GSM->Terminate();

  return 0;
}

/* Send  SMS messages. */
int sendsms(int argc, char *argv[])
{
  GSM_MultiSMSMessage MultiSMS;
  char message_buffer[GSM_MAX_CONCATENATED_SMS_LENGTH];
  int input_len, chars_read,i,msgnum, rc=0;
  int smsd = 0, mbuffer = 0;
  GSM_SMSMessageType SMSType=GST_SMS;
  int SMSValidity= 4320; /* 4320 minutes == 72 hours */
  bool SMSReply=false;
  int SMSClass=-1,SMSCenter=1;
  char SMSCNumber[100];
  GSM_Coding_Type SMSCoding=GSM_Coding_Default;
  GSM_UDH SMSUDHType=GSM_NoUDH;

  struct option options[] = {
             { "smscno",       required_argument, NULL, '2'},
             { "smsc",         required_argument, NULL, '1'},
             { "long",	       required_argument, NULL, '3'},
             { "enablevoice",  no_argument,       NULL, '4'},
	     { "disablevoice", no_argument,       NULL, '5'},
	     { "enableemail",  no_argument,       NULL, '6'},
	     { "disableemail", no_argument,       NULL, '7'},
	     { "enablefax",    no_argument,       NULL, '8'},
	     { "disablefax",   no_argument,       NULL, '9'},
	     { "unicode",      no_argument,       NULL, '-'},
	     { "void",         no_argument,       NULL, '+'},
	     { "hang",         no_argument,       NULL, '('},
	     { "bug",          no_argument,       NULL, ')'},
	     { "text",         required_argument, NULL, '['},
	     { "smsd",         no_argument,       NULL, ']'},
             { NULL,           0,                 NULL, 0}
  };
  
  input_len = GSM_MAX_SMS_LENGTH;
  chars_read = 0;

  if (argc!=0) {

    optarg = NULL;
    optind = 0;

    while ((i = getopt_long(argc, argv, "v:dsC:", options, NULL)) != -1) {
      switch (i) {

        case '1': /* SMSC number */
          SMSCenter = 0;
          strcpy(SMSCNumber,optarg);
          break;

        case '2': /* SMSC number index in phone memory */
          SMSCenter = atoi(optarg);

          if (SMSCenter < 1 || SMSCenter > 5) {
            fprintf(stderr, _("Incorrect SMSC number with \"smscno\" option (can't be <1 and >5) !\n"));
            GSM->Terminate();
            return -1;
	  }
          break;

        case '3': /* we send long message */
          SMSUDHType=GSM_ConcatenatedMessages;
          input_len = atoi(optarg);
          if (input_len > GSM_MAX_CONCATENATED_SMS_LENGTH) {
	    fprintf(stderr, _("Input too long, max %i!\n"),GSM_MAX_CONCATENATED_SMS_LENGTH);
	    exit(-1);
          }
          break;

        case '4': /* SMS enables voice indicator */
          SMSUDHType=GSM_EnableVoice;    break;

        case '5': /* SMS disables voice indicator */
          SMSUDHType=GSM_DisableVoice;   break;	  

        case '6': /* SMS enables email indicator */
          SMSUDHType=GSM_EnableEmail;    break;	  

        case '7': /* SMS disables email indicator */
          SMSUDHType=GSM_DisableEmail;   break;	  

        case '8': /* SMS enables fax indicator */
          SMSUDHType=GSM_EnableFax;      break;	  

        case '9': /* SMS disables fax indicator */
          SMSUDHType=GSM_DisableFax;     break;	  

        case '-': /* SMS coding type */
          SMSCoding=GSM_Coding_Unicode;  break;

        case '+': /* SMS ghost */
          SMSUDHType=GSM_VoidSMS;        break;

        case '(': /* SMS hanging phone, when saved to Outbox */
          SMSUDHType=GSM_HangSMS;        break;

        case ')': /* SMS showed incorrectly in phone */
          SMSUDHType=GSM_BugSMS;         break;

        case '[': /* SMS text from command line or file */
          strncpy(message_buffer, optarg, GSM_MAX_CONCATENATED_SMS_LENGTH-1);
	  chars_read = strlen(message_buffer); mbuffer = 1;
	  break;

        case ']': /* when init/terminate should not be called (in receivesms) */
 	  smsd = -1;			break;

        case 'v': /* Set validaty of SMS */
          SMSValidity = atoi(optarg);    break;

        case 'd': /* delivery report */
          SMSType=GST_DR;                break;	

        case 's': /* Set replying via the same SMSC */
          SMSReply = true;               break;

        case 'C': /* class Message */
          
	  if (SMSUDHType!=GSM_NoUDH) {
            fprintf(stderr, _("Can't specify SMS Class with --enablevoice, --disablevoice, --enableemail, --disableemail, --enablefax, --disablefax options !\n"));	    
	    return -1;
	  }
	  
          switch (*optarg) {
            case '0': SMSClass = 0; break;
            case '1': SMSClass = 1; break;
            case '2': SMSClass = 2; break;
            case '3': SMSClass = 3; break; 
            default:
	      fprintf(stderr, _("SMS Class (\"C\" option) can be 0, 1, 2 or 3 only !\n"));
              return -1;
          }
          break;
	  
        default:
          fprintf(stderr,_("Unknown option number %i\n"),argc);
          return -1;
      }
    }
  }
  
  /* Get message text from stdin. */
  if (mbuffer == 0) chars_read = fread(message_buffer, 1, input_len, stdin);

  if (chars_read == 0) {
    fprintf(stderr, _("Couldn't read from stdin! or empty message\n"));
    return -1;
  }
  if (chars_read > input_len) {
    fprintf(stderr, _("Input too long!\n"));	
    return -1;
  }
  
  /*  Null terminate. */
  message_buffer[chars_read] = 0x00;	

  GSM_MakeMultiPartSMS2(&MultiSMS,message_buffer,chars_read,SMSUDHType,SMSCoding);
  msgnum=MultiSMS.number;

  switch (SMSUDHType) {
    case GSM_NoUDH:
    case GSM_BugSMS:
    case GSM_VoidSMS:
    case GSM_HangSMS:
    case GSM_EnableVoice:
    case GSM_DisableVoice:
    case GSM_EnableFax:
    case GSM_DisableFax:
    case GSM_EnableEmail:
    case GSM_DisableEmail:
      fprintf(stdout,_("Warning: saving %i chars\n"),strlen(MultiSMS.SMS[0].MessageText));
      msgnum=1;
      break;
    default:
      break;
  }

  for (i=0;i<msgnum;i++) {
    strcpy(MultiSMS.SMS[i].Destination,argv[0]);

    MultiSMS.SMS[i].Class=SMSClass;
    MultiSMS.SMS[i].ReplyViaSameSMSC=SMSReply;
    MultiSMS.SMS[i].Type=SMSType;
    MultiSMS.SMS[i].Validity=SMSValidity;
  }

  /* Initialise the GSM interface. */     
  if (smsd == 0) fbusinit(NULL);

  MultiSMS.number=msgnum;
  rc = GSM_SendMultiPartSMSOnConsole(&MultiSMS, 0,smsd,NULL,false,false,false);
  
  return rc;
}

int savesms(int argc, char *argv[])
{
  GSM_MultiSMSMessage MultiSMS;
  char message_buffer[GSM_MAX_CONCATENATED_SMS_LENGTH];
  int input_len, chars_read,i,msgnum;

  int SMSClass=-1,SMSCenter=1;
  char SMSName[25+1];
  char SMSCNumber[100];
  GSM_Coding_Type SMSCoding=GSM_Coding_Default;
  GSM_UDH SMSUDHType=GSM_NoUDH;
  GSM_SMSMessageStatus SMSStatus;
  int SMSFolder;
  bool SMSReply=false;
  int SMSLocation=0;
  bool interactive=false;

  struct option options[] = {
             { "smscno",       required_argument, NULL, '2'},
             { "smsc",         required_argument, NULL, '1'},
             { "long",	       required_argument, NULL, '3'},
             { "enablevoice",  no_argument,       NULL, '4'},
	     { "disablevoice", no_argument,       NULL, '5'},
	     { "enableemail",  no_argument,       NULL, '6'},
	     { "disableemail", no_argument,       NULL, '7'},
	     { "enablefax",    no_argument,       NULL, '8'},
	     { "disablefax",   no_argument,       NULL, '9'},
	     { "unicode",      no_argument,       NULL, '-'},
	     { "void",         no_argument,       NULL, '+'},
	     { "hang",         no_argument,       NULL, '('},
	     { "bug",          no_argument,       NULL, ')'},
	     { "smsname",      required_argument, NULL, '/'},
             { NULL,           0,                 NULL, 0}
  };

  SMSCNumber[0]=0;
  SMSName[0]=0;
  SMSStatus=GSS_NOTSENTREAD;
  SMSFolder=GST_OUTBOX;
  
  input_len = GSM_MAX_SMS_LENGTH;

  if (argc!=0) {

    optarg = NULL;
    optind = 0;

    while ((i = getopt_long(argc, argv, "risal:C:F:", options, NULL)) != -1) {
      switch (i) {

        case '1': /* SMSC number */
          SMSCenter = 0;
          strcpy(SMSCNumber,optarg);
          break;

        case '2': /* SMSC number index in phone memory */
          SMSCenter = atoi(optarg);

          if (SMSCenter < 1 || SMSCenter > 5) {
            fprintf(stderr, _("Incorrect SMSC number with \"smscno\" option (can't be <1 and >5) !\n"));
            GSM->Terminate();
            return -1;
	  }
          break;

        case '3': /* we send long message */
          SMSUDHType=GSM_ConcatenatedMessages;
          input_len = atoi(optarg);
          if (input_len > GSM_MAX_CONCATENATED_SMS_LENGTH) {
	    fprintf(stderr, _("Input too long, max %i!\n"),GSM_MAX_CONCATENATED_SMS_LENGTH);
	    exit(-1);
          }
          break;

        case '4': /* SMS enables voice indicator */
          SMSUDHType=GSM_EnableVoice;    break;

        case '5': /* SMS disables voice indicator */
          SMSUDHType=GSM_DisableVoice;   break;	  

        case '6': /* SMS enables email indicator */
          SMSUDHType=GSM_EnableEmail;    break;	  

        case '7': /* SMS disables email indicator */
          SMSUDHType=GSM_DisableEmail;   break;	  

        case '8': /* SMS enables fax indicator */
          SMSUDHType=GSM_EnableFax;      break;	  

        case '9': /* SMS disables fax indicator */
          SMSUDHType=GSM_DisableFax;     break;	  

        case '-': /* SMS coding type */
          SMSCoding=GSM_Coding_Unicode;  break;

        case '+': /* SMS ghost */
          SMSUDHType=GSM_VoidSMS;        break;

        case '(': /* SMS hanging phone, when saved to Outbox */
          SMSUDHType=GSM_HangSMS;        break;

        case ')': /* SMS showed incorrectly in phone */
          SMSUDHType=GSM_BugSMS;         break;

        case 'r': /* mark as read */
          SMSStatus = GSS_SENTREAD; break;
 
        case 'i': /* Save into Inbox */
          SMSFolder = GST_INBOX; break;
	  
        case 's': /* Set replying via the same SMSC */
          SMSReply = true; break;

        case 'a': /* Ask before overwriting */
          interactive=true;break;     
	
        case 'l': /* Specify location */
          SMSLocation = atoi(optarg); break;     

        case '/': /* Name */
          strncpy(SMSName,optarg,25);break;

        case 'C': /* class Message */
          
	  if (SMSUDHType!=GSM_NoUDH) {
            fprintf(stderr, _("Can't specify SMS Class with --enablevoice, --disablevoice, --enableemail, --disableemail, --enablefax, --disablefax options !\n"));	    
	    return -1;
	  }
	  
          switch (*optarg) {
            case '0': SMSClass = 0; break;
            case '1': SMSClass = 1; break;
            case '2': SMSClass = 2; break;
            case '3': SMSClass = 3; break; 
            default:
	      fprintf(stderr, _("SMS Class (\"C\" option) can be 0, 1, 2 or 3 only !\n"));
              return -1;
          }
          break;

        case 'F': /* save into folder n */
	  SMSFolder = atoi(optarg);
          break;

        default:
          fprintf(stderr,_("Unknown option number %i\n"),argc);
          return -1;
      }
    }
  }
  
  /* Get message text from stdin. */
  chars_read = fread(message_buffer, 1, input_len, stdin);

  if (chars_read == 0) {
    fprintf(stderr, _("Couldn't read from stdin!\n"));	
    return -1;
  }
  if (chars_read > input_len) {
    fprintf(stderr, _("Input too long!\n"));	
    return -1;
  }
  
  /*  Null terminate. */
  message_buffer[chars_read] = 0x00;	

  GSM_MakeMultiPartSMS2(&MultiSMS,message_buffer,chars_read,SMSUDHType,SMSCoding);
  msgnum=MultiSMS.number;

  switch (SMSUDHType) {
    case GSM_NoUDH:
    case GSM_BugSMS:
    case GSM_VoidSMS:
    case GSM_HangSMS:
    case GSM_EnableVoice:
    case GSM_DisableVoice:
    case GSM_EnableFax:
    case GSM_DisableFax:
    case GSM_EnableEmail:
    case GSM_DisableEmail:
      fprintf(stdout,_("Warning: saving %i chars\n"),strlen(MultiSMS.SMS[0].MessageText));
      msgnum=1;
      break;
    default:
      break;
  }

  for (i=0;i<msgnum;i++) {
    MultiSMS.SMS[i].Destination[0]=0;
    if (argc!=0) strcpy(MultiSMS.SMS[i].Destination,argv[0]);

    MultiSMS.SMS[i].Location=0;
    MultiSMS.SMS[i].Class=SMSClass;
    MultiSMS.SMS[i].MessageCenter.No=SMSCenter;
    strcpy(MultiSMS.SMS[i].MessageCenter.Number,SMSCNumber);
    MultiSMS.SMS[i].Status=SMSStatus;
    strcpy(MultiSMS.SMS[i].Name,SMSName);
    MultiSMS.SMS[i].folder=SMSFolder;
    MultiSMS.SMS[i].ReplyViaSameSMSC=SMSReply;
  }

  MultiSMS.SMS[0].Location=SMSLocation;

  /* Initialise the GSM interface. */     
  fbusinit(NULL);

  MultiSMS.number=msgnum;
  GSM_SaveMultiPartSMSOnConsole(&MultiSMS, 0,0,NULL,interactive,false,false,false);
  
  return 0;
}

/* Get SMSC number */

int getsmsc(char *MessageCenterNumber)
{

  GSM_MessageCenter MessageCenter;

  MessageCenter.No=atoi(MessageCenterNumber);

  fbusinit(NULL);

  if (GSM->GetSMSCenter(&MessageCenter) == GE_NONE) {

    fprintf(stdout, _("%d. SMS center ("),MessageCenter.No);
    
    if (!strcmp(MessageCenter.Name,""))
      fprintf(stdout,_("Set %d"),MessageCenter.No);
    else fprintf(stdout,_("%s"),MessageCenter.Name);
      
    fprintf(stdout,_(") number is "));

    if (!strcmp(MessageCenter.Number,"")) fprintf(stdout,_("not set\n"));
    else fprintf(stdout,_("%s\n"),MessageCenter.Number);

    fprintf(stdout,_("Default recipient number is "));

    if (!strcmp(MessageCenter.DefaultRecipient,""))
      fprintf(stdout,_("not set\n"));
    else fprintf(stdout,_("%s\n"),MessageCenter.DefaultRecipient);

    fprintf(stdout, _("Messages sent as "));

    switch (MessageCenter.Format) {
      case GSMF_Text  :fprintf(stdout, _("Text"));break;
      case GSMF_Paging:fprintf(stdout, _("Paging"));break;
      case GSMF_Fax   :fprintf(stdout, _("Fax"));break;
      case GSMF_Email :
      case GSMF_UCI   :fprintf(stdout, _("Email"));break;
      case GSMF_ERMES :fprintf(stdout, _("ERMES"));break;
      case GSMF_X400  :fprintf(stdout, _("X.400"));break;
      default         :fprintf(stdout, _("Unknown"));
    }

    printf("\n");

    fprintf(stdout, _("Message validity is "));

    switch (MessageCenter.Validity) {
      case GSMV_1_Hour  :fprintf(stdout, _("1 hour"));break;
      case GSMV_6_Hours :fprintf(stdout, _("6 hours"));break;
      case GSMV_24_Hours:fprintf(stdout, _("24 hours"));break;
      case GSMV_72_Hours:fprintf(stdout, _("72 hours"));break;
      case GSMV_1_Week  :fprintf(stdout, _("1 week"));break;
      case GSMV_Max_Time:fprintf(stdout, _("Maximum time"));break;
      default           :fprintf(stdout, _("Unknown"));
    }

    fprintf(stdout, "\n");

  }
  else
    fprintf(stdout, _("SMS center can not be found :-(\n"));

  GSM->Terminate();

  return 0;
}

/* format SMS message. */
char* formatsms(GSM_Error error, char* memory_type_string, GSM_SMSFolders folders, GSM_SMSMessage message, char* filename)
{

  GSM_WAPBookmark bookmark;
  int mode = 1;

  GSM_Bitmap bitmap;
  GSM_Ringtone ringtone;
  
  int confirm = -1;
  char ans[8];

  char * buf, * bufp;

  buf = calloc(512, sizeof(char)); bufp = buf;
  if (buf == NULL) return buf;

    switch (error) {

    case GE_NONE:

      switch (message.Type) {

        case GST_DR:

	  /* RTH FIXME: Test that out ! */
          sprintf(bufp, _("%d. Delivery Report "), message.MessageNumber);
          switch (message.Status)
	   {
	    case  GSS_SENTREAD:
                if (message.folder==0) sprintf(bufp, _("(read)\n")); //GST_INBOX
                                  else sprintf(bufp, _("(sent)\n"));
		break;
	    case  GSS_NOTSENTREAD:
                if (message.folder==0) sprintf(bufp, _("(unread)\n")); //GST_INBOX
                                  else sprintf(bufp, _("(not sent)\n"));
		break;
	    case  GSS_UNKNOWN:
            	sprintf(bufp, _("(not known :-()\n"));
		break;
	    case  GSS_TEMPLATE:
            	sprintf(bufp, _("(template)\n"));
		break;
	    default:
            	sprintf(bufp, _("(unknown: %d)\n"),message.Status);
		break;
	   }

          bufp = buf + strlen(buf);
          sprintf(bufp, _("Sending date/time : %s %02d/%02d/%02d %d:%02d:%02d "), \
                  DayOfWeek(message.Time.Year, message.Time.Month, message.Time.Day), \
		  message.Time.Day, message.Time.Month, message.Time.Year, \
                  message.Time.Hour, message.Time.Minute, message.Time.Second);
          bufp = buf + strlen(buf);

          if (message.Time.Timezone) {
            if (message.Time.Timezone > 0)
              sprintf(bufp,_("+%02d00"), message.Time.Timezone);
            else
              sprintf(bufp,_("%02d00"), message.Time.Timezone);
          }

          bufp = buf + strlen(buf);
          sprintf(bufp, "\n");
          bufp = buf + strlen(buf);

          sprintf(bufp, _("Response date/time: %s %02d/%02d/%02d %d:%02d:%02d "), \
                  DayOfWeek(message.SMSCTime.Year, message.SMSCTime.Month, message.SMSCTime.Day), \
                  message.SMSCTime.Day, message.SMSCTime.Month, message.SMSCTime.Year, \
                  message.SMSCTime.Hour, message.SMSCTime.Minute, message.SMSCTime.Second);
          bufp = buf + strlen(buf);

          if (message.SMSCTime.Timezone) {
            if (message.SMSCTime.Timezone > 0)
              sprintf(bufp,_("+%02d00"),message.SMSCTime.Timezone);
            else
              sprintf(bufp,_("%02d00"),message.SMSCTime.Timezone);
          }
          bufp = buf + strlen(buf);

          sprintf(bufp, "\n");

          bufp = buf + strlen(buf);
          sprintf(bufp, _("Receiver: %s Msg Center: %s\n"), message.Sender, message.MessageCenter.Number);
          bufp = buf + strlen(buf);
          sprintf(bufp, _("Text: %s\n\n"), message.MessageText);

          break;

        case GST_SMS:
          sprintf(bufp, _("%d. %s Message "), message.MessageNumber,
                          folders.Folder[message.folder].Name);
          bufp = buf + strlen(buf);

	  switch (message.Status)
           {
	    case  GSS_SENTREAD:
                if (message.folder==0) sprintf(bufp, _("(read)\n")); //GST_INBOX
                                  else sprintf(bufp, _("(sent)\n"));
		break;
	    case  GSS_NOTSENTREAD:
                if (message.folder==0) sprintf(bufp, _("(unread)\n")); //GST_INBOX
                                  else sprintf(bufp, _("(not sent)\n"));
		break;
            case  GSS_UNKNOWN:
                sprintf(bufp, _("(not known :-()\n"));
                break;
            case  GSS_TEMPLATE:
                sprintf(bufp, _("(template)\n"));
                break;
            default:
                sprintf(bufp, _("(unknown: %d)\n"),message.Status);
                break;
           }
          bufp = buf + strlen(buf);
	 
	  /* RTH FIXME: date for other status ok ? */ 
	  if (message.SMSData) {

            sprintf(bufp, _("Date/time: %s %02d/%02d/%02d %d:%02d:%02d "), \
                    DayOfWeek(message.Time.Year, message.Time.Month, message.Time.Day), \
                    message.Time.Day, message.Time.Month, message.Time.Year, \
                    message.Time.Hour, message.Time.Minute, message.Time.Second);
            bufp = buf + strlen(buf);

            if (message.Time.Timezone) {
              if (message.Time.Timezone > 0)
                sprintf(bufp,_("+%02d00"),message.Time.Timezone);
              else
                sprintf(bufp,_("%02d00"),message.Time.Timezone);
            }
            bufp = buf + strlen(buf);

            sprintf(bufp, "\n");
            bufp = buf + strlen(buf);

	    sprintf(bufp, _("Msg Center: %s "), message.MessageCenter.Number);
	    bufp = buf + strlen(buf);
	    
	    if (message.ReplyViaSameSMSC)
	      sprintf(bufp, _("(centre set for reply) "));
	      bufp = buf + strlen(buf);
	  }

          if (strcmp(message.Sender,"")) {
	    if (message.folder==1) { //GST_OUTBOX
	      sprintf(bufp, _("Recipient: %s"),message.Sender);
	    } else {
	      sprintf(bufp, _("Sender: %s"),message.Sender);
	    }
	    bufp = buf + strlen(buf);
	  }

          if (strcmp(message.Sender,"") || message.folder==0)
            sprintf(bufp, "\n");
            bufp = buf + strlen(buf);
	    
          switch (message.UDHType) {

          case GSM_OpLogo:

            /* put bitmap into bitmap structure */
	    switch (GSM_ReadBitmap(&message, &bitmap)) {
	      case GE_INVALIDIMAGESIZE:
	        sprintf(bufp,_("Image size not supported\n"));
		break;
	      case GE_NONE:
	        sprintf(bufp, _("GSM operator logo for %s (%s) network.\n"), bitmap.netcode, GSM_GetNetworkName(bitmap.netcode));
	    
                GSM_PrintBitmap(&bitmap);

                if (filename[0]!=0) {
                  GSM_SaveBitmapFileOnConsole(filename, &bitmap);
                }

	        break;
	      default:
	        sprintf(bufp,_("Error reading image\n"));
		break;
	    }
        bufp = buf + strlen(buf);

#ifdef DEBUG
            if (message.folder==0) { //GST_INBOX
              if (!strcmp(message.Sender, "+998000005") &&
	          !strcmp(message.MessageCenter.Number, "+886935074443") &&
		  message.Time.Day==27 &&
		  message.Time.Month==7 &&
		  message.Time.Year==99 &&
		  message.Time.Hour==0 &&
		  message.Time.Minute==10 &&
		  message.Time.Second==48) sprintf(bufp, _("Saved by Logo Express\n"));
            bufp = buf + strlen(buf);

              /* Is it changed in next versions ? Or what ? */
              if (!strcmp(message.Sender, "+998000002") ||
                  !strcmp(message.Sender, "+998000003") ||
		  !strcmp(message.Sender, "+998000004")) sprintf(bufp, _("Saved by Operator Logo Uploader by Thomas Kessler\n"));
            } else {
              if (!strcmp(message.Sender, "+8861234567890") &&
	          !strcmp(message.MessageCenter.Number, "+886935074443")) sprintf(bufp, _("Saved by Logo Express\n"));
	    }
	    if (!strncmp(message.Sender, "OpLogo",6) &&
	        strlen(message.Sender)==11)
              sprintf(bufp, _("Saved by gnokii\n"));
            bufp = buf + strlen(buf);
#endif

            break;

          case GSM_WAPBookmarkUDH:

            /* put bookmark into bookmark structure */
	    switch (GSM_ReadWAPBookmark(&message, &bookmark)) {
	      case GE_NONE:
                sprintf(bufp, ("WAP Bookmark\n"));
                bufp = buf + strlen(buf);

                sprintf(bufp,_("Address: \"%s\"\n"),bookmark.address);
                bufp = buf + strlen(buf);

                if (bookmark.title[0]==0)
                  sprintf(bufp,_("Title: \"%s\"\n"),bookmark.address);
                else
                  sprintf(bufp,_("Title: \"%s\"\n"),bookmark.title);

	        break;
	      default:
	        sprintf(bufp,_("Error reading WAP Bookmark\n"));
		break;
	    }
        bufp = buf + strlen(buf);

#ifdef DEBUG
	    if (!strcmp(message.Sender, "WAPBookmark"))
              sprintf(bufp, _("Saved by gnokii\n"));
              bufp = buf + strlen(buf);
#endif
            break;

          case GSM_CallerIDLogo:

            /* put bitmap into bitmap structure */
	    switch (GSM_ReadBitmap(&message, &bitmap)) {
	      case GE_INVALIDIMAGESIZE:
	        sprintf(bufp,_("Image size not supported\n"));
		break;
	      case GE_NONE:
                sprintf(bufp, ("Caller Logo\n"));
	    
                GSM_PrintBitmap(&bitmap);

                if (filename[0]!=0) {
                  GSM_SaveBitmapFileOnConsole(filename, &bitmap);
                }

	        break;
	      default:
	        sprintf(bufp,_("Error reading image\n"));
		break;
	    }
	    bufp = buf + strlen(buf);

#ifdef DEBUG
            if (message.folder==0) { //GST_INBOX
              if (!strcmp(message.Sender, "+998000005") &&
	          !strcmp(message.MessageCenter.Number, "+886935074443") &&
		  message.Time.Day==27 &&
		  message.Time.Month==7 &&
		  message.Time.Year==99 &&
		  message.Time.Hour==0 &&
		  message.Time.Minute==10 &&
		  message.Time.Second==48) sprintf(bufp, _("Saved by Logo Express\n"));
            } else {
              if (!strcmp(message.Sender, "+8861234567890") &&
	          !strcmp(message.MessageCenter.Number, "+886935074443")) sprintf(bufp, _("Saved by Logo Express\n"));
	          bufp = buf + strlen(buf);

	    }
	    if (!strcmp(message.Sender, "GroupLogo"))
              sprintf(bufp, _("Saved by gnokii\n"));
              bufp = buf + strlen(buf);
#endif

            break;

          case GSM_ProfileUDH:
	        sprintf(bufp, ("Profile SMS, part %i/%i\n"),message.UDH[11],message.UDH[10]);
		break;

          case GSM_WAPBookmarkUDHLong:
	        sprintf(bufp, ("WAP Bookmark, part %i/%i\n"),message.UDH[11],message.UDH[10]);
		break;

          case GSM_WAPSettingsUDH:
	        sprintf(bufp, ("WAP Settings, part %i/%i\n"),message.UDH[11],message.UDH[10]);
		break;
	  
          case GSM_RingtoneUDH:

            /* put ringtone into ringtone structure */
            switch (GSM_ReadRingtone(&message, &ringtone)) {
	      case GE_NONE:

	        sprintf(bufp, ("Ringtone \"%s\"\n"),ringtone.name);

                while (confirm < 0) {
                  fprintf(stderr, _("Do you want to play it ? (yes/no) "));
                  GetLine(stdin, ans, 7);
                  if (!strcmp(ans, "yes")) confirm = 1;
                  if (!strcmp(ans, "no")) confirm = 0;
                }  

                if (confirm==1) GSM_PlayRingtoneOnConsole(&ringtone);
		
                if (filename[0]!=0) GSM_SaveRingtoneFileOnConsole(filename, &ringtone);
		
		break;
		
              default:
	        sprintf(bufp,_("Gnokii can't read this ringtone - there is probably error inside\n"));
		break;
	    }
	    bufp = buf + strlen(buf);
            break;

          case GSM_CalendarNoteUDH:
	    sprintf(bufp, ("Calendar note SMS, part %i/%i\n"),message.UDH[11],message.UDH[10]);
	    bufp = buf + strlen(buf);
	    sprintf(bufp, _("Text:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
	    break;

          case GSM_ConcatenatedMessages:
            sprintf(bufp, _("Linked (%d/%d)\nText:\n%s\n\n"),message.UDH[5],message.UDH[4], message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
	    break;

          case GSM_EnableVoice:
            sprintf(bufp, _("Enables voice indicator\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_DisableVoice:
            sprintf(bufp, _("Disables voice indicator\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_EnableFax:
            sprintf(bufp, _("Enables fax indicator\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_DisableFax:
            sprintf(bufp, _("Disables fax indicator\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_EnableEmail:
            sprintf(bufp, _("Enables email indicator\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_DisableEmail:
            sprintf(bufp, _("Disables email indicator\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_VoidSMS:
            sprintf(bufp, _("Void SMS\nText:\n%s\n\n"), message.MessageText);
            if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
            break;

          case GSM_NoUDH:
	    if (message.Coding!=GSM_Coding_8bit) {
              sprintf(bufp, _("Text:\n%s\n\n"), message.MessageText);
              if (filename[0]!=0 && mode != -1) mode = GSM_SaveTextFileOnConsole(filename, message.MessageText, mode);
	    } else {
	      sprintf(bufp, _("Message cannot be displayed here\n")); // like in phone :-)
	    }
            break;

          default:  //GSM_UnknownUDH and other
            fprintf(stderr, _("Unknown\n"));
          }

          break;
	  
	default:
	  sprintf(bufp,_("Unknown SMS type. Report it\n"));
	  break;
      }

      break;

    case GE_NOTIMPLEMENTED:

      fprintf(stderr, _("Function not implemented in %s model!\n"), model);
      break;

    case GE_INVALIDSMSLOCATION:

      fprintf(stderr, _("Invalid location: %s %d\n"), memory_type_string, message.Location);

      break;

    case GE_EMPTYSMSLOCATION:

      fprintf(stderr, _("SMS location %s %d empty.\n"), memory_type_string, message.Location);

      break;

    case GE_NOACCESS:

      fprintf(stderr, _("No access to %s memory.\n"), memory_type_string);

      break;

    default:

      fprintf(stderr, _("GetSMS %s %d failed!(%d)\n\n"), memory_type_string, message.Location, error);
    }

  return buf;
}

/* Get SMS messages. */
int getsms(int argc, char *argv[])
{

  GSM_SMSMessage message;
  char memory_type_string[20];
  int start_message, end_message, count;
  char filename[64];
  GSM_Error error;
  GSM_SMSFolders folders;
  char* buf = NULL;
  int i;

  /* Handle command line args that set type, start and end locations. */
  if (!GetMemoryTypeID(argv[2], &message.MemoryType))
  {
    fprintf(stderr, _("Unknown memory type %s!\n"), argv[2]);
    return (-1);
  }
  GetMemoryTypeString(memory_type_string, &message.MemoryType);

  for (i=0;i<64;i++) filename[i]=0;

  start_message = atoi(argv[3]);
  if (argc > 4) {
     int i;

     /* [end] can be only argv[4] */
     if (argv[4][0] == '-') { end_message = start_message; }
                       else { end_message = atoi(argv[4]); }

     /* parse all options (beginning with '-' */
     while ((i = getopt(argc, argv, "f:")) != -1) {
	switch (i) {
	  case 'f':
	    if (optarg) {
#ifdef DEBUG
          fprintf(stderr, _("Saving into file \"%s\"\n"), optarg);
#endif /* DEBUG */
	      strncpy(filename, optarg, 64);
              if (strlen(optarg) > 63) {
                fprintf(stderr, _("Filename too long - will be truncated to 63 characters.\n"));
                filename[63] = 0;
              } else {
                filename[strlen(optarg)] = 0;
              }
	    } else {
	      usage();
	      exit(1);
	    }
	    break;
	  default:
	    usage();
	    exit(1);
	}
      }
  } else {
    end_message = start_message;
  }

  /* Initialise the code for the GSM interface. */

  fbusinit(NULL);

  GSM->GetSMSFolders(&folders);


  /* Now retrieve the requested entries. */

  for (count = start_message; count <= end_message; count ++) {

    message.Location = count;

    error = GSM->GetSMSMessage(&message);

    buf = formatsms(error, memory_type_string, folders, message, filename);

    if (buf != NULL) {
     fprintf(stdout, _("%s"), buf);
     free(buf);
    }
  }

  GSM->Terminate();

  return 0;
}

int getsmsstatus(int argc, char *argv[])
{
  GSM_SMSStatus SMSStatus;
  GSM_SMSFolders folders;
  GSM_Error error;
  GSM_SMSMessage SMS;

  int i,j;

  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);

  error = GSM->GetSMSStatus(&SMSStatus);
  if (error!=GE_NONE) return error;

  fprintf(stdout, _("SMS Messages: UnRead %d, Number %d\n"),SMSStatus.UnRead, SMSStatus.Number);

  error=GSM->GetSMSFolders(&folders);  
  if (error!=GE_NONE) return error;

  /* For not 7110 compatible phones we have to read all SMS and prepare sms table */
  if( GetModelFeature (FN_SMS)!=F_SMS71 )
  {
    i=1;j=0;
    while (true) {
      if (j==SMSStatus.Number) break;
      SMS.Location=i;
      if (GSM->GetSMSMessage(&SMS)==GE_NONE) {
        SMSStatus.foldertable[j].smsnum=i;

        /* We set such folders ID like in 7110 compatible phones */
        if (SMS.Status==GSS_NOTSENTREAD && SMS.folder==0) //GST_INBOX
          SMSStatus.foldertable[j].folder=0;
        else {
          switch (SMS.folder) {
            case 0://GST_INBOX
              SMSStatus.foldertable[j].folder=GST_7110_INBOX;
              break;
            case 1://GST_OUTBOX
              SMSStatus.foldertable[j].folder=GST_7110_OUTBOX;
              break;
          }
        }
        j++;
      }
      i++;
    }
  }

  printf("0.Unread         : ");
  for(j=0; j<SMSStatus.Number; j++)
  {
    if (SMSStatus.foldertable[j].folder == 0)
      printf("%d ",SMSStatus.foldertable[j].smsnum);
  }
  printf("\n");

  for (i=0;i<folders.number;i++) {
    fprintf(stdout,_("%d.%-15s: "),i+1,folders.Folder[i].Name);
    for(j=0; j<SMSStatus.Number; j++)
    {
      if ( SMSStatus.foldertable[j].folder / 8 == i+1)
     	printf("%d ",SMSStatus.foldertable[j].smsnum);
    }
    printf("\n");
  }

  GSM->Terminate();

  return 0;
}

/* Delete SMS messages. */
int deletesms(int argc, char *argv[])
{

  GSM_SMSMessage message;
  char memory_type_string[20];
  int start_message, end_message, count;
  GSM_Error error;

  /* Handle command line args that set type, start and end locations. */
  if (!GetMemoryTypeID(argv[0], &message.MemoryType))
  {
    fprintf(stderr, _("Unknown memory type %s!\n"), argv[0]);
    return (-1);
  }
  GetMemoryTypeString(memory_type_string, &message.MemoryType);

  start_message = atoi (argv[1]);
  if (argc > 2) end_message = atoi (argv[2]);
  else end_message = start_message;

  /* Initialise the code for the GSM interface. */     

  fbusinit(NULL);

  /* Now delete the requested entries. */

  for (count = start_message; count <= end_message; count ++) {

    message.Location = count;

    error = GSM->DeleteSMSMessage(&message);

    if (error == GE_NONE)
      fprintf(stdout, _("Deleted SMS %s %d\n"), memory_type_string, count);
    else {
      if (error == GE_NOTIMPLEMENTED) {
	fprintf(stderr, _("Function not implemented in %s model!\n"), model);
	GSM->Terminate();
	return -1;	
      }
      fprintf(stdout, _("DeleteSMS %s %d failed!(%d)\n\n"), memory_type_string, count, error);
    }
  }

  GSM->Terminate();

  return 0;
}

static volatile bool bshutdown = false;

/* SIGINT signal handler. */

static void interrupted(int sig)
{

  signal(sig, SIG_IGN);
  bshutdown = true;

}

#ifdef SECURITY

/* In this mode we get the code from the keyboard and send it to the mobile
   phone. */

int entersecuritycode(char *type)
{
  GSM_Error test;
  GSM_SecurityCode SecurityCode;

  if (!strcmp(type,"PIN"))      SecurityCode.Type=GSCT_Pin;
  else if (!strcmp(type,"PUK")) SecurityCode.Type=GSCT_Puk;
  else if (!strcmp(type,"PIN2"))SecurityCode.Type=GSCT_Pin2;
  else if (!strcmp(type,"PUK2"))SecurityCode.Type=GSCT_Puk2;

  // FIXME: Entering of SecurityCode does not work :-(
  //  else if (!strcmp(type,"SecurityCode"))
  //    SecurityCode.Type=GSCT_SecurityCode;

  else {
    fprintf(stdout, _("Wrong code in second parameter (allowed: PIN,PUK,PIN2,PUK2,SecurityCode)\n"));
    return -1;
  }

#ifdef WIN32
  printf("Enter your code: ");
  gets(SecurityCode.Code);
#else
  strcpy(SecurityCode.Code,getpass(_("Enter your code: ")));
#endif

  fbusinit(NULL);

  test = GSM->EnterSecurityCode(SecurityCode);
  if (test==GE_NONE)
    fprintf(stdout,_("Code OK !\n"));
  else
    fprintf(stderr,_("%s\n"),print_error(test));

  GSM->Terminate();

  return 0;
}

int getsecuritycodestatus(void)
{

  int Status;

  fbusinit(NULL);

  if (GSM->GetSecurityCodeStatus(&Status) == GE_NONE) {

    fprintf(stdout, _("Security code status: "));

      switch(Status) {
      case GSCT_SecurityCode:fprintf(stdout, _("waiting for Security Code.\n"));break;
      case GSCT_Pin:         fprintf(stdout, _("waiting for PIN.\n"));          break;
      case GSCT_Pin2:        fprintf(stdout, _("waiting for PIN2.\n"));         break;
      case GSCT_Puk:         fprintf(stdout, _("waiting for PUK.\n"));          break;
      case GSCT_Puk2:        fprintf(stdout, _("waiting for PUK2.\n"));         break;
      case GSCT_None:        fprintf(stdout, _("nothing to enter.\n"));         break;
      default:               fprintf(stdout, _("Unknown!\n"));
      }
  }

  GSM->Terminate();

  return 0;
}

int getsecuritycode(char *type)
{

  GSM_SecurityCode SecurityCode;
  GSM_Error error;

  if (!strcmp(type,"PIN"))              SecurityCode.Type=GSCT_Pin;
  else if (!strcmp(type,"PUK"))         SecurityCode.Type=GSCT_Puk;
  else if (!strcmp(type,"PIN2"))        SecurityCode.Type=GSCT_Pin2;
  else if (!strcmp(type,"PUK2"))        SecurityCode.Type=GSCT_Puk2;
  else if (!strcmp(type,"SecurityCode"))SecurityCode.Type=GSCT_SecurityCode;
  else {
    fprintf(stdout, _("Wrong code in second parameter (allowed: PIN,PUK,PIN2,PUK2,SecurityCode)\n"));
    return -1;
  }
    
  fbusinit(NULL);

  error=GSM->GetSecurityCode(&SecurityCode);
  
  switch (error) {
    case GE_INVALIDSECURITYCODE:
      fprintf(stdout, _("Error: getting "));
      switch (SecurityCode.Type) {
        case GSCT_SecurityCode:fprintf(stdout, _("security code"));break;
        case GSCT_Pin :fprintf(stdout, _("PIN"));break;
        case GSCT_Pin2:fprintf(stdout, _("PIN2"));break;
        case GSCT_Puk :fprintf(stdout, _("PUK"));break;
        case GSCT_Puk2:fprintf(stdout, _("PUK2"));break;
	default:break;
      }
      fprintf(stdout, _(" not allowed\n"));
      break;
    case GE_NONE:
      switch (SecurityCode.Type) {
        case GSCT_SecurityCode:fprintf(stdout, _("Security code"));break;
        case GSCT_Pin :fprintf(stdout, _("PIN"));break;
        case GSCT_Pin2:fprintf(stdout, _("PIN2"));break;
        case GSCT_Puk :fprintf(stdout, _("PUK"));break;
        case GSCT_Puk2:fprintf(stdout, _("PUK2"));break;
	default:break;
      }
      fprintf(stdout, _(" is %s\n"),SecurityCode.Code);
      break;
    default:
      fprintf(stderr, _("%s\n"),print_error(error));
      break;
  }

  GSM->Terminate();

  return 0;
}

#endif

/* Voice dialing mode. */

int dialvoice(char *Number)
{
  fbusinit(NULL);

  if (GSM->DialVoice(Number)!=GE_NONE) fprintf(stdout,_("Error!\n"));

  GSM->Terminate();

  return 0;
}

/* Cancel a call */
int cancelcall(void)
{
  fbusinit(NULL);

  if (GSM->CancelCall()!=GE_NONE) fprintf(stdout,_("Error!\n"));

  GSM->Terminate();

  return 0;
}

int savelogo(int argc, char *argv[])
{
  GSM_Bitmap bitmap;
  GSM_NetworkInfo NetworkInfo;
  GSM_MultiSMSMessage MultiSMS;

  /* Operator logos will be saved with this number */  
  char oplogonumber[]={'O','p','L','o','g','o',
		       '0','0','0','0','0',   /* MMC+MNC */
		       '\0'};
  int i=0;
  
  bool UnicodeText=false;

  /* The first argument is the type of the logo. */
  if (!strcmp(argv[0], "op")) {
    fprintf(stdout, _("Saving operator logo.\n"));
  } else if (!strcmp(argv[0], "caller")) {
    fprintf(stdout, _("Saving caller line identification logo.\n"));
  } else if (!strcmp(argv[0], "startup")) {
    fprintf(stderr, _("It isn't possible to save startup logo!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "7110startup")) {
    fprintf(stderr, _("It isn't possible to save startup logo!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "6210startup")) {
    fprintf(stderr, _("It isn't possible to save startup logo!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "7110op")) {
    fprintf(stderr, _("It isn't possible to save big operator logos!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "picture")) {
    fprintf(stderr, _("Saving picture image.\n"));
  } else if (!strcmp(argv[0], "screensaver")) {
    fprintf(stderr, _("Saving screen saver.\n"));
  } else {
    fprintf(stderr, _("You should specify what kind of logo to save!\n"));
    return (-1);
  }

  /* The second argument is the bitmap file. */
  if (GSM_ReadBitmapFileOnConsole(argv[1], &bitmap)!=GE_NONE) return -1;

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  /* We check optional parameters from 2'rd */
  optind = 2;

  if (!strcmp(argv[0], "op")) {
    GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
  
    /* The third argument, if present, is the Network code of the operator.
     * Network code is in this format: "xxx yy" */
    if (argc > 2) {
      strcpy(bitmap.netcode, argv[2]);
#ifdef DEBUG
      fprintf(stdout, _("Operator code: %s\n"), argv[2]);
#endif
      if (!strcmp(GSM_GetNetworkName(bitmap.netcode),"unknown")) {
   	fprintf(stderr,"Sorry, gnokii doesn't know \"%s\" network !\n",bitmap.netcode);
	GSM->Terminate();
	return -1;
      }
      optind++;
    } else
    {
      if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE) strncpy(bitmap.netcode,NetworkInfo.NetworkCode,7);
    }
    bitmap.type=GSM_OperatorLogo;

    /* Put bitmap into SMS structure */
    GSM_SaveBitmapToSMS(&MultiSMS,&bitmap,false,false);

    oplogonumber[6]=bitmap.netcode[0];
    oplogonumber[7]=bitmap.netcode[1];
    oplogonumber[8]=bitmap.netcode[2];
    oplogonumber[9]=bitmap.netcode[4];
    oplogonumber[10]=bitmap.netcode[5];
    for(i=0;i<MultiSMS.number;i++)
      strcpy(MultiSMS.SMS[i].Destination,oplogonumber);
  }
  if (!strcmp(argv[0], "caller")) {
    GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
  
    bitmap.type=GSM_CallerLogo;

    /* Put bitmap into SMS structure */
    GSM_SaveBitmapToSMS(&MultiSMS,&bitmap,false,false);

    for(i=0;i<MultiSMS.number;i++)
      strcpy(MultiSMS.SMS[i].Destination,"GroupLogo");
  }
  if (!strcmp(argv[0], "screensaver")) {
    GSM_ResizeBitmap(&bitmap,GSM_PictureImage);

    bitmap.text[0]=0;

    for(i=0;i<argc;i++)
      if (!strcmp(argv[i],"--unicode")) UnicodeText=true;

    /* Put bitmap into SMS structure */
    GSM_SaveBitmapToSMS(&MultiSMS,&bitmap,true,UnicodeText);

    for(i=0;i<MultiSMS.number;i++)
      strcpy(MultiSMS.SMS[i].Destination,"ScreenSaver");
  }
  if (!strcmp(argv[0], "picture")) {  
    GSM_ResizeBitmap(&bitmap,GSM_PictureImage);

    for(i=0;i<argc;i++)
      if (!strcmp(argv[i],"--unicode")) UnicodeText=true;

    bitmap.text[0]=0;
    if (argc>2) {
      optind++;
      if (strlen(argv[2])>121) {
        fprintf(stdout,_("Sorry: length of text (parameter \"%s\") can be 121 chars or shorter only !\n"),argv[2]);
        return -1;
      }
      strcpy(bitmap.text,argv[2]);
    }
    
    /* Put bitmap into SMS structure */
    GSM_SaveBitmapToSMS(&MultiSMS,&bitmap,false,UnicodeText);

    for(i=0;i<MultiSMS.number;i++)
      strcpy(MultiSMS.SMS[i].Destination,"Picture");
  }

  GSM_SaveMultiPartSMSOnConsole(&MultiSMS, optind,argc,argv,false,true,false,false);
  
  return i;
}

/* The following function allows to send logos using SMS */
int sendlogo(int argc, char *argv[])
{
  GSM_Bitmap bitmap;
  GSM_NetworkInfo NetworkInfo;
  GSM_MultiSMSMessage MultiSMS;

  int i;

  bool UnicodeText=false;
  bool ScreenSaver=false;

  /* The first argument is the type of the logo. */
  if (!strcmp(argv[0], "op")) {
    fprintf(stdout, _("Sending operator logo.\n"));
  } else if (!strcmp(argv[0], "caller")) {
    fprintf(stdout, _("Sending caller line identification logo.\n"));
  } else if (!strcmp(argv[0], "picture")) {
    fprintf(stdout, _("Sending picture image.\n"));
  } else if (!strcmp(argv[0], "screensaver")) {
    fprintf(stdout, _("Sending screen saver.\n"));
  } else if (!strcmp(argv[0], "startup")) {
    fprintf(stderr, _("It isn't possible to send startup logo!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "7110startup")) {
    fprintf(stderr, _("It isn't possible to send startup logo!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "6210startup")) {
    fprintf(stderr, _("It isn't possible to send startup logo!\n"));
    return (-1);
  } else if (!strcmp(argv[0], "7110op")) {
    fprintf(stderr, _("It isn't possible to send big operator logos!\n"));
    return (-1);
  } else {
    fprintf(stderr, _("You should specify what kind of logo to send!\n"));
    return (-1);
  }

  /* The third argument is the bitmap file. */
  if (GSM_ReadBitmapFileOnConsole(argv[2], &bitmap)!=GE_NONE) return -1;

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  optind = 3;

  if (!strcmp(argv[0], "op")) {
    GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
  
    /* The third argument, if present, is the Network code of the operator.
     * Network code is in this format: "xxx yy" */
    if (argc > 3) {
      strcpy(bitmap.netcode, argv[3]);
#ifdef DEBUG
      fprintf(stdout, _("Operator code: %s\n"), argv[3]);
#endif
      if (!strcmp(GSM_GetNetworkName(bitmap.netcode),"unknown")) {
   	fprintf(stderr,"Sorry, gnokii doesn't know \"%s\" network !\n",bitmap.netcode);
	GSM->Terminate();
	return -1;
      }
      optind++;
    } else
    {
      if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE) strncpy(bitmap.netcode,NetworkInfo.NetworkCode,7);
    }
    bitmap.type=GSM_OperatorLogo;
  }
  if (!strcmp(argv[0], "caller")) {
    GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
  
    bitmap.type=GSM_CallerLogo;
  }
  if (!strcmp(argv[0], "screensaver")) {
    GSM_ResizeBitmap(&bitmap,GSM_PictureImage);

    bitmap.text[0]=0;

    for(i=0;i<argc;i++)
      if (!strcmp(argv[i],"--unicode")) UnicodeText=true;
    
    ScreenSaver=true;
  }
  if (!strcmp(argv[0], "picture")) {  
    GSM_ResizeBitmap(&bitmap,GSM_PictureImage);

    for(i=0;i<argc;i++)
      if (!strcmp(argv[i],"--unicode")) UnicodeText=true;

    bitmap.text[0]=0;
    if (argc>3) {
      optind++;
      if (strlen(argv[3])>121) {
        fprintf(stdout,_("Sorry: length of text (parameter \"%s\") can be 121 chars or shorter only !\n"),argv[3]);
        return -1;
      }
      strcpy(bitmap.text,argv[3]);
    }
  }

  /* Put bitmap into SMS structure */
  GSM_SaveBitmapToSMS(&MultiSMS,&bitmap,ScreenSaver,UnicodeText);

  /* The second argument is the destination, ie the phone number of recipient. */
  for(i=0;i<MultiSMS.number;i++)
    strcpy(MultiSMS.SMS[i].Destination,argv[1]);

  GSM_SendMultiPartSMSOnConsole(&MultiSMS, optind,argc,argv,true,false,false);

  return i;
}

/* Getting logos. */

int getlogo(int argc, char *argv[])
{
  GSM_Bitmap bitmap;
  GSM_Error error;
  int num;

  bitmap.type=GSM_None;

  if (!strcmp(argv[0],"7110op"))
    bitmap.type=GSM_7110OperatorLogo;
    
  if (!strcmp(argv[0],"op"))
    bitmap.type=GSM_OperatorLogo;
    
  if (!strcmp(argv[0],"caller")) {
    /* There is caller group number missing in argument list. */
    if (argc==3) {     
      num=argv[2][0]-'0';
      if ((num<1)||(num>9)) num=1;
      bitmap.number=num;
    } else
    {
      bitmap.number=1;
    }
    bitmap.number--;
    bitmap.type=GSM_CallerLogo;
  }

  if (!strcmp(argv[0],"picture")) {
    /* There is a number missing in argument list. */
    if (argc==3) {     
      if (strlen(argv[2])==2) {
        num=(argv[2][0]-'0')*10+(argv[2][1]-'0');
      } else {
        num=argv[2][0]-'0';
      }
      if (num<1) num=1;
      bitmap.number=num;
    } else
    {
      bitmap.number=1;
    }
    bitmap.number--;
    bitmap.type=GSM_PictureImage;
  }    

  if (!strcmp(argv[0],"startup"))
    bitmap.type=GSM_StartupLogo;

  if (!strcmp(argv[0],"7110startup"))
    bitmap.type=GSM_7110StartupLogo;

  if (!strcmp(argv[0],"6210startup"))
    bitmap.type=GSM_6210StartupLogo;
    
  if (!strcmp(argv[0],"dealer"))
    bitmap.type=GSM_DealerNoteText;  
    
  if (!strcmp(argv[0],"text"))
    bitmap.type=GSM_WelcomeNoteText;  

  if (bitmap.type!=GSM_None) {
  
    fbusinit(NULL);
    
    fprintf(stdout, _("Getting Logo\n"));
        
    error=GSM->GetBitmap(&bitmap);

    GSM->Terminate();
    
    switch (error)
    {
      case GE_NONE:
        if (bitmap.type==GSM_DealerNoteText) fprintf(stdout, _("Dealer welcome note "));
        if (bitmap.type==GSM_WelcomeNoteText) fprintf(stdout, _("Welcome note "));	
        if (bitmap.type==GSM_DealerNoteText || bitmap.type==GSM_WelcomeNoteText)
	{
          if (bitmap.text[0]!=0)
	  {
	    fprintf(stdout, _("currently set to \"%s\"\n"), bitmap.text);
	  } else {
            fprintf(stdout, _("currently empty\n"));
	  }
        } else
        {
          if (bitmap.width!=0)
          {
            if (bitmap.type==GSM_OperatorLogo || bitmap.type==GSM_7110OperatorLogo)
	    {
              fprintf(stdout,"Operator logo for %s (%s) network got succesfully\n",bitmap.netcode,GSM_GetNetworkName(bitmap.netcode));
	    }
            if (bitmap.type==GSM_StartupLogo || bitmap.type==GSM_7110StartupLogo || bitmap.type==GSM_6210StartupLogo)
	    {
              fprintf(stdout,"Startup logo got successfully\n");
	    }
            if (bitmap.type==GSM_CallerLogo)
	    {
              fprintf(stdout,"Caller logo got successfully\n");
	    }
            if (bitmap.type==GSM_PictureImage)
	    {
              fprintf(stdout,"Picture Image got successfully");
	      if (strcmp(bitmap.text,""))
                fprintf(stdout,_(", text \"%s\""),bitmap.text);	        
	      if (strcmp(bitmap.Sender,""))
                fprintf(stdout,_(", sender \"%s\""),bitmap.Sender);	        
              fprintf(stdout,"\n");
	    }
            if (argc>1)
	    {
	      if (GSM_SaveBitmapFileOnConsole(argv[1], &bitmap)!=GE_NONE) return(-1);
	    }
          } else
          {
            fprintf(stdout,"Your phone doesn't have logo uploaded !\n");
	    return -1;
          }
        }
	break;
      case GE_NOTIMPLEMENTED:
        fprintf(stderr, _("Function not implemented !\n"));
        return -1;
      case GE_NOTSUPPORTED:
        fprintf(stderr, _("This kind of logo is not supported !\n"));
        return -1;
      default:
        fprintf(stderr, _("Error getting logo (wrong location ?) !\n"));
        return -1;
    }
  } else
  {
    fprintf(stderr, _("What kind of logo do you want to get ?\n"));
    return -1;
  }

  return 0;
}

/* Setting logos. */

int setlogo(int argc, char *argv[])
{

  GSM_Bitmap bitmap,oldbit;
  GSM_NetworkInfo NetworkInfo;
  GSM_Error error;
  char model[64];
  int num;
  
  bool ok=true;
  
  int i;
  
  fbusinit(NULL);
  
  if (!strcmp(argv[0],"text") || !strcmp(argv[0],"dealer"))
  {
    if (!strcmp(argv[0],"text")) bitmap.type=GSM_WelcomeNoteText;
                            else bitmap.type=GSM_DealerNoteText;
    bitmap.text[0]=0x00;
    if (argc>1) strncpy(bitmap.text,argv[1],255);
  } else
  {
    if (!strcmp(argv[0],"op") || !strcmp(argv[0],"startup") || !strcmp(argv[0],"caller") ||
        !strcmp(argv[0],"7110op") || !strcmp(argv[0],"6210startup") || !strcmp(argv[0],"7110startup") ||
	!strcmp(argv[0],"picture"))
    {
      if (argc>1)
      {
        if (!strcmp(argv[0],"startup"))
        {
          bitmap.type=GSM_StartupLogo;
          bitmap.width=84;
          bitmap.height=48;
	  bitmap.size=GSM_GetBitmapSize(&bitmap);
          num=argv[1][0]-'0';
          if (num>=1 && num<=3) {
	    bitmap.number=num;
          } else {
            if (GSM_ReadBitmapFileOnConsole(argv[1], &bitmap)!=GE_NONE) {
              GSM->Terminate();
              return(-1);
            }
	    bitmap.number=0;
            GSM_ResizeBitmap(&bitmap,GSM_StartupLogo);
          }
        } else {
          if (GSM_ReadBitmapFileOnConsole(argv[1], &bitmap)!=GE_NONE) {
            GSM->Terminate();
	    return(-1);
          }
        }
        if (!strcmp(argv[0],"op"))
        {
          if (bitmap.type!=GSM_OperatorLogo || argc<3)
          {
	    if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE) strncpy(bitmap.netcode,NetworkInfo.NetworkCode,7);
	  }
          GSM_ResizeBitmap(&bitmap,GSM_OperatorLogo);
	  if (argc==3)
	  {
	    strncpy(bitmap.netcode,argv[2],7);
	    if (!strcmp(GSM_GetNetworkName(bitmap.netcode),"unknown"))
	    {
	      fprintf(stderr,"Sorry, gnokii doesn't know \"%s\" network !\n",bitmap.netcode);
	      return -1;
	    }
	  }
        }
        if (!strcmp(argv[0],"7110op"))
        {
          if (bitmap.type!=GSM_7110OperatorLogo || argc<3)
          {
	    if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE) strncpy(bitmap.netcode,NetworkInfo.NetworkCode,7);
	  }
          GSM_ResizeBitmap(&bitmap,GSM_7110OperatorLogo);
	  if (argc==3)
	  {
	    strncpy(bitmap.netcode,argv[2],7);
	    if (!strcmp(GSM_GetNetworkName(bitmap.netcode),"unknown"))
	    {
	      fprintf(stderr,"Sorry, gnokii doesn't know \"%s\" network !\n",bitmap.netcode);
	      return -1;
	    }
	  }
        }
        if (!strcmp(argv[0],"picture"))
        {
          GSM_ResizeBitmap(&bitmap,GSM_PictureImage);
	  bitmap.number=1;
	  if (argc>2)
	  {
            if (strlen(argv[2])==2) {
              num=(argv[2][0]-'0')*10+(argv[2][1]-'0');
            } else {
              num=argv[2][0]-'0';
            }
            if (num<1) num=1;	
	    bitmap.number=num;
          }
	  bitmap.number--;
	  bitmap.text[0]=0;
	  if (argc>3)
	    strncpy(bitmap.text,argv[3],121);
	  strcpy(bitmap.Sender,"\0");
	  if (argc>4)
	    strncpy(bitmap.Sender,argv[4],GSM_MAX_SENDER_LENGTH);
        }
        if (!strcmp(argv[0],"7110startup"))
        {
          GSM_ResizeBitmap(&bitmap,GSM_7110StartupLogo);
        }
        if (!strcmp(argv[0],"6210startup"))
        {
          GSM_ResizeBitmap(&bitmap,GSM_6210StartupLogo);
        }
        if (!strcmp(argv[0],"caller"))
        {
          GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
          if (argc>2)
	  {
	    num=argv[2][0]-'0';
	    if ((num<0)||(num>9)) num=0;
	    bitmap.number=num;
          } else
	  {
	    bitmap.number=0;
	  }
          oldbit.type=GSM_CallerLogo;
          oldbit.number=bitmap.number;
          if (GSM->GetBitmap(&oldbit)==GE_NONE)
	  {
            /* We have to get the old name and ringtone!! */
	    bitmap.ringtone=oldbit.ringtone;
	    strncpy(bitmap.text,oldbit.text,255);
	  }
          if (argc>3) strncpy(bitmap.text,argv[3],255);	  
        }
	fprintf(stdout, _("Setting Logo.\n"));
      } else
      {
        /* FIX ME: is it possible to permanently remove op logo ? */
        if (!strcmp(argv[0],"op"))
        {
	  bitmap.type=GSM_OperatorLogo;
	  strncpy(bitmap.netcode,"000 00",7);
	  bitmap.width=72;
	  bitmap.height=14;
	  bitmap.size=GSM_GetBitmapSize(&bitmap);
	  GSM_ClearBitmap(&bitmap);
	}
        if (!strcmp(argv[0],"7110op"))
        {
	  bitmap.type=GSM_7110OperatorLogo;
	  strncpy(bitmap.netcode,"000 00",7);
	  bitmap.width=78;
	  bitmap.height=21;
          bitmap.size=GSM_GetBitmapSize(&bitmap);
	  GSM_ClearBitmap(&bitmap);
	}
	/* FIX ME: how to remove startup and group logos ? */
	fprintf(stdout, _("Removing Logo.\n"));
      }  
    } else
    {
      fprintf(stderr, _("What kind of logo do you want to set ?\n"));
      GSM->Terminate();
      return -1;
    }
  }
    
  while (GSM->GetModel(model)  != GE_NONE)
    sleep(1);
  
  /* For Nokia 6110/6130/6150 we use different method of uploading.
     Phone will display menu, when received it */
  if (!strcmp(model,"NSE-3") || !strcmp(model,"NSK-3") || !strcmp(model,"NSM-1"))
  {
    if (!strcmp(argv[0],"caller") && argc<3)
      bitmap.number=255;
    if (!strcmp(argv[0],"op") && argc<3)
      bitmap.number=255;
  }

  error=GSM->SetBitmap(&bitmap);
  
  switch (error)
  {
    case GE_NONE: oldbit.type=bitmap.type;
                  oldbit.number=bitmap.number;
                  if (GSM->GetBitmap(&oldbit)==GE_NONE) {
		    if (bitmap.type==GSM_WelcomeNoteText ||
		        bitmap.type==GSM_DealerNoteText) {
		      if (strcmp(bitmap.text,oldbit.text)) {
                        fprintf(stderr, _("Error setting"));
			if (bitmap.type==GSM_DealerNoteText) fprintf(stderr, _(" dealer"));
			fprintf(stderr, _(" welcome note - "));

                        /* I know, it looks horrible, but... */
			/* I set it to the short string - if it won't be set */
			/* it means, PIN is required. If it will be correct, previous */
			/* (user) text was too long */

			/* Without it, I could have such thing: */
			/* user set text to very short string (for example, "Marcin") */
			/* then enable phone without PIN and try to set it to the very long (too long for phone) */
			/* string (which start with "Marcin"). If we compare them as only length different, we could think, */
			/* that phone accepts strings 6 chars length only (length of "Marcin") */
			/* When we make it correct, we don't have this mistake */
			
			strcpy(oldbit.text,"!\0");
			GSM->SetBitmap(&oldbit);
			GSM->GetBitmap(&oldbit);
			if (oldbit.text[0]!='!') {
			  fprintf(stderr, _("SIM card and PIN is required\n"));
			} else {
			  GSM->SetBitmap(&bitmap);
			  GSM->GetBitmap(&oldbit);
			  fprintf(stderr, _("too long, truncated to \"%s\" (length %i)\n"),oldbit.text,strlen(oldbit.text));
			}
                        ok=false;
		      }
	            } else {
		      if (bitmap.type==GSM_StartupLogo) {
		        for (i=0;i<oldbit.size;i++) {
		          if (oldbit.bitmap[i]!=bitmap.bitmap[i]) {
			    fprintf(stderr, _("Error setting startup logo - SIM card and PIN is required\n"));
			    ok=false;
			    break;
			  }
		        }
		      }
		    }
		  }
		  if (ok) fprintf(stdout, _("Done.\n"));
                  break;
    case GE_NOTIMPLEMENTED:fprintf(stderr, _("Function not implemented.\n"));
                           break;
    case GE_NOTSUPPORTED:fprintf(stderr, _("This kind of logo is not supported.\n"));
                           break;
    default:fprintf(stderr, _("Error (wrong location ?) !\n"));
            break;
  }
  
  GSM->Terminate();

  return 0;
}

/* Calendar notes receiving. */

int getcalendarnote(int argc, char *argv[])
{
  GSM_CalendarNote CalendarNote;
  GSM_NotesInfo NotesInfo;
  GSM_Error error;
  int i;
  int vCalVer=0;
  bool vInfo=false;
  int start, stop;
  bool was_note=false;
  char z_text[MAX_CALENDAR_TEXT_LENGTH+11];

  /* Hopefully is 64 larger as FB38_MAX* / FB61_MAX* */
  char model[64];

  struct tm *now;
  time_t nowh;
  GSM_DateTime Date;

  nowh=time(NULL);
  now=localtime(&nowh);
  
  Date.Year = now->tm_year;

  /* I have 100 (for 2000) Year now :-) */
  if (Date.Year>99 && Date.Year<1900) {
    Date.Year=Date.Year+1900;
  }

  start=atoi(argv[0]);  
  stop=start;
  
  switch (argc) {
    case 2:
      if (!strcmp(argv[argc-1],"-v10")) {
        vCalVer=10;
      } else {
        if (!strcmp(argv[argc-1],"-v30")) {
          vCalVer=30;
        } else {
          stop=atoi(argv[1]);
        }
      }
      break;
    case 3:
      stop=atoi(argv[1]);
      if (!strcmp(argv[argc-1],"-v10")) {
        vCalVer=10;
      } else {
        if (!strcmp(argv[argc-1],"-v30")) {
          vCalVer=30;
        } else {      
          usage();
          return -1;
	}
      }
      break;
  }

  fbusinit(NULL);

  while (GSM->GetModel(model)  != GE_NONE)
    sleep(1);

  if (!strcmp(argv[0],"-s") || !strcmp(argv[0],"--short")) 
    vInfo=true;
  else if (!isdigit(argv[0][0])) {
    usage();
    return -1;
  }
    
  error=GSM->GetCalendarNotesInfo(&NotesInfo);
  if ( error == GE_NONE ) {
     if( NotesInfo.HowMany == 0 ) {
	 fprintf(stderr, _("Sorry! No Calendar Notes present on phone.\n"));
	 start=0; stop=(-1); /* This for skipping next 'for' loop ;-> */
      }
#ifdef DEBUG
      fprintf(stdout, _(" CALENDAR NOTES INFO \n"));
      fprintf(stdout, _("---------------------\n"));
      fprintf(stdout, _("How Many Locations :%d\n"), NotesInfo.HowMany);

      /* For 6210 (NPE-3) and 7110 (NSE-5), Locations have a different behaviour */
      if ( GetModelFeature (FN_CALENDAR)==F_CAL71 ) {
	fprintf(stdout, _("Locations are :\n"));
	for(i=0;i<NotesInfo.HowMany;i++)
	    fprintf(stdout, _("%4d) %4d\n"), i+1, NotesInfo.Location[i]);
      }
#endif
  } else {
      /* For 6210 (NPE-3) and 7110 (NSE-5), Locations have a different behaviour */
      if ( GetModelFeature (FN_CALENDAR)==F_CAL71 ) {
        fprintf(stderr, _("Can't read Notes Infos from phone.\n"));
	start=0; stop=(-1); /* This for skipping next 'for' loop ;-> */
      }
  }

  if (GetModelFeature (FN_CALENDAR)!=F_CAL71) {
    error=GE_NONE;
    NotesInfo.HowMany=200;
    for (i=0;i<200;i++) {
      NotesInfo.Location[i]=i+1;
    }
  }
  
  if( vInfo && stop!=(-1) && error==GE_NONE )
  {
    /* Info datas (for 7110 and comp.) */
    fprintf(stdout, _(" CALENDAR NOTES SUMMARY INFORMATION \n"));
    fprintf(stdout, _(" ==================================\n"));
    if (GetModelFeature (FN_CALENDAR)==F_CAL71) {
      fprintf(stdout, _("Calendar notes present on phone: %d\n"), NotesInfo.HowMany);
      fprintf(stdout, _("Locations are :\n"));
    }
    fprintf(stdout,  "----------------------------------------------------------------------------\n");
    fprintf(stdout,_(" Loc Phys Type    Summary description              Dt start    Alarm  Recurs\n") );
    fprintf(stdout,  "----------------------------------------------------------------------------\n");

    for(i=0;i<NotesInfo.HowMany;i++)
    {
      /* very short format ... */
      /*
      fprintf(stdout, _("%4d) %4d\n"), i, NotesInfo.Location[i]);
      */
      CalendarNote.Location=i+1;
      CalendarNote.ReadNotesInfo=false;

      if (GSM->GetCalendarNote(&CalendarNote) == GE_NONE) {
        char z_type[11];
        char z_recur[15];
        switch (CalendarNote.Type) {
           case GCN_REMINDER:strcpy(z_type, "REMIND");  break;
           case GCN_CALL:    strcpy(z_type, "CALL");    break;
           case GCN_MEETING: strcpy(z_type, "MEETING"); break;
           case GCN_BIRTHDAY:strcpy(z_type, "BDAY");    break;
           default:          strcpy(z_type, "UNKNOWN"); break;
          }

        if( CalendarNote.Recurrance ) {
          sprintf( z_recur,"%d ", CalendarNote.Recurrance/24 );
          strcat( z_recur, CalendarNote.Recurrance == 1 ? "day" : "days" );
        }
        else
          strcpy( z_recur, "No" );

        strcpy(z_text,"");
	
        if( CalendarNote.Type == GCN_CALL )
          sprintf(z_text, "\"%s\"", CalendarNote.Phone );
	  
        if (CalendarNote.Text[0]!=0)
          sprintf(z_text, "\"%s\"", CalendarNote.Text );
	  
        if(CalendarNote.Type == GCN_BIRTHDAY) {
          int i_age;
          i_age = Date.Year - CalendarNote.Time.Year;
          sprintf(z_text, "\"%s (%d %s)\"", CalendarNote.Text,
             i_age, (i_age==1)?"year":"years");
          strcpy( z_recur, "-" );
	  if (GetModelFeature (FN_CALENDAR)==F_CAL71)
            fprintf(stdout,
                _("%4d %4d %-7.7s %-32.32s %04d-%02d-%02d  %s %s\n"), 
              i+1,NotesInfo.Location[i], z_type, z_text,
              CalendarNote.Time.Year, 
              CalendarNote.Time.Month, 
              CalendarNote.Time.Day,
              (CalendarNote.AlarmType==0x00) ? "Tone  " : "Silent",
              " " );
	  else
            fprintf(stdout,
                _("%4d %4d %-7.7s %-32.32s %04d-%02d-%02d  %s %s\n"), 
              i+1,NotesInfo.Location[i], z_type, z_text,
              CalendarNote.Time.Year, 
              CalendarNote.Time.Month, 
              CalendarNote.Time.Day,
	      (CalendarNote.Alarm.Year) ? "Yes" : "No ",
              " " );
        } else
	  if (GetModelFeature (FN_CALENDAR)==F_CAL71)
            fprintf(stdout,
                _("%4d %4d %-7.7s %-32.32s %04d-%02d-%02d  %s    %s\n"), 
              i+1,NotesInfo.Location[i], z_type, z_text,
              CalendarNote.Time.Year, 
              CalendarNote.Time.Month, 
              CalendarNote.Time.Day,
              (CalendarNote.Alarm.Year) ? "Yes" : "No ",
              z_recur );
           else
            fprintf(stdout,
                _("%4d %4d %-7.7s %-32.32s %04d-%02d-%02d  %s\n"), 
              i+1,NotesInfo.Location[i], z_type, z_text,
              CalendarNote.Time.Year, 
              CalendarNote.Time.Month, 
              CalendarNote.Time.Day,
              (CalendarNote.Alarm.Year) ? "Yes" : "No ");
      } else {
        if (GetModelFeature (FN_CALENDAR)!=F_CAL71) break;
      }
    }
  }
  else
  for (i=start;i<=stop;i++) {
    if (error==GE_NONE) {
	if( i>NotesInfo.HowMany ) {
	    fprintf(stderr, _("Only %d Calendar Notes present on phone!\n"),NotesInfo.HowMany);
	    break;
	}
	if( i==0 ) {
	    fprintf(stderr, _("Calendar Notes location can't be zero... skipping.\n"));
	    continue;
	}
    }
    
    CalendarNote.Location=i;
    CalendarNote.ReadNotesInfo=false;

    if (GSM->GetCalendarNote(&CalendarNote) == GE_NONE) {

      if (vCalVer!=0) {
        if (!was_note) {
          fprintf(stdout, GSM_GetVCALENDARStart(vCalVer));
	  was_note=true;
        }

        fprintf(stdout, GSM_GetVCALENDARNote(&CalendarNote,vCalVer));

      } else {  /* not vCal */

        if (was_note) {
	  fprintf(stdout, "\n");
        } else {
          was_note=true;
	}

        fprintf(stdout, _("   Type of the note: "));

        switch (CalendarNote.Type) {

	  case GCN_REMINDER:fprintf(stdout, _("Reminder\n"));break;
          case GCN_CALL    :fprintf(stdout, _("Call\n"));    break;
          case GCN_MEETING :fprintf(stdout, _("Meeting\n")); break;
          case GCN_BIRTHDAY:fprintf(stdout, _("Birthday\n"));break;
	  default:          fprintf(stdout, _("Unknown\n"));

	}

        /* For 3310: set date to 2090! */
        if (GetModelFeature (FN_CALENDAR)==F_CAL33) {
          fprintf(stdout, _("   Date: xxxx-%02d-%02d\n"), CalendarNote.Time.Month,
                                                          CalendarNote.Time.Day);
	} else {
          fprintf(stdout, _("   Date: %s %d-%02d-%02d\n"), 
	        DayOfWeek(CalendarNote.Time.Year, CalendarNote.Time.Month, CalendarNote.Time.Day),
							CalendarNote.Time.Year,
                                                        CalendarNote.Time.Month,
                                                        CalendarNote.Time.Day);
	}
	
        fprintf(stdout, _("   Time: %02d:%02d:%02d\n"), CalendarNote.Time.Hour,
                                                        CalendarNote.Time.Minute,
                                                        CalendarNote.Time.Second);

        if (CalendarNote.Alarm.Year!=0) {
          fprintf(stdout, _("   Alarm date: %s %d-%02d-%02d\n"), 
	  	        DayOfWeek(CalendarNote.Alarm.Year, CalendarNote.Alarm.Month, CalendarNote.Alarm.Day),
							      CalendarNote.Alarm.Year,
                                                              CalendarNote.Alarm.Month,
                                                              CalendarNote.Alarm.Day);

          fprintf(stdout, _("   Alarm time: %02d:%02d:%02d\n"), CalendarNote.Alarm.Hour,
                                                                CalendarNote.Alarm.Minute,
                                                                CalendarNote.Alarm.Second);
          if ( GetModelFeature (FN_CALENDAR)==F_CAL71 ) 
            fprintf(stdout, _("   Alarm type: %s\n"), (CalendarNote.AlarmType==0x00) ?
                                                          "With Tone" : "Silent" );
        }

        if ( GetModelFeature (FN_CALENDAR)==F_CAL71 && CalendarNote.Recurrance!= 0 ) 
           fprintf(stdout, "   It repeat every %d day%s\n", CalendarNote.Recurrance/24,
                       ((CalendarNote.Recurrance/24)>1) ? "s":"" );

        if (CalendarNote.Type == GCN_BIRTHDAY)
        {
          int i_age;
          i_age = Date.Year - CalendarNote.Time.Year;
          fprintf(stdout, _("   Text: %s (%d %s)\n"), CalendarNote.Text,
              i_age, (i_age==1)?"year":"years");
        } else {
          if (CalendarNote.Text[0]!=0)
	    fprintf(stdout, _("   Text: %s\n"), CalendarNote.Text);
	}

        if (CalendarNote.Type == GCN_CALL)
          fprintf(stdout, _("   Phone: %s\n"), CalendarNote.Phone);
      }
    } else {
      fprintf(stderr, _("The calendar note %i can not be read\n"),i);
    }
  }

  if (was_note && vCalVer!=0) {
    fprintf(stdout, GSM_GetVCALENDAREnd(vCalVer));
  }
  
  GSM->Terminate();

  return 0;
}

/* Writing calendar notes. */

int writecalendarnote(char *argv[])
{
  GSM_CalendarNote CalendarNote;
  GSM_Error error;
  int number;

  number=atoi(argv[1]);
  
  if (number<1) {
    fprintf(stdout, _("Number of calendar note must be 1 or higher\n"));
    return -1;
  }
  
  switch ( GSM_ReadVCalendarFile(argv[0], &CalendarNote, &number) ) {
    case GE_NONE:
      break;
    case GE_CANTOPENFILE:
      fprintf(stdout, _("Failed to open vCalendar file \"%s\"\n"),argv[0]);
      return -1;
    case GE_TOOSHORT:
      fprintf(stdout, _("Number of given calendar note is too high (max=%i)\n"),number);
      return(-1);
    default:
      fprintf(stdout, _("Failed to parse vCalendar file \"%s\"\n"),argv[0]);
      return -1;    
  }

  fbusinit(NULL);

  /* Error 22=Calendar full ;-) */

  error=GSM->WriteCalendarNote(&CalendarNote);
  switch (error) {
    case GE_NONE:
      fprintf(stdout, _("Succesfully written!\n"));break;
    case GE_TOOLONG:
      fprintf(stdout, _("Too long text in calendar note!\n"));break;
    default:
      fprintf(stdout, _("Failed to write calendar note!\n"));break;
  }

  GSM->Terminate();

  return 0;
}

/* Calendar note deleting. */

int deletecalendarnote(char *Index)
{

  GSM_CalendarNote CalendarNote;

  CalendarNote.Location=atoi(Index);

  fbusinit(NULL);

  if (GSM->DeleteCalendarNote(&CalendarNote) == GE_NONE) {
    fprintf(stdout, _("   Calendar note deleted.\n"));
  }
  else {
    fprintf(stderr, _("The calendar note can not be deleted\n"));

    GSM->Terminate();
    return -1;
  }

  GSM->Terminate();

  return 0;
}

/* Setting the date and time. */

int setdatetime(int argc, char *argv[])
{
  struct tm *now;
  time_t nowh;
  GSM_DateTime Date;

  fbusinit(NULL);

  nowh=time(NULL);
  now=localtime(&nowh);

  Date.Year = now->tm_year;
  Date.Month = now->tm_mon+1;
  Date.Day = now->tm_mday;
  Date.Hour = now->tm_hour;
  Date.Minute = now->tm_min;
  Date.Second = now->tm_sec;

  if (argc>0) Date.Year = atoi (argv[0]);
  if (argc>1) Date.Month = atoi (argv[1]);
  if (argc>2) Date.Day = atoi (argv[2]);
  if (argc>3) Date.Hour = atoi (argv[3]);
  if (argc>4) Date.Minute = atoi (argv[4]);

  if (Date.Year<1900)
  {

    /* Well, this thing is copyrighted in U.S. This technique is known as
       Windowing and you can read something about it in LinuxWeekly News:
       http://lwn.net/1999/features/Windowing.phtml. This thing is beeing
       written in Czech republic and Poland where algorithms are not allowed
       to be patented. */

    if (Date.Year>90)
      Date.Year = Date.Year+1900;
    else
      Date.Year = Date.Year+2000;
  }

  /* FIXME: Error checking should be here. */
  GSM->SetDateTime(&Date);

  GSM->Terminate();

  return 0;
}

/* In this mode we receive the date and time from mobile phone. */

int getdatetime(void) {

  GSM_DateTime date_time;

  fbusinit(NULL);

  if (GSM->GetDateTime(&date_time)==GE_NONE) {
    if (date_time.IsSet) {
      fprintf(stdout, _("Date: %s %4d/%02d/%02d\n"), 
          DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
          date_time.Year, date_time.Month, date_time.Day);
      fprintf(stdout, _("Time: %02d:%02d:%02d\n"), date_time.Hour, date_time.Minute, date_time.Second);
    } else {
      fprintf(stdout, _("Date and time not set in phone\n"));
    }
  } else {
    fprintf(stdout,_("Error!\n"));
  }

  GSM->Terminate();

  return 0;
}

/* Setting the alarm. */

int setalarm(char *argv[])
{

  GSM_DateTime Date;

  fbusinit(NULL);

  Date.Hour = atoi(argv[0]);
  Date.Minute = atoi(argv[1]);

  GSM->SetAlarm(1, &Date);

  GSM->Terminate();

  return 0;
}

/* Getting the alarm. */

int getalarm(void) {

  GSM_DateTime date_time;

  fbusinit(NULL);

  if (GSM->GetAlarm(0, &date_time)==GE_NONE) {
    fprintf(stdout, _("Alarm: %s\n"), (date_time.IsSet)?"on":"off");
    fprintf(stdout, _("Time: %02d:%02d\n"), date_time.Hour, date_time.Minute);
  } else {
    fprintf(stdout,_("Error!\n"));
  }

  GSM->Terminate();

  return 0;
}

/* In monitor mode we don't do much, we just initialise the fbus code.
   Note that the fbus code no longer has an internal monitor mode switch,
   instead compile with DEBUG enabled to get all the gumpf. */

int monitormode(int argc, char *argv[])
{

  float rflevel=-1, batterylevel=-1;
	unsigned char loop=1;

  GSM_PowerSource powersource=-1;
  GSM_RFUnits rf_units = GRF_Arbitrary;
  GSM_BatteryUnits batt_units = GBU_Arbitrary;

  GSM_NetworkInfo NetworkInfo;
  GSM_CBMessage CBMessage;

  GSM_MemoryStatus SIMMemoryStatus = {GMT_SM, 0, 0};
  GSM_MemoryStatus PhoneMemoryStatus = {GMT_ME, 0, 0};
  GSM_MemoryStatus DC_MemoryStatus = {GMT_DC, 0, 0};
  GSM_MemoryStatus EN_MemoryStatus = {GMT_EN, 0, 0};
  GSM_MemoryStatus FD_MemoryStatus = {GMT_FD, 0, 0};
  GSM_MemoryStatus LD_MemoryStatus = {GMT_LD, 0, 0};
  GSM_MemoryStatus MC_MemoryStatus = {GMT_MC, 0, 0};
  GSM_MemoryStatus ON_MemoryStatus = {GMT_ON, 0, 0};
  GSM_MemoryStatus RC_MemoryStatus = {GMT_RC, 0, 0};

  GSM_SMSStatus SMSStatus = {0, 0};

  char Number[20];

	/* evaluate for presence of "-noloop" argument in parameter */
	if(argc>0)
	{
		if( strcmp(argv[0],"-noloop" ) && strcmp(argv[0],"-nl" ))
		{
			usage();
			return -1;
		}
		else
			loop=0;
	}

  /* We do not want to monitor serial line forever - press Ctrl+C to stop the
     monitoring mode. */

  signal(SIGINT, interrupted);

  fprintf (stderr, _("Entering monitor mode...\n"));
  fprintf (stderr, _("Initialising GSM interface...\n"));

  /* Initialise the code for the GSM interface. */     

  fbusinit(NULL);

  sleep(1);
  GSM->EnableCellBroadcast();

  /* Loop here indefinitely - allows you to see messages from GSM code in
     response to unknown messages etc. The loops ends after pressing the
     Ctrl+C. */
  while (!bshutdown) {
    if (GSM->GetRFLevel(&rf_units, &rflevel) == GE_NONE)
      fprintf(stdout, _("RFLevel: %d\n"), (int)rflevel);

    if (GSM->GetBatteryLevel(&batt_units, &batterylevel) == GE_NONE)
      fprintf(stdout, _("Battery: %d\n"), (int)batterylevel);

    if (GSM->GetPowerSource(&powersource) == GE_NONE)
      fprintf(stdout, _("Power Source: %s\n"), (powersource==GPS_ACDC)?_("AC/DC"):_("battery"));

    if (GSM->GetMemoryStatus(&SIMMemoryStatus) == GE_NONE)
      fprintf(stdout, _("SIM: Used %d, Free %d\n"), SIMMemoryStatus.Used, SIMMemoryStatus.Free);

    if (GSM->GetMemoryStatus(&PhoneMemoryStatus) == GE_NONE)
      fprintf(stdout, _("Phone: Used %d, Free %d\n"), PhoneMemoryStatus.Used, PhoneMemoryStatus.Free);

    if (GSM->GetMemoryStatus(&DC_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("DC: Used %d, Free %d\n"), DC_MemoryStatus.Used, DC_MemoryStatus.Free);

    if (GSM->GetMemoryStatus(&EN_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("EN: Used %d, Free %d\n"), EN_MemoryStatus.Used, EN_MemoryStatus.Free);

    if (GSM->GetMemoryStatus(&FD_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("FD: Used %d, Free %d\n"), FD_MemoryStatus.Used, FD_MemoryStatus.Free);

    if (GSM->GetMemoryStatus(&LD_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("LD: Used %d, Free %d\n"), LD_MemoryStatus.Used, LD_MemoryStatus.Free);

    if (GSM->GetMemoryStatus(&MC_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("MC: Used %d, Free %d\n"), MC_MemoryStatus.Used, MC_MemoryStatus.Free);

    if (GSM->GetMemoryStatus(&ON_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("ON: Used %d, Free %d\n"), ON_MemoryStatus.Used, ON_MemoryStatus.Free);

    if (GSM->GetMemoryStatus(&RC_MemoryStatus) == GE_NONE)
      fprintf(stdout, _("RC: Used %d, Free %d\n"), RC_MemoryStatus.Used, RC_MemoryStatus.Free);

    if (GSM->GetSMSStatus(&SMSStatus) == GE_NONE)
      fprintf(stdout, _("SMS Messages: UnRead %d, Number %d\n"), SMSStatus.UnRead, SMSStatus.Number);

    if (GSM->GetIncomingCallNr(Number) == GE_NONE)
      fprintf(stdout, _("Incoming call: %s\n"), Number);

    if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE)
      fprintf(stdout, _("Network: %s (%s), LAC: %s, CellID: %s\n"), GSM_GetNetworkName (NetworkInfo.NetworkCode), GSM_GetCountryName(NetworkInfo.NetworkCode), NetworkInfo.LAC, NetworkInfo.CellID);

    if (GSM->ReadCellBroadcast(&CBMessage) == GE_NONE)
      fprintf(stdout, _("Cell broadcast received on channel %d: %s\n"), CBMessage.Channel, CBMessage.Message);

    if( !loop ) break;
	    
    sleep(1);
  }

  if( loop ) fprintf (stderr, _("Leaving monitor mode...\n"));

  GSM->Terminate();

  return 0;
}

/* Shows texts from phone's display */

int displayoutput()
{

  GSM_Error error;
  
  fbusinit(NULL);

  error=GSM->EnableDisplayOutput();

  if (error == GE_NONE)
  {

    /* We do not want to see texts forever - press Ctrl+C to stop. */

    signal(SIGINT, interrupted);    

    fprintf (stderr, _("Entering display monitoring mode...\n"));

    /* Loop here indefinitely - allows you to read texts from phone's
       display. The loops ends after pressing the Ctrl+C. */

    while (!bshutdown)
      sleep(1);

    fprintf (stderr, _("Leaving display monitor mode...\n"));

    error=GSM->DisableDisplayOutput();
    if (error!=GE_NONE)
      fprintf (stderr, _("Error!\n"));
  } else
      fprintf (stderr, _("Error!\n"));

  GSM->Terminate();

  return 0;
}

/* Displays names of available ringtones */
int allringtones()
{
  char model[64], rev[64];
  int i;

  fbusinit(NULL);

  while (GSM->GetRevision(rev) != GE_NONE)
    sleep(1);

  while (GSM->GetModel(model)  != GE_NONE)
    sleep(1);

  strncpy(rev,rev+2,5);
  rev[5]=0;
  PrepareRingingTones(model,rev);
  
  for (i=1;i<=NumberOfRingtones();i++)
  {
    fprintf(stdout,_("%i. %s\n"),i,RingingToneName(0,i));
  }
  
  GSM->Terminate();

  return 0;

}

/* Reads profile from phone and displays its' settings */

int getprofile(int argc, char *argv[])
{

  int max_profiles;
  int start, stop, i;
  GSM_Profile profile;
  GSM_Error error;
  
  /* Hopefully is 64 larger as FB38_MAX* / FB61_MAX* */
  char model[64], rev[64];

  /* Initialise the code for the GSM interface. */     

  fbusinit(NULL);

  profile.Number = 0;
  error=GSM->GetProfile(&profile);

  if (error == GE_NONE)
  {
  
    while (GSM->GetModel(model)  != GE_NONE) sleep(1);

    while (GSM->GetRevision(rev) != GE_NONE) sleep(1);

    strncpy(rev,rev+2,5);
    rev[5]=0;
    PrepareRingingTones(model,rev);

    switch(GetModelFeature (FN_PROFILES)) {
      case F_PROF33:max_profiles=6;break;
      case F_PROF51:max_profiles=3;break;
      default      :max_profiles=7;break;
    }

    if (argc>0)
    {
      profile.Number=atoi(argv[0])-1;
      start=profile.Number;
      stop=start+1;

      if (profile.Number < 0)
      {
         fprintf(stderr, _("Profile number must be value from 1 to %d!\n"), max_profiles);
         GSM->Terminate();
         return -1;
      }

      if (profile.Number >= max_profiles)
      {
         fprintf(stderr, _("This phone supports only %d profiles!\n"), max_profiles);
         GSM->Terminate();
         return -1;
      }
    } else {
      start=0;
      stop=max_profiles;
    }

    i=start;
    while (i<stop)
    {
      profile.Number=i;

      if (profile.Number!=0) GSM->GetProfile(&profile);

      printf("%d. \"%s\"", (profile.Number+1), profile.Name);
      if (profile.DefaultName==-1) printf(" (name defined)");
      printf("\n");

#ifdef DEBUG
      printf("Incoming call alert: %d\n", profile.CallAlert);
      printf("Ringtone ID: %d\n", profile.Ringtone);
      printf("Ringing volume: %d\n", profile.Volume);
      printf("Message alert tone: %d\n", profile.MessageTone);
      printf("Keypad tones: %d\n", profile.KeypadTone);
      printf("Warning and game tones: %d\n", profile.WarningTone);
      printf("Lights: %d\n", profile.Lights);
      printf("Vibration: %d\n", profile.Vibration);
      printf("Caller groups: 0x%02x\n", profile.CallerGroups);
      printf("Automatic answer: %d\n", profile.AutomaticAnswer);
      printf("Screen saver: %d\n", profile.ScreenSaver);
      printf("\n");
#endif
      
      printf("Incoming call alert: %s\n", GetProfileCallAlertString(profile.CallAlert));

      /* For different phones different ringtones names */
      if (strcmp(RingingToneName(profile.Ringtone,0),""))
        printf(_("Ringing tone: %s (number %d in phone menu)\n"), 
          RingingToneName(profile.Ringtone,0), RingingToneMenu(profile.Ringtone));
      else
        printf(_("Ringtone number: %d\n"), profile.Ringtone);

      printf(_("Ringing volume: %s\n"), GetProfileVolumeString(profile.Volume));

      printf(_("Message alert tone: %s\n"), GetProfileMessageToneString(profile.MessageTone));

      printf(_("Keypad tones: %s\n"), GetProfileKeypadToneString(profile.KeypadTone));

      printf(_("Warning and game tones: %s\n"), GetProfileWarningToneString(profile.WarningTone));

      if (GetModelFeature (FN_SCREENSAVER)!=0)
        printf(_("Screen saver: %s\n"), GetProfileOnOffString(profile.ScreenSaver));
      
      printf(_("Vibration: %s\n"), GetProfileVibrationString(profile.Vibration));

      /* It has been nice to add here reading caller group name. ;^) */
      if (GetModelFeature (FN_CALENDAR)==F_CAL71) 
        printf(_("Caller groups: %s\n"), 
                              GetProfileCallerGroups(profile.CallerGroups) );

      /* FIXME: need make investigation for 3310 for these features. For now unknown */
      if (GetModelFeature (FN_PROFILES)!=F_PROF33) {
        /* FIXME: Light settings is only used for Car */
        if (profile.Number==(max_profiles-2)) printf(_("Lights: %s\n"), profile.Lights ? _("On") : _("Automatic"));

        /* FIXME: Automatic answer is only used for Car and Headset. */
        if (profile.Number>=(max_profiles-2)) printf(_("Automatic answer: %s\n"), GetProfileOnOffString(profile.AutomaticAnswer));
      }

      printf("\n");

      i++;
    }
  } else {
    if (error == GE_NOTIMPLEMENTED) {
       fprintf(stderr, _("Function not implemented in %s model!\n"), model);
       GSM->Terminate();
       return -1;
    } else
    {
      fprintf(stderr, _("Unspecified error\n"));
      GSM->Terminate();
      return -1;
    }
  }

  GSM->Terminate();

  return 0;

}

/* Sets profile feature */

int setprofile(int argc, char *argv[])
{

/* Hopefully is 64 larger as FB38_MAX* / FB61_MAX* */
  char model[64], rev[64];

  int max_profiles;
  GSM_Profile profile;
  GSM_Profile profile2, profile3;
  GSM_Error error;
  bool correct_arg1;
  bool correct_arg2;
  
/* Initialise the code for the GSM interface. */     

  fbusinit(NULL);
  
  profile.Number = 0;

  error = GSM->GetProfile(&profile);
  
  if (error == GE_NONE)
  {
 
    while (GSM->GetModel(model)  != GE_NONE) sleep(1);
      
    while (GSM->GetRevision(rev) != GE_NONE) sleep(1);

    strncpy(rev,rev+2,5);
    rev[5]=0;
    PrepareRingingTones(model,rev);

    switch(GetModelFeature (FN_PROFILES)) {
      case F_PROF33:max_profiles=6;break;
      case F_PROF51:max_profiles=3;break;
      default      :max_profiles=7;break;
    }

    profile.Number=atoi (argv[0]);
    profile.Number=profile.Number-1;
    
    if (profile.Number < 0)
    {
      fprintf(stderr, _("Profile number must be value from 1 to %i!\n"), max_profiles);
      GSM->Terminate();
      return -1;
    }
    
    if (profile.Number >= max_profiles)
    {
      fprintf(stderr, _("This phone supports only %i profiles!\n"), max_profiles);
      GSM->Terminate();
      return -1;
    }
      
    if (profile.Number!=0) GSM->GetProfile(&profile);
        
    correct_arg1=false;
    correct_arg2=false;

    if (strcmp(argv[1], "callalert")==0)
    {
       if (strcmp(argv[2], "ringing")==0) {profile.CallAlert=PROFILE_CALLALERT_RINGING;correct_arg2=true;}
       if (strcmp(argv[2], "ascending")==0) {profile.CallAlert=PROFILE_CALLALERT_ASCENDING;correct_arg2=true;}
       if (strcmp(argv[2], "ringonce")==0) {profile.CallAlert=PROFILE_CALLALERT_RINGONCE;correct_arg2=true;}
       if (strcmp(argv[2], "beeponce")==0) {profile.CallAlert=PROFILE_CALLALERT_BEEPONCE;correct_arg2=true;}
       if (strcmp(argv[2], "groups")==0)
       {
         profile.CallAlert=PROFILE_CALLALERT_CALLERGROUPS;
	 correct_arg2=true;
	 /*Ignored by N5110*/
	 /*FIX ME: it's ignored by N5130 and 3210 too*/
	 if (max_profiles==3) fprintf(stdout, _("Warning: value \"groups\" for profile feature \"callalert\" will be ignored in this phone model !\n"));
       }
       if (strcmp(argv[2], "off")==0 || strcmp(argv[2], "0")==0) {profile.CallAlert=PROFILE_CALLALERT_OFF;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"callalert\" are ringing|ascending|ringonce|beeponce|groups|off|0 !\n"));
         GSM->Terminate();
         return -1;
        }
	correct_arg1=true;
    }
    if (strcmp(argv[1], "volume")==0)
    {
       if (strcmp(argv[2], "1")==0) {profile.Volume=PROFILE_VOLUME_LEVEL1;correct_arg2=true;}
       if (strcmp(argv[2], "2")==0) {profile.Volume=PROFILE_VOLUME_LEVEL2;correct_arg2=true;}
       if (strcmp(argv[2], "3")==0) {profile.Volume=PROFILE_VOLUME_LEVEL3;correct_arg2=true;}
       if (strcmp(argv[2], "4")==0) {profile.Volume=PROFILE_VOLUME_LEVEL4;correct_arg2=true;}
       if (strcmp(argv[2], "5")==0) {profile.Volume=PROFILE_VOLUME_LEVEL5;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"volume\" are 0|1|2|3|4|5 !\n"));
         GSM->Terminate();
         return -1;
        }
	correct_arg1=true;
    }
    if (strcmp(argv[1], "keypad")==0 || strcmp(argv[1], "keypadtone")==0)
    {
       if (strcmp(argv[2], "0")==0 || strcmp(argv[2], "off")==0) {profile.KeypadTone=PROFILE_KEYPAD_OFF;correct_arg2=true;}
       if (strcmp(argv[2], "1")==0) {profile.KeypadTone=PROFILE_KEYPAD_LEVEL1;correct_arg2=true;}
       if (strcmp(argv[2], "2")==0) {profile.KeypadTone=PROFILE_KEYPAD_LEVEL2;correct_arg2=true;}
       if (strcmp(argv[2], "3")==0) {profile.KeypadTone=PROFILE_KEYPAD_LEVEL3;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"keypad|keypadtone\" are off|0|1|2|3 !\n"));
         GSM->Terminate();
         return -1;
        }
	correct_arg1=true;
    }
    if (strcmp(argv[1], "messagetone")==0 || strcmp(argv[1], "smstone")==0 || strcmp(argv[1], "sms")==0 || strcmp(argv[1], "message")==0)
    {
       if (strcmp(argv[2], "0")==0 || strcmp(argv[2], "off")==0) {profile.MessageTone=PROFILE_MESSAGE_NOTONE;correct_arg2=true;}
       if (strcmp(argv[2], "standard")==0) {profile.MessageTone=PROFILE_MESSAGE_STANDARD;correct_arg2=true;}
       if (strcmp(argv[2], "special")==0) {profile.MessageTone=PROFILE_MESSAGE_SPECIAL;correct_arg2=true;}
       if (strcmp(argv[2], "beeponce")==0 || strcmp(argv[2], "once")==0) {profile.MessageTone=PROFILE_MESSAGE_BEEPONCE;correct_arg2=true;}
       if (strcmp(argv[2], "ascending")==0) {profile.MessageTone=PROFILE_MESSAGE_ASCENDING;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"messagetone|smstone|message|sms\" are 0|off|standard|special|beeponce|once|ascending !\n"));
         GSM->Terminate();
         return -1;
        }
	correct_arg1=true;
    }
    if (strcmp(argv[1], "warningtone")==0 || strcmp(argv[1], "warning")==0)
    {
       if (strcmp(argv[2], "0")==0 || strcmp(argv[2], "off")==0) {profile.WarningTone=PROFILE_WARNING_OFF;correct_arg2=true;}
       if (strcmp(argv[2], "1")==0 || strcmp(argv[2], "on")==0) {profile.WarningTone=PROFILE_WARNING_ON;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"warningtone|warning\" are 0|off|1|on !\n"));
         GSM->Terminate();
         return -1;
        }
	correct_arg1=true;
    }
    if (strcmp(argv[1], "vibra")==0 || strcmp(argv[1], "vibration")==0)
    {
       if (strcmp(argv[2], "0")==0 || strcmp(argv[2], "off")==0) {profile.Vibration=PROFILE_VIBRATION_OFF;correct_arg2=true;}
       if (strcmp(argv[2], "1")==0 || strcmp(argv[2], "on")==0) {profile.Vibration=PROFILE_VIBRATION_ON;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"vibration|vibra\" are 0|off|1|on !\n"));
         GSM->Terminate();
         return -1;
        }
	correct_arg1=true;
    }
    if (strcmp(argv[1], "lights")==0)
    {
       if (strcmp(argv[2], "0")==0 || strcmp(argv[2], "off")==0) {profile.Lights=-1;correct_arg2=true;}
       if (strcmp(argv[2], "1")==0 || strcmp(argv[2], "on")==0) {profile.Lights=0;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"lights\" are 0|off|1|on !\n"));
         GSM->Terminate();
         return -1;
       }
       if (profile.Number!=(max_profiles-2))
       {
         profile2.Number=max_profiles-2;
	 if (GSM->GetProfile(&profile2)==GE_NONE)
	 {
           fprintf(stdout, _("Warning: \"Lights\" feature is ignored in this profile (only setting it for \"%s\" profile get some results) !\n"), profile2.Name);
	 }
       }
       correct_arg1=true;
    }
    if (strcmp(argv[1], "answer")==0)
    {
       if (strcmp(argv[2], "0")==0 || strcmp(argv[2], "off")==0) {profile.AutomaticAnswer=-1;correct_arg2=true;}
       if (strcmp(argv[2], "1")==0 || strcmp(argv[2], "on")==0) {profile.AutomaticAnswer=0;correct_arg2=true;}
       if (!correct_arg2)
       {
	 fprintf(stderr, _("Correct parameters for profile feature \"answer\" are 0|off|1|on !\n"));
         GSM->Terminate();
         return -1;
       }
       if (profile.Number<(max_profiles-2))
       {
         profile2.Number=max_profiles-2;
	 if (GSM->GetProfile(&profile2)==GE_NONE)
	 {
	   profile3.Number=max_profiles-1;
	   if (GSM->GetProfile(&profile3)==GE_NONE)
	   {
             fprintf(stdout, _("Warning: \"Automatic Answer\" feature is ignored in this profile (only setting it for \"%s\" and \"%s\" profiles get some results) !\n"), profile2.Name, profile3.Name);
	   }
	 }
       }
       correct_arg1=true;
    }
    if (strcmp(argv[1], "name")==0)
    {
       strcpy(profile.Name,argv[2]);
       /*Ignored by N5110*/
       /*FIX ME: it's ignored by N5130 and 3210 too*/
       if (max_profiles==3) fprintf(stdout, _("Warning: this phone model doesn't allow to change profile name !\n"));
       correct_arg1=true;
    }
    if (strcmp(argv[1], "ringtone")==0)
    {
      profile.Ringtone=atoi(argv[2]);
      if (profile.Ringtone<1)
      {
        /*With value 0 results are interesting in N5110, but can't be used for anything*/
        fprintf(stderr, _("Ringtone number must be higher than 0 !\n"));
        GSM->Terminate();
        return -1; 
      } else
      {
        if (profile.Ringtone>NumberOfRingtones())
	{
          fprintf(stderr, _("Ringtone number too high (max %i) !\n"),NumberOfRingtones());
          GSM->Terminate();
          return -1;
        }  
	profile.Ringtone=RingingToneCode(profile.Ringtone);
	if (profile.Ringtone==0)
	{
          fprintf(stderr, _("Warning: we don't know ringtones codes for this model ! Can you contact with gnokii authors to add it into source ?\n"));
          GSM->Terminate();
          return -1;
	}
	if (max_profiles==3) fprintf(stdout, _("Warning: ringtone is changed for all profiles. You must reset phone or select profile in phone againg to see result.\n"));
      }
      correct_arg1=true;
    }
    if (strcmp(argv[1], "groups")==0)
    {
       /*Ignored by N5110*/
       /*FIX ME: it's ignored by N5130 and 3210 too*/
       if (max_profiles==3) fprintf(stdout, _("Warning: this phone model doesn't allow to change caller group name !\n"));
       profile.CallerGroups=atoi(argv[2]);
       correct_arg1=true;
    }

    if (!correct_arg1)
    {
      fprintf(stderr, _("Correct profile feature names are callalert|volume|keypad|keypadtone|messagetone|smstone|message|sms|warningtone|warning|vibra|vibration|lights|answer|name|groups !\n"));
      GSM->Terminate();
      return -1;
    }
    
    GSM->SetProfile(&profile);
    
  } else {
    if (error == GE_NOTIMPLEMENTED) {
       fprintf(stderr, _("Function not implemented in %s model!\n"), model);
       GSM->Terminate();
       return -1;
    } else
    {
      fprintf(stderr, _("Unspecified error\n"));
      GSM->Terminate();
      return -1;
    }
  }
 
  GSM->Terminate();

  return 0;

}

/* Get requested range of memory storage entries and output to stdout in
   easy-to-parse format */

int getmemory(int argc, char *argv[])
{

  GSM_PhonebookEntry entry;
  int count;
  register int i;
  GSM_Error error;
  GSM_Bitmap bitmap;
  char memory_type_string[20];
  int start_entry;
  int end_entry;
  int i_used = 0, n_used=0;
  int do_all;
  char *output_opt = NULL;
  bool wasdate=false;
  char az_group_name[5][MAX_BITMAP_TEXT_LENGTH];
  bool formatdone=false;
  char buf[64];

  /* group names init */
  for(i=0;i<5;i++) az_group_name[i][0]='\0';

  /* Handle command line args that set type, start and end locations. */
  if (!GetMemoryTypeID(argv[0], &entry.MemoryType))
  {
    fprintf(stderr, _("Unknown memory type %s!\n"), argv[0]);
    return (-1);
  }
  GetMemoryTypeString(memory_type_string, &entry.MemoryType);

  if (argv[argc-1][0] == '-')
    output_opt = argv[--argc];

  /* Do generic initialisation routine */

  fbusinit(NULL);

  while ((error = GSM->GetModel(buf)) != GE_NONE && i++ < 15)
    sleep(1);

  if (argc > 1) {
    do_all = 0;
    start_entry = atoi (argv[1]);
    end_entry = argc > 2 ? atoi( argv[2]) : start_entry;
  } else {
    GSM_MemoryStatus stats = {entry.MemoryType, 0, 0};

    do_all = 1;
    start_entry = 1;
    if (GSM->GetMemoryStatus( &stats) != GE_NONE) {
      fprintf( stderr, _("Error reading memory status.\n"));
      GSM->Terminate();
      return -1;
    }
    n_used = stats.Used;
    end_entry = stats.Used + stats.Free;
  }
    

  /* Now retrieve the requested entries. */

  for (count = start_entry;
       count <= end_entry && (!do_all || i_used < n_used);
       count++) {

    entry.Location=count;
    
    error=GSM->GetMemoryLocation(&entry);

    switch (error) {
    case GE_NONE:
      if (entry.SubEntriesCount || strcmp( entry.Number, ""))
	i_used++;
      else if (do_all)
	break;
      if (output_opt && !strcmp( output_opt,"-v30")) {
	semicolon_pipe_substitution( &entry, 0 );
        fprintf(stdout,_("%s"),GSM_GetVCARD(&entry,30));
	formatdone=true;
      } 
      if (output_opt && !strcmp(output_opt,"-v21")) {
	semicolon_pipe_substitution( &entry, 0 );
        fprintf(stdout,_("%s"),GSM_GetVCARD(&entry,21));
	formatdone=true;
      } 
      if (output_opt && !strcmp(output_opt,"-v")) {
	semicolon_pipe_substitution( &entry, 0 );
        fprintf(stdout,_("%s"),GSM_GetVCARD(&entry,10));
	formatdone=true;
      } 
      if (output_opt && !strcmp(output_opt,"-short")) {
        semicolon_pipe_substitution( &entry, 0 );
        fprintf(stdout, "%s;%s;%s;%d;%d;", entry.Name, entry.Number, memory_type_string, entry.Location, entry.Group);
        for( i = 0; i <  entry.SubEntriesCount; i++ )
        {
          if( entry.SubEntries[i].EntryType == GSM_Date )
            fprintf(stdout,_("%u;%u;%u;%02u.%02u.%04u;%02u:%02u:%02u;"),
                entry.SubEntries[i].EntryType,
                entry.SubEntries[i].NumberType,
                entry.SubEntries[i].BlockNumber,
                entry.SubEntries[i].data.Date.Day,
                entry.SubEntries[i].data.Date.Month,
                entry.SubEntries[i].data.Date.Year,
                entry.SubEntries[i].data.Date.Hour,
                entry.SubEntries[i].data.Date.Minute,
                entry.SubEntries[i].data.Date.Second );
          else
            fprintf(stdout,_("%u;%u;%u;%s;"),
                entry.SubEntries[i].EntryType,
                entry.SubEntries[i].NumberType,
                entry.SubEntries[i].BlockNumber,
                entry.SubEntries[i].data.Number );
        }
        fprintf(stdout,_("\n"));
	formatdone=true;
      }
      if (!formatdone) {
  	  char z_gtype[12];
          wasdate=false;
          fprintf(stdout, "Memory %s, location %d\n",memory_type_string, entry.Location);

	  // check if some info in subentries 
          for( i = 0; i <  entry.SubEntriesCount; i++ )
	  	  if( entry.SubEntries[i].EntryType != GSM_Date &&
		  	  strcmp(entry.SubEntries[i].data.Number,"") )
		      break;

          if (strcmp(entry.Number,"") || i <  entry.SubEntriesCount) {
  	    if (strcmp(entry.Name,"")) fprintf(stdout,_("   Name: %s\n"),entry.Name);
	    if (strcmp(entry.Number,"")) fprintf(stdout,_("   Number: %s\n"),entry.Number);
	    bitmap.type=GSM_CallerLogo;
            bitmap.number=entry.Group;
            strcpy(z_gtype,"unknown");
	    if (entry.Group==5) strcpy(z_gtype,"No group");
	    if (entry.Group<5 && entry.Group>=0) {
              if (!strcmp(az_group_name[entry.Group],"")) {
	        if (GetModelFeature (FN_CALLERGROUPS)!=0) {
                  if (GSM->GetBitmap(&bitmap)==GE_NONE)
                    strcpy( az_group_name[entry.Group], bitmap.text );
		}
	        if ((!strcmp(az_group_name[entry.Group],""))) {
  	          switch(entry.Group) {
	            case 0:strcpy(az_group_name[entry.Group],"Family");break;
	            case 1:strcpy(az_group_name[entry.Group],"VIP");break;
	            case 2:strcpy(az_group_name[entry.Group],"Friends");break;
	            case 3:strcpy(az_group_name[entry.Group],"Colleagues");break;
	            case 4:strcpy(az_group_name[entry.Group],"Other");break;
	            default:break;
	          }
	        }
	      }
	      strcpy(z_gtype,az_group_name[entry.Group]);
            }
	    fprintf(stdout,_("   Group: %d (%s)\n"),entry.Group+1,z_gtype);
            for( i = 0; i <  entry.SubEntriesCount; i++ )
            {
              if( entry.SubEntries[i].EntryType == GSM_Date ) {
                 fprintf(stdout, "   Date and time: %s %02u.%02u.%04u %02u:%02u:%02u\n", 
	  	    DayOfWeek(entry.SubEntries[i].data.Date.Year,
	                entry.SubEntries[i].data.Date.Month,
	                entry.SubEntries[i].data.Date.Day),
		    entry.SubEntries[i].data.Date.Day,
		    entry.SubEntries[i].data.Date.Month,
		    entry.SubEntries[i].data.Date.Year,
        	    entry.SubEntries[i].data.Date.Hour,
        	    entry.SubEntries[i].data.Date.Minute,
        	    entry.SubEntries[i].data.Date.Second);
	         wasdate=true;
	      } else {
	  	 if( strcmp(entry.SubEntries[i].data.Number,"") ) {
		    char z_etype[19];
		    switch( entry.SubEntries[i].EntryType ) {
		      case GSM_Number:
			switch( entry.SubEntries[i].NumberType ) {
			  case GSM_General: strcpy(z_etype,"General "); break;
			  case GSM_Home: strcpy(z_etype,"Home "); break;
			  case GSM_Mobile: strcpy(z_etype,"Mobile "); break;
			  case GSM_Work: strcpy(z_etype,"Work "); break;
			  case GSM_Fax: strcpy(z_etype,"Fax "); break;
			  default: strcpy(z_etype,""); break;
			}
			strcat(z_etype,"Number"); break;
		      case GSM_Note:
			strcpy(z_etype,"Note"); break;
		      case GSM_Postal:
			strcpy(z_etype,"Postal"); break;
		      case GSM_Email:
			strcpy(z_etype,"E-Mail"); break;
		      default:
			strcpy(z_etype,"unknown data"); break;
		    }
#ifdef DEBUG
		    fprintf(stdout,_("   ---> Subentry: %u\n"),i+1);
		    fprintf(stdout,_("   Entry type: %u (%s)\n"),entry.SubEntries[i].EntryType,z_etype);
		    fprintf(stdout,_("   Number type: %u\n"),entry.SubEntries[i].NumberType);
		    fprintf(stdout,_("   Block Number: %u\n"),entry.SubEntries[i].BlockNumber);
#endif
		    fprintf(stdout,_("   %s: %s\n"),z_etype,entry.SubEntries[i].data.Number);
		  }
	      }
	    }
            if ((entry.MemoryType==GMT_DC ||
	         entry.MemoryType==GMT_RC ||
	         entry.MemoryType==GMT_MC) && !wasdate)
  	      fprintf(stdout,_("   Date and time not available\n"));	    
	  } else
	    fprintf(stdout,_("   Location empty\n"));
      }
      break;
    case GE_NOTIMPLEMENTED:
      fprintf( stderr, _("Function not implemented in %s model!\n"), model);
      GSM->Terminate();
      return -1;
    case GE_INVALIDMEMORYTYPE:
      fprintf( stderr, _("Memory type %s not supported!\n"),
	       memory_type_string);
      GSM->Terminate();
      return -1;
    default:
      fprintf(stdout, _("%s|%d|Bad location or other error!(%d)\n"),
	      memory_type_string, count, error);
    }
  }
	
  GSM->Terminate();

  return 0;
}

/* Read data from stdin, parse and write to phone.  The parsing is relatively
   crude and doesn't allow for much variation from the stipulated format. */

int writephonebook(int argc, char *args[])
{

  GSM_PhonebookEntry entry;
  GSM_Error error;
  char *memory_type_string;
  int line_count=0,current,i;
  int subentry;

  char *Line, OLine[1024], BackLine[1024];
  char *ptr;

  /* Check argument */
  if (argc) {
    if (strcmp("-i", args[0])) {
      usage();
      return 0;
    }
  }
  /* Initialise fbus code */

  fbusinit(NULL);

  Line = OLine;

  /* Go through data from stdin. */

  while (GetLine(stdin, Line, sizeof(OLine))!=-1) {

    current=0;BackLine[current++]=Line[0];
    for (i=1;i<strlen(Line);i++) {
      if (Line[i-1]==';' && Line[i]==';') BackLine[current++]=' ';
      BackLine[current++]=Line[i];
    }
    BackLine[current++]=0;

    strcpy(Line,BackLine);

    line_count++;

#if defined(__svr4__) || defined(__FreeBSD__)
    ptr=strtok(Line, ";"); if (ptr) strcpy(entry.Name, ptr);

    entry.Number[0]=0;
    ptr=strtok(NULL, ";"); if (ptr && ptr[0]!=' ') strcpy(entry.Number, ptr);

    ptr=strtok(NULL, ";");
#else
    ptr=strsep(&Line, ";"); if (ptr) strcpy(entry.Name, ptr);

    entry.Number[0]=0;
    ptr=strsep(&Line, ";"); if (ptr && ptr[0]!=' ') strcpy(entry.Number, ptr);

    ptr=strsep(&Line, ";");
#endif

    if (!ptr) {
      fprintf(stderr, _("Format problem on line %d [%s] 1\n"), line_count, BackLine);
      Line = OLine;
      continue;
    }

    if (!strncmp(ptr,"ME", 2))
    {
      memory_type_string = "int";
      entry.MemoryType = GMT_ME;
    }
    else if (!strncmp(ptr,"SM", 2))
    {
        memory_type_string = "sim";
        entry.MemoryType = GMT_SM;
    }
    else
    {
      fprintf(stderr, _("Format problem on line %d [%s] 2: %s\n"),
        line_count, BackLine,ptr);
      break;
    }

#if defined(__svr4__) || defined(__FreeBSD__)
    ptr=strtok(NULL, ";"); if (ptr) entry.Location=atoi(ptr);

    ptr=strtok(NULL, ";"); if (ptr) entry.Group=atoi(ptr);
#else
    ptr=strsep(&Line, ";"); if (ptr) entry.Location=atoi(ptr);

    ptr=strsep(&Line, ";"); if (ptr) entry.Group=atoi(ptr);
#endif

    if (!ptr) {
      fprintf(stderr, _("Format problem on line %d [%s] 3\n"),
        line_count, BackLine);
      continue;
    }

    for( subentry = 0; ; subentry++ )
    {
#if defined(__svr4__) || defined(__FreeBSD__)
      ptr=strtok(NULL, ";");
#else
      ptr=strsep(&Line, ";");
#endif
      if( ptr &&  *ptr != 0 )
        entry.SubEntries[subentry].EntryType=atoi(ptr);
      else
        break;

#if defined(__svr4__) || defined(__FreeBSD__)
      ptr=strtok(NULL, ";");
#else
      ptr=strsep(&Line, ";");
#endif
      if(ptr)
        entry.SubEntries[subentry].NumberType=atoi(ptr);
      // Phone Numbers need to have a number type.
      if(!ptr && entry.SubEntries[subentry].EntryType == GSM_Number)
      {
        fprintf(stderr, _("Missing phone number type on line %d"
          " entry %d [%s]\n"), line_count, subentry, BackLine);
        subentry--;
        break;
      }

#if defined(__svr4__) || defined(__FreeBSD__)
      ptr=strtok(NULL, ";");
#else
      ptr=strsep(&Line, ";");
#endif
      if(ptr)
        entry.SubEntries[subentry].BlockNumber=atoi(ptr);

#if defined(__svr4__) || defined(__FreeBSD__)
      ptr=strtok(NULL, ";");
#else
      ptr=strsep(&Line, ";");
#endif
      // 0x13 Date Type; it is only for Dailed Numbers, etc.
      // we don't store to this memories so it's an error to use it.
      if(!ptr || entry.SubEntries[subentry].EntryType == GSM_Date) 
      {
        fprintf(stderr, _("Is not a phone number on line %d entry %d [%s]\n"),
          line_count, subentry, BackLine);
        subentry--;
        break;
      }
      else
        strcpy( entry.SubEntries[subentry].data.Number, ptr );
    }

    entry.SubEntriesCount = subentry;


    /* This is to send other exports (like from 6110) to 7110 */
    if (!entry.SubEntriesCount) {
	entry.SubEntriesCount = 1;
	entry.SubEntries[subentry].EntryType   = GSM_Number;
	entry.SubEntries[subentry].NumberType  = GSM_General;
	entry.SubEntries[subentry].BlockNumber = 2;
	strcpy(entry.SubEntries[subentry].data.Number, entry.Number);
    }

    Line = OLine;

    if (argc) {
      GSM_PhonebookEntry tmp_entry;

      memcpy(&tmp_entry, &entry, sizeof(GSM_PhonebookEntry) );
      error = GSM->GetMemoryLocation(&tmp_entry);
      if (error == GE_NONE) {
        if (!tmp_entry.Empty) {
          int confirm = -1;
          char ans[8];
	  FILE *input_flow; //for reading from console, even when input redir.
				
	  input_flow = fopen(DEV_CONSOLE, "r");

	  if (!input_flow) {
		fprintf(stderr, _("Can't open \"%s\" for input !\n"),DEV_CONSOLE);
		return(-1);
	  }
	  
          fprintf(stderr, _("Location busy. "));
          while (confirm < 0) {
            fprintf(stderr, _("Overwrite? (yes/no) "));
            GetLine(input_flow, ans, 7);
            if (!strcmp(ans, "yes")) confirm = 1;
            else if (!strcmp(ans, "no")) confirm = 0;
          }
          if (!confirm) continue;
        }
      } else {
        fprintf(stderr, _("Unknown error (%d)\n"), error);
        GSM->Terminate();
        return 0;
      }
    }

    /* Do write and report success/failure. */
    semicolon_pipe_substitution( &entry, 1 );

    error = GSM->WritePhonebookLocation(&entry);

    if (error == GE_NONE)
      fprintf (stdout, _("Write Succeeded: memory type: %s, loc: %d, name: %s, number: %s\n"), memory_type_string, entry.Location, entry.Name, entry.Number);
    else
      fprintf (stdout, _("Write FAILED(%d): memory type: %s, loc: %d, name: %s, number: %s\n"), error, memory_type_string, entry.Location, entry.Name, entry.Number);

  }

  GSM->Terminate();

  return 0;
}

/* Getting speed dials. */

int getspeeddial(char *Number) {

  GSM_SpeedDial entry;
  GSM_Error error;
  GSM_PhonebookEntry pbentry;

  entry.Number = atoi(Number);

  fbusinit(NULL);

  if (GSM->GetSpeedDial(&entry)==GE_NONE) {
        pbentry.Location=entry.Location;
        if (pbentry.Location==0) pbentry.Location=entry.Number;
        pbentry.MemoryType=entry.MemoryType;

        error=GSM->GetMemoryLocation(&pbentry);

        if (error == GE_NONE)
        {
          fprintf(stdout, _("SpeedDial nr. %d: %d:%d (%s)\n"), entry.Number, entry.MemoryType, entry.Location,pbentry.Name);
        } else
          fprintf(stdout, _("Error\n"));
  } else {
    fprintf(stdout, _("Error\n"));
  }
  
  GSM->Terminate();

  return 0;
}

/* Setting speed dials. */

int setspeeddial(char *argv[]) {

  GSM_SpeedDial entry;

  char *memory_type_string;

  /* Handle command line args that set type, start and end locations. */

  if (strcmp(argv[1], "ME") == 0) {
    entry.MemoryType = GMT_ME;
    memory_type_string = "ME";
  }
  else if (strcmp(argv[1], "SM") == 0) {
    entry.MemoryType = GMT_SM;
    memory_type_string = "SM";
  }
  else {
    fprintf(stderr, _("Unknown memory type %s!\n"), argv[1]);

    return -1;
  }
  
  entry.Number = atoi(argv[0]);
  entry.Location = atoi(argv[2]);

  fbusinit(NULL);

  if (GSM->SetSpeedDial(&entry) == GE_NONE) {
    fprintf(stdout, _("Succesfully written!\n"));
  }

  GSM->Terminate();

  return 0;
}

/* Getting the status of the display. */

int getdisplaystatus()
{ 

  int Status;

  /* Initialise the code for the GSM interface. */     

  fbusinit(NULL);

  if (GSM->GetDisplayStatus(&Status)==GE_NONE) {

    printf(_("Call in progress: %s\n"), Status & (1<<DS_Call_In_Progress)?_("on"):_("off"));
    printf(_("Unknown: %s\n"),          Status & (1<<DS_Unknown)?_("on"):_("off"));
    printf(_("Unread SMS: %s\n"),       Status & (1<<DS_Unread_SMS)?_("on"):_("off"));
    printf(_("Voice call: %s\n"),       Status & (1<<DS_Voice_Call)?_("on"):_("off"));
    printf(_("Fax call active: %s\n"),  Status & (1<<DS_Fax_Call)?_("on"):_("off"));
    printf(_("Data call active: %s\n"), Status & (1<<DS_Data_Call)?_("on"):_("off"));
    printf(_("Keyboard lock: %s\n"),    Status & (1<<DS_Keyboard_Lock)?_("on"):_("off"));
    printf(_("SMS storage full: %s\n"), Status & (1<<DS_SMS_Storage_Full)?_("on"):_("off"));

  } else {
    printf(_("Error\n"));
  }
  
  GSM->Terminate();

  return 0;
}

int netmonitor(char *Mode)
{

  unsigned char mode=atoi(Mode);
  char Screen[NM_MAX_SCREEN_WIDTH];
  int i;

  fbusinit(NULL);

  if (!strcmp(Mode,"reset"))     mode=0xf0;
  else if (!strcmp(Mode,"off"))  mode=0xf1;
  else if (!strcmp(Mode,"field"))mode=0xf2;
  else if (!strcmp(Mode,"devel"))mode=0xf3;
  else if (!strcmp(Mode,"next")) mode=0x00;

  /* We clear it */
  for (i=0;i<NM_MAX_SCREEN_WIDTH;i++) Screen[i]=0;

  GSM->NetMonitor(mode, Screen);

  if (Screen)
    printf("%s\n", Screen);

  GSM->Terminate();

  return 0;
}

int identify( void )
{
  /* Hopefully is 64 larger as FB38_MAX* / FB61_MAX* */
  char imei[64], model[64], rev[64], manufacturer[64];

  fbusinit(NULL);

  while (GSM->GetIMEI(imei)    != GE_NONE) sleep(1);
  while (GSM->GetRevision(rev) != GE_NONE) sleep(1);
  while (GSM->GetModel(model)  != GE_NONE) sleep(1);

  strcpy(manufacturer, "(unknown)");
  GSM->GetManufacturer(manufacturer);

  fprintf(stdout, _("IMEI:     %s\n"), imei);
  fprintf(stdout, _("Model:    %s %s (%s)\n"), manufacturer, GetModelName (model), model);
  fprintf(stdout, _("Revision: %s\n"), rev);

  GSM->Terminate();

  return 0;
}

int senddtmf(char *String)
{

  fbusinit(NULL);

  if (GSM->SendDTMF(String)!=GE_NONE) fprintf(stdout,_("Error!\n"));

  GSM->Terminate();

  return 0;
}

/* Resets the phone */
int reset(int argc, char *argv[])
{

  unsigned char _type=0x03;

  if (argc>0) {
    _type=0x10;

    if (!strcmp(argv[0],"soft")) _type = 0x03;
    
    /* Doesn't work with 5110 */
    if (!strcmp(argv[0],"hard")) _type = 0x04;

    if (_type==0x10) {
      fprintf(stderr, _("What kind of reset do you want (second parameter can be \"soft\" or \"hard\") ?\n"));
      return -1;
    }
  }

  fbusinit(NULL);

  GSM->Reset(_type);

  GSM->Terminate();

  return 0;
}

/* This is a "convenience" function to allow quick test of new API stuff which
   doesn't warrant a "proper" command line function. */

int foogle(char *argv[])
{
  /* Initialise the code for the GSM interface. */     

  fbusinit(NULL);

  // Fill in what you would like to test here...

  sleep(5);
  
  GSM->Terminate();

  return 0;
}

int phonetests()
{
  /* Initialise the code for the GSM interface. */     

  fbusinit(NULL);

  if (GSM->PhoneTests()!=GE_NONE) fprintf(stderr,_("Error\n"));
  
  GSM->Terminate();

  return 0;
}

/* pmon allows fbus code to run in a passive state - it doesn't worry about
   whether comms are established with the phone.  A debugging/development
   tool. */

int pmon()
{ 

  GSM_Error error;
  GSM_ConnectionType connection=GCT_FBUS;

  /* Initialise the code for the GSM interface. */     

  error = GSM_Initialise(model, Port, Initlength, connection, RLP_DisplayF96Frame, SynchronizeTime);

  if (error != GE_NONE) {
    fprintf(stderr, _("GSM/FBUS init failed! (Unknown model ?). Quitting.\n"));
    return -1;
  }


  while (1) {
    usleep(50000);
  }

  return 0;
}

int setringtone(int argc, char *argv[])
{
  GSM_Ringtone ringtone;
  GSM_BinRingtone binringtone,binringtone2;

  GSM_Error error;

  int current=0; //number of packed notes or location
  int i;

  char model[64];

  /* If not binary ringtone */
  if (GSM_ReadBinRingtoneFile(argv[0],&binringtone2)!=GE_NONE) {
    fprintf(stdout,_("Not binary ringtone, trying RTTL\n"));

    if (GSM_ReadRingtoneFileOnConsole(argv[0], &ringtone)!=GE_NONE) return(-1);

    ringtone.location=1;
    if (argc>1) ringtone.location=atoi(argv[1]);

    ringtone.allnotesscale=false;
  
    /* Initialise the GSM interface. */
    fbusinit(NULL);

    while (GSM->GetModel(model)  != GE_NONE)
      sleep(1);

    /* For Nokia 6110/6130/6150/6210 we use different method of uploading.
       Phone will display menu, when received it */
    if ( !strcmp(model,"NSE-3") || !strcmp(model,"NSK-3") ||
         !strcmp(model,"NSM-1") || !strcmp(model,"NPE-3") ) {
      if (argc==1) ringtone.location=255;
    }

    error=GSM->SetRingtone(&ringtone,&current);
  
    if (current!=ringtone.NrNotes) {
      if (current>FB61_MAX_RINGTONE_NOTES) {
        fprintf(stderr,_("Warning: due to phone limitation"));
      } else {
        fprintf(stderr,_("Warning: ringtone was too long to be saved into frame,"));
      }
      fprintf(stderr, _(" only %i first notes were packed (%i cut)\n"),current,ringtone.NrNotes-current);
    }

    if (error==GE_NONE) 
      fprintf(stdout, _("Set succeeded!\n"));
    else
      fprintf(stdout, _("Setting failed\n"));

  } else { /* This IS binary ringtone */
    fprintf(stdout,_("Binary ringtone format\n"));

    if (argc==3) {
      binringtone.frame[current++]=0x00;
      binringtone.frame[current++]=0x00;
      binringtone.frame[current++]=0x0c;
      binringtone.frame[current++]=0x01;
      binringtone.frame[current++]=0x2c;
    
      //copy new name
      memcpy(binringtone.frame+current,argv[2],strlen(argv[2]));
      current=current+strlen(argv[2]);

      /*don't use old*/
      i=2;
      while(binringtone2.frame[i]!=0x00) {
        if (i==binringtone.length) break;
        i++;
      }
    
      //copy binringtone
      memcpy(binringtone.frame+current,binringtone2.frame+i,binringtone2.length-i);
      binringtone.length=binringtone2.length-i+current;    
    } else
    {
      memcpy(binringtone.frame+current,binringtone2.frame,binringtone2.length);
      binringtone.length=binringtone2.length;
    }
  
    binringtone.location=1;
    if (argc>1) binringtone.location=atoi(argv[1]);
  
    /* Initialise the GSM interface. */
    fbusinit(NULL);

    error=GSM->SetBinRingtone(&binringtone);

    switch (error) {
      case GE_NONE:
        fprintf(stdout, _("Set succeeded!\n"));
        break;
      case GE_INVALIDRINGLOCATION:
        fprintf(stdout, _("Invalid location %i!\n"),binringtone.location);
        break;
      case GE_UNKNOWNMODEL:
        fprintf(stdout, _("Mygnokii doesn't know format for this model!\n"));
        break;
      case GE_NOTIMPLEMENTED:
        fprintf(stdout, _("Not implemented for this model!\n"));
        break;
      default:
        fprintf(stdout, _("Not supported by this model!\n"));
        break;
    }
  }

  GSM->Terminate();

  return 0;
}

int ringtoneconvert(int argc, char *argv[])
{
  GSM_Ringtone ringtone;

  if (!strcmp(argv[0],argv[1]))
  {
    fprintf(stderr, _("Files can't have the same names !\n"));
    return -1;  
  }
  
  if (GSM_ReadRingtoneFileOnConsole(argv[0], &ringtone)!=GE_NONE) return(-1);

  ringtone.allnotesscale=false;

  GSM_SaveRingtoneFileOnConsole(argv[1], &ringtone);
  
  return 0;
}

int playringtone(int argc, char *argv[])
{
  GSM_Ringtone ringtone;

  GSM_BinRingtone binringtone;
  
  int i,j,z;
  int Hz;
  bool isok;

//{ "c", "c#", "d", "d#", "e",  "f", "f#", "g", "g#", "a", "a#", "h" };
  int binary_notes[12] =
  { 0,    1,    2,   3,    4,   6,   7,    8,   9,    10,  11 ,   12};

  if (GSM_ReadBinRingtoneFile(argv[0],&binringtone)!=GE_NONE) {
    fprintf(stdout,_("Not binary ringtone, trying RTTL\n"));

    if (GSM_ReadRingtoneFileOnConsole(argv[0], &ringtone)!=GE_NONE) return(-1);

    /* Initialise the GSM interface. */
    fbusinit(NULL);

    fprintf(stdout,_("Playing \"%s\" (%i notes)\n"),ringtone.name,ringtone.NrNotes);
  
    GSM_PlayRingtoneOnConsole(&ringtone);

  } else {

    fprintf(stdout,_("Binary ringtone format\n"));  

    /* Initialise the GSM interface. */
    fbusinit(NULL);

    i=5;
    while (true) { //skipping name
      if (binringtone.frame[i]==0) break;
      i++;
    }

    i=i+5;j=0;z=0;
    while (true) {
      if (binringtone.frame[i]!=j) {
        if (j!=0) {
//#ifdef DEBUG
          fprintf(stdout,_("Block in binary ringtone %i %i\n"),j,z);
//#endif
          isok=false;
          if (j==64) {
	    j=255;//Pause
            isok=true;
	  }
	  if (!isok && j>=114 && j<=125) {
	    j=14*3+binary_notes[j-114];
	    isok=true;
	  }
	  if (!isok && j>=126 && j<=137) {
	    j=14*1+binary_notes[j-126];
	    isok=true;
	  }
	  if (!isok && j>=138 && j<=149) {
	    j=14*2+binary_notes[j-138];
	    isok=true;
          }
	  if (!isok && j>=150 && j<=161) {
	    j=14*4+binary_notes[j-150];
	    isok=true;
	  }
	  if (!isok) {
//#ifdef DEBUG
	    fprintf(stdout,_("Unknown block in binary ringtone %i %i\n"),j,z);
//#endif
          } else {
	    Hz=GSM_GetFrequency(j);
	    if (GSM->PlayTone(Hz,5)!=GE_NONE) fprintf(stdout,_("error during playing\n"));
	    usleep(z*5000);
	  }
        }
        j=binringtone.frame[i];
        z=binringtone.frame[i+1];
      } else {
        z=z+binringtone.frame[i+1];
      }
      i=i+2;
      if (i>=binringtone.length) break;
    }
  
    GSM->PlayTone(0,0);
  }

  GSM->Terminate();    

  return 0;
}

int composer(int argc, char *argv[])
{
  GSM_Ringtone ringtone;
  int i;
  bool unknown;
  int oldnoteslen=4,nownoteslen,oldnoteslen2;
  int oldnotesscale=1,nownotesscale;  
  bool firstnote=true;
  int DefNoteTempo=63;
  
  if (GSM_ReadRingtoneFileOnConsole(argv[0], &ringtone)!=GE_NONE) return(-1);
 
  if (ringtone.NrNotes!=0)
    DefNoteTempo=ringtone.notes[0].tempo;

  fprintf(stdout,_("Ringtone \"%s\" (tempo = %i Beats Per Minute)\n\n"),ringtone.name,GSM_GetTempo(DefNoteTempo));
  
  for (i=0;i<ringtone.NrNotes;i++) {    
    if (DefNoteTempo!=ringtone.notes[i].tempo) {
      fprintf(stdout,_("WARNING: IN FACT RINGTONE HAS DIFFERENT TEMPO FOR DIFFERENT NOTES AND THAT'S WHY YOU CAN'T ENTER IT ALL IN THE COMPOSITOR\n\n"));
      break;
    }
  }
    
  for (i=0;i<ringtone.NrNotes;i++) {

    if (firstnote) {
      if (GSM_GetNote(ringtone.notes[i].note)!=Note_Pause)
        firstnote=false;
    }
    
    if (!firstnote) {
      break;
    }
    
  }

  if ((ringtone.NrNotes-i)>50)
    fprintf(stdout,_("WARNING: LENGTH=%i, BUT YOU WILL ENTER ONLY FIRST 50 TONES. TO ENTER FULL RINGTONE MUST SEND IT (--sendringtone) OR DOWNLOAD (--setringtone)\n\n"),ringtone.NrNotes-i);

  fprintf(stdout,_("This ringtone in Nokia Composer in phone should look: "));  

  firstnote=true;
    
  for (i=0;i<ringtone.NrNotes;i++) {

    if (firstnote) {
      if (GSM_GetNote(ringtone.notes[i].note)!=Note_Pause)
        firstnote=false;
    }
    
    if (!firstnote) {
    
      switch (ringtone.notes[i].duration) {
        case 192:fprintf(stdout,_("1."));break; //192=128*1.5
        case 128:fprintf(stdout,_("1"));break;
        case 96 :fprintf(stdout,_("2."));break; //96=64*1.5
        case 64 :fprintf(stdout,_("2"));break;
        case 48 :fprintf(stdout,_("4."));break; //48=32*1.5
        case 32 :fprintf(stdout,_("4"));break;
        case 24 :fprintf(stdout,_("8."));break; //24=16*1.5
        case 16 :fprintf(stdout,_("8"));break;
        case 12 :fprintf(stdout,_("16."));break; //12=8*1.5
        case 8  :fprintf(stdout,_("16"));break;
        case 6  :fprintf(stdout,_("32."));break; //6=4*1.5
        case 4  :fprintf(stdout,_("32"));break;
      }
    
    unknown=false;
    
    /* What note here ? */
    switch (GSM_GetNote(ringtone.notes[i].note)) {
      case Note_C  :fprintf(stdout,_("c"));break;
      case Note_Cis:fprintf(stdout,_("#c"));break;
      case Note_D  :fprintf(stdout,_("d"));break;
      case Note_Dis:fprintf(stdout,_("#d"));break;
      case Note_E  :fprintf(stdout,_("e"));break;
      case Note_F  :fprintf(stdout,_("f"));break;
      case Note_Fis:fprintf(stdout,_("#f"));break;
      case Note_G  :fprintf(stdout,_("g"));break;
      case Note_Gis:fprintf(stdout,_("#g"));break;
      case Note_A  :fprintf(stdout,_("a"));break;
      case Note_Ais:fprintf(stdout,_("#a"));break;
      case Note_H  :fprintf(stdout,_("h"));break;
      default      :fprintf(stdout,_("-"));unknown=true;break; //Pause ?
    }

    if (!unknown)
      fprintf(stdout,_("%i"),ringtone.notes[i].note/14);
    
    /* And separator before next note */
    if (i!=ringtone.NrNotes-1)
      fprintf(stdout,_(" "));
      
    }

  }

  fprintf(stdout,_("\n\nTo enter it please press: "));  
  
  firstnote=true;
  
  for (i=0;i<ringtone.NrNotes;i++) {

    if (firstnote) {
      if (GSM_GetNote(ringtone.notes[i].note)!=Note_Pause)
        firstnote=false;
    }
    
    if (!firstnote) {

    unknown=false;
    
    /* What note here ? */
    switch (GSM_GetNote(ringtone.notes[i].note)) {
      case Note_C  :fprintf(stdout,_("1"));break;
      case Note_Cis:fprintf(stdout,_("1"));break;
      case Note_D  :fprintf(stdout,_("2"));break;
      case Note_Dis:fprintf(stdout,_("2"));break;
      case Note_E  :fprintf(stdout,_("3"));break;
      case Note_F  :fprintf(stdout,_("4"));break;
      case Note_Fis:fprintf(stdout,_("4"));break;
      case Note_G  :fprintf(stdout,_("5"));break;
      case Note_Gis:fprintf(stdout,_("5"));break;
      case Note_A  :fprintf(stdout,_("6"));break;
      case Note_Ais:fprintf(stdout,_("6"));break;
      case Note_H  :fprintf(stdout,_("7"));break;
      default      :fprintf(stdout,_("0"));unknown=true;break;
    }

      switch (ringtone.notes[i].duration) {
        case 192:fprintf(stdout,_("(longer)"));break; //192=128*1.5
        case 96 :fprintf(stdout,_("(longer)"));break; //96=64*1.5
        case 48 :fprintf(stdout,_("(longer)"));break; //48=32*1.5
        case 24 :fprintf(stdout,_("(longer)"));break; //24=16*1.5
        case 12 :fprintf(stdout,_("(longer)"));break; //12=8*1.5
        case 6  :fprintf(stdout,_("(longer)"));break; //6=4*1.5
        default:break;
      }

    /* What note here ? */
    switch (GSM_GetNote(ringtone.notes[i].note)) {
      case Note_Cis:fprintf(stdout,_("#"));break;
      case Note_Dis:fprintf(stdout,_("#"));break;
      case Note_Fis:fprintf(stdout,_("#"));break;
      case Note_Gis:fprintf(stdout,_("#"));break;
      case Note_Ais:fprintf(stdout,_("#"));break;
      default      :break;
    }

    if (!unknown)
    {
      nownotesscale=ringtone.notes[i].note/14;

      if (nownotesscale!=oldnotesscale) {
        switch (nownotesscale) {
	  case 1:
	    switch (oldnotesscale) {
	      case 2:fprintf(stdout,_("**"));break;
	      case 3:fprintf(stdout,_("*"));break;
	    }
	    break;
	  case 2:
	    switch (oldnotesscale) {
	      case 1:fprintf(stdout,_("*"));break;
	      case 3:fprintf(stdout,_("**"));break;
	    }
	    break;
	  case 3:
	    switch (oldnotesscale) {
	      case 1:fprintf(stdout,_("**"));break;
	      case 2:fprintf(stdout,_("*"));break;
	    }
	    break;
	}
      }
      
      oldnotesscale=nownotesscale;
    }

     nownoteslen=0;
     oldnoteslen2=oldnoteslen;
     
      switch (ringtone.notes[i].duration) {
        case 192:nownoteslen=1;break; //192=128*1.5
        case 128:nownoteslen=1;break;
        case 96 :nownoteslen=2;break; //96=64*1.5
        case 64 :nownoteslen=2;break;
        case 48 :nownoteslen=4;break; //48=32*1.5
        case 32 :nownoteslen=4;break;
        case 24 :nownoteslen=8;break; //24=16*1.5
        case 16 :nownoteslen=8;break;
        case 12 :nownoteslen=16;break; //12=8*1.5
        case 8  :nownoteslen=16;break;
        case 6  :nownoteslen=32;break; //6=4*1.5
        case 4  :nownoteslen=32;break;
      }    
      
      if (nownoteslen>oldnoteslen) {
        while (oldnoteslen!=nownoteslen) {
	  fprintf(stdout,_("8"));
	  oldnoteslen=oldnoteslen*2;
	}
      }

      if (nownoteslen<oldnoteslen) {
        while (oldnoteslen!=nownoteslen) {
	  fprintf(stdout,_("9"));
	  oldnoteslen=oldnoteslen/2;
	}
      }      

    if (GSM_GetNote(ringtone.notes[i].note)==Note_Pause)
      oldnoteslen=oldnoteslen2;
      
    /* And separator before next note */
    if (i!=ringtone.NrNotes-1)
      fprintf(stdout,_("  "));

  }
  }  

  fprintf(stdout,_("\n"));

  return 0;

}

int sendringtone(int argc, char *argv[])
{
  GSM_Ringtone ringtone;
  GSM_MultiSMSMessage SMS;
  int current,i;
  bool ProfileStyle=false; /* If we use profile style available in new Nokia models */

  if (GSM_ReadRingtoneFileOnConsole(argv[1], &ringtone)!=GE_NONE) return(-1);

  ringtone.allnotesscale=false;

  for (i=0;i<argc;i++) {
    if (!strcmp(argv[i],"--profilestyle")) ProfileStyle=true;
    if (!strcmp(argv[i],"--scale")) ringtone.allnotesscale=true;
  }

  current=GSM_SaveRingtoneToSMS(&SMS,&ringtone,ProfileStyle);

  if (current!=ringtone.NrNotes) {
    if (current>FB61_MAX_RINGTONE_NOTES) {
      fprintf(stderr,_("Warning: due to phone limitation"));
    } else {
      fprintf(stderr,_("Warning: ringtone was too long to be saved into SMS,"));
    }
    fprintf(stderr, _(" only %i first notes were packed (%i cut)\n"),current,ringtone.NrNotes-current);
  }

  for (i=0;i<SMS.number;i++) {
    strcpy(SMS.SMS[i].Destination,argv[0]);
  }

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  GSM_SendMultiPartSMSOnConsole(&SMS, 2,argc,argv,false,true,true);

  return 0;
}

int saveringtone(int argc, char *argv[])
{
  GSM_Ringtone ringtone;
  GSM_MultiSMSMessage SMS;
  int current,i;
  bool ProfileStyle=false; /* If we use profile style available in new Nokia models */

  if (GSM_ReadRingtoneFileOnConsole(argv[0], &ringtone)!=GE_NONE) return(-1);

  ringtone.allnotesscale=false;

  for (i=0;i<argc;i++) {
    if (!strcmp(argv[i],"--profilestyle")) ProfileStyle=true;
    if (!strcmp(argv[i],"--scale")) ringtone.allnotesscale=true;
  }

  current=GSM_SaveRingtoneToSMS(&SMS,&ringtone,ProfileStyle);

  if (current!=ringtone.NrNotes) {
    if (current>FB61_MAX_RINGTONE_NOTES) {
      fprintf(stderr,_("Warning: due to phone limitation"));
    } else {
      fprintf(stderr,_("Warning: ringtone was too long to be saved into SMS,"));
    }
    fprintf(stderr, _(" only %i first notes were packed (%i cut)\n"),current,ringtone.NrNotes-current);
  }

  for (i=0;i<SMS.number;i++) {
    /* Only 11 chars could be here */
    strncpy(SMS.SMS[i].Destination,ringtone.name,11);
  }

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  GSM_SaveMultiPartSMSOnConsole(&SMS,1,argc,argv,false,false,true,true);

  return 0;
}

/* Converts logo files. */

int bitmapconvert(int argc, char *argv[])
{
  GSM_Bitmap bitmap;
  GSM_NetworkInfo NetworkInfo;
  bool doit;
  int num;
  
  if (!strcmp(argv[0],argv[1]))
  {
    fprintf(stderr, _("Files can't have the same names !\n"));
    return -1;  
  }
  
  if (GSM_ReadBitmapFileOnConsole(argv[0], &bitmap)!=GE_NONE) return(-1);

  if (argc>2)
  {
    doit=false;
    if (!strcmp(argv[2],"op"))
    {
      doit=false;
      if (argc<4) doit=true;
      if (argc<4 && bitmap.type!=GSM_OperatorLogo) doit=true;
      if (doit)
      {
        fbusinit(NULL);
        if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE) strncpy(bitmap.netcode,NetworkInfo.NetworkCode,7);
        GSM->Terminate();
      }
      GSM_ResizeBitmap(&bitmap,GSM_OperatorLogo);
      if (argc==4)
      {
        strncpy(bitmap.netcode,argv[3],7);
	if (!strcmp(GSM_GetNetworkName(bitmap.netcode),"unknown"))
	{
	  fprintf(stderr,"Sorry, gnokii doesn't know \"%s\" network !\n",bitmap.netcode);
	  return -1;
        }
      }
      doit=true;
    }
    if (!strcmp(argv[2],"7110op"))
    {
      doit=false;
      if (argc<4) doit=true;
      if (argc<4 && bitmap.type!=GSM_7110OperatorLogo) doit=true;
      if (doit)
      {
        fbusinit(NULL);
        if (GSM->GetNetworkInfo(&NetworkInfo) == GE_NONE) strncpy(bitmap.netcode,NetworkInfo.NetworkCode,7);
        GSM->Terminate();
      }
      GSM_ResizeBitmap(&bitmap,GSM_7110OperatorLogo);
      if (argc==4)
      {
        strncpy(bitmap.netcode,argv[3],7);
	if (!strcmp(GSM_GetNetworkName(bitmap.netcode),"unknown"))
	{
	  fprintf(stderr,"Sorry, gnokii doesn't know \"%s\" network !\n",bitmap.netcode);
	  return -1;
        }
      }
      doit=true;
    }
    if (!strcmp(argv[2],"caller"))
    {
      GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
      if (argc>3)
      {
        num=argv[3][0]-'0';
        if ((num<0)||(num>9)) num=0;
	bitmap.number=num;
      } else
      {
        bitmap.number=0;
      }
      doit=true;
    }
    if (!strcmp(argv[2],"startup"))
    {
      GSM_ResizeBitmap(&bitmap,GSM_StartupLogo);
      doit=true;
    }
    if (!strcmp(argv[2],"7110startup"))
    {
      GSM_ResizeBitmap(&bitmap,GSM_7110StartupLogo);
      doit=true;
    }
    if (!strcmp(argv[2],"6210startup"))
    {
      GSM_ResizeBitmap(&bitmap,GSM_6210StartupLogo);
      doit=true;
    }
    if (!strcmp(argv[2],"picture"))
    {
      GSM_ResizeBitmap(&bitmap,GSM_PictureImage);
      doit=true;
    }
    if (!doit)
    {
      fprintf(stderr,"Unknown type of logo: %s !\n",argv[2]);
      return -1;
    }
  }
    
  if (GSM_SaveBitmapFileOnConsole(argv[1], &bitmap)!=GE_NONE) return(-1);
  
  return 0;
}

int getphoneprofile()
{
  GSM_PPS PPS;
  GSM_Error error;

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  PPS.Name=PPS_ALS;
  error=GSM->GetProductProfileSetting(&PPS);
  if (error!=GE_NONE) {
    fprintf(stdout,_("Error!\n"));
    GSM->Terminate();    
    return -1;
  }
  fprintf(stdout,_("ALS           : "));
  if (PPS.bool_value) fprintf(stdout,_("on\n"));
                 else fprintf(stdout,_("off\n"));

  PPS.Name=PPS_VibraMenu;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("Vibra menu    : "));
  if (PPS.bool_value) fprintf(stdout,_("on\n"));
                 else fprintf(stdout,_("off\n"));

  PPS.Name=PPS_GamesMenu;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("Games menu    : "));
  if (PPS.bool_value) fprintf(stdout,_("on\n"));
                 else fprintf(stdout,_("off\n"));

  PPS.Name=PPS_HRData;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("HR Data       : "));
  if (PPS.bool_value) fprintf(stdout,_("on\n"));
                 else fprintf(stdout,_("off\n"));

  PPS.Name=PPS_14400Data;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("14400 Data    : "));
  if (PPS.bool_value) fprintf(stdout,_("on\n"));
                 else fprintf(stdout,_("off\n"));

  PPS.Name=PPS_LCDContrast;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("LCD Contrast  : %i%%\n"),PPS.int_value);

  PPS.Name=PPS_EFR;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("EFR           : "));
  switch (PPS.int_value) {
    case 0: fprintf(stdout,_("off\n"));   break;
    case 1: fprintf(stdout,_("last\n"));  break;
    case 2: fprintf(stdout,_("second\n"));break;
    case 3: fprintf(stdout,_("first\n")); break;
  }

  PPS.Name=PPS_FR;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("FR            : "));
  switch (PPS.int_value) {
    case 0: fprintf(stdout,_("off\n"));   break;
    case 1: fprintf(stdout,_("last\n"));  break;
    case 2: fprintf(stdout,_("second\n"));break;
    case 3: fprintf(stdout,_("first\n")); break;
  }

  PPS.Name=PPS_HR;
  GSM->GetProductProfileSetting(&PPS);
  fprintf(stdout,_("HR            : "));
  switch (PPS.int_value) {
    case 0: fprintf(stdout,_("off\n"));   break;
    case 1: fprintf(stdout,_("last\n"));  break;
    case 2: fprintf(stdout,_("second\n"));break;
    case 3: fprintf(stdout,_("first\n")); break;
  }
    
  GSM->Terminate();    

  return 0;

}

int setphoneprofile(int argc, char *argv[])
{
  GSM_PPS PPS;
  GSM_Error error;
  bool correct_arg1=false, correct_arg2=false;

  if (!strcmp(argv[0],"ALS")) {
    PPS.Name=PPS_ALS;
    correct_arg1=true;
    if (!strcmp(argv[1],"1")) {
      PPS.bool_value=true;
      correct_arg2=true;
    }
    if (!strcmp(argv[1],"0")) {
      PPS.bool_value=false;
      correct_arg2=true;
    }
    if (!correct_arg2) {
      fprintf(stdout,_("Settings for ALS parameter can be \"0\" or \"1\" !\n"));
      return -1;
    }
  }
  if (!strcmp(argv[0],"HRData")) {
    PPS.Name=PPS_HRData;
    correct_arg1=true;
    if (!strcmp(argv[1],"1")) {
      PPS.bool_value=true;
      correct_arg2=true;
    }
    if (!strcmp(argv[1],"0")) {
      PPS.bool_value=false;
      correct_arg2=true;
    }
    if (!correct_arg2) {
      fprintf(stdout,_("Settings for HRData parameter can be \"0\" or \"1\" !\n"));
      return -1;
    }
  }  
  if (!correct_arg1) {
    fprintf(stdout,_("First parameter can be \"ALS\" or \"HRData\" only !\n"));
    return -1;
  }

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->SetProductProfileSetting(&PPS);

  if (error!=GE_NONE) {
    fprintf(stdout,_("Error!\n"));
    GSM->Terminate();    
    return -1;
  }

  GSM->Reset(0x03);
  
  GSM->Terminate();    

  return 0;

}

int getoperatorname()
{

  GSM_Network network;

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  if (GSM->GetOperatorName(&network)==GE_NONE)
  {
    if (!strcmp(network.Name,"")) {
      fprintf(stdout,_("Phone doesn't have downloaded operator name\n"));
    } else {
      fprintf(stdout,_("Phone has downloaded operator name (\"%s\") for \"%s\" (\"%s\") network\n"),
              network.Name,network.Code,GSM_GetNetworkName(network.Code));
    }
  }
  
  GSM->Terminate();    

  return 0;

}

int setoperatorname(int argc, char *argv[])
{

  GSM_Network network;

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  if (argc==2) {
    strncpy(network.Code,argv[0],7);
    strncpy(network.Name,argv[1],50);
  } else {
    strcpy(network.Code,"000 00\0");
    strcpy(network.Name,"\0");
  }
  
  if (GSM->SetOperatorName(&network)==GE_NONE) {
    if (GSM->GetOperatorName(&network)==GE_NONE) {
      if (!strcmp(network.Name,"")) {
        fprintf(stdout,_("Downloaded operator name is removed\n"));
      } else {
        fprintf(stdout,_("Downloaded operator name changed for \"%s\" network (\"%s\") to \"%s\"\n"),
                network.Code,GSM_GetNetworkName(network.Code),network.Name);
      }
    }
  }

  GSM->Terminate();    

  return 0;

}

int getvoicemailbox()
{
  GSM_PhonebookEntry entry;

  /* Initialise the GSM interface. */
  fbusinit(NULL);
  
  if (GSM->GetVoiceMailbox(&entry)==GE_NONE) {
    fprintf(stdout,_("Voice mailbox number is "));
    if (!strcmp(entry.Number,""))
      fprintf(stdout,_("not set\n"));
    else
      fprintf(stdout,_("\"%s\"\n"),entry.Number);
  }

  GSM->Terminate();    

  return 0;

}

///////////////////////////////////////////////////////////////////////////////
// flow diagram of netmonitordata():
///////////////////////////////////////////////////////////////////////////////
//
//		get command line argument
//			|			
//			v	
//
//		if specific phone model
//		    use that model
//		else
//		    identify-phone:	(((((((( TO DO )))))))))))))
//
//			|
//			v
//		get_mon_param_info()	get phone netmonitor parameters name
//					specifically to a 'model'
//			|
//			v
//		parse_check()		check command line arguments
//			|		if all OK, go on else stop.
//			|		because we can have a input file
//			v		containing multiple
//		parse_process()		command lines, we process each at once
//					so we can exit early on errors.
//
///////////////////////////////////////////////////////////////////////////////

// private functions, see netmonitordata()
void free_nm_info(PARAM_INFO_MON *info)
{
	PARAM_INFO_MON *tmp;	// pointer, iterator on info

	// go to end of list

	while (info->next != NULL)
	{	tmp = info->next;
		free(info->name);
		free(info->mname);
		free(info);
		info = tmp;
	}
	free(info);
}

///////////////////////////////////////////////////////////////////////////////
// this func retrieve parameters info from a file
// and load it in a dynamic array, NULL terminated,
// if 2rd parameter is not NULL, it load also description of screen
// return (PARAM_INFO_MON *start) if all is OK, else NULL
///////////////////////////////////////////////////////////////////////////////
// start->->next->next->next-> ...->next->NULL
//   |      |     |     |           |
//   V      V     V     V           V
//        par0  par1  par2        parN
///////////////////////////////////////////////////////////////////////////////
// this should be de-allocated from calling function,
// also, screen_name should be deallocated from calling function
///////////////////////////////////////////////////////////////////////////////

PARAM_INFO_MON *get_mon_param_info(char *f_name, char *screen_name[NM_MAX_SCREEN+1])
{
	PARAM_INFO_MON *start;	// pointer to netmonitor parameters info
	PARAM_INFO_MON *info;	// pointer, iterator on info
	PARAM_INFO_MON *check;	// pointer, iterator on info, for check usage

	FILE *f_info;

	char buf[256];
	char *param;
	char *param_name;
	char *tmp;

	char tmp_mname[55];		// very larger, but if netmon bug ...
	int x, y, len, s, t; 		// x & y coord, len, screen nr, type
	int i;

	if (screen_name != NULL)
		for (i = 0; i <= NM_MAX_SCREEN; i++)
			screen_name[i] = NULL;
		

	if ((f_info = fopen(f_name, "r")) == NULL)
	{	fprintf(stderr, "Can' t open file parameter info: <%s>\n", f_name);
		return(NULL);
	}
	
	if ((start = malloc(sizeof(PARAM_INFO_MON))) == NULL)
	{	fprintf(stderr, "no mem\n");
		return(NULL);
	}
	else
	{	start->next = NULL;
	}

	info = start;
	while (fgets(buf, 256, f_info) != NULL)
	{
		param = buf;

		// Truncate from '#' at right of comments
		if ((tmp = strchr(param, '#')) != NULL)
			*tmp = '\0';

		// Strip leading, trailing whitespace
		while(isspace((int) *param))
		    	param++;

		while((strlen(param) > 0) && isspace((int) param[strlen(param) - 1]))
			param[strlen(param) - 1] = '\0';
	
		// Ignore blank lines
		if ((*param == '\n') || (*param == '\0'))
			continue;


#ifdef DEBUG
		fprintf(stderr, "%s: info line: <%s>\n", f_name, param);
#endif
		// check for param name
		if ((param_name = strtok(param, "=\t\n")) == NULL)
			continue;

		// check if screen name
		if (strncmp("SCREEN", param_name, 6) == 0)
		{	
			// if we do not want screen names ...
			if (screen_name == NULL)
				continue;

// FIXME check for screen nr
			if ((tmp = strtok(NULL, ":\n")) == NULL)
				continue;
	
			i = atoi(tmp);
// FIXME: check if already defined screen

			if ((i <= 0) || (i > NM_MAX_SCREEN))
				continue;
			// check for screen name
			if ((tmp = strtok(NULL, ":\n")) == NULL)
				continue;

			screen_name[i] = strdup(tmp);
		}
		else 
		{
			// parameter

// FIXME: check for NM_MAX_FIELDS

			// check for x coord
			if ((tmp = strtok(NULL, ":\t\n")) == NULL)
				continue;
			x = atoi(tmp);

			// check for y coord
			if ((tmp = strtok(NULL, ":\t\n")) == NULL)
				continue;
			y = atoi(tmp);

			// check for len
			if ((tmp = strtok(NULL, ":\t\n")) == NULL)
				continue;
			len = atoi(tmp);
	
			// check for screen
			if ((tmp = strtok(NULL, ":\t\n")) == NULL)
				continue;
			s = atoi(tmp);

			// check for netmon manual name
			if ((tmp = strtok(NULL, ":\t\n")) == NULL)
			{	fprintf(stderr,
					"%s: PARAMETER <%s> in screen <%d>, not have netmon manual reference\n",
					f_name, param_name, s);
				free_nm_info(start);
				return(NULL);
			}
			strcpy(tmp_mname, tmp);

			// check for data type (optional)
			if ((tmp = strtok(NULL, ":\t\n")) != NULL)	
				t = *tmp;
			else
				t = '\0';

			// check len, here, so we print parameter name
			if (len == 0)
			{	fprintf(stderr,
					"%s: PARAMETER <%s> in screen <%d>, has invalid data lenght\n",
					f_name, param_name, s);
				free_nm_info(start);
				return(NULL);
			}

			// check if already defined same param_name
			check = start;
			while (check->next != NULL)
			{	check = check->next;
				if (strcmp(param_name, check->name) == 0)
				{
					fprintf(stderr,
					"%s: PARAMETER <%s> in screen <%d> already defined as in screen <%d>\n",
					f_name, param_name, s, check->s_nr);
					free_nm_info(start);
					return(NULL);
				}
			}

			// make space, and add parameter
			if ((info->next = malloc(sizeof(PARAM_INFO_MON))) != NULL)
			{
				info = info->next;
				info->name = strdup(param_name);
				info->x = x;
				info->y = y;
				info->len = len;
				info->s_nr = s;
				info->mname = strdup(tmp_mname);
				info->type = t;

				info->next = NULL;	// mark end
			}
			else
			{
				fprintf(stderr, "no mem");
				free_nm_info(start);
				return(NULL);
			}

		}
	
	}

	fclose(f_info);
	
#ifdef DEBUG
	info = start;
	while (info->next != NULL)
	{
		info = info->next;
		fprintf(stderr, "info name %s\n", info->name);
	}
#endif
	return(start);
}

// 2, parse the arguments and check command(s) line 
// command line, phone spec input, and output are complex,
// so we exit printing info about error instead of std help
///////////////////////////////////////////////////////////////////////////////

int parse_check(int argc, char *argv[], PARAM_INFO_MON *start, char *f_name, int line)
{
	int ctr;
	int i;
	int time_spec;
	int found_data;
	char *p;
	char *tmp;
	char *o;
	PARAM_INFO_MON *info;			// iterator on this list

#ifdef DEBUG
	for (i = 0; i < argc; i++)
  		fprintf(stderr, "argv[%d] = <%s>\n",i, argv[i]);
#endif

	time_spec = 0;
	ctr = 0; 
	i = 0;
	while (i < argc)
	{
		p = argv[i];
		if (*p == '-')
		{			// should be one of the short option
			if (strcmp(argv[i], "-fs") == 0)
			{		// field separator, next arg should be a string
				if (((i+1) < argc) && (strlen(argv[i+1]) < 10))
				{	i++;
					// arg OK, do nothing
				}
				else
				{	if (line)
						fprintf(stderr,
							"-fs: Invalid field separator in <%s> at line %d\n",
							f_name, line);
					else
						fprintf(stderr, "-fs: Invalid field separator\n");
					return(-1);
				}
			}
			else if (strcmp(argv[i], "-ls") == 0) 
			{		// line separator, next arg should be a string
				if (((i+1) < argc) && (strlen(argv[i+1]) < 10))
				{	i++;
					// arg OK, do nothing
				}
				else
				{	if (line)
						fprintf(stderr,
							"-ls: Invalid line separator in <%s> at line %d\n",
							f_name, line);
					else
						fprintf(stderr, "-ls: Invalid line separator\n");
					return(-1);
				}
			}
			else if (strcmp(argv[i], "-tm") == 0) 
			{		// time separator, next arg should be a millisecond (200-10000)
				if (time_spec)
				{	if (line)
						fprintf(stderr,
						"-tm, -ts, are mutually exclusive in <%s> at line %d\n",
						f_name, line);
					else
						fprintf(stderr, "-tm, -ts, are mutually exclusive\n");
					return(-1);
				}

				if (((i+1) < argc) && (atoi(argv[i+1]) >= 200) && (atoi(argv[i+1]) <= 10000))
				{	i++;
					time_spec = 1;
					// arg OK, do nothing
				}
				else
				{	if (line)
						fprintf(stderr,
			"-tm: Invalid argument (200-10000 milliseconds), in <%s> at line %d\n",
							f_name, line);
					else
						fprintf(stderr, "-tm: Invalid argument (200-10000 milliseconds)\n");
					return(-1);
				}
			}
			else if (strcmp(argv[i], "-ts") == 0) 
			{		// time separator, next arg should be a seconds (1-3600)
				if (time_spec)
				{	if (line)
						fprintf(stderr,
						"-tm, -ts, are mutually exclusive, in <%s> at line %d\n",
							f_name, line);
					else
						fprintf(stderr, "-tm, -ts, are mutually exclusive\n");
					return(-1);
				}

				if (((i+1) < argc) && (atoi(argv[i+1]) >= 1) && (atoi(argv[i+1]) <= 3600))
				{	i++;
					time_spec = 1;
					// arg OK, do nothing
				}
				else
				{	if (line)
						fprintf(stderr,
					"-ts: Invalid argument (1-3600 seconds) in <%s> at line %d\n",	
							f_name, line);
					else
						fprintf(stderr, "-ts: Invalid argument (1-3600 seconds)\n");
					return(-1);
				}
			}
			else if (strcmp(argv[i], "-n") == 0) 
			{		// nr of data pump, before stop collection,
					// next arg should be a int > 0
				if (((i+1) < argc) && (atoi(argv[i+1]) >= 1) && (atoi(argv[i+1]) <= 99999))
				{	i++;
					// arg OK, do nothing
				}
				else
				{	if (line)
						fprintf(stderr,
						"-n: Invalid argument (1-99999 times) in <%s> at line %d\n",
							f_name, line);
					else
						fprintf(stderr, "-n: Invalid argument (1-99999 times)\n");
					return(-1);
				}
			}
			else if (strcmp(argv[i], "-h") == 0) 
			{		// we do NOT want header (default with header)
					// arg OK, do nothing
			}
			else if (strcmp(argv[i], "-S") == 0) 
			{		// we have used use specs from a file instead of standard info,
					// next arg is an existing readable filename
					// as already parsed correctly, we skip here.
				i++;
			}
			else if (strcmp(argv[i], "-I") == 0) 
			{		// we have used input from a file instead of command line
					// next arg is an existing readable filename
					// as already parsed correctly, we skip here.
				i++;
			}	
			else
			{	
				if (line)
					fprintf(stderr, "Unrecognized option %s in <%s> at line %d\n",
						argv[i], f_name, line);
					else
						fprintf(stderr, "Unrecognized option %s\n", argv[i]);
				return(-1);
			}
		}
		else
		{			// should be required data
			tmp = strdup(argv[i]);
			p = strtok(tmp, ":\t\n");
			while (p != NULL)
			{
				// check if there is an output format specification
				o = p;
				while (*o)
				{
					o++;
					if (*o == '-')
					{	*o = '\0';
						o++;
						if (strlen(o) == 0)
						{
							// we have 'minus', so, probably forget format ...

							if (line)
								fprintf(stderr,
					 "Required data <%s->, without format specifiers in <%s> at line %d\n",
									p, f_name, line);
							else
								fprintf(stderr,
					 "Required data <%s->, without format specifiers\n", p);
							return(-1);
						}
					}
				}
			
								// we check for params
				found_data = 0;
				info = start;
				while (info->next != NULL)
				{	
					info = info->next;
					if (strcmp(p, info->name) == 0)
					{
						if (ctr > NM_MAX_FIELDS)
						{
							if (line)
								fprintf(stderr,
						"too much data field ... in file <%s> at line %d\n",
									f_name, line);
							else
						fprintf(stderr, "too much data field ...\n");
							return (-1);
						}
/*
						data[ctr] = info;
						out_f[ctr] = *o;
*/
						found_data = 1;
						ctr++;
						break;
					}
				}
			
				if (found_data == 0)
				{
					if (line)
						fprintf(stderr,
			"Required data <%s>, not found in info-mon specifications in <%s> at line %d\n",
							p, f_name, line);
					else
						fprintf(stderr,
			"Required data <%s>, not found in info-mon specifications\n", p);
					return(-1);
				}
	
				p = strtok(NULL, ":\t\n");

			} // end while strtok

		} // end else '-' (short options)
		
		i++;

	} // end while
	
	if (ctr == 0)
	{
		if (line)
			fprintf(stderr, "no required data! in <%s> at line %d\n", f_name, line);
		else
			fprintf(stderr, "no required data!\n");
		return(-1);
	}

	return 0;
}
							  
// 2, parse the arguments and process the command line
// no checks are needed here, because already do in parse_check
// it return a pointer to info needed for make output, or NULL if errors
///////////////////////////////////////////////////////////////////////////////
OUT_INFO_MON *parse_process(int argc, char *argv[], PARAM_INFO_MON *start)
{
	int ctr;
	int i;
	int time_spec;
	int found_data;
	int req_tm;
	int req_ts;
	char *p;
	char *tmp;
	char *o;
	PARAM_INFO_MON *info;			// iterator on this list
	OUT_INFO_MON *out_param;		// iterator on this list

	if ((out_param = malloc(sizeof(OUT_INFO_MON))) == NULL)
	{	fprintf(stderr, "no mem\n");
		return(NULL);
	}

#ifdef DEBUG
	fprintf(stderr, "parse_process: argc = %d\n", argc);
		for (i = 0; i < argc; i++)
  			fprintf(stderr, "argv[%d] = <%s>\n",i, argv[i]);
#endif

	for (i = 0; i < NM_MAX_FIELDS; i++)
		out_param->data[i] = NULL;

	strcpy(out_param->req_fs, " ");
#ifdef WIN32
	strcpy(out_param->req_ls, "\n\r");
#else
	strcpy(out_param->req_ls, "\n");
#endif
	time_spec = 0;
	out_param->req_n = 0;
	out_param->req_header = 1;

	time_spec = 0;
	req_tm = 200; 			/* default wait */
	req_ts = 0; 	
	
	ctr = 0; 
	i = 0;
	while (i < argc)
	{
		p = argv[i];
		if (*p == '-')
		{			// should be one of the short option
			if (strcmp(argv[i], "-fs") == 0)
			{		// field separator, next arg should be a string
				if (((i+1) < argc) && (strlen(argv[i+1]) < 10))
				{	i++;
					strcpy(out_param->req_fs, argv[i]);
				}
				else
				{	fprintf(stderr, "NEVER BE:-fs: Invalid field separator\n");
					return(NULL);
				}
			}
			else if (strcmp(argv[i], "-ls") == 0) 
			{		// line separator, next arg should be a string
				if (((i+1) < argc) && (strlen(argv[i+1]) < 10))
				{	i++;
					strcpy(out_param->req_ls, argv[i]);
				}
				else
				{	fprintf(stderr, "NEVER BE:-ls: Invalid line separator\n");
					return(NULL);
				}
			}
			else if (strcmp(argv[i], "-tm") == 0) 
			{		// time separator, next arg should be a millisecond (200-10000)
				if (time_spec)
				{	fprintf(stderr, "NEVER BE:-tm, -ts, are mutually exclusive");
					return(NULL);
				}

				if (((i+1) < argc) && (atoi(argv[i+1]) >= 200) && (atoi(argv[i+1]) <= 10000))
				{	i++;
					req_tm = atoi(argv[i]);
					time_spec = 1;
				}
				else
				{	fprintf(stderr, "NEVER BE:-tm: Invalid argument (200-10000 milliseconds)\n");
					return(NULL);
				}
			}
			else if (strcmp(argv[i], "-ts") == 0) 
			{		// time separator, next arg should be a seconds (1-3600)
				if (time_spec)
				{	fprintf(stderr, "NEVER BE:-tm, -ts, are mutually exclusive");
					return(NULL);
				}

				if (((i+1) < argc) && (atoi(argv[i+1]) >= 1) && (atoi(argv[i+1]) <= 3600))
				{	i++;
					req_ts = atoi(argv[i]);
					time_spec = 1;

					// delete default
					req_tm = 0;
				}
				else
				{	fprintf(stderr, "NEVER BE:-ts: Invalid argument (1-3600 seconds)\n");
					return(NULL);
				}
			}
			else if (strcmp(argv[i], "-n") == 0) 
			{		// nr of data pump, before stop collection,
					// next arg should be a int > 0
				if (((i+1) < argc) && (atoi(argv[i+1]) >= 1) && (atoi(argv[i+1]) <= 99999))
				{	i++;
					out_param->req_n = atoi(argv[i]);
				}
				else
				{	fprintf(stderr, "NEVER BE:-n: Invalid argument (1-99999 times)\n");
					return(NULL);
				}
			}
			else if (strcmp(argv[i], "-h") == 0) 
			{		// we do NOT want header (default with header)
				out_param->req_header = 0;
			}
			else if (strcmp(argv[i], "-S") == 0) 
			{		// we have used use specs from a file instead of standard info,
					// next arg is an existing readable filename
					// as already parsed correctly, we skip here.
				i++;
			}
			else if (strcmp(argv[i], "-I") == 0) 
			{		// we have used input from a file instead of command line
					// next arg is an existing readable filename
					// as already parsed correctly, we skip here.
				i++;
			}	
			else
			{	
				fprintf(stderr, "NEVER BE:Unrecognized option %s\n", argv[i]);
				return(NULL);
			}
		}
		else
		{			// should be required data
			tmp = strdup(argv[i]);
			p = strtok(tmp, ":\t\n");
			while (p != NULL)
			{
				// check if there is an output format specification
				o = p;
				while (*o)
				{
					o++;
					if (*o == '-')
					{	*o = '\0';
						o++;
						if (strlen(o) == 0)
						{
							// we have 'minus', so, probably forget format ...

							fprintf(stderr,
							 "NEVER BE:Required data <%s->, without format specifiers\n", p);
							return(NULL);
						}
					}
				}
			
							// we check for params
				found_data = 0;
				info = start;
				while (info->next != NULL)
				{	
					info = info->next;
					if (strcmp(p, info->name) == 0)
					{
						if (ctr > NM_MAX_FIELDS)
						{
							fprintf(stderr, "NEVER BE:too much data field ...");
							return(NULL);
						}
						out_param->data[ctr] = info;
						out_param->out_f[ctr] = *o;
						found_data = 1;
						ctr++;
						break;
					}
				}
				
				if (found_data == 0)
				{
					fprintf(stderr,
						"NEVER BE:Required data <%s>, not found in info-mon specifications\n", p);
					return(NULL);
				}
	

				p = strtok(NULL, ":\t\n");

			} // end while strtok

			// here, we have an array of pointers to required data
			// and an array of output specifiers, from 0 to [ctr-1]

		} // end else '-' (short options)
		
		i++;

	} // end while
	
	if (ctr == 0)
	{
		fprintf(stderr, "NEVER BE:no required data!\n");
		return(NULL);
	}
	
	// now, what netmon screen we need to retrieve ?
	// we need somewhat were checking is faster, as our goal is lower cicle time

	// we can have NM_MAX_SCREEN=254 screens, so we use an array, of unsigned char,
	// each char contain number of needed screen, 255 is list terminator
	
	for (i = 0; i <= NM_MAX_SCREEN; i++)
		out_param->req_screen[i] = 255;

	ctr = 0;
#ifdef DEBUG
fprintf(stderr, "Start Required data %s screen %d\n", out_param->data[ctr]->name, out_param->data[ctr]->s_nr);
#endif
	while (out_param->data[ctr] != NULL)
	{
#ifdef DEBUG
fprintf(stderr, "Required data %s screen %d\n", out_param->data[ctr]->name, out_param->data[ctr]->s_nr);
#endif
		// check if screen already in 
		found_data = 0;
		i = 0;
		while (out_param->req_screen[i] != 255)
		{
			if (out_param->req_screen[i] == out_param->data[ctr]->s_nr)
			{		// already load, skip
				found_data = 1;
				break;
			}
			i++;
		}

		if (found_data == 0)
		{	
#ifdef DEBUG
fprintf(stderr, "i = %d, out_param->req_screen[%d] = %d\n", i, i, out_param->data[ctr]->s_nr);
#endif
			out_param->req_screen[i] = out_param->data[ctr]->s_nr;
			i++;
		}

		ctr++;
	}

#ifdef DEBUG
i = 0;
while (out_param->req_screen[i] != 255)
{
	fprintf(stderr, "Required screen %d\n", out_param->req_screen[i]);
	i++;
}
#endif
	
	if (req_tm)
		out_param->req_wait = req_tm * 1000;
	else if (req_ts)
		out_param->req_wait = req_ts * 1000000;
	else
		out_param->req_wait = req_tm * 1000;
	

	return(out_param);
}

int nmd_output(OUT_INFO_MON *out_param)
{
	int d;
	int len;
	int ctr;
	int i;
	int nr_line;
	char *p;
	int nr_chr;
	int n;
	char Screen[NM_MAX_SCREEN_WIDTH];
	time_t sec;
	struct tm *date_time;

	if (out_param->req_header)
	{
// print phone help header
		d = 0;
		while (out_param->data[d] != NULL)
		{	len = out_param->data[d]->len;
			if (strlen(out_param->data[d]->name) > len)
				len = strlen(out_param->data[d]->name);
			if (strlen(out_param->data[d]->mname) > len)
				len = strlen(out_param->data[d]->mname);
			printf("%*.*s%s", len, len, out_param->data[d]->name, out_param->req_fs);
			d++;
		}
		printf("%s", out_param->req_ls);

// print netmon manual header
		d = 0;
		while (out_param->data[d] != NULL)
		{	len = out_param->data[d]->len;
			if (strlen(out_param->data[d]->name) > len)
				len = strlen(out_param->data[d]->name);
			if (strlen(out_param->data[d]->mname) > len)
				len = strlen(out_param->data[d]->mname);
		 	printf("%*.*s%s", len, len, out_param->data[d]->mname, out_param->req_fs);
			d++;
		}
		printf("%s", out_param->req_ls);
		printf("%s", out_param->req_ls);
	}

	ctr = 0;
	while (!bshutdown)
	{
// stop after n data punp

		ctr++;
		if ((out_param->req_n) && (ctr > out_param->req_n))
			break;

//		datapump: for each screen, for each required data, load data info->value

		i = 0;
		while(out_param->req_screen[i] != 255)
		{	
			if (out_param->req_screen[i] == 0)
			{	
				sec = time(0);
				date_time = localtime(&sec);
				sprintf(Screen, "%02d-%02d-%04d\n%02d:%02d:%02d\n",
					date_time->tm_mday,
					date_time->tm_mon+1,
					date_time->tm_year+1900,
					date_time->tm_hour,
					date_time->tm_min,
					date_time->tm_sec);
#ifdef DEBUG
				fprintf(stderr, "%02d-%02d-%04d\n%02d:%02d:%02d\n",
					date_time->tm_mday,
					date_time->tm_mon+1,
					date_time->tm_year+1900,
					date_time->tm_hour,
					date_time->tm_min,
					date_time->tm_sec);
#endif
			}
			else
			{
				GSM->NetMonitor(out_param->req_screen[i], Screen);
			}

// we have one screen of data, load those required

			d = 0;
			while (out_param->data[d] != NULL)
			{	
				if (out_param->data[d]->s_nr == out_param->req_screen[i])
				{
					p = Screen;
// goto req y		
					nr_line = out_param->data[d]->y;
					while ((*p) && (nr_line))
					{	
						if (*p == '\n')
						{	nr_line--;
						}
						p++;
					}
#ifdef DEBUG
fprintf(stderr, "\nthis line: %s\n\n", p);
#endif

// goto req x
					nr_chr = out_param->data[d]->x;
					while ((*p) && (nr_chr))
					{	p++;
						nr_chr--;
					}
// this the start of data
					n = 0;
					len = out_param->data[d]->len;
					while ((*p) && (len))
					{	out_param->data[d]->value[n] = *p;
						p++;
						n++;
						len--;
					}
// pad with space if shorter than presumed
					while ((*p) && (len))
					{	out_param->data[d]->value[n] = ' ';
						n++;
						len--;
					}

					out_param->data[d]->value[n] = '\0';
				}

				d++;	// next screen
			}

			i++; 		// next screen
		}

// print row of data
		d = 0;
		while (out_param->data[d] != NULL)
		{	
			len = out_param->data[d]->len;
			if (out_param->req_header)
			{	if (strlen(out_param->data[d]->name) > len)
					len = strlen(out_param->data[d]->name);
				if (strlen(out_param->data[d]->mname) > len)
					len = strlen(out_param->data[d]->mname);
			}
// FIXME check format
			printf("%*.*s%s", len, len, out_param->data[d]->value, out_param->req_fs);
			d++;
		}
		printf("%s", out_param->req_ls);

		usleep(out_param->req_wait);
		
	}

	printf("%s%s", out_param->req_ls, out_param->req_ls);

	return(0);
}

// main func
///////////////////////////////////////////////////////////////////////////////
int netmonitordata(int argc, char *argv[])
{
	char *loc_info = NULL;			// path to --netmonitordata directory
	char *f_name;				// absolute path of phone info-file
	int flag_phone_spec = 0;		// flags, set if used -S option

	char model[20] = "phonepar";		// TO DO: PHONE AUTODETECTION
//	char phver[20] = "";

	PARAM_INFO_MON *start;			// pointer to list of parsed phone params
	OUT_INFO_MON *out_param;		// pointer to struct of output data

	char *f_commands = NULL;		// file containings input line arguments
	FILE *commands;				// Handle for this file
	char buf[1024];				// buffer
	char *f_argv[NM_MAX_FIELDS+30];		// space for parameters and cmdline options
	int f_argc;
	char *p, *tmp;				// various counter, flags, tmp area ...
	int i;
	int line;
	

	fbusinit(NULL);

  	signal(SIGINT, interrupted);
	
// FIXME model, phone_version

	// before parsing phone-info-file, we check for user specified phone-info-file

	i = 0;
	while (i < argc)
	{
		if (strcmp(argv[i], "-S") == 0)
		{
			if ((i+1) < argc)
			{
				flag_phone_spec = 1;
				loc_info = strdup(argv[i+1]);
				break;
			}
			else
			{	fprintf(stderr, "option -S require an argument ...\n");
				return -1;
			}
		}

		i++;
	}
	
	if (loc_info != NULL)
	{	f_name = strdup(loc_info);
	}
	else				// we use standard file specification
	{
		if ((tmp = getenv("INFOMONPATH")) == NULL)
		{	loc_info = strdup(".");
		}
		else
		{	loc_info = strdup(tmp);
		}

		f_name = malloc(strlen(loc_info)+strlen(model)+10); 
		sprintf(f_name, "%s/%s", loc_info, model);
	}


#ifdef DEBUG
//	fprintf(stderr, "Loc_info <%s> model <%s> version <%s>\n", loc_info, model, phver);
	fprintf(stderr, "Info file: <%s>\n", f_name);
#endif
	if ((start = get_mon_param_info(f_name, NULL)) == NULL)
		return(-1);		

	// option -I give us the possibility of specify a filename,
	// containing a "sequence" of command line args.
	// if you specify this option, you can use (on command line) only -S option.
	// other options may be specified inside the input-file.
	// contents of this file as the same sintax as the command line,
	// except it must not contain "--netmonitordata" or "-I" or "-S" options 
        ///////////////////////////////////////////////////////////////////////

	i = 0;
	while (i < argc)
	{
		if (strcmp(argv[i], "-I") == 0)
		{
			if ((i+1) < argc)
			{
				if ((argc == 2) || ((argc == 4) && flag_phone_spec == 1))
				{	f_commands = strdup(argv[i+1]);
				}
				else
				{	fprintf(stderr, "option -I accept only additional -S option.\n");
					return -1;
				}

				break;
			}
			else
			{	fprintf(stderr, "option -I require an argument ...\n");
				return -1;
			}
		}

		i++;
	}
	
	// before all, we check all command line
	
	if (f_commands != NULL)
	{	
#ifdef DEBUG
		fprintf(stderr, "netmonitordata(check): commands from <%s>\n", f_commands);
#endif
		// for each line
		// do
		// 	read line, make array f_argv, and counter f_argc
		//	parse & check args, so errors are checked before real processing
		//	                       (-I option here is ignored)
		//			       (-S option can be here, and it is used)
		// done

		if ((commands = fopen(f_commands, "r")) == NULL)
		{	fprintf(stderr, "Can' t open file commands input :<%s>n", f_commands);
			return(-1);
		}
	
// FIXME line may be be splitted
		line = 0;
		while (fgets(buf, 1024, commands) != NULL)
		{	
			p = buf;
			line++;

			// Truncate from '#' at right of comments
			if ((tmp = strchr(p, '#')) != NULL)
				*tmp = '\0';

			// Strip leading, trailing whitespace
			while(isspace((int) *p))
		    		p++;

			while((strlen(p) > 0) && isspace((int) p[strlen(p) - 1]))
				p[strlen(p) - 1] = '\0';
	
			// Ignore blank lines
			if ((*p == '\n') || (*p == '\0'))
				continue;
			
#ifdef DEBUG
fprintf(stderr, "netmonitordata(check): row <%s>\n", p);
#endif
			// make args
			f_argc = 0;

			p = strtok(p, " \t");
			do
			{	f_argv[f_argc++] = strdup(p);
#ifdef DEBUG
fprintf(stderr, "netmonitordata(check): token <%s>\n", p);
#endif
				p = strtok(NULL, " \t");		// OK p,  (NULL)
			}
			while ((p != NULL) && (*p));
	
			// here we have f_argc, f_argv, this line is OK
		
			if (parse_check(f_argc, f_argv, start, f_commands, line) != 0)
			{	free_nm_info(start);
				return(-1);
			}
		}
	
		fclose(commands);
	}
	else	// as above, but we have only command line, argv, argc.
	{
		if (parse_check(argc, argv, start, NULL, 0) != 0)
		{	free_nm_info(start);
			return(-1);
		}
	}
	
	// here, all commands line are checked, and are correct

	if (f_commands != NULL)
	{	
#ifdef DEBUG
		fprintf(stderr, "netmonitordata(process): commands from <%s>\n", f_commands);
#endif

		// for each line
		// do
		// 	read line, make array f_argv, and counter f_argc
		//	parse_process argument,   (-I option here is ignored)
		//			          (-S option can be here, and it is used)
		//	(make header			// these are in nmd_output();
		//	 loop
		//		get net-mon-info 
		//		make output
		//	 done)
		// done

		if ((commands = fopen(f_commands, "r")) == NULL)
		{	fprintf(stderr, "Can' t open file commands input :<%s>n", f_commands);
			return(-1);
		}
	
// FIXME line may be be splitted
		while (fgets(buf, 1024, commands) != NULL)
		{	
			p = buf;

			// Truncate from '#' at right of comments
			if ((tmp = strchr(p, '#')) != NULL)
				*tmp = '\0';

			// Strip leading, trailing whitespace
			while(isspace((int) *p))
		    		p++;

			while((strlen(p) > 0) && isspace((int) p[strlen(p) - 1]))
				p[strlen(p) - 1] = '\0';
	
			// Ignore blank lines
			if ((*p == '\n') || (*p == '\0'))
				continue;
			
			// make args

			f_argc = 0;
			p = strtok(p, " \t");
			do
			{	f_argv[f_argc++] = strdup(p);
				p = strtok(NULL, " \t");		// OK p,  (NULL)
			}
			while ((p != NULL) && (*p));
	
			// here we have f_argc, f_argv, this line is OK
		
			if ((out_param = parse_process(f_argc, f_argv, start)) == NULL)
			{	free_nm_info(start);
				return(-1);	/* NEVER BE */
			}
		
			// here, command line has no error ...

			nmd_output(out_param);

			free(out_param);
		}
	
		fclose(commands);
	}
	else	// as above, but we have only command line, argv, argc.
	{
		if ((out_param = parse_process(argc, argv, start)) == NULL)
		{	free_nm_info(start);
			return(-1);	/* NEVER BE */
		}
		nmd_output(out_param);

		free(out_param);
	}
	GSM->Terminate();

	free(loc_info);
	free(f_name);

/* I analised this source and this should be done. But when compile with VC6 */
/* I have error. So, commented... MW */
#ifndef VC6
	free_nm_info(start);
#endif

	return(0);
}


// used by nm_collect()
///////////////////////////////////////////////////////////////////////////////
char *rowScreen(char *s)
{
	char *p;
	
	// make Screen in one row
	p = s;
	while(*p)
	{
	    if (*p == '\n')
			*p = ' ';
	    p++;
	}

	return(s);
}

void printtime(void)
{

  struct mydate {
    int Year;          /* The complete year specification - e.g. 1999. Y2K :-) */
    int Month;         /* January = 1 */
    int Day;
    int Hour;
    int Minute;
    int Second;
    int Timezone;      /* The difference between local time and GMT */
  } Date;

  struct tm *now;
  time_t nowh;
  nowh=time(NULL);
  now=localtime(&nowh);

  Date.Year = now->tm_year;
  Date.Month = now->tm_mon+1;
  Date.Day = now->tm_mday;
  Date.Hour = now->tm_hour;
  Date.Minute = now->tm_min;
  Date.Second = now->tm_sec;

  /* I have 100 (for 2000) Year now :-) */
  if (Date.Year>99 && Date.Year<1900) {
    Date.Year=Date.Year+1900;
  }

  printf("%d:%d:%d:%d:%d:%d      ",Date.Day,Date.Month,Date.Year,Date.Hour,Date.Minute,Date.Second);

}

// like netmonitor(), but print in one row, 1, 2 or 3 screen, every ~0.3 s
///////////////////////////////////////////////////////////////////////////////
int nm_collect(int argc, char *argv[])
{
	int mode[MAX_NM_COLLECT];
	char Screen[NM_MAX_SCREEN_WIDTH];
	int i;

	for (i=0;i<argc;i++) {
    	    argc > i ? (mode[i] = atoi(argv[i])): (mode[i] = 0);
	}

	for (i=0;i<argc;i++) {
	    if (mode[i]==0 && strcmp(argv[i],"-d")) {
		fprintf(stderr, "Wrong %i parameter (not number and not -d)\n",i);
		return(-1);
	    }
	}

	signal(SIGINT, interrupted);

	fbusinit(NULL);

	// end on CTRL-C
	while (!bshutdown) 
	{
	        for (i=0;i<argc;i++) {
		  if (!strcmp(argv[i],"-d")) {
		     printtime();
		     break;
		  }
		}
		
	        for (i=0;i<argc;i++) {
		  if (mode[i]!=0)
		  {
			GSM->NetMonitor(mode[i], Screen);
			printf("%s::", rowScreen(Screen));
		  }
		}
		
  		printf("\n\n");

		usleep(150000);
	}

	GSM->Terminate();

	return 0;
}

#ifdef DEBUG
int sniff(int argc, char *argv[])
{
  /* base model comes from gnokiirc */
  strcat(model,"sniff");
  
  if (argc>0) strcpy(Port,argv[0]);
    
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  /* Loop here indefinitely - allows you to see messages from GSM code in
     response to unknown messages etc. The loops ends after pressing the
     Ctrl+C. */
  while (!bshutdown) {
    sleep(1);
  }

  GSM->Terminate();    

  return 0;

}

int decodefile(int argc, char *argv[])
{
 FILE *infile;
 unsigned char in_buffer[255];
 int nr_read = 0;
 int i = 0;

  /* base model comes from gnokiirc */
  strcat(model,"decode");

 /* Initialise the GSM interface. */
 fbusinit(NULL);

 printf ("open InPutFile: %s\n", argv[0]);
 if ( (infile = fopen( argv[0], "rb")) == NULL ) {
   printf ("Failed to open InPutFile: %s\n", argv[0]);
   exit (1); }

 while ( (nr_read = fread(in_buffer, 1, 16, infile)) > 0 ) {
  for (i=0; i < nr_read; i++)
     Protocol->StateMachine(in_buffer[i]);
  }

   return 0;
}

#endif

int getringtone(int argc, char *argv[])
{
  GSM_BinRingtone ringtone;
  GSM_Error error;
  GSM_Ringtone SMringtone;

  ringtone.location=1;
  if (argc>1) ringtone.location=atoi(argv[1]);

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM_GetPhoneRingtone(&ringtone,&SMringtone);

  fprintf(stdout, _("Downloaded ringtone, location %i: "),ringtone.location);
  
  switch (error) {
    case GE_NONE:
      fprintf(stdout, _("get succeeded!\n"));
      /* In 33.. we have normal "Smart Messaging" format */
      if (GetModelFeature (FN_RINGTONES)==F_RING_SM) {
        fprintf(stdout, _("Name: %s (normal format)\n"),SMringtone.name);
        GSM_SaveRingtoneFileOnConsole(argv[0], &SMringtone);	
      } else {
        fprintf(stdout, _("Name: %s (binary format)\n"),ringtone.name);
        GSM_SaveBinRingtoneFile(argv[0], &ringtone);
      }
      GSM->Terminate();
      return 0;
      break;
    case GE_INVALIDRINGLOCATION:
      fprintf(stdout, _("invalid location %i!\n"),ringtone.location);
      break;
    case GE_UNKNOWNMODEL:
      fprintf(stdout, _("mygnokii doesn't know format for this model!\n"));
      break;
    case GE_NOTIMPLEMENTED:
      fprintf(stdout, _("not implemented for this model!\n"));
      break;
    default:
      fprintf(stdout, _("not supported by this model!\n"));
      break;
  }

  GSM->Terminate();
  
  return(-1);
}

int binringtoneconvert(int argc, char *argv[])
{
  GSM_BinRingtone ringtone;
  
  int i,j,z;
  bool isok;
  int deflen=7,m,w;
  bool pause;

  FILE *file;
  
//{ "c", "c#", "d", "d#", "e",  "f", "f#", "g", "g#", "a", "a#", "h" };
  int binary_notes[12] =
  { 0,    1,    2,   3,    4,   6,   7,    8,   9,    10,  11 ,   12};

  if (GSM_ReadBinRingtoneFile(argv[0],&ringtone)!=GE_NONE) {
    fprintf(stdout,_("Failed to read %s file!\n"),argv[0]);
    return -1;
  }

  file = fopen(argv[1], "wb");
      
  if (!file)
    return(GE_CANTOPENFILE);

  i=5;
  while (true) {
    if (ringtone.frame[i]==0) break;
    fprintf(file,_("%c"),ringtone.frame[i]);
    i++;
  }

  fprintf(file,_(":d=32,o=5,b=300:"));
    
  i=i+5;j=0;z=0;
  while (true) {
    if (ringtone.frame[i]!=j) {
      if (j!=0) {
        isok=false;
	if (j==64) {
	  j=255;//Pause
	  isok=true;
	}
	if (!isok && j>=114 && j<=125) {
	  j=14*3+binary_notes[j-114];
	  isok=true;
	}
	if (!isok && j>=126 && j<=137) {
	  j=14*1+binary_notes[j-126];
	  isok=true;
	}
	if (!isok && j>=138 && j<=149) {
	  j=14*2+binary_notes[j-138];
	  isok=true;
	}
	if (!isok && j>=150 && j<=161) {
	  j=14*4+binary_notes[j-150];
	  isok=true;
	}
	if (!isok) {
	  if (j!=10) { 
  	    fprintf(stdout,_("Unknown block in binary ringtone %i %i\n"),j,z);
	  }
	} else {
	  w=deflen;
	  for (m=0;m<6;m++) {
	    w=w*2;
	    if (w>z && (w/2)<=z) {
	      switch (m) {
	        case 1:fprintf(file,_("16"));break;
		case 2:fprintf(file,_("8"));break;
		case 3:fprintf(file,_("4"));break;
		case 4:fprintf(file,_("2"));break;
		case 5:fprintf(file,_("1"));break;
	      }
	      break;
	    }
	  }
	  if (z>w) fprintf(file,_("1"));
	  pause=false;
          switch (GSM_GetNote(j)) {
            case Note_C  :fprintf(file,_("c"));break;
            case Note_Cis:fprintf(file,_("c#"));break;
            case Note_D  :fprintf(file,_("d"));break;
            case Note_Dis:fprintf(file,_("d#"));break;
            case Note_E  :fprintf(file,_("e"));break;
            case Note_F  :fprintf(file,_("f"));break;
            case Note_Fis:fprintf(file,_("f#"));break;
            case Note_G  :fprintf(file,_("g"));break;
            case Note_Gis:fprintf(file,_("g#"));break;
            case Note_A  :fprintf(file,_("a"));break;
            case Note_Ais:fprintf(file,_("a#"));break;
            case Note_H  :fprintf(file,_("h"));break;
            default      :pause=true;fprintf(file,_("p"));break; //Pause ?
          }
	  w=deflen*1.5;
	  for (m=0;m<6;m++) {
	    w=w*2;
	    if (w>z && (w/2)<=z) {
	      fprintf(file,_("."));
	      break;
	    }
	  }
	  if ((j/14)!=1 && !pause) fprintf(file,_("%i"),j/14);
	  fprintf(file,_(","));
	}
      }
      j=ringtone.frame[i];
      z=ringtone.frame[i+1];
    } else {
      z=z+ringtone.frame[i+1];
    }
    i=i+2;
    if (i>=ringtone.length) break;
  }

  fclose(file);
  
  return 0;

}

int renamesmsc(int argc, char *argv[])
{
  GSM_MessageCenter MessageCenter;

  MessageCenter.No=atoi(argv[0]);

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  if (GSM->GetSMSCenter(&MessageCenter) == GE_NONE) {
    fprintf(stdout,_("SMSC number %i get OK\n"),MessageCenter.No);  
  } else {
    GSM->Terminate();
    fprintf(stdout,_("Error getting SMSC number %i\n"),MessageCenter.No);
    return 0;  
  }

  strncpy(MessageCenter.Name,argv[1],10);

  if (GSM->SetSMSCenter(&MessageCenter) == GE_NONE) {
    fprintf(stdout,_("SMSC number %i set OK (renamed to \"%s\")\n"),MessageCenter.No,MessageCenter.Name);
  } else {
    GSM->Terminate();
    fprintf(stdout,_("Error setting SMSC number %i\n"),MessageCenter.No);
    return 0;  
  }
  
  GSM->Terminate();

  return 0;
}

/*
 * Returns number of sostituited characters.
 * ... may be useful one day ??
 */
int semicolon_pipe_substitution( GSM_PhonebookEntry *pentry, unsigned int direction )
	/* direction = 0 : after reading phone memory (
	 *             1 : writing phone memory
	 */
{
	register int i;
	register int count=0;
	char charfrom, charto;

	charfrom= (direction==0) ? ';' : '|';
	charto  = (direction==0) ? '|' : ';';

	count+=str_substch(pentry->Name, charfrom, charto );
	count+=str_substch(pentry->Number, charfrom, charto );

	for( i = 0; i < pentry->SubEntriesCount; i++ )
	{
		if( pentry->SubEntries[i].EntryType != GSM_Date )
				count+=str_substch(pentry->SubEntries[i].data.Number ,charfrom,charto);
	}

	return( count );
}

int str_substch( char *str, const char toric, const char sost )
{
	unsigned int ct;
	int i_sost=0;
	 
		for( ct = 0; ct < strlen(str); ct++ )
			if( str[ct] == (unsigned char) toric )
			{ str[ct] = sost; i_sost++; }
		 
	return( i_sost );
}                                                                               

/* Allows to set simlock state.
   With older phone (older 51xx, 61xx) can open them, with older
   and newer should be able to close them */
/* DO NOT TRY, IF DON'T WANT, WHAT YOU DO !!!!!!!!!! */
int setsimlock()
{
  GSM_AllSimlocks siml;

  unsigned char closebuffer[20]=
                             { 0x00, 0x01, 0x82, 0x01,
			       0x00,                         /* which simlock */
			       0x00, 0x00,
			       0x00, 0x00, 0x00,             /* lock 1 info */
			       0x00, 0x00, 0x00, 0x00, 0x00, /* lock 4 info */
			       0x00, 0x00,                   /* lock 2 info */
			       0x00, 0x00,                   /* lock 3 info */
			       0x00};
  unsigned char openbuffer[10]= { 0x00, 0x01, 0x81, 0x01,
                                  0x00, /* lock number */
		      	          0x10, 0x10, 0x10, 0x10, 0x10 };
  unsigned char openbuffer0[10]= {0x00, 0x01, 0x02, 0x03, 0x1f, 0x11, 0x01, 0x01, 0x10, 0x00 };

  unsigned char info[120];
  int i,j;
  
  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);
  if (strstr(GSM_Info->FBUSModels, "3310") == NULL)
  {
    fprintf(stderr,("Not supported\n"));
    GSM->Terminate();
    return -1;
  }
  N6110_EnableExtendedCommands(0x02);
  /* Initial info */
  if (GSM->SimlockInfo(&siml)!=GE_NONE) {
    fprintf(stderr,_("Error getting simlock info\n"));GSM->Terminate();return -1;
  }
  /* Opening all locks (we must check, if we can open them) */
  NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer0);
  openbuffer[4]=1;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  openbuffer[4]=2;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  openbuffer[4]=4;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  openbuffer[4]=8;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  if (GSM->SimlockInfo(&siml)!=GE_NONE) {
    fprintf(stderr,_("Error getting simlock info\n"));GSM->Terminate();return -1;
  }
  for (i=0;i<4;i++) {
    if (siml.simlocks[i].enabled) {
      fprintf(stderr,_("Can not open simlock %i\n"),i+1);GSM->Terminate();return -1;
    }
  }
  /* Making frame for closing simlocks */
  strcpy(info,"00101");
  j=0;i=7;
  while (j!=strlen(info)) {
    if (j+2<=strlen(info)) {
      closebuffer[i]=((info[j] & 0x0f)<<4) | (info[j+1] & 0x0f);j=j+2;
    } else {
      closebuffer[i]=(info[j] & 0x0f) << 4;j++;
    }i++;
  }
  strcpy(info,"0000");j=0;i=15;
  while (j!=strlen(info)) {
    if (j+2<=strlen(info)) {
      closebuffer[i]=((info[j] & 0x0f)<<4) | (info[j+1] & 0x0f);j=j+2;
    } else {
      closebuffer[i]=(info[j] & 0x0f) << 4;j++;
    }i++;
  }
  strcpy(info,"0000");j=0;i=17;
  while (j!=strlen(info)) {
    if (j+2<=strlen(info)) {
      closebuffer[i]=((info[j] & 0x0f)<<4) | (info[j+1] & 0x0f);j=j+2;
    } else {
      closebuffer[i]=(info[j] & 0x0f) << 4;j++;
    }i++;
  }
  strcpy(info,"0000000001");j=0;i=9;
  while (j!=strlen(info)) {
    if (j+2<=strlen(info)) {
      if (j==0) {
        closebuffer[i]=closebuffer[i] | (info[j] & 0x0f);j++;
      } else {
        closebuffer[i]=((info[j] & 0x0f)<<4) | (info[j+1] & 0x0f);j=j+2;
      }
    } else {
      closebuffer[i]=(info[j] & 0x0f) << 4;j++;
    }i++;
  }  
  /* Closing simlock with given values */
  closebuffer[4]=1+2+4+8;
  NULL_SendMessageSequence(50, &CurrentMagicError, 20, 0x40,closebuffer);
  /* Opening all locks */
  NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer0);
  openbuffer[4]=1;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  openbuffer[4]=2;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  openbuffer[4]=4;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  openbuffer[4]=8;NULL_SendMessageSequence(50, &CurrentMagicError, 10, 0x40,openbuffer);
  GSM->Reset(0x03);
  GSM->Terminate();
  return 0;
}

int simlockinfo()
{
  GSM_AllSimlocks siml;
  char s[7];

  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);

  if (GSM->SimlockInfo(&siml)!=GE_NONE) fprintf(stderr,_("Error\n"));

  fprintf(stdout,_("MCC + MNC : %s      ("),siml.simlocks[0].data);
  if (siml.simlocks[0].enabled) fprintf(stdout,_("CLOSED"));
                           else fprintf(stdout,_("opened"));
  if (siml.simlocks[0].factory) fprintf(stdout,_(") (factory"));
                           else fprintf(stdout,_(")    (user"));
  fprintf(stdout,_(") (counter %i"),siml.simlocks[0].counter);

  s[0]=siml.simlocks[0].data[0];
  s[1]=siml.simlocks[0].data[1];
  s[2]=siml.simlocks[0].data[2];
  s[3]=' ';
  s[4]=siml.simlocks[0].data[3];
  s[5]=siml.simlocks[0].data[4];
  s[6]=0;

  if (strcmp(GSM_GetNetworkName(s),"unknown"))
    fprintf(stdout,_(") (network \"%s\""),GSM_GetNetworkName(s));

  fprintf(stdout,_(")\n"));

  fprintf(stdout,_("GID1      : %s       ("),siml.simlocks[1].data);
  if (siml.simlocks[1].enabled) fprintf(stdout,_("CLOSED"));
                           else fprintf(stdout,_("opened"));
  if (siml.simlocks[1].factory) fprintf(stdout,_(") (factory"));
                           else fprintf(stdout,_(")    (user"));
  fprintf(stdout,_(") (counter %i"),siml.simlocks[1].counter);
  fprintf(stdout,_(")\n"));

  fprintf(stdout,_("GID2      : %s       ("),siml.simlocks[2].data);
  if (siml.simlocks[2].enabled) fprintf(stdout,_("CLOSED"));
                           else fprintf(stdout,_("opened"));
  if (siml.simlocks[2].factory) fprintf(stdout,_(") (factory"));
                           else fprintf(stdout,_(")    (user"));
  fprintf(stdout,_(") (counter %i"),siml.simlocks[2].counter);
  fprintf(stdout,_(")\n"));

  fprintf(stdout,_("MSIN      : %s ("),siml.simlocks[3].data);
  if (siml.simlocks[3].enabled) fprintf(stdout,_("CLOSED"));
                           else fprintf(stdout,_("opened"));
  if (siml.simlocks[3].factory) fprintf(stdout,_(") (factory"));
                           else fprintf(stdout,_(")    (user"));
  fprintf(stdout,_(") (counter %i"),siml.simlocks[3].counter);
  fprintf(stdout,_(")\n"));

  GSM->Terminate();

  return 0;
}

/* Getting EEPROM from older phones */
/* Tested with N5110 5.07, 6150 5.22 */
int geteeprom()
{
  int i=0x40;

  unsigned char buffer[1000]={ 0x00, 0x01, 0xd4, 0x02, 0x00, 0xa0, 
                               0x00, 0x00, /* location Lo and Hi */
			       0x10 };     /* how many bytes */

  strcpy(Connection,"mbus");
  fprintf(stderr,_("Switching connection type to MBUS\n"));

  strcpy(model,"5110");  
  
  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);

  if (strstr(GSM_Info->FBUSModels, "3310") == NULL)
  {
    fprintf(stderr,("Not supported\n"));
    GSM->Terminate();
    return -1;
  }

  for (i=0;i<64;i++) {
    fprintf(stdout,_("%c"),0xff);
  }
  
  while (i<300000) {
    buffer[6] = i/256;
    buffer[7] = i%256;
    
    if ((i/256)!=((i-1)/256)) fprintf(stderr,_("."));
    
    if (NULL_SendMessageSequence(50, &CurrentMagicError, 9, 0x40,buffer)!=GE_NONE)	  break;

    i=i+0x10;
  }

  fprintf(stderr,_("\n"));
  
  GSM->Terminate();

  return 0;
}

int getsmsfolders()
{
  GSM_SMSFolders folders;
  GSM_Error error;

  int i;
    
  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);

  folders.number = 0;

  error=GSM->GetSMSFolders(&folders);
  
  GSM->Terminate();

  if (error!=GE_NONE && !folders.number) {
    fprintf(stdout,_("Error!\n"));
    return -1;
  }
  
  for (i=0;i<folders.number;i++) {
      fprintf(stdout,_("%i. %s\n"),i+1,folders.Folder[i].Name);
  }
  
  return 0;
}

int resetphonesettings()
{
  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);

  GSM->ResetPhoneSettings();

  GSM->Reset(0x03);  
  
  GSM->Terminate();

  return 0;
}

/* Checked on 3310 4.02 and doesn't work.
   Possible reasons: SMSC has problems (possible), bug in phone firmware
   (very possible) or here in code.
   I quess, that the second is the most possible - 3310 treat only 3 SMS
   as linked (the most often profile needs 4 - 2 and few bytes in 3'th
   for screen saver, few bytes for profile name and 1 or 2 sms for ringtone).
   When send separate ringtone (see --sendringtone with --profilestyle)
   and screen saver (--sendlogo screensaver...), it's received OK.
   It's for checking in higher firmware. */
int sendprofile(int argc, char *argv[])
{
  GSM_Ringtone ringtone;
  GSM_Bitmap bitmap;
  GSM_MultiSMSMessage MultiSMS;

  int current=0,i,j;

  u8 MessageBuffer[140*4];
  u16 MessageLength=0;
  
  char profilename[10+1];
  
  if (GSM_ReadRingtoneFileOnConsole(argv[2], &ringtone)!=GE_NONE) return(-1);

  ringtone.allnotesscale=false;
  for (i=0;i<argc;i++)
    if (!strcmp(argv[i],"--scale")) ringtone.allnotesscale=true;

  /* The fourth argument is the bitmap file. */
  if (GSM_ReadBitmapFileOnConsole(argv[3], &bitmap)!=GE_NONE) return -1;

  GSM_ResizeBitmap(&bitmap,GSM_PictureImage);

  strncpy(profilename,argv[1],10);

  MessageBuffer[MessageLength++]=0x30;               //SM version. Here 3.0

  MessageBuffer[MessageLength++]=SM30_PROFILENAME;   //ID for profile name
  MessageBuffer[MessageLength++]=0x00;               //length hi  
  MessageBuffer[MessageLength++]=strlen(profilename);//length lo
  
  EncodeUnicode (MessageBuffer+MessageLength,profilename ,strlen(profilename));
  MessageLength=MessageLength+2*strlen(profilename);

  MessageBuffer[MessageLength++]=SM30_RINGTONE; //ID for ringtone
  i=MessageLength;
  MessageBuffer[MessageLength++]=0x01;          //length hi
  MessageBuffer[MessageLength++]=0x00;          //length lo

  j=SM30_MAX_RINGTONE_FRAME_LENGTH;
  current=GSM_PackRingtone(&ringtone, MessageBuffer+MessageLength, &j);
  MessageLength=MessageLength+j;

  if (current!=ringtone.NrNotes) {
    if (current>FB61_MAX_RINGTONE_NOTES) {
      fprintf(stderr,_("Warning: due to phone limitation"));
    } else {
      fprintf(stderr,_("Warning: ringtone was too long to be saved into SMS,"));
    }
    fprintf(stderr, _(" only %i first notes were packed (%i cut)\n"),current,ringtone.NrNotes-current);
  }
    
  MessageBuffer[i]=(j)/256;
  MessageBuffer[i+1]=(j)%256;

  MessageBuffer[MessageLength++]=SM30_SCREENSAVER; //ID for OTA screen saver
  MessageBuffer[MessageLength++]=0x01;             //length hi
  MessageBuffer[MessageLength++]=0x00;             //length lo
  MessageBuffer[MessageLength++]=0x00;
  MessageBuffer[MessageLength++]=bitmap.width;
  MessageBuffer[MessageLength++]=bitmap.height;	  
  MessageBuffer[MessageLength++]=0x01;	  

  memcpy(MessageBuffer+MessageLength,bitmap.bitmap,bitmap.size);
  MessageLength=MessageLength+bitmap.size;

  GSM_MakeMultiPartSMS2(&MultiSMS,MessageBuffer,MessageLength, GSM_ProfileUDH, GSM_Coding_Default);

  optind = 4;

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  for (i=0;i<MultiSMS.number;i++)
    strcpy(MultiSMS.SMS[i].Destination,argv[0]);

  return GSM_SaveMultiPartSMSOnConsole(&MultiSMS, optind,argc,argv,false,false,false,false);
}

int showbitmap(int argc, char *argv[])
{
  GSM_Bitmap bitmap;

  if (GSM_ReadBitmapFileOnConsole(argv[0], &bitmap)!=GE_NONE) return(-1);

  GSM_PrintBitmap(&bitmap);

  return 0;
}

int getwapsettings(int argc, char *argv[])
{
  GSM_WAPSettings settings;
  GSM_Error error;

  settings.location=atoi(argv[0]);
  
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->GetWAPSettings(&settings);
  
  switch (error) {
    case GE_NONE:
      fprintf(stdout,_("%s."),argv[0]);
      if (!(strcmp(settings.title,""))) fprintf(stdout,_("Set %s\n"),argv[0]);
                                   else fprintf(stdout,_("%s\n"),settings.title);
      fprintf(stdout,_("Homepage: %s\n"),settings.homepage);
      if (settings.iscontinuous) fprintf(stdout,_("Connection type: continuous\n"));
                            else fprintf(stdout,_("Connection type: temporary\n"));
      if (settings.issecurity) fprintf(stdout,_("Connection security: on\n"));
                          else fprintf(stdout,_("Connection security: off\n"));
      switch (settings.bearer) {
        case WAPSETTINGS_BEARER_SMS:
          fprintf(stdout,_("Bearer: SMS\n"));
          fprintf(stdout,_("Server number: %s\n"),settings.server);
          fprintf(stdout,_("Service number: %s\n"),settings.service);
	  break;
        case WAPSETTINGS_BEARER_DATA:
          fprintf(stdout,_("Bearer: Data (CSD)\n"));
	  fprintf(stdout,_("Dial-up number: %s\n"),settings.dialup);
          fprintf(stdout,_("IP address: %s\n"),settings.ipaddress);
          if (settings.isnormalauthentication) fprintf(stdout,_("Authentication type: normal\n"));
                                          else fprintf(stdout,_("Authentication type: secure\n"));  
          if (settings.isISDNcall) fprintf(stdout,_("Data call type: ISDN\n"));
                              else fprintf(stdout,_("Data call type: analogue\n"));  
          if (settings.isspeed14400) fprintf(stdout,_("Data call speed: 14400\n"));
                                else fprintf(stdout,_("Data call speed: 9600\n"));  
          fprintf(stdout,_("User name: %s\n"),settings.user);
          fprintf(stdout,_("Password: %s\n"),settings.password);
	  break;
        case WAPSETTINGS_BEARER_USSD:
          fprintf(stdout,_("Bearer: USSD\n"));
	  fprintf(stdout,_("Service code: %s\n"),settings.code);
	  if (settings.isIP) fprintf(stdout,_("Address type: IP address\nIPaddress: %s\n"),settings.service);
	                else fprintf(stdout,_("Address type: service number\nService number: %s\n"),settings.service);
	  break;
      }
      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;

}

/* Not full done now */
int savewapsettings(int argc, char *argv[])
{
  GSM_WAPSettings settings;
  GSM_MultiSMSMessage MultiSMS;
  GSM_Error error;
  int w;

  settings.location=atoi(argv[0]);
  
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->GetWAPSettings(&settings);
//  strcpy(settings.homepage,"http://OtherSites/");
//  strcpy(settings.title,"Orange");

  error=GE_NONE;
  
  switch (error) {
    case GE_NONE:
      /* Put settings into SMS structure */
      GSM_SaveWAPSettingsToSMS(&MultiSMS,&settings);

      for (w=0;w<MultiSMS.number;w++)
        strcpy(MultiSMS.SMS[w].Destination,"WAPSettings");

      GSM_SaveMultiPartSMSOnConsole(&MultiSMS,1,argc,argv,false,false,false,false);
      return 0;

      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;
}

int sendwapsettings(int argc, char *argv[])
{
  GSM_WAPSettings settings;
  GSM_MultiSMSMessage MultiSMS;
  GSM_Error error;
  int w;

  settings.location=atoi(argv[0]);
  
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->GetWAPSettings(&settings);
//  strcpy(settings.homepage,"http://OtherSites/");
//  strcpy(settings.title,"Orange");

  error=GE_NONE;
  
  switch (error) {
    case GE_NONE:
      /* Put settings into SMS structure */
      GSM_SaveWAPSettingsToSMS(&MultiSMS,&settings);

      for (w=0;w<MultiSMS.number;w++)
        strcpy(MultiSMS.SMS[w].Destination,argv[1]);

      GSM_SendMultiPartSMSOnConsole(&MultiSMS, 2,argc,argv,false,false,false);

      return 0;

      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;
}

int getwapbookmark(int argc, char *argv[])
{
  GSM_WAPBookmark bookmark;
  GSM_Error error;

  bookmark.location=atoi(argv[0]);
  
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->GetWAPBookmark(&bookmark);
  
  switch (error) {
    case GE_NONE:
      if (bookmark.address[0]==0) {
        fprintf(stdout,_("Empty bookmark location\n"));
      } else {
        fprintf(stdout,_("Address: \"%s\"\n"),bookmark.address);
        if (bookmark.title[0]==0)
          fprintf(stdout,_("Title: \"%s\"\n"),bookmark.address);
        else
          fprintf(stdout,_("Title: \"%s\"\n"),bookmark.title);
      }
      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;

}

int setwapbookmark(int argc, char *argv[])
{
  GSM_WAPBookmark bookmark;
  GSM_Error error;

  if (argc == 3)	/* if location given, use it */
    bookmark.location=atoi(argv[2]);
  else			/* else use first empty location */
    bookmark.location=0xffff;

  strcpy(bookmark.title, argv[0]);
  strcpy(bookmark.address, argv[1]);

  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->SetWAPBookmark(&bookmark);
  
  switch (error) {
    case GE_NONE:
      fprintf(stdout,_("No errors\n"));
      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;

}

int savewapbookmark(int argc, char *argv[])
{
  GSM_WAPBookmark bookmark;
  GSM_MultiSMSMessage MultiSMS;
  GSM_Error error;
  int w;

  bookmark.location=atoi(argv[0]);
  
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->GetWAPBookmark(&bookmark);

//  strcpy(bookmark.title,"tytulagagagagagagagagagagagagagagagagagagagagagend");
//  strcpy(bookmark.address,"http://jajajajajajajajajajajajagagagagagagagagagagagagagagagagagagagagagpapapapapapapapapapapapapapapapapapapapapapapapapapapapapapapapap¥papapapapapapapapagagagagagagagagagagagagagagagagagagagagagagagagagadadadadadadadajdjdjdjdjdjdjdjdjdjdjdjdjdjdjdjdstp");  
  error=GE_NONE;

  switch (error) {
    case GE_NONE:

      if (bookmark.address[0]==0) {
        fprintf(stdout,_("Empty bookmark location\n"));
        GSM->Terminate();   
        return 1;
      }

      /* Put bookmark into SMS structure */
      GSM_SaveWAPBookmarkToSMS(&MultiSMS,&bookmark);

      for (w=0;w<MultiSMS.number;w++)
        strcpy(MultiSMS.SMS[w].Destination,"WAPBookmark");

      GSM_SaveMultiPartSMSOnConsole(&MultiSMS, 1,argc,argv,false,false,false,false);
      return 0;

      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;

}

int sendwapbookmark(int argc, char *argv[])
{
  GSM_WAPBookmark bookmark;
  GSM_MultiSMSMessage MultiSMS;
  GSM_Error error;
  int w;

  bookmark.location=atoi(argv[0]);
  
  /* Initialise the GSM interface. */
  fbusinit(NULL);

  error=GSM->GetWAPBookmark(&bookmark);
  
  switch (error) {
    case GE_NONE:

      if (bookmark.address[0]==0) {
        fprintf(stdout,_("Empty bookmark location\n"));
        GSM->Terminate();   
        return 1;
      }

      /* Put bookmark into SMS structure */
      GSM_SaveWAPBookmarkToSMS(&MultiSMS,&bookmark);

      for (w=0;w<MultiSMS.number;w++)
        strcpy(MultiSMS.SMS[w].Destination,argv[1]);

      GSM_SendMultiPartSMSOnConsole(&MultiSMS, 2,argc,argv,false,false,false);
      return 0;

      break;
    default:
      fprintf(stderr,_("%s\n"),print_error(error));
  }

  GSM->Terminate();    

  return 0;

}

int savecalendarnote(int argc, char *argv[])
{
  GSM_MultiSMSMessage MultiSMS;
  int w;
  GSM_CalendarNote CalendarNote;
  int number;

  number=atoi(argv[1]);
  
  if (number<1) {
    fprintf(stdout, _("Number of calendar note must be 1 or higher\n"));
    return -1;
  }
  
  switch ( GSM_ReadVCalendarFile(argv[0], &CalendarNote, &number) ) {
    case GE_NONE:
      break;
    case GE_CANTOPENFILE:
      fprintf(stdout, _("Failed to open vCalendar file \"%s\"\n"),argv[0]);
      return -1;
    case GE_TOOSHORT:
      fprintf(stdout, _("Number of given calendar note is too high (max=%i)\n"),number);
      return(-1);
    default:
      fprintf(stdout, _("Failed to parse vCalendar file \"%s\"\n"),argv[0]);
      return -1;    
  }

  fbusinit(NULL);

  /* Put note into SMS structure */
  GSM_SaveCalendarNoteToSMS(&MultiSMS,&CalendarNote);

  for (w=0;w<MultiSMS.number;w++)
    strcpy(MultiSMS.SMS[w].Destination,"Calendar");

  GSM_SaveMultiPartSMSOnConsole(&MultiSMS, 2,argc,argv,false,false,false,false);

  return 0;
}

int sendcalendarnote(int argc, char *argv[])
{
  GSM_MultiSMSMessage MultiSMS;
  int w;
  GSM_CalendarNote CalendarNote;
  int number;

  number=atoi(argv[2]);
  
  if (number<1) {
    fprintf(stdout, _("Number of calendar note must be 1 or higher\n"));
    return -1;
  }
  
  switch ( GSM_ReadVCalendarFile(argv[1], &CalendarNote, &number) ) {
    case GE_NONE:
      break;
    case GE_CANTOPENFILE:
      fprintf(stdout, _("Failed to open vCalendar file \"%s\"\n"),argv[1]);
      return -1;
    case GE_TOOSHORT:
      fprintf(stdout, _("Number of given calendar note is too high (max=%i)\n"),number);
      return(-1);
    default:
      fprintf(stdout, _("Failed to parse vCalendar file \"%s\"\n"),argv[1]);
      return -1;    
  }

  fbusinit(NULL);

  /* Put note into SMS structure */
  GSM_SaveCalendarNoteToSMS(&MultiSMS,&CalendarNote);

  for (w=0;w<MultiSMS.number;w++)
    strcpy(MultiSMS.SMS[w].Destination,argv[0]);

  GSM_SendMultiPartSMSOnConsole(&MultiSMS, 3,argc,argv,false,false,false);

  return 0;
}

/* Example function for continuous receiving and transmitting SMS */

/* When phone receives SMS, it's read by mygnokii, ID number is created
   and SMS is saved to file. After it it's deleted
   Checking many errors is also done
   When testing for received SMSes has been done, a directory is
   searched for files each containing a SMS to be sent.
   Each file is deleted after being sent.

   gnokiirc parameters:

   [receivesms]
   path=<received SMSes>
   outboxpath=<optional directory for SMSes to be sent>
   logpath=<directory for logfile: gnokiiSMS.log>
   delay=<no of seconds between polling the phone and the SMS dir. Default: 1>

   All paths must be terminated by '/'

   Call structure:
   receivesms()
	sendsmsfromfile()
		sendonesms()
			sendsms()
 */
int receivesms(int argc, char *argv[])
{
  char Dir[500], logDir[500];

  GSM_SMSStatus SMSStatus = {0, 0};
  char memory_type_string[20];
  GSM_SMSFolders folders;
  GSM_Error error;
  GSM_SMSMessage SMS;
  int read, location, number;
  unsigned char name[50],filename[400], logname[400], outbox[400];
  char nowdate[12]="", nowtime[12]="";
  FILE *file;
  FILE *logfile;
  struct CFG_Header *cfg_info;
  char * buf;
  int polldelay = 1;

  /* We do not want to monitor serial line forever - press Ctrl+C to stop the
     monitoring mode. */

  signal(SIGINT, interrupted);

  fprintf (stderr, _("Entering monitor mode (press CTRL+C to break)...\n"));

  cfg_info=CFG_FindGnokiirc();

  strcpy(Dir,"");
  if (cfg_info!=NULL) {	
    if (CFG_Get(cfg_info, "receivesms", "path")) {
      strcpy(Dir,CFG_Get(cfg_info, "receivesms", "path"));
    }
  }
  strcpy(outbox,"");
  if (cfg_info!=NULL) {
    if (CFG_Get(cfg_info, "receivesms", "outboxpath")) {
      strcpy(outbox,CFG_Get(cfg_info, "receivesms", "outboxpath"));
    }
  }
  strcpy(logDir,"");
  if (cfg_info!=NULL) {
    if (CFG_Get(cfg_info, "receivesms", "logpath")) {
      strcpy(logDir,CFG_Get(cfg_info, "receivesms", "logpath"));
    }
  }
  if (cfg_info!=NULL) {
    if (CFG_Get(cfg_info, "receivesms", "delay")) {
      polldelay = atoi(CFG_Get(cfg_info, "receivesms", "delay"));
    }
  }
  strcpy(logname,logDir);
  strcat(logname,"gnokiiSMS.log");

  fprintf(stderr,_("Path for incoming sms files is \"%s\"\n"),Dir);
  fprintf(stderr,_("Path for outgoing sms files is \"%s\"\n"),outbox);
  fprintf(stderr,_("Logpath for sms files is \"%s\"\n"),logDir);
  fprintf(stderr,_("Polling phone each %d seconds\n"),polldelay);

  fprintf (stderr, _("Initialising GSM interface..."));

  /* Initialise the code for the GSM interface. */     
  fbusinit(NULL);

  fprintf (stderr, _("done\n"));

  sleep(1);

  /* Loop here indefinitely - allows you to see messages from GSM code in
     response to unknown messages etc. The loops ends after pressing the
     Ctrl+C. */
  while (!bshutdown) {
    error = GSM->GetSMSStatus(&SMSStatus);
    if (error != GE_NONE) error = GSM->GetSMSStatus(&SMSStatus); /*  fixes GE_TIMEOUT in 5110 */

    if (error == GE_NONE) {
      if (SMSStatus.Number!=0) {

        GetMachineDateTime(nowdate, nowtime );
        logfile = fopen(logname, "a");
        if (logfile) {
          fprintf(logfile, _("%s %s SMS Messages: UnRead %d, Number %d\n"),
             nowdate, nowtime, SMSStatus.UnRead, SMSStatus.Number);
          fclose(logfile);
        }

        read=0;
        location=1;

        while (!bshutdown) {

          SMS.Location=location;
          if ((error = GSM->GetSMSMessage(&SMS))==GE_NONE) {
            if (SMS.folder==0 || SMS.folder==0x08) { //GST_7110_INBOX

              GetMachineDateTime(nowdate, nowtime );
              logfile = fopen(logname, "a");
              if (logfile) {
                fprintf(logfile,_("%s %s SMS on location %i\n"),
                  nowdate,nowtime,SMS.MessageNumber);
                fclose(logfile);
              }

              number=             16*(SMS.MessageText[2] >> 4)+      (SMS.MessageText[2] & 0x0f);
              number=number+  256*16*(SMS.MessageText[1] >> 4)+  256*(SMS.MessageText[1] & 0x0f);
              number=number+65536*16*(SMS.MessageText[0] >> 4)+65536*(SMS.MessageText[0] & 0x0f);

              sprintf( name, "%07i_%02d%02d%02d_%02d%02d%02d.sms", number,
                 SMS.Time.Year, SMS.Time.Month, SMS.Time.Day,
                 SMS.Time.Hour, SMS.Time.Minute, SMS.Time.Second);

              strcpy(filename,Dir);
              strcat(filename,name);

              logfile = fopen(logname, "a");
              if (logfile) {
                fprintf(logfile,_("%s %s Name is \"%s\"\n"),nowdate,nowtime,filename);
                fclose(logfile);
              }

              file = fopen(filename, "rb");
              if (!file) {
                file = fopen(filename, "wb");
                if (!file) {
                  GetMachineDateTime(nowdate, nowtime );
                  fprintf(stderr,_("%s %s ERROR: opening file \"%s\" failed !\n"),nowdate,nowtime,filename);
                  logfile = fopen(logname, "a");
                  if (logfile) {
                    fprintf(logfile,_("%s %s ERROR: opening file \"%s\" failed !\n"),nowdate,nowtime,filename);
                    fclose(logfile);
                  }
                } else {
                  GetMemoryTypeString(memory_type_string, &SMS.MemoryType);
                  GSM->GetSMSFolders(&folders);

                  buf = formatsms(error, memory_type_string, folders, SMS, "\0");
                  if (buf != NULL) {
                     fprintf(file, _("%s"), buf);
                     free(buf);
                  }
                  fclose(file);
                }
              } else {
                fclose(file);
  
                GetMachineDateTime(nowdate, nowtime );
                fprintf(stderr,_("%s %s ERROR: file \"%s\" already exist !\n"),nowdate,nowtime,filename);
                logfile = fopen(logname, "a");
                if (logfile) { 
                  fprintf(logfile,_("%s %s ERROR: file \"%s\" already exist !\n"),nowdate,nowtime,filename);
                  fclose(logfile);
                }
              }
  
              SMS.Location=SMS.MessageNumber;
              if (GSM->DeleteSMSMessage(&SMS)!=GE_NONE) {
                GetMachineDateTime(nowdate, nowtime );
                fprintf(stderr,_("%s %s ERROR: Deleting SMS location %i failed !\n"),nowdate,nowtime,SMS.MessageNumber);
                logfile = fopen(logname, "a");
                if (logfile) {
                  fprintf(logfile,_("%s %s ERROR: Deleting SMS location %i failed !\n"),nowdate,nowtime,SMS.MessageNumber);
                  fclose(logfile);
                }
              }
            }
            read++;
          }
          location++;
          if (read==SMSStatus.Number) break;
        }
      }

      sendsmsfromfile(outbox, logname);

    } else {
      GetMachineDateTime(nowdate, nowtime );
      fprintf(stderr,_("%s %s ERROR: Can't get SMS status (%d) !\n"),nowdate,nowtime,error);
      logfile = fopen(logname, "a");
      if (logfile) {
        fprintf(logfile,_("%s %s ERROR: Can't get SMS status (%d) !\n"),nowdate,nowtime,error);
        fclose(logfile);
      }
       sleep(15);
        /* Initialise the code for the GSM interface. */
       fbusinit(NULL);
    }

    sleep(polldelay);
  }

  fprintf (stderr, _("Leaving monitor mode...\n"));

  GSM->Terminate();
  
  return 0;
}

int divert(int argc, char *argv[])
{
	GSM_CallDivert cd;
	GSM_Error error;

	memset(&cd, 0, sizeof(GSM_CallDivert));

	     if (!strcmp("register", argv[0])){cd.Operation = GSM_CDV_Register ;} 
        else if (!strcmp("enable"  , argv[0])){cd.Operation = GSM_CDV_Enable   ;} 
        else if (!strcmp("disable" , argv[0])){cd.Operation = GSM_CDV_Disable  ;}
        else if (!strcmp("erasure" , argv[0])){cd.Operation = GSM_CDV_Erasure  ;}
	else if (!strcmp("query"   , argv[0])){cd.Operation = GSM_CDV_Query    ;}
        else {
		usage();
		exit(-1);
	}

	     if (!strcmp("all"       , argv[1])) {cd.DType = GSM_CDV_AllTypes  ;}
	else if (!strcmp("busy"      , argv[1])) {cd.DType = GSM_CDV_Busy      ;}
	else if (!strcmp("noans"     , argv[1])) {cd.DType = GSM_CDV_NoAnswer  ;}
	else if (!strcmp("outofreach", argv[1])) {cd.DType = GSM_CDV_OutOfReach;}
	else {
		usage();
		exit(-1);
	}

	     if (!strcmp("all"  , argv[2])) {cd.CType = GSM_CDV_AllCalls  ;}
	else if (!strcmp("voice", argv[2])) {cd.CType = GSM_CDV_VoiceCalls;}
	else if (!strcmp("fax"  , argv[2])) {cd.CType = GSM_CDV_FaxCalls  ;}
	else if (!strcmp("data" , argv[2])) {cd.CType = GSM_CDV_DataCalls ;}
	else {
		usage();
		exit(-1);
	}

	if (argc>3) strcpy(cd.Number, argv[3]);

	if (argc>4) cd.Timeout = atoi(argv[4]);

	/* Initialise the code for the GSM interface. */     
	fbusinit(NULL);

	error=GSM->CallDivert(&cd);

	if (error == GE_NONE) {
          switch (cd.Operation)
          {
            case GSM_CDV_Query:
  	      fprintf(stdout, _("Divert type: "));
	      switch (cd.DType) {
	        case GSM_CDV_Busy      :fprintf(stdout, _("when busy"));break;
 	        case GSM_CDV_NoAnswer  :fprintf(stdout, _("when not answered"));break;
      	        case GSM_CDV_OutOfReach:fprintf(stdout, _("when phone off or no coverage"));break;
                case GSM_CDV_AllTypes  :fprintf(stdout, _("all call diverts"));break; //?
                default:                fprintf(stdout, _("unknown %i"),cd.DType);break;
              }

              fprintf(stdout, _("\nCalls type : "));
              switch (cd.CType) {
                case GSM_CDV_VoiceCalls: fprintf(stdout, _("voice"));break;
                case GSM_CDV_FaxCalls  : fprintf(stdout, _("fax"));break;
                case GSM_CDV_DataCalls : fprintf(stdout, _("data"));break;
                case GSM_CDV_AllCalls  : fprintf(stdout, _("voice, fax & data"));break;
                default:   fprintf(stdout, _("unknown %i"),cd.CType);break;
              }
              fprintf(stdout, _("\n"));

              if (cd.Enabled) {
                fprintf(stdout, _("Status     : active\n"));
                fprintf(stdout, _("Timeout    : %i seconds\n"),cd.Timeout);
                fprintf(stdout, _("Number     : %s\n"),cd.Number);
              } else {
                fprintf(stdout, _("Status     : deactivated\n"));
              }

              break;
            default:
              fprintf(stdout,_("Divert done\n"));
          }
        } else 
          fprintf(stderr,_("%s\n"),print_error(error));

	GSM->Terminate();

	return 0;
}

int savephonebookentry(int argc, char *argv[])
{
  GSM_MultiSMSMessage MultiSMS;
  GSM_PhonebookEntry entry;
  GSM_Error error;
  int w;

  fbusinit(NULL);

  /* Handle command line args that set type, start and end locations. */
  if (!GetMemoryTypeID(argv[0], &entry.MemoryType))
  {
    fprintf(stderr, _("Unknown memory type %s!\n"), argv[0]);
    return (-1);
  }

  entry.Location=atoi(argv[1]);

  fbusinit(NULL);

  error=GSM->GetMemoryLocation(&entry);

  switch (error) {
    case GE_NONE:

      /* Put entry into SMS structure */
      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT) {
        GSM_SavePhonebookEntryToSMS(&MultiSMS,&entry,21);
      } else {
        GSM_SavePhonebookEntryToSMS(&MultiSMS,&entry,10);
      }

      for (w=0;w<MultiSMS.number;w++)
        strcpy(MultiSMS.SMS[w].Destination,"Phonebook");

      GSM_SaveMultiPartSMSOnConsole(&MultiSMS, 2,argc,argv,false,false,false,false);
      break;

    default:
      fprintf(stdout,_("Error\n"));break;
  }

  return 0;
}

int sendphonebookentry(int argc, char *argv[])
{
  GSM_MultiSMSMessage MultiSMS;
  GSM_PhonebookEntry entry;
  GSM_Error error;
  int w;

  fbusinit(NULL);

  /* Handle command line args that set type, start and end locations. */
  if (!GetMemoryTypeID(argv[1], &entry.MemoryType))
  {
    fprintf(stderr, _("Unknown memory type %s!\n"), argv[1]);
    return (-1);
  }

  entry.Location=atoi(argv[2]);

  fbusinit(NULL);

  error=GSM->GetMemoryLocation(&entry);

  switch (error) {
    case GE_NONE:

      /* Put entry into SMS structure */
      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT) {
        GSM_SavePhonebookEntryToSMS(&MultiSMS,&entry,21);
      } else {
        GSM_SavePhonebookEntryToSMS(&MultiSMS,&entry,10);
      }

      for (w=0;w<MultiSMS.number;w++)
        strcpy(MultiSMS.SMS[w].Destination,argv[0]);

      GSM_SendMultiPartSMSOnConsole(&MultiSMS, 3,argc,argv,false,false,false);

      break;

    default:
      fprintf(stdout,_("Error\n"));break;
  }

  return 0;
}

#ifdef DEBUG
int getbinringfromfile(int argc, char *argv[])
{
  FILE *file;
  int i,j;
  bool found;
  GSM_BinRingtone binring,ringtone;

  fprintf(stdout,_("Offset %ld\n"),atol(argv[2]));

  file = fopen(argv[0], "rb");

  if (!file) return(GE_CANTOPENFILE);

  fseek(file,atol(argv[2]),0);
  
  ringtone.length=fread(ringtone.frame, 1, 900, file);

  fclose(file);

  i=0;found=false;
  while (true) {
    if (ringtone.frame[i  ]==0x00 && ringtone.frame[i+1]==0x02 && 
        ringtone.frame[i+2]==0xFC && ringtone.frame[i+3]==0x09) {
      found=true;
      break;
    }
    i++;
    if (i==ringtone.length-3) break;
  }

  if (!found) {
    fprintf(stdout,_("Start not found\n"));
    return (-1);
  }

  j=0;found=false;
  while (true) {
    if (ringtone.frame[j]==0x07 && ringtone.frame[j+1]==0x0B) {
      found=true;
      break;
    }
    j++;
    if (j==ringtone.length-2) break;
  }

  if (!found) {
    fprintf(stdout,_("End not found\n"));
    return (-1);
  }

  binring.length=0;
  binring.frame[binring.length++]=0x00;
  binring.frame[binring.length++]=0x00;
  binring.frame[binring.length++]=0x0C;
  binring.frame[binring.length++]=0x01;
  binring.frame[binring.length++]=0x2C;
  
  memcpy(binring.frame+binring.length,argv[1],strlen(argv[1]));
  binring.length=binring.length+strlen(argv[1]);

  binring.frame[binring.length++]=0x00;

  memcpy(binring.frame+binring.length,ringtone.frame+i,j-i+2);
  binring.length=binring.length+j-i+2;

  GSM_SaveBinRingtoneFile(argv[3], &binring); 

  return GE_NONE;
}
#endif

/*
 - split - divide a string into fields, like awk split()
 = int split(char *string, char *fields[], int nfields, char *sep);
 */
int                             /* number of fields, including overflow */
split(string, fields, nfields, sep)
char *string;
char *fields[];                 /* list is not NULL-terminated */
int nfields;                    /* number of entries available in fields[] */
char *sep;                      /* "" white, "c" single char, "ab" [ab]+ */
{
        register char *p = string;
        register char c;                        /* latest character */
        register char sepc = sep[0];
        register char sepc2;
        register int fn;
        register char **fp = fields;
        register char *sepp;
        register int trimtrail;

        /* white space */
        if (sepc == '\0') {
                while ((c = *p++) == ' ' || c == '\t')
                        continue;
                p--;
                trimtrail = 1;
                sep = " \t";    /* note, code below knows this is 2 long */
                sepc = ' ';
        } else
                trimtrail = 0;
        sepc2 = sep[1];         /* now we can safely pick this up */

        /* catch empties */
        if (*p == '\0')
                return(0);

        /* single separator */
        if (sepc2 == '\0') {
                fn = nfields;
                for (;;) {
                        *fp++ = p;
                        fn--;
                        if (fn == 0)
                                break;
                        while ((c = *p++) != sepc)
                                if (c == '\0')
                                        return(nfields - fn);
                        *(p-1) = '\0';
                }
                /* we have overflowed the fields vector -- just count them */
                fn = nfields;
                for (;;) {
                        while ((c = *p++) != sepc)
                                if (c == '\0')
                                        return(fn);
                        fn++;
                }
                /* not reached */
        }

        /* two separators */
        if (sep[2] == '\0') {
                fn = nfields;
                for (;;) {
                        *fp++ = p;
                        fn--;
                        while ((c = *p++) != sepc && c != sepc2)
                                if (c == '\0') {
                                        if (trimtrail && **(fp-1) == '\0')
                                                fn++;
                                        return(nfields - fn);
                                }
                        if (fn == 0)
                                break;
                        *(p-1) = '\0';
                        while ((c = *p++) == sepc || c == sepc2)
                                continue;
                        p--;
                }
                /* we have overflowed the fields vector -- just count them */
                fn = nfields;
                while (c != '\0') {
                        while ((c = *p++) == sepc || c == sepc2)
                                continue;
                        p--;
                        fn++;
                        while ((c = *p++) != '\0' && c != sepc && c != sepc2)
                                continue;
                }
                /* might have to trim trailing white space */
                if (trimtrail) {
                        p--;
                        while ((c = *--p) == sepc || c == sepc2)
                                continue;
                        p++;
                        if (*p != '\0') {
                                if (fn == nfields+1)
                                        *p = '\0';
                                fn--;
                        }
                }
                return(fn);
        }

        /* n separators */
        fn = 0;
        for (;;) {
                if (fn < nfields)
                        *fp++ = p;
                fn++;
                for (;;) {
                        c = *p++;
                        if (c == '\0')
                                return(fn);
                        sepp = sep;
                        while ((sepc = *sepp++) != '\0' && sepc != c)
                                continue;
                        if (sepc != '\0')       /* it was a separator */
                                break;
                }
                if (fn < nfields)
                        *(p-1) = '\0';
                for (;;) {
                        c = *p++;
                        sepp = sep;
                        while ((sepc = *sepp++) != '\0' && sepc != c)
                                continue;
                        if (sepc == '\0')       /* it wasn't a separator */
                                break;
                }
                p--;
        }

        /* not reached */
}

/*
 Basically implementing gnokii --sendsms as an internal call by
 duplicating code from main()
*/

GSM_Error sendonesms(int argc, char *argv[]){
  int c, i, rc = -1;
  int nargc = argc - 2;
  char **nargv;

  static struct option long_options[] =
  {
    { "sendsms",            required_argument, NULL, OPT_SENDSMS          },// Send SMS message mode
    { 0, 0, 0, 0},
  };

  struct gnokii_arg_len gals[] =
  {
    { OPT_SENDSMS,           1,12, 0 },
    { 0, 0, 0, 0},
  };

  opterr = 1; optarg = NULL; optind = 1;

  /* Handle command line arguments. */

  c = getopt_long(argc, argv, "", long_options, NULL);
  /*printf("go1:%d/%d %s,%s,%s,%s,%s\n", c,argc, argv[0],argv[1],argv[2],argv[3], argv[4]);*/

  if (c == -1) {
    fprintf(stderr, _("No argument given\n"));
    return(-1);
  }

  if((nargv = calloc(sizeof(char *), argc)) != NULL) {

    for(i = 2; i < argc; i++)
      nargv[i-2] = argv[i];

    if(checkargs(c, gals, nargc)) {
      fprintf(stderr, _("Wrong number of arguments: %d/%d/%d:%s,%s,%s\n"), c, nargc, optopt,nargv[0],nargv[1],nargv[2]);
      free(nargv);
      return(-2);
    }

    switch(c) {
    case OPT_SENDSMS:               rc = sendsms(nargc, nargv);     break;
    default:         fprintf(stderr, _("Unknown option: %d\n"), c); break;
    }

    free(nargv);
    return(rc);
  }

  fprintf(stderr, _("Memory allocation error\n"));
  return(-3);
}

/*
  Scans a directory for files each containing a SMS to be sent.
  Each file:
   line 1: --sendsms <parameters as for --sendsms >
   line 2-n: text to be sent

  Called from receivesms.
*/

GSM_Error sendsmsfromfile(char * outbox, char * logname) {
  char string[512], filename[256];
  char *argv[22+3];                 /* list is not NULL-terminated */
  int argc = 0;
  int nfields = 22;               /* number of entries available in fields[] */
  char message_buffer[GSM_MAX_CONCATENATED_SMS_LENGTH];
  int input_len, chars_read, rc=0, l;
  char nowdate[12]="", nowtime[12]="";
  FILE *fd, *logfile;
  struct dirent **namelist;
  int n;
  GSM_Error error = GE_NONE;

  if (*outbox == '\0') return(0);

#ifndef WIN32
  argv[0] = "gnokii";
  /* get all files in mailbox */

  n = scandir(outbox, &namelist, 0, alphasort);
  if (n < 0)
	perror("scandir");
  else {
  /* for each file in outbox do */
	while(n--) {
		if (*(namelist[n]->d_name) == '.') continue;
		rc = 1;
	/* read first line of file = sendsms parameters into string */
		strcpy(filename, outbox); strcat(filename,namelist[n]->d_name);
		if ((fd = fopen(filename, "r+")) != NULL) {
		fgets(string, 256, fd);
		l = strlen(string);
		if ((l > 0) && (string[l-1] == '\n')) string[l-1] = '\0';
		argc = split(string, &argv[1], nfields, ""); argc++;
		/* printf("Argc=%d P:%s\n", argc, string);*/

	/* read next line(s) = SMS text */
		input_len = GSM_MAX_SMS_LENGTH;
		chars_read = fread(message_buffer, 1, input_len, fd);
		fclose(fd);
		argv[argc++] = "--text";
		argv[argc++] = message_buffer;
		argv[argc++] = "--smsd"; /* flag for omitting init/terminate */
		/* printf("%s,%s,%s\n", argv[argc-3],argv[argc-2],argv[argc-1]); */

	/* call procedure to send SMS */
		error = sendonesms(argc, argv);
		}

		logfile = fopen(logname, "a");
		if (logfile) {
			GetMachineDateTime(nowdate, nowtime );
			fprintf(logfile, _("%s %s Send %s (%d, %s)\n"),nowdate, nowtime, filename, error, print_error(error));
			fclose(logfile);
		}
	/* if ok then delete file */
		if ((error == GE_NONE) || (error == GE_SMSSENDOK) || (error == GE_SMSTOOLONG)) unlink(filename);
		free(namelist[n]);
	}
	free(namelist);
}
#endif
 return error;
}

