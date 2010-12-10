/**
 * \file gammu-settings.h
 * \author Michal Čihař
 *
 * Phone settings data and functions.
 */
#ifndef __gammu_settings_h
#define __gammu_settings_h

/**
 * \defgroup Settings Settings
 * Phone settings manipulations.
 */

#include <gammu-wap.h>
#include <gammu-limits.h>

typedef struct {
	int Location;
	gboolean Active;
	gboolean SyncPhonebook;
	gboolean SyncCalendar;
	char Name[(20 + 1) * 2];
	char PhonebookDataBase[(50 + 1) * 2];
	char CalendarDataBase[(50 + 1) * 2];
	char User[(30 + 1) * 2];
	char Password[(20 + 1) * 2];
	char Server[(128 + 1) * 2];
	GSM_MultiWAPSettings Connection;
} GSM_SyncMLSettings;

/* ------------------------------------------------------------------------ */

typedef enum {
	GSM_RESET_PHONESETTINGS = 1,
	GSM_RESET_USERINTERFACE,
	GSM_RESET_USERINTERFACE_PHONESETTINGS,
	GSM_RESET_DEVICE,
	GSM_RESET_FULLFACTORY
} GSM_ResetSettingsType;

typedef struct {
	char Name[(50 + 1) * 2];
	char HomePage[(200 + 1) * 2];
	char User[(50 + 1) * 2];
	char Password[(50 + 1) * 2];
	int Location;
	gboolean Active;
	GSM_MultiWAPSettings Connection;
} GSM_ChatSettings;

typedef enum {
	PROFILE_KEYPAD_LEVEL1 = 1,
	PROFILE_KEYPAD_LEVEL2,
	PROFILE_KEYPAD_LEVEL3,
	PROFILE_KEYPAD_OFF,
	PROFILE_CALLALERT_RINGING,
	PROFILE_CALLALERT_BEEPONCE,
	PROFILE_CALLALERT_OFF,
	PROFILE_CALLALERT_RINGONCE,
	PROFILE_CALLALERT_ASCENDING,
	PROFILE_CALLALERT_CALLERGROUPS,
	PROFILE_VOLUME_LEVEL1,
	PROFILE_VOLUME_LEVEL2,
	PROFILE_VOLUME_LEVEL3,
	PROFILE_VOLUME_LEVEL4,
	PROFILE_VOLUME_LEVEL5,
	PROFILE_MESSAGE_NOTONE,
	PROFILE_MESSAGE_STANDARD,
	PROFILE_MESSAGE_SPECIAL,
	PROFILE_MESSAGE_BEEPONCE,
	PROFILE_MESSAGE_ASCENDING,
	PROFILE_MESSAGE_PERSONAL,
	PROFILE_VIBRATION_OFF,
	PROFILE_VIBRATION_ON,
	PROFILE_VIBRATION_FIRST,
	PROFILE_WARNING_ON,
	PROFILE_WARNING_OFF,
	PROFILE_AUTOANSWER_ON,
	PROFILE_AUTOANSWER_OFF,
	PROFILE_LIGHTS_OFF,
	PROFILE_LIGHTS_AUTO,
	PROFILE_SAVER_ON,
	PROFILE_SAVER_OFF,
	PROFILE_SAVER_TIMEOUT_5SEC,
	PROFILE_SAVER_TIMEOUT_20SEC,
	PROFILE_SAVER_TIMEOUT_1MIN,
	PROFILE_SAVER_TIMEOUT_2MIN,
	PROFILE_SAVER_TIMEOUT_5MIN,
	PROFILE_SAVER_TIMEOUT_10MIN
} GSM_Profile_Feat_Value;

typedef enum {
	Profile_KeypadTone = 1,
	Profile_CallAlert,
	Profile_RingtoneVolume,
	Profile_MessageTone,
	Profile_Vibration,
	Profile_WarningTone,
	Profile_AutoAnswer,
	Profile_Lights,
	Profile_ScreenSaverTime,
	Profile_ScreenSaver,

	Profile_ScreenSaverNumber,
	Profile_RingtoneID,
	Profile_MessageToneID,
	Profile_CallerGroups
} GSM_Profile_Feat_ID;

/**
 * It contains phone profiles
 */
