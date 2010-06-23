
#ifndef DJGPP
#ifndef unixirda_h
#define unixirda_h

#ifndef WIN32
#  include "irda_unx.h"
#else
#  include "irda_w32.h"
#endif

typedef struct {
	int 			hPhone;
	struct sockaddr_irda	peer;
} GSM_Device_IrdaData;

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
