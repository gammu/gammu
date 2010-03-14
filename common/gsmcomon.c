
#include <ctype.h>
#include <string.h>
#include <time.h>
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

char *GetMsg (CFG_Header *cfg, int number, char *default_string)
{
	char *retval;
	char buffer[10];

	if (cfg==NULL) return default_string;
	sprintf(buffer,"%06i",number);
        retval = CFG_Get(cfg, "common", buffer);
        if (!retval) {
		retval = default_string;
	} else {
		retval=retval+1;
		retval[strlen(retval)-1]=0;
	}
	return retval;
}

typedef struct {
	GSM_Error	ErrorNum;
	int		LocalizationNum;
	char		*ErrorText;
} PrintErrorEntry;

static PrintErrorEntry PrintErrorEntries[] = {
	{GE_NONE,			 1,"No error."},
	{GE_DEVICEOPENERROR,		 2,"Error opening device. Unknown or busy device."},
	{GE_DEVICEDTRRTSERROR,		 3,"Error setting device DTR or RTS."},
	{GE_DEVICECHANGESPEEDERROR,	 4,"Error setting device speed. Maybe speed not supported."},
	{GE_DEVICEWRITEERROR,		 5,"Error writing device."},
	{GE_DEVICEREADERROR,		43,"Error during reading device"},
	{GE_DEVICEPARITYERROR,		44,"Can't set parity on device"},
	{GE_TIMEOUT,			 6,"No response in specified timeout. Probably phone not connected."},
	/* Some missed */
	{GE_UNKNOWNRESPONSE,		 7,"Unknown response from phone. See /readme, how to report it."},
	/* Some missed */
	{GE_UNKNOWNCONNECTIONTYPESTRING, 8,"Unknown connection type string. Check config file."},
	{GE_UNKNOWNMODELSTRING,		 9,"Unknown model type string. Check config file."},
	{GE_SOURCENOTAVAILABLE,		10,"Some required functions not compiled for your OS. Please contact."},
	{GE_NOTSUPPORTED,		11,"Function not supported by phone."},
	{GE_EMPTY,			12,"Entry is empty"},
	{GE_SECURITYERROR,		15,"Security error. Maybe no PIN ?"},
	{GE_INVALIDLOCATION,		16,"Invalid location. Maybe too high ?"},
	{GE_NOTIMPLEMENTED,		17,"Function not implemented. Help required."},
	{GE_FULL,			19,"Memory full."},
	{GE_UNKNOWN,			13,"Unknown error."},
	/* Some missed */
	{GE_CANTOPENFILE,		18,"Can't open specified file. Read only ?"},
	{GE_MOREMEMORY,			20,"More memory required..."},
	{GE_PERMISSION,			21,"Permission to file/device required..."},
	{GE_EMPTYSMSC,			29,"Empty SMSC number. Set in phone or use -smscnumber"},
	{GE_INSIDEPHONEMENU,		30,"You're inside phone menu (during editing ?). Leave it and try again."},
	{GE_WORKINPROGRESS,		42,"Function is during writing. If want help, please contact with authors."},

	{0,				 0,""}
};

char *print_error(GSM_Error e, FILE *df, CFG_Header *cfg)
{
	char 	*def 	= NULL;
	int 	i	= 0;

	while (PrintErrorEntries[i].LocalizationNum != 0) {
		if (PrintErrorEntries[i].ErrorNum == e) {
			def 	= PrintErrorEntries[i].ErrorText;
			i	= PrintErrorEntries[i].LocalizationNum;
			break;
		}
		i++;
	}
	if (def == NULL) {
		def 	= "Unknown error.";
		i 	= 14;
	}
	if (df!=NULL) fprintf(df,"[ERROR %i: %s]\n",e,def);

	return GetMsg(cfg,i,def);
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
