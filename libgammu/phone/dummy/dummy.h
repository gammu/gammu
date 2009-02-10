/* Copyright (c) 2009 by Michal Čihař */

/**
 * \file dummy.h
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @addtogroup DummyPhone
 * @{
 */
#ifndef __phone_dummy_h__
#define __phone_dummy_h__

#include <stdio.h>
#include <limits.h>
#ifdef WIN32
#include "../../../helper/win32-dirent.h"
#else
#include <dirent.h>
#endif

/* Win32 compatibility */
#ifndef PATH_MAX
#define PATH_MAX (MAX_PATH)
#endif

#define DUMMY_MAX_LOCATION (10000)
#define DUMMY_MAX_SMS (10000)
#define DUMMY_MAX_MEM (10000)
#define DUMMY_MAX_TODO (10000)
#define DUMMY_MAX_FS_DEPTH (20)

typedef struct {
	FILE *log_file;
	GSM_SMSC SMSC;
	DIR *dir[DUMMY_MAX_FS_DEPTH + 1];
	char dirnames[DUMMY_MAX_FS_DEPTH + 1][PATH_MAX];
	int fs_depth;
	size_t devlen;
} GSM_Phone_DUMMYData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
