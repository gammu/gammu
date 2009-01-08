#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/device/devfunc.h"

#include "common.h"

#define lock_path 	"/var/lock/LCK.."
#define TEST_DEVICE "/dev/foo/bar"
#define TEST_LOCK lock_path "bar"

void create_lock(const char *name, const void *lock_data, const size_t lock_data_len)
{
	FILE *fd;
	fd = fopen(name, "w");
	test_result(fd != NULL);
	test_result(fwrite(lock_data, 1, lock_data_len, fd) == lock_data_len);
	test_result(fclose(fd) == 0);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	char *lock = NULL;
	int pid;
	char pids[20];

	pid = getpid();
	sprintf(pids, "%d", pid);

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Non existing PID, ASCII */
	create_lock(TEST_LOCK, "1234567890", 10);
	test_result(lock_device(NULL, TEST_DEVICE, &lock) == ERR_NONE);
	test_result(lock != NULL);
	test_result(unlock_device(NULL, &lock) == true);

	unlink(TEST_LOCK);

	/* Existing PID, ASCII */
	create_lock(TEST_LOCK, pids, strlen(pids));
	test_result(lock_device(NULL, TEST_DEVICE, &lock) == ERR_DEVICELOCKED);
	test_result(lock == NULL);
	test_result(unlock_device(NULL, &lock) == false);

	unlink(TEST_LOCK);

	/* Existing PID, binary */
	create_lock(TEST_LOCK, &pid, sizeof(int));
	test_result(lock_device(NULL, TEST_DEVICE, &lock) == ERR_DEVICELOCKED);
	test_result(lock == NULL);
	test_result(unlock_device(NULL, &lock) == false);

	unlink(TEST_LOCK);

	/* Existing PID, binary */
	pid = 0xfffffff;
	create_lock(TEST_LOCK, &pid, sizeof(int));
	test_result(lock_device(NULL, TEST_DEVICE, &lock) == ERR_NONE);
	test_result(lock != NULL);
	test_result(unlock_device(NULL, &lock) == true);

	unlink(TEST_LOCK);

	/* No existing lock */
	test_result(lock_device(NULL, TEST_DEVICE, &lock) == ERR_NONE);
	test_result(lock != NULL);
	test_result(unlock_device(NULL, &lock) == true);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
