
#ifndef DJGPP
#ifndef unixirda_h
#define unixirda_h

#ifndef WIN32
#  include "unix.h"
#else
#  define _WIN32_WINNT
#  include "win32.h"
#endif

typedef struct {
    int 			hPhone;
    struct sockaddr_irda	peer;
} GSM_Device_IrdaData;

#endif
#endif
