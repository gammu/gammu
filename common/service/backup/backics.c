/* (c) 2003 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/coding/coding.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backics.h"

#ifdef GSM_ENABLE_BACKUP

GSM_Error SaveICS(char *FileName, GSM_Backup *backup)
{
	FILE 		*file;
 
	file = fopen(FileName, "wb");      
	if (file == NULL) return ERR_CANTOPENFILE;

	fclose(file);
	return ERR_NONE;
}

GSM_Error LoadICS(char *FileName, GSM_Backup *backup)
{
	GSM_File 		File;
	GSM_Error		error;
	
	File.Buffer = NULL;
	error = GSM_ReadFile(FileName, &File);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
