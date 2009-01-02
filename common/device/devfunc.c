/* Some source from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */

#include <string.h>
#include <fcntl.h>
#ifdef WIN32
#  include <winsock2.h>
#  include <io.h>
#else
#  include <errno.h>
#  include <ctype.h>
#  include <signal.h>
#  include <sys/socket.h>
#  include <sys/stat.h>
#endif

#include "devfunc.h"
#include "../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef BLUETOOTH_RF_SEARCHING

int bluetooth_checkservicename(GSM_StateMachine *s, const char *name)
{
	/* Phonet */
	if (s->ConnectionType == GCT_BLUEPHONET) {
		if (strstr(name, "Nokia PC Suite") != NULL) return 1;

	/* OBEX */
	} else if (s->ConnectionType == GCT_BLUEOBEX) {
		/* Prefer this on Nokia as this gives better access to filesystem */
		if (strstr(name, "Nokia OBEX PC Suite Services") != NULL) {
			if (strcmp(s->CurrentConfig->Model, "obex") == 0) {
				return 5;
			}
			return 3;
		}
		/* For filesystem, we prefer file transfer */
		if (strstr(name, "OBEX File Transfer") != NULL || strstr(name, "OBEX file transfer") != NULL) {
			if (strcmp(s->CurrentConfig->Model, "obex") == 0) {
				return 4;
			}
			return 3;
		}
		/* Ususally this name also contains OBEX, prefered for irmc */
		if (strstr(name, "IrMC Sync") != NULL || strstr(name, "OBEX Synchronisation")) {
			if (strcmp(s->CurrentConfig->Model, "obexirmc") == 0 ||
				strcmp(s->CurrentConfig->Model, "seobex") == 0) {
				return 4;
			}
			return 3;
		}
		/* Simple send of files should work here */
		if (strstr(name, "OBEX Object Push") != NULL) {
			if (strcmp(s->CurrentConfig->Model, "obexnone") == 0) {
				return 3;
			}
			return 2;
		}
		/* Phone Book Access Profile is subset of IrMC */
		if (strstr(name, "PBAP") != NULL) {
			if (strcmp(s->CurrentConfig->Model, "obexirmc") == 0) {
				return 3;
			}
			return 2;
		}
		/* Object Push Profile */
		if (strstr(name, "OPP") != NULL) {
			if (strcmp(s->CurrentConfig->Model, "obexnone") == 0) {
				return 3;
			}
			return 2;
		}
		/* Anything matching OBEX has lowest priority */
		if (strstr(name, "OBEX") != NULL) return 1;

	/* AT */
	} else if (s->ConnectionType == GCT_BLUEAT) {
		/* Sony-Ericsson */
		if (strstr(name, "Serial Port 1") != NULL) return 3;
		if (strstr(name, "Serial Port") != NULL) return 2;
		/* Motorola */
		if (strstr(name, "Dial-up networking Gateway") != NULL) return 3;
		if (strstr(name, "Serial port") != NULL) return 2;
		/* Samsung */
		if (strstr(name, "Dial-up Networking") != NULL) return 3;
		/* Siemens, Thomas Eitzenberger */
		if (strstr(name, "SerialPort1") != NULL) return 3;
		if (strstr(name, "SerialPort") != NULL) return 2;

		if (strstr(name, "COM1") != NULL) return 3;
		if (strstr(name, "COM") != NULL) return 1;
	}

        return 0;
}

#endif
#endif

#if defined (GSM_ENABLE_BLUETOOTHDEVICE) || defined (GSM_ENABLE_IRDADEVICE)

/* Windows do not have this, but we don't seem to need it there */
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

int socket_read(GSM_StateMachine *s UNUSED, void *buf, size_t nbytes, socket_type hPhone)
{
	fd_set 		readfds;
	int result = 0;
	struct timeval 	timer;

	FD_ZERO(&readfds);
	FD_SET(hPhone, &readfds);

	timer.tv_sec = 0;
	timer.tv_usec = 0;

	if (select(hPhone + 1, &readfds, NULL, NULL, &timer) > 0) {
		result = recv(hPhone, buf, nbytes, MSG_DONTWAIT);
#ifndef WIN32
		if (result < 0 && errno != EINTR) {
 			return 0;
 		}
#endif
	}

	return result;
}

int socket_write(GSM_StateMachine *s, unsigned const char *buf, size_t nbytes, socket_type hPhone)
{
	int		ret;
	size_t		actual = 0;

	do {
		ret = send(hPhone, buf, nbytes - actual, 0);
        	if (ret < 0) {
            		if (actual != nbytes) {
				GSM_OSErrorInfo(s,"socket_write");
#ifndef WIN32
				if (errno != EINTR) {
					return 0;
				}
#endif
			}
            		return actual;
        	}
		actual 	+= ret;
		buf 	+= ret;
	} while (actual < nbytes);

	return actual;
}

