
#ifndef obexgen_h
#define obexgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmsms.h"

#ifndef GSM_USED_IRDAOBEX
#  define GSM_USED_IRDAOBEX
#endif
#ifndef GSM_USED_BLUEOBEX
#  define GSM_USED_BLUEOBEX
#endif

typedef struct {
	int				FileLev;

	int				FilesLocations[500];
	int				FilesParents[500];
	int				FilesLevels[500];
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;
	char				FilesNames[500][200];
	bool				FilesFolder[500];
	int				FilesSizes[500];
} GSM_Phone_OBEXGENData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
