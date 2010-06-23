
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <wchar.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <string.h>
#  include <stdlib.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <signal.h>
#endif

#include "gsmcomon.h"
#include "misc/coding.h"

GSM_Error NoneFunction(void)
{
	return GE_NONE;
}

GSM_Error NotImplementedFunction(void)
{
	return GE_NOTIMPLEMENTED;
}

GSM_Error NotSupportedFunction(void)
{
	return GE_NOTSUPPORTED;
}

unsigned char *GetMsg (CFG_Header *cfg, unsigned char *default_string)
{
	unsigned char 		*retval, buffer[40], buff2[40], buff[2000];
	static unsigned char	def_str[2000];
	CFG_Entry		*e;
	CFG_Header 		*h;
	unsigned int		num;

	if (cfg==NULL) return default_string;

	EncodeUnicode (buff2, "common", 6);

	/* Set all 0x0a to \n */
	memset(def_str,0,sizeof(def_str));
	for (num=0;num<((int)strlen(default_string));num++) {
		if (default_string[num] == 0x0a) {
			def_str[strlen(def_str)] = '\\';
			def_str[strlen(def_str)] = 'n';
		} else def_str[strlen(def_str)] = default_string[num];
	}

	e = NULL;
	/* First find our section */
        for (h = cfg; h != NULL; h = h->next) {
		if (mywstrncasecmp(buff2, h->section, 0)) {
			e = h->entries;
			break;
		}
	}
	while (1) {
		if (e == NULL) break;
		num = -1;
		DecodeUnicode(e->key,buffer);
		if (strlen(buffer) == 5 && (buffer[0] == 'F' || buffer[0] == 'f')) {
			num = atoi(buffer+2);
		}
		if (num!=-1) {
			DecodeUnicode(e->value+2,buff);
			if (strncmp(buff,def_str,strlen(def_str))==0) {
				sprintf(buff,"T%04i",num);
				EncodeUnicode (buffer, buff, 5);
			        retval = CFG_Get(cfg, buff2, buffer, true);
			        if (retval) {
					DecodeUnicode(retval+2,buff);
					buff[strlen(buff)-1] = 0;
					/* Set all \n to 0x0a */
					memset(def_str,0,sizeof(def_str));
					num = 0;
					while (num != strlen(buff)) {
						if (num < strlen(buff) - 1) {
							if (buff[num] == '\\' && buff[num+1] == 'n') {
								def_str[strlen(def_str)] = 0x0a;
								num+=2;
							} else {
								def_str[strlen(def_str)] = buff[num++];
							}
						} else {
							def_str[strlen(def_str)] = buff[num++];
						}
					}
					retval = def_str;
				} else {
					retval = default_string;
				}
				return retval;
			}
		}
		e = e->next;
	}
	return default_string;
}

typedef struct {
	GSM_Error		ErrorNum;
	unsigned char		*ErrorText;
} PrintErrorEntry;

static PrintErrorEntry PrintErrorEntries[] = {
	{GE_NONE,			"No error."},
	{GE_DEVICEOPENERROR,		"Error opening device. Unknown or busy device."},
	{GE_DEVICEDTRRTSERROR,		"Error setting device DTR or RTS."},
	{GE_DEVICECHANGESPEEDERROR,	"Error setting device speed. Maybe speed not supported."},
	{GE_DEVICEWRITEERROR,		"Error writing device."},
	{GE_DEVICEREADERROR,		"Error during reading device"},
	{GE_DEVICEPARITYERROR,		"Can't set parity on device"},
	{GE_TIMEOUT,			"No response in specified timeout. Probably phone not connected."},
	/* Some missed */
	{GE_UNKNOWNRESPONSE,		"Unknown response from phone. See /readme, how to report it."},
	/* Some missed */
	{GE_UNKNOWNCONNECTIONTYPESTRING,"Unknown connection type string. Check config file."},
	{GE_UNKNOWNMODELSTRING,		"Unknown model type string. Check config file."},
	{GE_SOURCENOTAVAILABLE,		"Some required functions not compiled for your OS. Please contact."},
	{GE_NOTSUPPORTED,		"Function not supported by phone."},
	{GE_EMPTY,			"Entry is empty"},
	{GE_SECURITYERROR,		"Security error. Maybe no PIN ?"},
	{GE_INVALIDLOCATION,		"Invalid location. Maybe too high ?"},
	{GE_NOTIMPLEMENTED,		"Function not implemented. Help required."},
	{GE_FULL,			"Memory full."},
	{GE_UNKNOWN,			"Unknown error."},
	/* Some missed */
	{GE_CANTOPENFILE,		"Can't open specified file. Read only ?"},
	{GE_MOREMEMORY,			"More memory required..."},
	{GE_PERMISSION,			"Permission to file/device required..."},
	{GE_EMPTYSMSC,			"Empty SMSC number. Set in phone or use -smscnumber"},
	{GE_INSIDEPHONEMENU,		"You're inside phone menu (during editing ?). Leave it and try again."},
	{GE_WORKINPROGRESS,		"Function is during writing. If want help, please contact with authors."},

	{0,				""}
};

