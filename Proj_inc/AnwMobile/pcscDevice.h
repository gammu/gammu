#ifndef _PCSC_DEVICE_H_
#define _PCSC_DEVICE_H_

#define PCSC_MAXLEN_NAME			256
#define PCSC_MAXREADER				50

typedef struct{
	char* Name[PCSC_MAXREADER][PCSC_MAXLEN_NAME];
	int nCount;
} PCSC_List;


typedef struct{
	bool bIgnore;
	bool bChanged;
	bool bUnknown;
	bool bUnavailable;
	bool bEmpty;
	bool bPresent;
	bool bAtrMatch;
	bool bExclusive;
	bool bInUse;
	bool bMute;
	int nATR;
	char ATR[36];
} PCSC_ReaderStatus;


GSM_Error Pcsc_GSMError(LONG nError);

////////////////////////////////////////////////////////////////////////////
//
GSM_Error WINAPI Pcsc_Initialise();
GSM_Error WINAPI Pcsc_Terminate();
GSM_Error WINAPI Pcsc_ListReader(PCSC_List* pReader);
GSM_Error WINAPI Pcsc_ListCard(PCSC_List* pSCard);
GSM_Error WINAPI Pcsc_GetReaderStatus(LPCTSTR lpszReader, PCSC_ReaderStatus* pStatus);

#endif