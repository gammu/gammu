/**
 * \info gammu-info.h
 * \author Michal Čihař
 * 
 * Phone information.
 */
#ifndef __gammu_info_h
#define __gammu_info_h

/**
 * \defgroup Info Info
 * Phone information.
 */


/**
 * Find network name from given network code.
 *
 * \ingroup Info
 */
char *GSM_GetNetworkName(char *NetworkCode);

/**
 * Find country name from given country code.
 *
 * \ingroup Info
 */
char *GSM_GetCountryName(char *CountryCode);

/**
 * Status of network logging
 *
 * \ingroup Info
 */
typedef enum {
	/**
	 * Home network for used SIM card.
	 */
	GSM_HomeNetwork = 1,
	/**
	 * No network available for used SIM card.
	 */
	GSM_NoNetwork,
	/**
	 * SIM card uses roaming.
	 */
	GSM_RoamingNetwork,
	/**
	 * Network registration denied - card blocked or expired or disabled.
	 */
	GSM_RegistrationDenied,
	/**
	 * Unknown network status.
	 */
	GSM_NetworkStatusUnknown,
	/**
	 * Network explicitely requested by user.
	 */
	GSM_RequestingNetwork
} GSM_NetworkInfo_State;

/**
 * Structure for getting the current network info.
 *
 * \ingroup Info
 */
typedef struct {
	/**
	 * Cell ID (CID)
	 */
	unsigned char		   CID[10];
	/**
	 * GSM network code.
	 */
	char			   NetworkCode[10];
	/**
	 * Status of network logging. If phone is not logged into any network,
         * some values are not filled
	 */
	GSM_NetworkInfo_State	   State;
	/**
	 * LAC (Local Area Code).
	 */
	unsigned char		   LAC[10];
	/**
	 * Name of current network like returned from phone (or empty).
	 */
	unsigned char		   NetworkName[15*2];
} GSM_NetworkInfo;

/**
 * Information about signal quality, all these should be -1 when unknown.
 *
 * \ingroup Info
 */
typedef struct {
	/*
	 * Signal strength in dBm
	 */
	int     SignalStrength;
	/**
	 * Signal strength in percent.
	 */
	int     SignalPercent;
	/**
	 * Bit error rate in percent.
	 */
	int     BitErrorRate;
} GSM_SignalQuality;

/**
 * Power source
 *
 * \ingroup Info
 */
typedef enum {
	GSM_BatteryPowered = 1, /**< Powered from battery */
	GSM_BatteryConnected, /**< Powered from AC, battery connected */
	GSM_BatteryCharging, /**< Powered from AC, battery is charging */
	GSM_BatteryNotConnected, /**< Powered from AC, no battery */
	GSM_BatteryFull, /**< Powered from AC, battery is fully charged */
	GSM_PowerFault, /**< Power failure */
} GSM_ChargeState;

/**
 * Power source
 *
 * \ingroup Info
 */
typedef enum {
	GSM_BatteryUnknown = 0, /**< Unknown battery */
	GSM_BatteryNiMH = 1, /**< NiMH battery */
	GSM_BatteryLiIon, /**< Lithium Ion battery */
	GSM_BatteryLiPol, /**< Lithium Polymer battery */
} GSM_BatteryType;

/**
 * Battery status
 *
 * \ingroup Info
 */
typedef struct {
	/**
	 * Signal strength in percent, -1 = unknown
	 */
	int BatteryPercent;
	/**
	 * Charge state
	 */
	GSM_ChargeState ChargeState;
	/**
	 * Current battery voltage (in mV).
	 */
	int BatteryVoltage;
	/**
	 * Voltage from charger (in mV)
	 */
	int ChargeVoltage;
	/**
	 * Current from charger (in mA)
	 */
	int ChargeCurrent;
	/**
	 * Phone current consumption (in mA)
	 */
	int PhoneCurrent;
	/**
	 * Battery temperature (in degrees Celsius)
	 */
	int BatteryTemperature;
	/**
	 * Phone temperature (in degrees Celsius)
	 */
	int PhoneTemperature;
	/**
	 * Remaining battery capacity (in mAh)
	 */
	int BatteryCapacity;
	/**
	 * Battery type
	 */
	GSM_BatteryType BatteryType;
} GSM_BatteryCharge;

