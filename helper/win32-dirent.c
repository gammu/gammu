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


#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <dirent.h>

DIR *
opendir ( const char *name )
{
  DIR *dir;

  dir = calloc (1, sizeof *dir + strlen (name));
  if (!dir)
    return NULL;
  strcpy (dir->dd_name, name);
  return dir;
}

int
closedir (DIR *dir)
{
  FindClose( (HANDLE)dir->dd_handle );
  free( dir );
  return 0;
}

struct dirent *
readdir( DIR *dir )
{
  WIN32_FIND_DATA fInfo;
  struct dirent *xdirent;
  int ret;

  if ( !dir->dd_handle )
    {
      char *dirname;

      if (*dir->dd_name)
        {
          size_t n = strlen (dir->dd_name);
          dirname = malloc (n + 4 + 1);
          if (dirname) {
            strcpy (dirname, dir->dd_name);
            strcpy (dirname + n, "\\*.*");
          }
        }
      else
        dirname = strdup( "\\*.*" );
      if (!dirname)
        return NULL; /* Error. */

      dir->dd_handle = (long)FindFirstFile( dirname, &fInfo );
      free( dirname );
      if ( !dir->dd_handle )
        ret = 0;
      else
        ret = 1;
    }
  else if ( dir->dd_handle != -1l )
    {
        ret = FindNextFile ((HANDLE)dir->dd_handle, &fInfo);
    }
  else
    ret = 0;
  if ( !ret )
    return NULL;

  xdirent = calloc ( 1, sizeof *xdirent);
  if (xdirent)
    {
      strncpy (xdirent->d_name, fInfo.cFileName, FILENAME_MAX -1 );
      xdirent->d_name[FILENAME_MAX-1] = 0;
      xdirent->d_namlen = strlen( xdirent->d_name );
    }
  return xdirent;
}

