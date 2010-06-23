#ifndef __gsm_wap_h
#define __gsm_wap_h

#include "../misc/misc.h"

typedef struct {
	char	Address		[(255+1)*2];
	char	Title		[(50+1)*2];
	int	Location;
} GSM_WAPBookmark;

typedef enum {
	WAPSETTINGS_SPEED_9600,
	WAPSETTINGS_SPEED_14400,
	WAPSETTINGS_SPEED_AUTO,
} WAPSettings_Speed;

typedef enum {
	WAPSETTINGS_BEARER_SMS = 1,
	WAPSETTINGS_BEARER_DATA,
	WAPSETTINGS_BEARER_USSD,
	WAPSETTINGS_BEARER_GPRS
} WAPSettings_Bearer;

typedef struct {
	char			Title		[(20+1)*2];
	char			HomePage	[(100+1)*2];
	WAPSettings_Bearer	Bearer;
	bool			IsSecurity;
	bool			IsContinuous;

  	/* for data bearer */
	char			IPAddress	[(20+1)*2];
  	bool			IsISDNCall;
	bool			IsNormalAuthentication;

	/* for sms bearer */
	char			Server		[(21+1)*2];

	/* for sms or ussd bearer */
	char			Service		[(20+1)*2];
	bool			IsIP;  

	/* for ussd bearer */
	char			Code		[(10+1)*2];

	/* for data or gprs */
	bool			ManualLogin;
	char			DialUp		[(20+1)*2];
	char			User		[(50+1)*2]; /*is length OK ?*/
	char			Password	[(50+1)*2]; /*is length OK ?*/
	WAPSettings_Speed	Speed;
} GSM_WAPSettings;

typedef struct {
	int			Location;
	unsigned char		Number;
	GSM_WAPSettings		Settings[4];
} GSM_MultiWAPSettings;

void NOKIA_EncodeWAPBookmarkSMSText(char *Buffer, int *Length, GSM_WAPBookmark bookmark);
void NOKIA_EncodeWAPSettingsSMSText(char *Buffer, int *Length, GSM_WAPSettings settings);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
