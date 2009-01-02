/* (c) 2002-2006 by Marcin Wiacek */

#ifndef __gsm_multi_h
#define __gsm_multi_h

#if defined(_MSC_VER) && defined(__cplusplus)
    extern "C" {
#endif

#include <gammu-message.h>

/* ---------------------- multi SMS --------------------------------------- */

/* Identifiers for Smart Messaging 3.0 multipart SMS */

#define SM30_ISOTEXT      0 /* ISO 8859-1 text */
#define SM30_UNICODETEXT  1
#define SM30_OTA	  2
#define SM30_RINGTONE     3
#define SM30_PROFILENAME  4
/* ... */
#define SM30_SCREENSAVER  6

/* Identifiers for Alcatel Terminal Data Download */
#define ALCATELTDD_PICTURE     4
#define ALCATELTDD_ANIMATION   5
#define ALCATELTDD_SMSTEMPLATE 6

void GSM_SMSCounter(GSM_Debug_Info *di, size_t	 	MessageLength,
		    unsigned char       *MessageBuffer,
		    GSM_UDH	     	UDHType,
		    GSM_Coding_Type     Coding,
		    int		 	*SMSNum,
		    size_t	 	*CharsLeft);

GSM_Error GSM_AddSMS_Text_UDH(GSM_Debug_Info *di, GSM_MultiSMSMessage       *SMS,
				GSM_Coding_Type	 	Coding,
				char		    	*Buffer,
				size_t		     	BufferLen,
				bool		    	UDH,
				size_t		     	*UsedText,
				size_t		     	*CopiedText,
				size_t		     	*CopiedSMSText);

void GSM_MakeMultiPartSMS(GSM_Debug_Info *di, GSM_MultiSMSMessage   *SMS,
			  unsigned char	 	*MessageBuffer,
			  size_t	   	MessageLength,
			  GSM_UDH	       	UDHType,
			  GSM_Coding_Type       Coding,
			  int		   	Class,
			  unsigned char	 	RejectDuplicates);

void GSM_Find_Free_Used_SMS2(GSM_Debug_Info *di, GSM_Coding_Type Coding,GSM_SMSMessage SMS, size_t *UsedText, size_t *FreeText, size_t *FreeBytes);

unsigned char GSM_MakeSMSIDFromTime(void);


#if defined(_MSC_VER) && defined(__cplusplus)
    }
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