typedef struct {
	gboolean Active;

	/**
	 * Profile number
	 */
	int Location;
	/**
 	 * Profile name
	 */
	char Name[40 * 2];
	/**
	 * Is it default name for profile ?
	 */
	gboolean DefaultName;
	gboolean HeadSetProfile;
	gboolean CarKitProfile;

	int FeaturesNumber;
	GSM_Profile_Feat_Value FeatureValue[15];
	GSM_Profile_Feat_ID FeatureID[15];

	gboolean CallerGroups[5];
} GSM_Profile;

typedef struct {
	int Location;
	double Frequency;
	char StationName[(GSM_MAX_FMSTATION_LENGTH + 1) * 2];
} GSM_FMStation;

typedef struct {
	int Location;
	gboolean Active;
	unsigned char Name[(GSM_MAX_GPRS_AP_NAME_LENGTH + 1) * 2];
	unsigned char URL[(GSM_MAX_GPRS_AP_URL_LENGTH + 1) * 2];
} GSM_GPRSAccessPoint;

typedef enum {
	GSM_Date_DDMMYYYY = 1,
	GSM_Date_MMDDYYYY,
	GSM_Date_YYYYMMDD,
	GSM_Date_DDMMMYY,
	GSM_Date_MMDDYY,
	GSM_Date_DDMMYY,
	GSM_Date_YYMMDD,
	GSM_Date_OFF
} GSM_DateFormat;

typedef struct {
	char DateSeparator;
	GSM_DateFormat DateFormat;
	gboolean AMPMTime;
} GSM_Locale;

typedef struct {
	GSM_Profile_Feat_ID ID;
	GSM_Profile_Feat_Value Value;
	unsigned char PhoneID;
	unsigned char PhoneValue;
} GSM_Profile_PhoneTableValue;

/**
 * Gets locale from phone.
 */
GSM_Error GSM_GetLocale(GSM_StateMachine * s, GSM_Locale * locale);

/**
 * Sets locale of phone.
 */
GSM_Error GSM_SetLocale(GSM_StateMachine * s, GSM_Locale * locale);

/**
 * Acquires SyncML settings.
 */
GSM_Error GSM_GetSyncMLSettings(GSM_StateMachine * s,
				GSM_SyncMLSettings * settings);
/**
 * Changes SyncML settings.
 */
GSM_Error GSM_SetSyncMLSettings(GSM_StateMachine * s,
				GSM_SyncMLSettings * settings);
/**
 * Acquires chat/presence settings.
 */
GSM_Error GSM_GetChatSettings(GSM_StateMachine * s,
			      GSM_ChatSettings * settings);
/**
 * Changes chat/presence settings.
 */
GSM_Error GSM_SetChatSettings(GSM_StateMachine * s,
			      GSM_ChatSettings * settings);
/**
 * Acquires MMS settings.
 */
GSM_Error GSM_GetMMSSettings(GSM_StateMachine * s,
			     GSM_MultiWAPSettings * settings);
/**
 * Changes MMS settings.
 */
GSM_Error GSM_SetMMSSettings(GSM_StateMachine * s,
			     GSM_MultiWAPSettings * settings);
/**
 * Enables network auto login.
 */
GSM_Error GSM_SetAutoNetworkLogin(GSM_StateMachine * s);

/**
 * Performs phone reset.
 */
GSM_Error GSM_Reset(GSM_StateMachine * s, gboolean hard);

/**
 * Resets phone settings.
 */
GSM_Error GSM_ResetPhoneSettings(GSM_StateMachine * s,
				 GSM_ResetSettingsType Type);
/**
 * Reads profile.
 */
GSM_Error GSM_GetProfile(GSM_StateMachine * s, GSM_Profile * Profile);

/**
 * Updates profile.
 */
GSM_Error GSM_SetProfile(GSM_StateMachine * s, GSM_Profile * Profile);

/**
 * Reads FM station.
 */
GSM_Error GSM_GetFMStation(GSM_StateMachine * s, GSM_FMStation * FMStation);

/**
 * Sets FM station.
 */
GSM_Error GSM_SetFMStation(GSM_StateMachine * s, GSM_FMStation * FMStation);

/**
 * Clears defined FM stations.
 */
GSM_Error GSM_ClearFMStations(GSM_StateMachine * s);

/**
 * Gets GPRS access point.
 */
GSM_Error GSM_GetGPRSAccessPoint(GSM_StateMachine * s,
				 GSM_GPRSAccessPoint * point);
/**
 * Sets GPRS access point.
 */
GSM_Error GSM_SetGPRSAccessPoint(GSM_StateMachine * s,
				 GSM_GPRSAccessPoint * point);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
