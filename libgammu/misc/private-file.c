#include "private-file.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

FILE *GSM_OpenPrivateFile(const char *path, const char *mode)
{
	int append, binary, update, fd, flags;
	const char *suffix;
	FILE *file;

	if (mode == NULL || (mode[0] != 'w' && mode[0] != 'a')) {
		errno = EINVAL;
		return NULL;
	}
	append = mode[0] == 'a';
	suffix = mode + 1;
	binary = *suffix == 'b';
	if (binary) suffix++;
	update = *suffix == '+';
	if (update) suffix++;
	if (*suffix != '\0'
#ifdef _MSC_VER
		&& !(*suffix == 'c' && suffix[1] == '\0')
#endif
	) {
		errno = EINVAL;
		return NULL;
	}

#ifdef WIN32
	flags = (update ? _O_RDWR : _O_WRONLY) | _O_CREAT
		| (append ? _O_APPEND : _O_TRUNC)
		| (binary ? _O_BINARY : _O_TEXT);
	fd = _open(path, flags, _S_IREAD | _S_IWRITE);
#else
	flags = (update ? O_RDWR : O_WRONLY) | O_CREAT
		| (append ? O_APPEND : O_TRUNC);
	fd = open(path, flags, 0600);
#endif
	if (fd < 0) {
		return NULL;
	}

	file = fdopen(fd, mode);
	if (file == NULL) {
		int saved_errno = errno;
#ifdef WIN32
		_close(fd);
#else
		close(fd);
#endif
		errno = saved_errno;
	}
	return file;
}

int GSM_CreatePrivateFileExclusive(const char *path, int binary)
{
#ifdef WIN32
	return _open(path, _O_WRONLY | _O_CREAT | _O_EXCL
		| (binary ? _O_BINARY : _O_TEXT), _S_IREAD | _S_IWRITE);
#else
	(void)binary;
	return open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif
}
