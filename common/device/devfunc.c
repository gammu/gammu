
#include <string.h>
#ifdef WIN32
#  include <io.h>
#else
#  include <errno.h>
#  include <signal.h>
#endif

#include "../gsmstate.h"

#if defined (GSM_ENABLE_BLUETOOTHDEVICE) || defined (GSM_ENABLE_IRDADEVICE)

int socket_read(GSM_StateMachine *s, void *buf, size_t nbytes, int hPhone)
{
#ifndef WIN32
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(hPhone, &readfds);
    if (select(hPhone+1, &readfds, NULL, NULL, 0)) {
	return(read(hPhone, buf, nbytes));
    } else {
	return 0;
    }
#else
    return(recv(hPhone, buf, nbytes, 0));
#endif
}

#ifdef WIN32
int socket_write(GSM_StateMachine *s, unsigned char *buf, size_t nbytes, int hPhone)
#else
int socket_write(GSM_StateMachine *s, void *buf, size_t nbytes, int hPhone)
#endif
{
    int		ret;
    size_t	actual = 0;

    do {
	if ((ret = send(hPhone, buf, nbytes - actual, 0)) < 0) return(actual);
	actual 	+= ret;
	buf 	+= ret;
    } while (actual < nbytes);
    return (actual);
}

GSM_Error socket_close(GSM_StateMachine *s, int hPhone)
{
	shutdown(hPhone, 0);
	close(hPhone); /*FIXME: error checking */

	return GE_NONE;
}

#endif

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
	const char 	*aux;
	int 		fd, len;
	GSM_Error	error = GE_NONE;

	dprintf("Locking device\n");

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
				dprintf("Device already locked by PID %d.\n", pid);
				error = GE_DEVICELOCKED;
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
			dprintf("Cannot create lockfile %s. Please check for existence of path\n", lock_file);
			error = GE_UNKNOWN;
		} else {
			dprintf("Unknown error with creating lockfile %s\n", lock_file);
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
bool unlock_device(char **lock_file)
{
#ifndef WIN32
	int err;

	if (!lock_file) {
		dprintf("Cannot unlock device\n");
		return false;
	}
	err = unlink(*lock_file);
	free(*lock_file);
	*lock_file = NULL;
	return (err + 1);
#else
	return true;
#endif /* WIN32 */
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
