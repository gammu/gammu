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

typedef struct {
	int			Location;
	bool			Active;
	bool			SyncPhonebook;
	bool			SyncCalendar;
	char			Name[(20+1)*2];
	char			PhonebookDataBase[(50+1)*2];
	char			CalendarDataBase[(50+1)*2];
	char			User[(30+1)*2];
	char			Password[(20+1)*2];
	char			Server[(128+1)*2];
	GSM_MultiWAPSettings	Connection;
} GSM_SyncMLSettings;

/* ------------------------------------------------------------------------ */

typedef struct {
	char			Name[(50+1)*2];
	char			HomePage[(200+1)*2];
	char			User[(50+1)*2];
	char			Password[(50+1)*2];
	int			Location;
	bool			Active;
	GSM_MultiWAPSettings	Connection;
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
	bool			Active;

	/**
	 * Profile number
	 */
	int 			Location;
	/**
 	 * Profile name
	 */
	char 			Name[40*2];
	/**
	 * Is it default name for profile ?
	 */
	bool 			DefaultName;
	bool			HeadSetProfile;
	bool			CarKitProfile;

	int			FeaturesNumber;
	GSM_Profile_Feat_Value	FeatureValue[15];
	GSM_Profile_Feat_ID	FeatureID[15];

	bool			CallerGroups[5];
} GSM_Profile;

#define	GSM_MAX_FMSTATION_LENGTH 18
#define	GSM_MAX_FM_STATION 	 20

typedef struct {
	int			Location;
        char 			StationName [(GSM_MAX_FMSTATION_LENGTH+1)*2];
	double			Frequency;
} GSM_FMStation;

typedef struct {
	int		Location;
	unsigned char 	Name[300];
	unsigned char 	URL[500];
	bool		Active;
} GSM_GPRSAccessPoint;
#endif