unsigned char *print_error(GSM_Error e, FILE *df, CFG_Header *cfg)
{
	unsigned char 	*def 	= NULL;
	int 		i	= 0;

	while (PrintErrorEntries[i].ErrorNum != 0) {
		if (PrintErrorEntries[i].ErrorNum == e) {
			def 	= PrintErrorEntries[i].ErrorText;
			break;
		}
		i++;
	}
	if (def == NULL) def = "Unknown error.";
	if (df!=NULL) fprintf(df,"[ERROR %i: %s]\n",e,def);

	return GetMsg(cfg,def);
}

char *GetGammuVersion()
{
	static char Buffer[1000]="";

	sprintf(Buffer, "%s",VERSION);
	return Buffer;
}

void GSM_GetCurrentDateTime (GSM_DateTime *Date)
{
	struct tm		*now;
	time_t			nowh;

	nowh = time(NULL);
	now  = localtime(&nowh);

	Date->Year	= now->tm_year;
	Date->Month	= now->tm_mon+1;
	Date->Day	= now->tm_mday;
	Date->Hour	= now->tm_hour;
	Date->Minute	= now->tm_min;
	Date->Second	= now->tm_sec;

	if (Date->Year<1900)
	{
		if (Date->Year>90) Date->Year = Date->Year+1900;
			      else Date->Year = Date->Year+2000;
	}
}

#define max_buf_len 	128
#define lock_path 	"/var/lock/LCK.."

/* Lock the device. Allocated string with a lock name is returned
 * in lock_device
 */
GSM_Error lock_device(const char* port, char **lock_device)
{
#ifndef WIN32
	char 		*lock_file = NULL;
	char 		buffer[max_buf_len];
	const char 	*aux = strrchr(port, '/');
	int 		fd, len = strlen(aux) + strlen(lock_path);
	GSM_Error	error = GE_NONE;

	/* Remove leading '/' */
	if (aux) aux++;
	else aux = port;

	memset(buffer, 0, sizeof(buffer));
	lock_file = calloc(len + 1, 1);
	if (!lock_file) {
		dprintf("Out of memory error while locking device\n");
		return GE_MOREMEMORY;
	}
	/* I think we don't need to use strncpy, as we should have enough
	 * buffer due to strlen results
	 */
	strcpy(lock_file, lock_path);
	strcat(lock_file, aux);

	/* Check for the stale lockfile.
	 * The code taken from minicom by Miquel van Smoorenburg */
	if ((fd = open(lock_file, O_RDONLY)) >= 0) {
		char 	buf[max_buf_len];
		int 	pid, n = 0;

		n = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (n > 0) {
			pid = -1;
			if (n == 4)
				/* Kermit-style lockfile. */
				pid = *(int *)buf;
			else {
				/* Ascii lockfile. */
				buf[n] = 0;
				sscanf(buf, "%d", &pid);
			}
			if (pid > 0 && kill((pid_t)pid, 0) < 0 && errno == ESRCH) {
				dprintf("Lockfile %s is stale. Overriding it..\n", lock_file);
				sleep(1);
				if (unlink(lock_file) == -1) {
					dprintf("Overriding failed, please check the permissions\n");
					dprintf("Cannot lock device\n");
					error = GE_PERMISSION;
					goto failed;
				}
			} else {
				dprintf("Device already locked.\n");
				error = GE_DEVICEOPENERROR;
				goto failed;
			}
		}
		/* this must not happen. because we could open the file   */
		/* no wrong permissions are set. only reason could be     */
		/* flock/lockf or a empty lockfile due to a broken binary */
		/* which is more likely					  */
		if (n == 0) {
			dprintf("Unable to read lockfile %s.\n", lock_file);
			dprintf("Please check for reason and remove the lockfile by hand.\n");
			dprintf("Cannot lock device\n");
			error = GE_UNKNOWN;
			goto failed;
		}
	}

	/* Try to create a new file, with 0644 mode */
	fd = open(lock_file, O_CREAT | O_EXCL | O_WRONLY, 0644);
	if (fd == -1) {
		if (errno == EEXIST) {
			dprintf("Device seems to be locked by unknown process\n");
			error = GE_DEVICEOPENERROR;		
		} else if (errno == EACCES) {
			dprintf("Please check permission on lock directory\n");
			error = GE_PERMISSION;
		} else if (errno == ENOENT) {
			dprintf("Cannot create lockfile %s. Please check for existence of path", lock_file);
			error = GE_UNKNOWN;
		}
		goto failed;
	}
	sprintf(buffer, "%10ld gammu\n", (long)getpid());
	write(fd, buffer, strlen(buffer));
	close(fd);
	*lock_device = lock_file;
	return GE_NONE;
failed:
	free(lock_file);
	*lock_device = 0;
	return error;
#else
	*lock_device = 0;
	return GE_NONE;
#endif /* WIN32 */
}

