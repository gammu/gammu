/* (c) 2002-2003 by Marcin Wiacek */

#ifndef __gsm_ems_h
#define __gsm_ems_h

#include "gsmmulti.h"

GSM_Error GSM_EncodeEMSMultiPartSMS(GSM_Debug_Info *di,
				    GSM_MultiPartSMSInfo 	*Info,
				    GSM_MultiSMSMessage 	*SMS,
				    GSM_UDH			UDHType);

gboolean GSM_DecodeEMSMultiPartSMS(GSM_Debug_Info *di,
			       GSM_MultiPartSMSInfo 		*Info,
			       GSM_MultiSMSMessage 		*SMS);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
