#include "../libgammu/misc/private-file.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

int main(int argc, char **argv)
{
	char directory[PATH_MAX], path[PATH_MAX];
	unsigned char bytes[3], actual[3];
	mode_t old_umask;
	struct stat info;
	FILE *file;
	int length;
	int fd;

	test_result(argc == 2);
	length = snprintf(directory, sizeof(directory), "%s/private-file-XXXXXX", argv[1]);
	test_result(length >= 0 && (size_t)length < sizeof(directory));
	test_result(mkdtemp(directory) != NULL);
	length = snprintf(path, sizeof(path), "%s/data", directory);
	test_result(length >= 0 && (size_t)length < sizeof(path));

	old_umask = umask(0);
	file = GSM_OpenPrivateFile(path, "w");
	test_result(file != NULL);
	test_result(fputs("first", file) >= 0);
	test_result(fclose(file) == 0);
	test_result(stat(path, &info) == 0);
	test_result((info.st_mode & 0777) == 0600);

	test_result(chmod(path, 0644) == 0);
	file = GSM_OpenPrivateFile(path, "a");
	test_result(file != NULL);
	test_result(fputs(" second", file) >= 0);
	test_result(fclose(file) == 0);
	test_result(stat(path, &info) == 0);
	test_result((info.st_mode & 0777) == 0644);

	file = GSM_OpenPrivateFile(path, "w+");
	test_result(file != NULL);
	test_result(fputs("third", file) >= 0);
	test_result(fseek(file, 0, SEEK_SET) == 0);
	test_result(fgetc(file) == 't');
	test_result(fclose(file) == 0);
	test_result(stat(path, &info) == 0);
	test_result((info.st_mode & 0777) == 0644);

	test_result(remove(path) == 0);
	bytes[0] = 0;
	bytes[1] = 255;
	bytes[2] = 42;
	file = GSM_OpenPrivateFile(path, "wb");
	test_result(file != NULL);
	test_result(fwrite(bytes, 1, 2, file) == 2);
	test_result(fclose(file) == 0);
	file = GSM_OpenPrivateFile(path, "ab");
	test_result(file != NULL);
	test_result(fwrite(bytes + 2, 1, 1, file) == 1);
	test_result(fclose(file) == 0);
	test_result(stat(path, &info) == 0);
	test_result((info.st_mode & 0777) == 0600);
	file = fopen(path, "rb");
	test_result(file != NULL);
	test_result(fread(actual, 1, 3, file) == 3);
	test_result(memcmp(bytes, actual, 3) == 0);
	test_result(fclose(file) == 0);

	errno = 0;
	test_result(GSM_OpenPrivateFile(path, "r") == NULL);
	test_result(errno == EINVAL);
	test_result(remove(path) == 0);

	fd = GSM_CreatePrivateFileExclusive(path, 1);
	test_result(fd >= 0);
	test_result(write(fd, bytes, sizeof(bytes)) == sizeof(bytes));
	test_result(close(fd) == 0);
	test_result(stat(path, &info) == 0);
	test_result((info.st_mode & 0777) == 0600);
	errno = 0;
	test_result(GSM_CreatePrivateFileExclusive(path, 1) == -1);
	test_result(errno == EEXIST);
	test_result(remove(path) == 0);
	test_result(rmdir(directory) == 0);
	umask(old_umask);
	return 0;
}