/* Removes lock and frees memory */
bool unlock_device(char *lock_file)
{
#ifndef WIN32
	int err;

	if (!lock_file) {
		dprintf("Cannot unlock device\n");
		return false;
	}
	err = unlink(lock_file);
	free(lock_file);
	return (err + 1);
#else
	return true;
#endif /* WIN32 */
}

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *di)
{
	FILE *file;

	file = di->df;
	if (info[0]!=0 && di->dl != 0) {
		if (di->df && di->df != stdout) fclose(di->df);
		switch (di->dl) {
		case DL_BINARY:
			di->df = fopen(info,"wcb");
			break;
		case DL_TEXTERROR:
			di->df = fopen(info,"ac");
			if (!di->df) {
				di->df = file;
				dprintf("Can't open debug file\n");
				return GE_CANTOPENFILE;
			}
			fseek(di->df, 0, SEEK_END);
			if (ftell(di->df) > 100000) {
				fclose(di->df);
				di->df = fopen(info,"wc");
			}
			break;
		default:
			di->df = fopen(info,"wc");
		}
		if (!di->df) {
			di->df = file;
			dprintf("Can't open debug file\n");
			return GE_CANTOPENFILE;
		}
	}
	return GE_NONE;
}

char *OSDateTime (GSM_DateTime dt, bool TimeZone)
{
	struct tm 	timeptr;
	static char 	retval[200],retval2[200];
	int 		p,q,r,w;

	/* Based on article in Polish PC-Kurier 8/1998 page 104
	 * Archive on http://www.pckurier.pl
	 */
	p=(14-dt.Month) / 12;
	q=dt.Month+12*p-2;
	r=dt.Year-p;
	w=(dt.Day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;

	timeptr.tm_yday 	= 0; 			/* FIXME */
	timeptr.tm_isdst 	= -1; 			/* FIXME */
	timeptr.tm_year 	= dt.Year - 1900;
	timeptr.tm_mon  	= dt.Month - 1;
	timeptr.tm_mday 	= dt.Day;
	timeptr.tm_hour 	= dt.Hour;
	timeptr.tm_min  	= dt.Minute;
	timeptr.tm_sec  	= dt.Second;
	timeptr.tm_wday 	= w;

#ifdef WIN32
	strftime(retval2, 200, "%#c", &timeptr);
#else
	strftime(retval2, 200, "%c", &timeptr);
#endif
	if (TimeZone) {
		if (dt.Timezone >= 0) {
			sprintf(retval," +%02i",dt.Timezone);
		} else {
			sprintf(retval," -%02i",dt.Timezone);
		}
		strcat(retval2,retval);
	}
	/* If don't have weekday name, include it */
	strftime(retval, 200, "%A", &timeptr);
	if (strstr(retval2,retval)==NULL) {
		strcat(retval2," (");
		strcat(retval2,retval);
		strcat(retval2,")");
	}

	return retval2;
}

