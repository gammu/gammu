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
 * Reads manufacturer from phone.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetManufacturer(GSM_StateMachine *s);
/**
 * Reads model from phone.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetModel(GSM_StateMachine *s);
/**
 * Reads firmware information from phone.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetFirmware(GSM_StateMachine *s);
/**
 * Reads IMEI/serial number from phone.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetIMEI(GSM_StateMachine *s);
/**
 * Gets date and time from phone.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetOriginalIMEI(GSM_StateMachine *s, char *value);
/**
 * Gets month when device was manufactured.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetManufactureMonth(GSM_StateMachine *s, char *value);
/**
 * Gets product code of device.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetProductCode(GSM_StateMachine *s, char *value);
/**
 * Gets hardware information about device.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetHardware(GSM_StateMachine *s, char *value);
/**
 * Gets PPM (Post Programmable Memory) info from phone
 * (in other words for Nokia get, which language pack is in phone)
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetPPM(GSM_StateMachine *s, char *value);
/**
 * Gets SIM IMSI from phone.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetSIMIMSI(GSM_StateMachine *s, char *IMSI);
/**
 * Gets information about batery charge and phone charging state.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat);
/**
 * Reads signal quality (strength and error rate).
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig);
/**
 * Gets network information.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo);
/**
 * Acquired display status.
 *
 * \ingroup Info
 */
GSM_Error GAMMU_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features);
#endif
