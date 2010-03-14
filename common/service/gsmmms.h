
#ifndef __gsm_mms_h
#define __gsm_mms_h

typedef struct {
	unsigned char	Address[500];
	unsigned char	Title[200];
	unsigned char	Sender[200];
} GSM_MMSIndicator;

void GSM_EncodeMMSIndicatorSMSText(char *Buffer, int *Length, GSM_MMSIndicator Indicator);

#endif	/* __gsm_mms_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