GSM_Error socket_close(GSM_StateMachine *s UNUSED, socket_type hPhone)
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

#define max_buf_len 	128
#define lock_path 	"/var/lock/LCK.."

/* Lock the device. Allocated string with a lock name is returned
 * in lock_device
 */
#if !defined(WIN32) && !defined(DJGPP)
GSM_Error lock_device(GSM_StateMachine *s, const char* port, char **lock_name)
{
	char 		*lock_file = NULL;
	char 		buffer[max_buf_len];
	const char 	*aux;
	int 		fd, len;
	GSM_Error	error = ERR_NONE;
	size_t wrotebytes;
	char 	buf[max_buf_len];
	int 	pid, n = 0;


	smprintf(s, "Locking device\n");

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
		smprintf(s, "Out of memory error while locking device\n");
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
		n = read(fd, buf, sizeof(buf) - 1);
		if (n <= 0) {
			goto failread;
		}
		if (n == 4 && 4 == sizeof(int) &&
			! (
				isdigit(buf[0]) &&
				isdigit(buf[1]) &&
				isdigit(buf[2]) &&
				isdigit(buf[3])
				)) {
			/* Rewind */
			lseek(fd, 0, SEEK_SET);
			/* Read PID */
			/* We could make it from buf, but we would have to care about endians. */
			n = read(fd, &pid, sizeof(int));
			if (n != 4) {
				smprintf(s, "Reading lock for second time failed\n");
				goto failread;

			}
		} else {
			/* Ascii lockfile. */
			buf[n] = 0;
			sscanf(buf, "%d", &pid);
		}


		if (pid > 0 && kill((pid_t)pid, 0) < 0 && errno == ESRCH) {
			smprintf(s, "Lockfile %s is stale. Overriding it..\n", lock_file);
			if (unlink(lock_file) != 0) {
				smprintf(s, "Overriding failed, please check the permissions\n");
				smprintf(s, "Cannot lock device\n");
				error = ERR_PERMISSION;
				goto failed;
			}
		} else {
			smprintf(s, "Device already locked by PID %d.\n", pid);
			error = ERR_DEVICELOCKED;
			goto failed;
		}
	}

	/* Try to create a new file, with 0644 mode */
	fd = open(lock_file, O_CREAT | O_EXCL | O_WRONLY, 0644);
	if (fd == -1) {
		if (errno == EEXIST) {
			smprintf(s, "Device seems to be locked by unknown process\n");
			error = ERR_DEVICEOPENERROR;
		} else if (errno == EACCES) {
			smprintf(s, "Please check permission on lock directory\n");
			error = ERR_PERMISSION;
		} else if (errno == ENOENT) {
			smprintf(s, "Cannot create lockfile %s. Please check for existence of path\n", lock_file);
			error = ERR_UNKNOWN;
		} else {
			smprintf(s, "Unknown error with creating lockfile %s\n", lock_file);
			error = ERR_UNKNOWN;
		}
		goto failed;
	}
	sprintf(buffer, "%10ld gammu\n", (long)getpid());
	wrotebytes = write(fd, buffer, strlen(buffer));
	if (wrotebytes != strlen(buffer)) {
		return ERR_WRITING_FILE;
	}
	close(fd);
	*lock_name = lock_file;
	return ERR_NONE;
failread:
	smprintf(s, "Unable to read lockfile %s.\n", lock_file);
	smprintf(s, "Please check for reason and remove the lockfile by hand.\n");
	smprintf(s, "Cannot lock device\n");
	error = ERR_UNKNOWN;
failed:
	free(lock_file);
	*lock_name = NULL;
	return error;
}
#else
GSM_Error lock_device(GSM_StateMachine *s UNUSED, const char* port UNUSED, char **lock_name)
{
	*lock_name = NULL;
	return ERR_NONE;
}
#endif

/* Removes lock and frees memory */
#if !defined(WIN32) && !defined(DJGPP)
bool unlock_device(GSM_StateMachine *s, char **lock_file)
{
	int err;

	if (lock_file == NULL || *lock_file == NULL) {
		smprintf(s, "Cannot unlock device\n");
		return false;
	}
	err = unlink(*lock_file);
	free(*lock_file);
	*lock_file = NULL;
	return (err + 1);
}
#else
bool unlock_device(GSM_StateMachine *s UNUSED, char **lock_file UNUSED)
{
	return true;
}
#endif

int FindSerialSpeed(const char *buffer)
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