/**
 * Display feature
 *
 * \ingroup Info
 */
typedef enum {
	GSM_CallActive = 1,
	/**
	 * blinking envelope
	 */
	GSM_SMSMemoryFull,
	GSM_FaxCall,
	GSM_UnreadSMS,
	GSM_DataCall,
	GSM_VoiceCall,
	GSM_KeypadLocked
} GSM_DisplayFeature;

/**
 * Display features
 *
 * \ingroup Info
 */
typedef struct {
	int 			Number;
	GSM_DisplayFeature 	Feature[7];
} GSM_DisplayFeatures;

/**
 * Phone features definition. This is usually used for things, which can not
 * be determined on run time.
 *
 * \ingroup Info
 */
typedef enum {
	/* n6110.c */
	F_CAL33 = 1,	 /**< Calendar,3310 style - 10 reminders, Unicode, 3 coding types */
	F_CAL52,	 /**< Calendar,5210 style - full Unicode, etc. */
	F_CAL82,	 /**< Calendar,8250 style - "normal", but with Unicode */
	F_RING_SM,	 /**< Ringtones returned in SM format - 33xx */
	F_NORING,	 /**< No ringtones */
	F_NOPBKUNICODE,	 /**< No phonebook in Unicode */
	F_NOWAP,      	 /**< No WAP */
	F_NOCALLER,	 /**< No caller groups */
	F_NOPICTURE,	 /**< No Picture Images */
	F_NOPICTUREUNI,	 /**< No Picture Images text in Unicode */
	F_NOSTARTUP,	 /**< No startup logo */
	F_NOCALENDAR,	 /**< No calendar */
	F_NOSTARTANI,	 /**< Startup logo is not animated */
	F_POWER_BATT,	 /**< Network and battery level get from netmonitor */
	F_PROFILES33,	 /**< Phone profiles in 3310 style */
	F_PROFILES51,	 /**< Phone profiles in 5110 style */
	F_MAGICBYTES,	 /**< Phone can make authentication with magic bytes */
	F_NODTMF,	 /**< Phone can't send DTMF */
	F_DISPSTATUS,	 /**< Phone return display status */
	F_NOCALLINFO,	 /**< Phone does not return call info */

	/* n3320.c */
	F_DAYMONTH,	 /**< Day and month reversed in pbk, when compare to GSM models */

	/* n6510.c */
	F_PBK35,	 /**< Phonebook in 3510 style with ringtones ID */
	F_PBKIMG,	 /**< Phonebook in 7250 style with picture ID */
	F_PBKTONEGAL,	 /**< Phonebook with selecting ringtones from gallery */
	F_PBKSMSLIST,	 /**< Phonebook with SMS list */
	F_PBKUSER,	 /**< Phonebook with user ID */
	F_6230iCALLER,	 /**< Caller groups like in 6230i */
	F_RADIO,	 /**< Phone with FM radio */
	F_TODO63,	 /**< ToDo in 6310 style - 0x55 msg type */
	F_TODO66,	 /**< ToDo in 6610 style - like calendar, with date and other */
	F_NOMIDI,	 /**< No ringtones in MIDI */
	F_BLUETOOTH,	 /**< Bluetooth support */
	F_NOFILESYSTEM,	 /**< No images, ringtones, java saved in special filesystem */
	F_NOMMS,	 /**< No MMS sets in phone */
	F_NOGPRSPOINT,	 /**< GPRS point are not useable */
	F_CAL35,	 /**< Calendar,3510 style - Reminder,Call,Birthday */
	F_CAL65,	 /**< Calendar,6510 style - CBMM, method 3 */
	F_WAPMMSPROXY,	 /**< WAP & MMS settings contains first & second proxy */
	F_CHAT,		 /**< Phone with Chat settings */
	F_SYNCML,	 /**< Phone with SyncML settings */
	F_FILES2,	 /**< Filesystem version 2 */
	F_NOFILE1,	 /**< No filesystem version 1 */
	F_6230iWAP,	 /**< WAP, MMS, etc. settings like in 6230i - unknown now */
	F_PROFILES,	 /**< Profiles support available */
	F_SERIES40_30,   /**< Series 40 3.0 */
	F_SMS_FILES,  	 /**< SMS are read from filesystem files like in Series 40 3.0 */
	F_3220_MMS,	 /**< MMS storage as in 3320 */

	/* n6510.c && n7110.c */
	F_VOICETAGS,	 /**< Voice tags available */
	F_CAL62,	 /**< Calendar,6210 style - Call,Birthday,Memo,Meeting */
	F_NOTES,	 /**< Notes supported */

	/* AT modules */
	F_SMSONLYSENT,	 /**< Phone supports only sent/unsent messages */
	F_BROKENCPBS, 	 /**< CPBS on some memories can hang phone */
	F_M20SMS,	 /**< Siemens M20 like SMS handling */
	F_SLOWWRITE,	 /**< Use slower writing which some phone need */
	F_SMSME900,	 /**< SMS in ME start from location 900 - case of Sagem */
	F_ALCATEL,	 /**< Phone supports Alcatel protocol */
	F_OBEX,		 /**< Phone can switch to OBEX protocol from AT mode */
	F_IRMC_LEVEL_2,	 /**< Phone supports IrMC level 2 even if it doesn't report it */
	F_MODE22,	 /**< Switching to OBEX mode is done using AT+MODE=22 */
	F_SMS_LOCATION_0, /**< Locations of SMS memories start from 0 */
} Feature;

