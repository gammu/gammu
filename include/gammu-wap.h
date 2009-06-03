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
 * WAP bookmars and settings manipulations.
 */

#include <gammu-types.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>

#include <stdlib.h>		/* Needed for size_t declaration */

/**
 * WAP bookmark data.
 *
 * \ingroup WAP
 */
typedef struct {
	/**
	 * Location where it is stored.
	 */
	int Location;
	/**
	 * Bookmark URL.
	 */
	unsigned char Address[(255 + 1) * 2];
	/**
	 * Bookmark title.
	 */
	unsigned char Title[(50 + 1) * 2];
} GSM_WAPBookmark;

/* --------------------------- WAP or MMS settings ------------------------- */

/**
 * Connection speed configuration.
 *
 * \ingroup WAP
 */
typedef enum {
	WAPSETTINGS_SPEED_9600,
	WAPSETTINGS_SPEED_14400,
	WAPSETTINGS_SPEED_AUTO
} WAPSettings_Speed;

/**
 * Connection bearer configuration.
 *
 * \ingroup WAP
 */
typedef enum {
	WAPSETTINGS_BEARER_SMS = 1,
	WAPSETTINGS_BEARER_DATA,
	WAPSETTINGS_BEARER_USSD,
	WAPSETTINGS_BEARER_GPRS
} WAPSettings_Bearer;

/**
 * WAP setting.
 *
 * \ingroup WAP
 */
typedef struct {
	/**
	 * Settings name.
	 */
	char Title[(20 + 1) * 2];
	/**
	 * Home page.
	 */
	char HomePage[(100 + 1) * 2];
	/**
	 * Bearer of WAP connection.
	 */
	WAPSettings_Bearer Bearer;
	/**
	 * Secure connection?
	 */
	gboolean IsSecurity;
	/**
	 * Is this connectin continuous?
	 */
	gboolean IsContinuous;

	/**
	 * Whether is ISDN for data bearer
	 */
	gboolean IsISDNCall;
	/**
	 * Whether is normal auth for data bearer
	 */
	gboolean IsNormalAuthentication;

	/**
	 * Server for sms bearer.
	 */
	char Server[(21 + 1) * 2];

	/**
	 * Service for sms or ussd bearer.
	 */
	char Service[(20 + 1) * 2];
	/**
	 * Whether is IP, for sms or ussd bearer.
	 */
	gboolean IsIP;

	/**
	 * Code for ussd bearer.
	 */
	char Code[(10 + 1) * 2];

	/**
	 * IP address for data or gprs.
	 */
	char IPAddress[(20 + 1) * 2];
	/**
	 * Login for data or gprs.
	 */
	gboolean ManualLogin;
	/**
	 * Dial up number for data or gprs.
	 */
	char DialUp[(20 + 1) * 2];
	/**
	 * User name for data or gprs.
	 *
	 * \todo Is length okay?
	 */
	char User[(50 + 1) * 2];
	/**
	 * User password for data or gprs.
	 *
	 * \todo Is length okay?
	 */
	char Password[(50 + 1) * 2];
	/**
	 * Speed settings for data or gprs.
	 */
	WAPSettings_Speed Speed;
} GSM_WAPSettings;

/**
 * Set of WAP settings.
 *
 * \ingroup WAP
 */
typedef struct {
	/**
	 * Location.
	 */
	int Location;
	/**
	 * Number of elements in Settings.
	 */
	unsigned char Number;
	/**
	 * Real WAP settings.
	 */
	GSM_WAPSettings Settings[4];
	/**
	 * Whether this configuration is active.
	 */
	gboolean Active;
	/**
	 * Whether this configuration is read only.
	 */
	gboolean ReadOnly;
	/**
	 * Proxy server.
	 */
	char Proxy[(100 + 1) * 2];
	/**
	 * Proxy port.
	 */
	int ProxyPort;
	/**
	 * Second proxy server.
	 */
	char Proxy2[(100 + 1) * 2];
	/**
	 * Second proxy port.
	 */
	int Proxy2Port;
	/**
	 * Bearer of current connection.
	 */
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
GSM_Error GSM_EncodeURLFile(unsigned char *Buffer, size_t * Length,
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
