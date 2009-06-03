/* (c) 2001-2004 by Marcin Wiacek */

#ifndef __gsm_wap_h
#define __gsm_wap_h

#include <gammu-wap.h>
#include <gammu-message.h>

void NOKIA_EncodeWAPMMSSettingsSMSText(unsigned char *Buffer, size_t *Length, GSM_WAPSettings *settings, gboolean MMS);

/* -------------------------------- WAP Bookmark --------------------------- */

void 		NOKIA_EncodeWAPBookmarkSMSText	(unsigned char *Buffer, size_t *Length, GSM_WAPBookmark *bookmark);

/* ------------------------------ MMS Indicator ---------------------------- */

void GSM_EncodeMMSIndicatorSMSText(unsigned char *Buffer, size_t *Length, GSM_MMSIndicator Indicator);

void GSM_EncodeWAPIndicatorSMSText(unsigned char *Buffer, size_t *Length, char *Text, char *URL);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