/**
 * Model identification, used for finding phone features.
 *
 * \ingroup Info
 */
typedef struct {
	char		*model; /**< Model as returned by phone */
	char		*number; /**< Identification by Gammu */
	char		*irdamodel; /**< Model as used over IrDA */
	Feature		features[14]; /**< List of supported features */
} OnePhoneModel;

/**
 * Reads manufacturer from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetManufacturer(GSM_StateMachine *s, char *value);
/**
 * Reads model from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetModel(GSM_StateMachine *s, char *value);
/**
 * Reads model info from state machine.
 *
 * \ingroup Info
 */
OnePhoneModel *GSM_GetModelInfo(GSM_StateMachine *s);
/**
 * Reads firmware information from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetFirmware(GSM_StateMachine *s, char *value, char *date, double *num);
/**
 * Reads IMEI/serial number from phone.
 *
 * \param value Pointer where to store IMEI, NULL to ignore.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetIMEI(GSM_StateMachine *s, char *value);
/**
 * Gets date and time from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetOriginalIMEI(GSM_StateMachine *s, char *value);
/**
 * Gets month when device was manufactured.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetManufactureMonth(GSM_StateMachine *s, char *value);
/**
 * Gets product code of device.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetProductCode(GSM_StateMachine *s, char *value);
/**
 * Gets hardware information about device.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetHardware(GSM_StateMachine *s, char *value);
/**
 * Gets PPM (Post Programmable Memory) info from phone
 * (in other words for Nokia get, which language pack is in phone)
 *
 * \ingroup Info
 */
GSM_Error GSM_GetPPM(GSM_StateMachine *s, char *value);
/**
 * Gets SIM IMSI from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetSIMIMSI(GSM_StateMachine *s, char *IMSI);
/**
 * Gets information about batery charge and phone charging state.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat);
/**
 * Reads signal quality (strength and error rate).
 *
 * \ingroup Info
 */
GSM_Error GSM_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig);
/**
 * Gets network information.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo);
/**
 * Acquired display status.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features);
#endif
