#define _GNU_SOURCE /* For asprintf */

#include "../../gsmstate.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "proxy.h"

#include "../../../libgammu/misc/string.h"

/* Expand a proxy command */
static char *
expand_proxy_command(const char *proxy_command)
{
	char *ret;

	ret = NULL;
	if (asprintf(&ret, "exec %s", proxy_command) < 0) {
		return NULL;
	}
	return ret;
}

#ifndef _PATH_BSHELL
# define _PATH_BSHELL "/bin/sh"
#endif

/*
 * Connect to the given ssh server using a proxy command.
 */
static GSM_Error
proxy_connect(GSM_StateMachine *s, GSM_Device_ProxyData *proxydata, const char *proxy_command)
{
	char *command_string;
	int pin[2], pout[2];
	pid_t pid;
	const char *shell;

	if ((shell = getenv("SHELL")) == NULL || *shell == '\0') {
		shell = _PATH_BSHELL;
	}

	/* Create pipes for communicating with the proxy. */
	if (pipe(pin) < 0 || pipe(pout) < 0) {
		GSM_OSErrorInfo(s, "Could not create pipes to communicate with the proxy");
		return ERR_DEVICEOPENERROR;
	}

	command_string = expand_proxy_command(proxy_command);
	if (command_string == NULL) {
		return ERR_MOREMEMORY;
	}
	dbgprintf(NULL, "Executing proxy command: %.500s", command_string);

	/* Fork and execute the proxy command. */
	if ((pid = fork()) == 0) {
		const char * argv[10];

		/* Child.  Permanently give up superuser privileges. */
		//permanently_drop_suid(original_real_uid);

		/* Redirect stdin and stdout. */
		close(pin[1]);
		if (pin[0] != 0) {
			if (dup2(pin[0], 0) < 0)
				perror("dup2 stdin");
			close(pin[0]);
		}
		close(pout[0]);
		if (dup2(pout[1], 1) < 0)
			perror("dup2 stdout");
		/* Cannot be 1 because pin allocated two descriptors. */
		close(pout[1]);

		/* Stderr is left as it is so that error messages get
		   printed on the user's terminal. */
		argv[0] = shell;
		argv[1] = "-c";
		argv[2] = command_string;
		argv[3] = NULL;

		/* Execute the proxy command.  Note that we gave up any
		   extra privileges above. */
		signal(SIGPIPE, SIG_DFL);
		execv(argv[0], (char **)argv);
		perror(argv[0]);
		exit(1);
	}
	/* Parent. */
	if (pid < 0) {
		GSM_OSErrorInfo(s, "fork failed");
		return ERR_DEVICEOPENERROR;
	} else
		proxydata->hProcess = pid; /* save pid to clean up later */

	/* Close child side of the descriptors. */
	close(pin[0]);
	close(pout[1]);

	/* Free the command name. */
	free(command_string);

	/* Set the connection file descriptors. */
	proxydata->hRead = pout[0];
	proxydata->hWrite = pin[1];

	/* Indicate OK return */
	return ERR_NONE;
}

void
kill_proxy_command(pid_t proxy_command_pid)
{
	/*
	 * Send SIGHUP to proxy command if used. We don't wait() in
	 * case it hangs and instead rely on init to reap the child
	 */
	if (proxy_command_pid > 1)
		kill(proxy_command_pid, SIGHUP);
}


GSM_Error proxy_open(GSM_StateMachine *s)
{
	return proxy_connect(s, &s->Device.Data.Proxy, s->CurrentConfig->Device);
}

ssize_t proxy_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	GSM_Device_ProxyData 		*d = &s->Device.Data.Proxy;
	struct timeval  		timeout2;
	fd_set	  			readfds;
	int	     			actual = 0;

	FD_ZERO(&readfds);
	FD_SET(d->hRead, &readfds);

	timeout2.tv_sec     = 0;
	timeout2.tv_usec    = 50000;

	if (select(d->hRead+1, &readfds, NULL, NULL, &timeout2)) {
		actual = read(d->hRead, buf, nbytes);
		if (actual == -1) GSM_OSErrorInfo(s,"proxy_read");
	}
	return actual;
}

static
ssize_t write_nosigpipe(int fd, const void *buf, size_t len)
{
    sigset_t oldset, newset;
    ssize_t result;
    siginfo_t si;
    struct timespec ts = {0, 0};

    sigemptyset(&newset);
    sigaddset(&newset, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &newset, &oldset);

    result = write(fd, buf, len);

    while (sigtimedwait(&newset, &si, &ts)>=0 || errno != EAGAIN);
    pthread_sigmask(SIG_SETMASK, &oldset, 0);

    return result;
}

ssize_t proxy_write(GSM_StateMachine *s, const void *buf, size_t nbytes)
{
	GSM_Device_ProxyData   *d = &s->Device.Data.Proxy;
	int		     	ret;
	size_t		  	actual = 0;
	const unsigned char *buffer = (const unsigned char *)buf; /* Just to have correct type */

	do {
		ret = write_nosigpipe(d->hWrite, buffer, nbytes - actual);
		if (ret < 0) {
			if (errno == EAGAIN) {
				usleep(1000);
				continue;
			}
	    		if (actual != nbytes) {
				GSM_OSErrorInfo(s, "proxy_write");
				smprintf(s, "Wanted to write %ld bytes, but %ld were written\n",
					(long)nbytes, (long)actual);
			}
	    		return actual;
		}
		actual  += ret;
		buffer  += ret;
		if (s->ConnectionType == GCT_FBUS2PL2303) usleep(1000);
	} while (actual < nbytes);
	return actual;
}

GSM_Error proxy_close(GSM_StateMachine *s)
{
	kill_proxy_command(s->Device.Data.Proxy.hProcess);
	close(s->Device.Data.Proxy.hRead);
	close(s->Device.Data.Proxy.hWrite);
	return ERR_NONE;
}

// aaa
GSM_Device_Functions ProxyDevice = {
	proxy_open,
	proxy_close,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	proxy_read,
	proxy_write
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
