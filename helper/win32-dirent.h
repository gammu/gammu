/* w32_dirent.c  - Posix emulation layer for Sylpheed (Claws)
 *
 * This file is part of w32lib.
 *
 * w32lib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * w32lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * For more information and a list of changes, see w32lib.h
 */

#ifndef __helper__win32_dirent_h_
#define __helper__win32_dirent_h_

#ifdef __MINGW32__
#include <dirent.h>
#else

#include <stdio.h>

/* For struct _finddata_t */
#include <io.h>

struct dirent {
	long d_ino;
	unsigned short d_reclen;
	unsigned short d_namlen;
	char d_name[FILENAME_MAX];
};

typedef struct
{
	struct _finddata_t	dd_dta;
	struct dirent		dd_dir;
	long			dd_handle;
	int			dd_stat;
	char			dd_name[1];
} DIR;

DIR *opendir( const char *name );
int closedir( DIR *dir );
struct dirent *readdir( DIR *dir );

#endif
#endif
