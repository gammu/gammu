/* Some source from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot 
 * GNU GPL version 2 or later
 */
/* Some source from Minicom (http://alioth.debian.org/projects/minicom)
 * (C) 1991,1992,1993,1994,1995,1996 by Miquel van Smoorenburg
 * GNU GPL version 2
 */

#include <string.h>
#ifdef WIN32
#  include <io.h>
#else
#  include <errno.h>
#  include <signal.h>
#endif

#include "../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef BLUETOOTH_RF_SEARCHING

GSM_Error bluetooth_checkservicename(GSM_StateMachine *s, char *name)
{
        if (s->ConnectionType == GCT_BLUEPHONET && strstr(name,"Nokia PC Suite")!=NULL) return ERR_NONE;
	if (s->ConnectionType == GCT_BLUEOBEX 	&& strstr(name,"OBEX")		!=NULL) return ERR_NONE;
        if (s->ConnectionType == GCT_BLUEAT 	&& strstr(name,"COM 1")		!=NULL) return ERR_NONE;
        return ERR_UNKNOWN;
}

#endif
#endif

#if defined (GSM_ENABLE_BLUETOOTHDEVICE) || defined (GSM_ENABLE_IRDADEVICE)

int socket_read(GSM_StateMachine *s, void *buf, size_t nbytes, int hPhone)
{
	fd_set 		readfds;
#ifdef WIN32
	struct timeval 	timer;
#endif

	FD_ZERO(&readfds);
	FD_SET(hPhone, &readfds);
#ifndef WIN32
	if (select(hPhone+1, &readfds, NULL, NULL, 0)) {
		return(read(hPhone, buf, nbytes));
	}
#else
	memset(&timer,0,sizeof(timer));
	if (select(0, &readfds, NULL, NULL, &timer) != 0) {
		return(recv(hPhone, buf, nbytes, 0));
	}
#endif
	return 0;
}

#ifdef WIN32
int socket_write(GSM_StateMachine *s, unsigned char *buf, size_t nbytes, int hPhone)
#else
int socket_write(GSM_StateMachine *s, void *buf, size_t nbytes, int hPhone)
#endif
{
	int		ret;
	size_t		actual = 0;

	do {
		ret = send(hPhone, buf, nbytes - actual, 0);
        	if (ret < 0) {
            		if (actual != nbytes) GSM_OSErrorInfo(s,"socket_write");
            		return actual;
        	}
		actual 	+= ret;
		buf 	+= ret;
	} while (actual < nbytes);

	return actual;
}

GSM_Error socket_close(GSM_StateMachine *s, int hPhone)
{
	shutdown(hPhone, 0);
#ifdef WIN32
	closesocket(hPhone); /*FIXME: error checking */
#else
	close(hPhone); /*FIXME: error checking */
#endif
	return ERR_NONE;
}

#endif

#ifdef ENABLE_LGPL

GSM_Error lock_device(const char* port, char **lock_device)
{
	*lock_device = 0;
	return ERR_NONE;
}

bool unlock_device(char **lock_file)
{
	return true;
}

#else

#define max_buf_len 	128
#define lock_path 	"/var/lock/LCK.."

/* Lock the device. Allocated string with a lock name is returned
 * in lock_device
 */
GSM_Error lock_device(const char* port, char **lock_device)
{
#if !defined(WIN32) && !defined(DJGPP)
	char 		*lock_file = NULL;
	char 		buffer[max_buf_len];
	const char 	*aux;
	int 		fd, len;
	GSM_Error	error = ERR_NONE;

	dbgprintf("Locking device\n");

	aux = strrchr(port, '/');
	/* Remove leading '/' */
	if (aux) {
		aux++;
	} else {
		/* No / in port */
		aux = port;
	}
	len = strlen(aux) + strlen(lock_path);

	memset(buffer, 0, sizeof(buffer));
	lock_file = calloc(len + 1, 1);
	if (!lock_file) {
		dbgprintf("Out of memory error while locking device\n");
		return ERR_MOREMEMORY;
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
				dbgprintf("Lockfile %s is stale. Overriding it..\n", lock_file);
				sleep(1);
				if (unlink(lock_file) == -1) {
					dbgprintf("Overriding failed, please check the permissions\n");
					dbgprintf("Cannot lock device\n");
					error = ERR_PERMISSION;
					goto failed;
				}
			} else {
				dbgprintf("Device already locked by PID %d.\n", pid);
				error = ERR_DEVICELOCKED;
				goto failed;
			}
		}
		/* this must not happen. because we could open the file   */
		/* no wrong permissions are set. only reason could be     */
		/* flock/lockf or a empty lockfile due to a broken binary */
		/* which is more likely					  */
		if (n == 0) {
			dbgprintf("Unable to read lockfile %s.\n", lock_file);
			dbgprintf("Please check for reason and remove the lockfile by hand.\n");
			dbgprintf("Cannot lock device\n");
			error = ERR_UNKNOWN;
			goto failed;
		}
	}

	/* Try to create a new file, with 0644 mode */
	fd = open(lock_file, O_CREAT | O_EXCL | O_WRONLY, 0644);
	if (fd == -1) {
		if (errno == EEXIST) {
			dbgprintf("Device seems to be locked by unknown process\n");
			error = ERR_DEVICEOPENERROR;		
		} else if (errno == EACCES) {
			dbgprintf("Please check permission on lock directory\n");
			error = ERR_PERMISSION;
		} else if (errno == ENOENT) {
			dbgprintf("Cannot create lockfile %s. Please check for existence of path\n", lock_file);
			error = ERR_UNKNOWN;
		} else {
			dbgprintf("Unknown error with creating lockfile %s\n", lock_file);
			error = ERR_UNKNOWN;
		}
		goto failed;
	}
	sprintf(buffer, "%10ld gammu\n", (long)getpid());
	write(fd, buffer, strlen(buffer));
	close(fd);
	*lock_device = lock_file;
	return ERR_NONE;
failed:
	free(lock_file);
	*lock_device = 0;
	return error;
#else
	*lock_device = 0;
	return ERR_NONE;
#endif
}

/* Removes lock and frees memory */
bool unlock_device(char **lock_file)
{
#if !defined(WIN32) && !defined(DJGPP)
	int err;

	if (!lock_file) {
		dbgprintf("Cannot unlock device\n");
		return false;
	}
	err = unlink(*lock_file);
	free(*lock_file);
	*lock_file = NULL;
	return (err + 1);
#else
	return true;
#endif
}

#endif

int FindSerialSpeed(char *buffer)
{
	switch (atoi(buffer)) {
		case 50		: return 50;
		case 75		: return 75;
		case 110	: return 110;
		case 134	: return 134;
		case 150	: return 150;
		case 200	: return 200;
		case 300	: return 300;
		case 600	: return 600;
		case 1200	: return 1200;
		case 1800	: return 1800;
		case 2400	: return 2400;
		case 4800	: return 4800;
		case 9600	: return 9600;
		case 19200	: return 19200;
		case 38400	: return 38400;
		case 57600	: return 57600;
		case 115200	: return 115200;
		case 230400	: return 230400;
		default		: return 0;	
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
