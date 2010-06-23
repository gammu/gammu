/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for miscellaneous defines, typedefs etc.

*/

#ifndef __misc_win32_h
#define __misc_win32_h    

#include <windows.h>

#define sleep(x) Sleep((x) * 1000)
#define usleep(x) Sleep(((x) < 1000) ? 1 : ((x) / 1000))

#endif