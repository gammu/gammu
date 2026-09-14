#define _GNU_SOURCE /* For asprintf */

#include "../../gsmstate.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "proxy.h"

#ifndef _PATH_BSHELL
# define _PATH_BSHELL "/bin/sh"
#endif

/* Keep all private descriptors above stderr, even in a daemon which closed it. */
static int proxy_pipe(int fds[2])
{
	int i, fd, saved_errno;

	if (pipe(fds) < 0) return -1;
	for (i = 0; i < 2; i++) {
		if (fds[i] <= STDERR_FILENO) {
			fd = fcntl(fds[i], F_DUPFD, STDERR_FILENO + 1);
			if (fd < 0) goto fail;
			close(fds[i]);
			fds[i] = fd;
		}
		if (fcntl(fds[i], F_SETFD, FD_CLOEXEC) < 0) goto fail;
	}
	return 0;

fail:
	saved_errno = errno;
	close(fds[0]);
	close(fds[1]);
	fds[0] = fds[1] = -1;
	errno = saved_errno;
	return -1;
}

static void proxy_reap(pid_t pid)
{
	pid_t result;
	int i;

	if (pid <= 1) return;
	/* Check ownership before signalling: another handler may have reaped it. */
	do {
		result = waitpid(pid, NULL, WNOHANG);
	} while (result < 0 && errno == EINTR);
	if (result != 0) return;

	kill(pid, SIGHUP);
	for (i = 0; i < 100; i++) {
		result = waitpid(pid, NULL, WNOHANG);
		if (result == pid || (result < 0 && errno != EINTR)) return;
		usleep(10000);
	}
	kill(pid, SIGKILL);
	do {
		result = waitpid(pid, NULL, 0);
	} while (result < 0 && errno == EINTR);
}

/* The status pipe is closed by exec, or carries errno on child setup failure. */
static void proxy_child_error(int fd)
{
	int saved_errno = errno;
	const char *buffer = (const char *)&saved_errno;
	size_t sent = 0;
	ssize_t result;

	while (sent < sizeof(saved_errno)) {
		result = write(fd, buffer + sent, sizeof(saved_errno) - sent);
		if (result < 0 && errno == EINTR) continue;
		if (result <= 0) break;
		sent += result;
	}
	_exit(127);
}

GSM_Error proxy_open(GSM_StateMachine *s)
{
	GSM_Device_ProxyData *d = &s->Device.Data.Proxy;
	char *command = NULL;
	int pin[2] = {-1, -1}, pout[2] = {-1, -1}, status[2] = {-1, -1};
	int child_errno = 0, saved_errno;
	pid_t pid = -1;
	ssize_t result;
	size_t received = 0;

	d->hRead = d->hWrite = -1;
	d->hProcess = -1;
	if (asprintf(&command, "exec %s", s->CurrentConfig->Device) < 0) {
		return ERR_MOREMEMORY;
	}
	if (proxy_pipe(pin) < 0 || proxy_pipe(pout) < 0 || proxy_pipe(status) < 0) goto fail;

	smprintf(s, "Executing proxy command: %.500s\n", command);
	pid = fork();
	if (pid < 0) goto fail;
	if (pid == 0) {
		close(pin[1]);
		close(pout[0]);
		close(status[0]);
		if (dup2(pin[0], STDIN_FILENO) < 0 || dup2(pout[1], STDOUT_FILENO) < 0) {
			proxy_child_error(status[1]);
		}
		close(pin[0]);
		close(pout[1]);
		if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) proxy_child_error(status[1]);
		execl(_PATH_BSHELL, _PATH_BSHELL, "-c", command, (char *)NULL);
		proxy_child_error(status[1]);
	}

	close(pin[0]);
	pin[0] = -1;
	close(pout[1]);
	pout[1] = -1;
	close(status[1]);
	status[1] = -1;
	while (received < sizeof(child_errno)) {
		result = read(status[0], (char *)&child_errno + received, sizeof(child_errno) - received);
		if (result < 0 && errno == EINTR) continue;
		if (result < 0) goto fail;
		if (result == 0) break;
		received += result;
	}
	if (received != 0) {
		errno = received == sizeof(child_errno) ? child_errno : EIO;
		goto fail;
	}
	close(status[0]);
	free(command);
	d->hRead = pout[0];
	d->hWrite = pin[1];
	d->hProcess = pid;
	return ERR_NONE;

