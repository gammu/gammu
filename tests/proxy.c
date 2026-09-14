#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../libgammu/gsmstate.h"
#include "common.h"

/* Exercise otherwise unreliable syscall failures without production hooks. */
static int pipe_failure, pipe_calls, fcntl_failure, fork_failure, dup_failure;
static int allocation_failure, poll_failure, poll_events, write_mode, mask_failure;
static int pending_failure, wait_failure, read_failure, reap_interrupt, exec_failure;

static int test_pipe(int fds[2])
{
	if (++pipe_calls == pipe_failure) { errno = EMFILE; return -1; }
	return pipe(fds);
}

static int test_fcntl(int fd, int cmd, int arg)
{
	if (fcntl_failure) { errno = EMFILE; return -1; }
	return fcntl(fd, cmd, arg);
}

static pid_t test_fork(void)
{
	if (fork_failure) { errno = EAGAIN; return -1; }
	return fork();
}

static int test_dup2(int from, int to)
{
	if (dup_failure) { errno = EBADF; return -1; }
	return dup2(from, to);
}

static int test_asprintf(char **str, const char *format, const char *command)
{
	if (allocation_failure) { errno = ENOMEM; return -1; }
	return asprintf(str, format, command);
}

static int test_execl(const char *path, const char *arg0, const char *flag,
		const char *command, char *end UNUSED)
{
	if (exec_failure) { errno = ENOENT; return -1; }
	return execl(path, arg0, flag, command, (char *)NULL);
}

static int test_poll(struct pollfd *fds, nfds_t count, int timeout)
{
	if (poll_failure) {
		errno = poll_failure;
		poll_failure = 0;
		return -1;
	}
	if (poll_events) {
		fds[0].revents = poll_events;
		poll_events = 0;
		return 1;
	}
	return poll(fds, count, timeout);
}

static ssize_t test_write(int fd, const void *buf, size_t length)
{
	int mode = write_mode;
	write_mode = 0;
	switch (mode) {
		case 1: errno = EINTR; return -1;
		case 2: errno = EAGAIN; return -1;
		case 3: return write(fd, buf, length > 1 ? 1 : length);
		case 4: return 0;
		case 5: write_mode = 6; return write(fd, buf, 1);
		case 6: errno = EIO; return -1;
		default: return write(fd, buf, length);
	}
}

static ssize_t test_read(int fd, void *buf, size_t length)
{
	if (read_failure) {
		errno = read_failure;
		read_failure = 0;
		return -1;
	}
	return read(fd, buf, length);
}

static pid_t test_waitpid(pid_t pid, int *status, int options)
{
	if (reap_interrupt) {
		reap_interrupt = 0;
		errno = EINTR;
		return -1;
	}
	return waitpid(pid, status, options);
}

static int test_sigmask(int how, const sigset_t *set, sigset_t *old)
{
	if (mask_failure) return EINVAL;
	return pthread_sigmask(how, set, old);
}

static int test_sigpending(sigset_t *set)
{
	if (pending_failure) { errno = EINVAL; return -1; }
	return sigpending(set);
}

static int test_sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
	if (wait_failure) { errno = EINVAL; return -1; }
	return sigtimedwait(set, info, timeout);
}

#define pipe test_pipe
#define fcntl test_fcntl
#define fork test_fork
#define dup2 test_dup2
#define asprintf test_asprintf
#define execl test_execl
#define poll test_poll
#define write test_write
#define read test_read
#define waitpid test_waitpid
#define pthread_sigmask test_sigmask
#define sigpending test_sigpending
#define sigtimedwait test_sigtimedwait
#define proxy_open tested_proxy_open
#define proxy_close tested_proxy_close
#define proxy_read tested_proxy_read
#define proxy_write tested_proxy_write
#define ProxyDevice TestedProxyDevice
#include "../libgammu/device/proxy/proxy.c"
#undef pipe
#undef fcntl
#undef fork
#undef dup2
#undef asprintf
#undef execl
#undef poll
#undef write
#undef read
#undef waitpid
#undef pthread_sigmask
#undef sigpending
#undef sigtimedwait

static GSM_StateMachine *state(const char *command)
{
	GSM_StateMachine *s = GSM_AllocStateMachine();
	test_result(s != NULL);
	s->CurrentConfig = GSM_GetConfig(s, 0);
	s->CurrentConfig->Device = strdup(command);
	test_result(s->CurrentConfig->Device != NULL);
	return s;
}

