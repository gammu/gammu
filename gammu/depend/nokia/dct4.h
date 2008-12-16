/* (c) 2002-2004 by Marcin Wiacek */

void DCT4SetPhoneMenus	   (int argc, char *argv[]);
void DCT4SelfTests	   (int argc, char *argv[]);
void DCT4SetVibraLevel	   (int argc, char *argv[]);
void DCT4GetSecurityCode   (int argc, char *argv[]);
void DCT4GetVoiceRecord	   (int argc, char *argv[]);
void DCT4Info		   (int argc, char *argv[]);
void DCT4GetT9		   (int argc, char *argv[]);
void DCT4DisplayTest	   (int argc, char *argv[]);
void DCT4GetADC		   (int argc, char *argv[]);
void DCT4VibraTest	   (int argc, char *argv[]);
void DCT4PlaySavedRingtone (int argc, char *argv[]);
void DCT4MakeCameraShoot   (int argc, char *argv[]);
void DCT4GetScreenDump	   (int argc, char *argv[]);
void DCT4GetPBKFeatures	   (int argc, char *argv[]);

#ifdef DEBUG
	void DCT4ResetSecurityCode (int argc, char *argv[]);
#endif
#ifdef GSM_ENABLE_NOKIA6510
	void DCT4SetLight	   (int argc, char *argv[]);
	void DCT4TuneRadio	   (int argc, char *argv[]);
#endif

/* ------------------- features matrix ------------------------------------- */

typedef enum {
	DCT4_ALWAYS_ONLINE = 1,
	DCT4_GPRS_PCCH,
	DCT4_GEA1,
	DCT4_EOTD,
	DCT4_WAP_PUSH,
	DCT4_USE_PREF_SIM_NET,
	DCT4_JAVA_TCK,

	DCT4_ALS,
	DCT4_A52,
	DCT4_CSP,
	DCT4_EONS,
	DCT4_3GINDICATOR,
	DCT4_DISPLAY_PHONE_NAME,
	DCT4_DISPLAY_WAP_PROFILE,

	DCT4_GAMES_WAP_DOWNLOAD,
	DCT4_GAMES_SCORE_SEND,
	DCT4_GAMES_URL_CHECK,

	DCT4_BLUETOOTH_MENU,
	DCT4_WAP_BOOKMARKS_MENU,
	DCT4_WAP_BOOKMARKS_MENU2,
	DCT4_WAP_GOTO_MENU,
	DCT4_WAP_SETTINGS_MENU,
	DCT4_SERVICES_GAMES_APP_GALLERY,
	DCT4_JAVA_GAMES_MENU,
	DCT4_SAT_CONFIRM_MENU,
	DCT4_INSTANT_MESS_MENU,
	DCT4_INSTANT_MESS_MENU2,
	DCT4_CONFIRM_ALS,
	DCT4_BOOKMARK_GOTO_MENU,

	DCT4_5100_IDENTIFY,

	DCT4_TEST
} DCT4_Feature_Name;

typedef struct {
	DCT4_Feature_Name		Name;
	const char			*Text;
	struct {
		const unsigned char		Value;
		const char		*Text;
	} Values[10];
} DCT4_Feature;

typedef struct {
	const char				*Model;
	struct {
		DCT4_Feature_Name	Name;
		int			Number;
	} Features[31];
} DCT4_Phone_Features;

typedef struct {
	struct {
		char			Name[100];
		unsigned char		ID;
		unsigned char		Value;
		bool			Startup;
	} Tests[50];
	int				Num;
} DCT4_Phone_Tests;

/* ------------------------------------------------------------------------- */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