fail:
	saved_errno = errno;
	if (pin[0] >= 0) close(pin[0]);
	if (pin[1] >= 0) close(pin[1]);
	if (pout[0] >= 0) close(pout[0]);
	if (pout[1] >= 0) close(pout[1]);
	if (status[0] >= 0) close(status[0]);
	if (status[1] >= 0) close(status[1]);
	free(command);
	proxy_reap(pid);
	errno = saved_errno;
	GSM_OSErrorInfo(s, "Could not start proxy command");
	errno = saved_errno;
	return ERR_DEVICEOPENERROR;
}

ssize_t proxy_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	struct pollfd fd = {s->Device.Data.Proxy.hRead, POLLIN, 0};
	ssize_t actual;
	int ready, saved_errno;

	if (nbytes == 0) return 0;
	if (fd.fd < 0) {
		errno = EBADF;
		goto fail;
	}
	ready = poll(&fd, 1, 50);
	if (ready == 0 || (ready < 0 && errno == EINTR)) return 0;
	if (ready < 0) goto fail;
	if (fd.revents & POLLNVAL) {
		errno = EBADF;
		goto fail;
	}
	if (!(fd.revents & (POLLIN | POLLHUP))) {
		errno = EIO;
		goto fail;
	}
	/* Read buffered data even when the writer has already closed its pipe. */
	actual = read(fd.fd, buf, nbytes);
	/* Direct protocol callers require EOF to remain a zero byte count. */
	if (actual >= 0) return actual;
	if (actual < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
fail:
	saved_errno = errno;
	GSM_OSErrorInfo(s, "proxy_read");
	errno = saved_errno;
	return -1;
}

static ssize_t write_nosigpipe(int fd, const void *buf, size_t len)
{
	sigset_t oldset, newset, pending;
	ssize_t result;
	struct timespec timeout = {0, 0};
	int error, saved_errno, was_pending;

	sigemptyset(&newset);
	sigaddset(&newset, SIGPIPE);
	error = pthread_sigmask(SIG_BLOCK, &newset, &oldset);
	if (error != 0) {
		errno = error;
		return -1;
	}
	if (sigpending(&pending) < 0) {
		saved_errno = errno;
		pthread_sigmask(SIG_SETMASK, &oldset, NULL);
		errno = saved_errno;
		return -1;
	}
	was_pending = sigismember(&pending, SIGPIPE);
	result = write(fd, buf, len);
	saved_errno = errno;
	if (result < 0 && saved_errno == EPIPE && !was_pending) {
		/* Consume only the signal generated by this write, if not ignored. */
		while (sigtimedwait(&newset, NULL, &timeout) < 0 && errno == EINTR) {}
	}
	error = pthread_sigmask(SIG_SETMASK, &oldset, NULL);
	if (error != 0 && result < 0 && saved_errno != EPIPE) saved_errno = error;
	errno = saved_errno;
	return result;
}

ssize_t proxy_write(GSM_StateMachine *s, const void *buf, size_t nbytes)
{
	struct pollfd fd = {s->Device.Data.Proxy.hWrite, POLLOUT, 0};
	const unsigned char *buffer = buf;
	size_t actual = 0;
	ssize_t result;
	int ready, saved_errno;

	while (actual < nbytes) {
		result = write_nosigpipe(fd.fd, buffer + actual, nbytes - actual);
		if (result > 0) {
			actual += result;
			continue;
		}
		if (result < 0 && errno == EINTR) continue;
		if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			do {
				ready = poll(&fd, 1, -1);
			} while (ready < 0 && errno == EINTR);
			if (ready > 0 && (fd.revents & POLLOUT)) continue;
			if (ready >= 0) errno = (fd.revents & POLLNVAL) ? EBADF : EPIPE;
		}
		if (result == 0) errno = EIO;
		saved_errno = errno;
		GSM_OSErrorInfo(s, "proxy_write");
		errno = saved_errno;
		return actual > 0 ? (ssize_t)actual : -1;
	}
	return actual;
}

GSM_Error proxy_close(GSM_StateMachine *s)
{
	GSM_Device_ProxyData *d = &s->Device.Data.Proxy;

	if (d->hRead >= 0) close(d->hRead);
	if (d->hWrite >= 0) close(d->hWrite);
	d->hRead = d->hWrite = -1;
	proxy_reap(d->hProcess);
	d->hProcess = -1;
	return ERR_NONE;
}

GSM_Device_Functions ProxyDevice = {
	proxy_open,
	proxy_close,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	proxy_read,
	proxy_write
};

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