static int descriptor_count(void)
{
	int fd, count = 0;
	for (fd = 0; fd < 256; fd++) {
		if (fcntl(fd, F_GETFD) >= 0) count++;
	}
	return count;
}

static double seconds(void)
{
	struct timespec now;
	test_result(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
	return now.tv_sec + now.tv_nsec / 1000000000.0;
}

static void receive(GSM_StateMachine *s, const void *expected, size_t length)
{
	char buffer[256];
	size_t got = 0;
	ssize_t result;
	double deadline = seconds() + 3;
	test_result(length <= sizeof(buffer));
	while (got < length && seconds() < deadline) {
		result = proxy_read(s, buffer + got, length - got);
		test_result(result >= 0);
		got += result;
	}
	test_result(got == length);
	test_result(memcmp(buffer, expected, length) == 0);
}

static void startup_failures(void)
{
	GSM_StateMachine *s = state("cat");
	int before = descriptor_count(), i;

	allocation_failure = 1;
	test_result(proxy_open(s) == ERR_MOREMEMORY);
	allocation_failure = 0;
	for (i = 1; i <= 3; i++) {
		pipe_calls = 0;
		pipe_failure = i;
		test_result(proxy_open(s) == ERR_DEVICEOPENERROR);
		test_result(descriptor_count() == before);
	}
	pipe_failure = 0;
	fcntl_failure = 1;
	test_result(proxy_open(s) == ERR_DEVICEOPENERROR);
	fcntl_failure = 0;
	fork_failure = 1;
	test_result(proxy_open(s) == ERR_DEVICEOPENERROR);
	fork_failure = 0;
	dup_failure = 1;
	test_result(proxy_open(s) == ERR_DEVICEOPENERROR);
	dup_failure = 0;
	exec_failure = 1;
	test_result(proxy_open(s) == ERR_DEVICEOPENERROR);
	test_result(errno == ENOENT);
	exec_failure = 0;
	test_result(descriptor_count() == before);
	test_result(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
	test_result(proxy_close(s) == ERR_NONE);
	test_result(s->Device.Data.Proxy.hRead == -1);
	test_result(s->Device.Data.Proxy.hWrite == -1);
	test_result(s->Device.Data.Proxy.hProcess == -1);
	GSM_FreeStateMachine(s);
}

static void roundtrip(void)
{
	GSM_StateMachine *s = state("cat");
	const char bytes[] = "AT\r\n\0\033\032> ";
	char buffer[32];
	int mode, i, before = descriptor_count();
	pid_t pid;

	for (i = 0; i < 10; i++) {
		test_result(proxy_open(s) == ERR_NONE);
		pid = s->Device.Data.Proxy.hProcess;
		test_result(fcntl(s->Device.Data.Proxy.hRead, F_GETFD) & FD_CLOEXEC);
		test_result(fcntl(s->Device.Data.Proxy.hWrite, F_GETFD) & FD_CLOEXEC);
		test_result(proxy_read(s, buffer, sizeof(buffer)) == 0);
		poll_failure = EINTR;
		test_result(proxy_read(s, buffer, sizeof(buffer)) == 0);
		poll_failure = EINVAL;
		test_result(proxy_read(s, buffer, sizeof(buffer)) == -1 && errno == EINVAL);
		poll_events = POLLERR;
		test_result(proxy_read(s, buffer, sizeof(buffer)) == -1 && errno == EIO);
		poll_events = POLLNVAL;
		test_result(proxy_read(s, buffer, sizeof(buffer)) == -1 && errno == EBADF);
		for (mode = 0; mode <= 3; mode++) {
			write_mode = mode;
			test_result(proxy_write(s, bytes, sizeof(bytes)) == sizeof(bytes));
			read_failure = EINTR;
			receive(s, bytes, sizeof(bytes));
		}
		test_result(proxy_write(s, bytes, 0) == 0);
		write_mode = 4;
		test_result(proxy_write(s, bytes, sizeof(bytes)) == -1 && errno == EIO);
		write_mode = 5;
		test_result(proxy_write(s, bytes, sizeof(bytes)) == 1 && errno == EIO);
		receive(s, bytes, 1);
		reap_interrupt = 1;
		test_result(proxy_close(s) == ERR_NONE);
		test_result(waitpid(pid, NULL, WNOHANG) == -1 && errno == ECHILD);
		test_result(proxy_close(s) == ERR_NONE);
		test_result(proxy_read(s, buffer, sizeof(buffer)) == -1 && errno == EBADF);
		test_result(descriptor_count() == before);
	}
	GSM_FreeStateMachine(s);
}

static void broken_pipe(void)
{
	GSM_StateMachine *s = state("unused");
	int fds[2];
	char byte;
	sigset_t blocked, old, pending, current;
	struct timespec timeout = {0, 0};

	test_result(pipe(fds) == 0);
	s->Device.Data.Proxy.hWrite = fds[1];
	s->Device.Data.Proxy.hRead = fds[0];
	close(fds[1]);
	test_result(proxy_read(s, &byte, 1) == 0);
	close(fds[0]);
	test_result(pipe(fds) == 0);
	s->Device.Data.Proxy.hWrite = fds[1];
	close(fds[0]);
	test_result(proxy_write(s, "x", 1) == -1 && errno == EPIPE);
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGPIPE);
	test_result(pthread_sigmask(SIG_BLOCK, &blocked, &old) == 0);
	test_result(raise(SIGPIPE) == 0);
	test_result(proxy_write(s, "x", 1) == -1 && errno == EPIPE);
	test_result(sigpending(&pending) == 0);
	test_result(sigismember(&pending, SIGPIPE) == 1);
	test_result(pthread_sigmask(SIG_SETMASK, NULL, &current) == 0);
	test_result(sigismember(&current, SIGPIPE) == 1);
	test_result(sigtimedwait(&blocked, NULL, &timeout) == SIGPIPE);
	test_result(proxy_write(s, "x", 1) == -1 && errno == EPIPE);
	test_result(sigpending(&pending) == 0);
	test_result(sigismember(&pending, SIGPIPE) == 0);
	mask_failure = 1;
	test_result(proxy_write(s, "x", 1) == -1 && errno == EINVAL);
	mask_failure = 0;
	pending_failure = 1;
	test_result(proxy_write(s, "x", 1) == -1 && errno == EINVAL);
	pending_failure = 0;
	/* An unexpected sigtimedwait failure must not spin indefinitely. */
	wait_failure = 1;
	test_result(proxy_write(s, "x", 1) == -1 && errno == EPIPE);
	wait_failure = 0;
	test_result(sigtimedwait(&blocked, NULL, &timeout) == SIGPIPE);
	test_result(pthread_sigmask(SIG_SETMASK, &old, NULL) == 0);
	test_result(pthread_sigmask(SIG_SETMASK, NULL, &current) == 0);
	test_result(sigismember(&current, SIGPIPE) == sigismember(&old, SIGPIPE));
	close(fds[1]);
	GSM_FreeStateMachine(s);
}

static void fixed_shell(void)
{
	const char *shells[] = {"/bin/false", "/nonexistent/gammu-test-shell"};
	GSM_StateMachine *s;
	size_t i;

	for (i = 0; i < sizeof(shells) / sizeof(shells[0]); i++) {
		test_result(setenv("SHELL", shells[i], 1) == 0);
		s = state("printf '%s' 'shell-independent'");
		test_result(proxy_open(s) == ERR_NONE);
		receive(s, "shell-independent", 17);
		test_result(proxy_close(s) == ERR_NONE);
		GSM_FreeStateMachine(s);
	}
	test_result(unsetenv("SHELL") == 0);
}

static void exited_child(void)
{
	GSM_StateMachine *s = state("printf done");
	pid_t pid;
	char byte;

	test_result(proxy_open(s) == ERR_NONE);
	pid = s->Device.Data.Proxy.hProcess;
	/* Buffered output must survive POLLHUP before EOF is reported. */
	test_result(waitpid(pid, NULL, 0) == pid);
	receive(s, "done", 4);
	test_result(proxy_read(s, &byte, 1) == 0);
	test_result(proxy_read(s, &byte, 1) == 0);
	/* An application SIGCHLD handler may already have reaped the child. */
	test_result(proxy_close(s) == ERR_NONE);
	GSM_FreeStateMachine(s);
}

static void closed_standard_descriptors(void)
{
	pid_t pid = fork();
	int status;
	test_result(pid >= 0);
	if (pid == 0) {
		GSM_StateMachine *s = state("cat");
		close(0);
		close(1);
		close(2);
		test_result(proxy_open(s) == ERR_NONE);
		test_result(proxy_write(s, "test", 4) == 4);
		receive(s, "test", 4);
		test_result(proxy_close(s) == ERR_NONE);
		GSM_FreeStateMachine(s);
		_exit(0);
	}
	test_result(waitpid(pid, &status, 0) == pid);
	test_result(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void unresponsive_child(const char *executable)
{
	GSM_StateMachine *s;
	char *command;
	double start;
	pid_t pid;
	test_result(asprintf(&command, "'%s' --ignore-hup", executable) >= 0);
	s = state(command);
	free(command);
	test_result(proxy_open(s) == ERR_NONE);
	pid = s->Device.Data.Proxy.hProcess;
	receive(s, "ready", 5);
	start = seconds();
	test_result(proxy_close(s) == ERR_NONE);
	test_result(seconds() - start < 3);
	test_result(waitpid(pid, NULL, WNOHANG) == -1 && errno == ECHILD);
	GSM_FreeStateMachine(s);
}

/* Exercise the documented local socat command against a TCP-only modem. */
static void tcp_proxy(const char *socat)
{
	const char bytes[] = "AT\r\n\0\033\032\021\023\377";
	struct sockaddr_in address;
	socklen_t address_length;
	GSM_StateMachine *s;
	struct pollfd readable;
	char *command, buffer[sizeof(bytes)];
	int listener, peer, connected;
	pid_t pid;
	ssize_t result;
	size_t got;

	test_result(setenv("GAMMU_TEST_SOCAT", socat, 1) == 0);
	for (connected = 0; connected <= 1; connected++) {
		listener = socket(AF_INET, SOCK_STREAM, 0);
		test_result(listener >= 0);
		test_result(fcntl(listener, F_SETFD, FD_CLOEXEC) == 0);
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		test_result(bind(listener, (struct sockaddr *)&address, sizeof(address)) == 0);
		address_length = sizeof(address);
		test_result(getsockname(listener, (struct sockaddr *)&address, &address_length) == 0);
		/* A bound but non-listening port gives a deterministic refused connection. */
		if (connected) test_result(listen(listener, 1) == 0);
		test_result(asprintf(&command, "\"$GAMMU_TEST_SOCAT\" STDIO TCP:127.0.0.1:%u",
			(unsigned int)ntohs(address.sin_port)) >= 0);
		s = state(command);
		free(command);
		test_result(proxy_open(s) == ERR_NONE);
		pid = s->Device.Data.Proxy.hProcess;
		if (connected) {
			readable.fd = listener;
			readable.events = POLLIN;
			test_result(poll(&readable, 1, 3000) == 1);
			peer = accept(listener, NULL, NULL);
			test_result(peer >= 0);
			test_result(proxy_write(s, bytes, sizeof(bytes)) == sizeof(bytes));
			got = 0;
			while (got < sizeof(bytes)) {
				readable.fd = peer;
				test_result(poll(&readable, 1, 3000) == 1);
				result = read(peer, buffer + got, sizeof(bytes) - got);
				test_result(result > 0);
				got += result;
			}
			test_result(memcmp(buffer, bytes, sizeof(bytes)) == 0);
			test_result(write(peer, bytes, sizeof(bytes)) == sizeof(bytes));
			receive(s, bytes, sizeof(bytes));
			/* Each fragment must arrive without waiting for another line. */
			test_result(write(peer, "\r\n>", 3) == 3);
			receive(s, "\r\n>", 3);
			test_result(write(peer, " ", 1) == 1);
			receive(s, " ", 1);
			close(peer);
		}
		readable.fd = s->Device.Data.Proxy.hRead;
		readable.events = POLLIN;
		test_result(poll(&readable, 1, 3000) == 1);
		test_result(readable.revents & POLLHUP);
		test_result(proxy_read(s, buffer, sizeof(buffer)) == 0);
		test_result(proxy_close(s) == ERR_NONE);
		test_result(waitpid(pid, NULL, WNOHANG) == -1 && errno == ECHILD);
		close(listener);
		GSM_FreeStateMachine(s);
	}
}

int main(int argc, char **argv)
{
	if (argc == 2 && strcmp(argv[1], "--ignore-hup") == 0) {
		signal(SIGHUP, SIG_IGN);
		if (write(STDOUT_FILENO, "ready", 5) != 5) return 1;
		for (;;) pause();
	}
	alarm(30);
	test_result(setenv("SHELL", "/bin/sh", 1) == 0);
	if (argc == 3 && strcmp(argv[1], "--tcp") == 0) {
		tcp_proxy(argv[2]);
		return 0;
	}
	startup_failures();
	fixed_shell();
	roundtrip();
	broken_pipe();
	exited_child();
	closed_standard_descriptors();
	unresponsive_child(argv[0]);
	return 0;
}
