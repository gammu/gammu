/**
 * \file gammu-wap.h
 * \author Michal Čihař
 * 
 * WAP data and functions.
 */
#ifndef __gammu_wap_h
#define __gammu_wap_h

/**
 * \defgroup WAP WAP
 * WAPs manipulations.
 */

/**
 * \ingroup WAP
 */
typedef struct {
	unsigned char Address[(255 + 1) * 2];
	unsigned char Title[(50 + 1) * 2];
	int Location;
} GSM_WAPBookmark;

/* --------------------------- WAP or MMS settings ------------------------- */

/**
 * \ingroup WAP
 */
typedef enum {
	WAPSETTINGS_SPEED_9600,
	WAPSETTINGS_SPEED_14400,
	WAPSETTINGS_SPEED_AUTO
} WAPSettings_Speed;

/**
 * \ingroup WAP
 */
typedef enum {
	WAPSETTINGS_BEARER_SMS = 1,
	WAPSETTINGS_BEARER_DATA,
	WAPSETTINGS_BEARER_USSD,
	WAPSETTINGS_BEARER_GPRS
} WAPSettings_Bearer;

/**
 * \ingroup WAP
 */
typedef struct {
	char Title[(20 + 1) * 2];
	char HomePage[(100 + 1) * 2];
	WAPSettings_Bearer Bearer;
	bool IsSecurity;
	bool IsContinuous;

	/* for data bearer */
	bool IsISDNCall;
	bool IsNormalAuthentication;

	/* for sms bearer */
	char Server[(21 + 1) * 2];

	/* for sms or ussd bearer */
	char Service[(20 + 1) * 2];
	bool IsIP;

	/* for ussd bearer */
	char Code[(10 + 1) * 2];

	/* for data or gprs */
	char IPAddress[(20 + 1) * 2];
	bool ManualLogin;
	char DialUp[(20 + 1) * 2];
	char User[(50 + 1) * 2];	/*is length OK ? */
	char Password[(50 + 1) * 2];	/*is length OK ? */
	WAPSettings_Speed Speed;
} GSM_WAPSettings;

/**
 * \ingroup WAP
 */
typedef struct {
	int Location;
	unsigned char Number;
	GSM_WAPSettings Settings[4];
	bool Active;
	bool ReadOnly;
	char Proxy[(100 + 1) * 2];
	int ProxyPort;
	char Proxy2[(100 + 1) * 2];
	int Proxy2Port;

	WAPSettings_Bearer ActiveBearer;
} GSM_MultiWAPSettings;

/**
 * Encodes URL to VBKM file.
 *
 * \param Buffer Storage for text.
 * \param Length Pointer to storage, will be updated.
 * \param bookmark Bookmark to encode.
 *
 * \return Error code.
 *
 * \ingroup WAP
 */
GSM_Error GSM_EncodeURLFile(unsigned char *Buffer, int *Length,
			    GSM_WAPBookmark * bookmark);

/**
 * Reads WAP bookmark.
 *
 * \param s State machine pointer.
 * \param bookmark Bookmark storage, need to contain location.
 *
 * \return Error code
 *
 * \ingroup WAP
 */
GSM_Error GSM_GetWAPBookmark(GSM_StateMachine * s, GSM_WAPBookmark * bookmark);

/**
 * Sets WAP bookmark.
 *
 * \param s State machine pointer.
 * \param bookmark Bookmark data.
 *
 * \return Error code
 *
 * \ingroup WAP
 */
GSM_Error GSM_SetWAPBookmark(GSM_StateMachine * s, GSM_WAPBookmark * bookmark);

/**
 * Deletes WAP bookmark.
 *
 * \param s State machine pointer.
 * \param bookmark Bookmark data, need to contain location.
 *
 * \return Error code
 *
 * \ingroup WAP
 */
GSM_Error GSM_DeleteWAPBookmark(GSM_StateMachine * s,
				GSM_WAPBookmark * bookmark);

/**
 * Acquires WAP settings.
 *
 * \param s State machine pointer.
 * \param settings Settings storage.
 *
 * \return Error code
 *
 * \ingroup WAP
 */
GSM_Error GSM_GetWAPSettings(GSM_StateMachine * s,
			     GSM_MultiWAPSettings * settings);

/**
 * Changes WAP settings.
 *
 * \param s State machine pointer.
 * \param settings Settings data.
 *
 * \return Error code
 *
 * \ingroup WAP
 */
GSM_Error GSM_SetWAPSettings(GSM_StateMachine * s,
			     GSM_MultiWAPSettings * settings);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
