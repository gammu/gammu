/**
 * \file gammu-info.h
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

#include <gammu-types.h>
#include <gammu-error.h>
#include <gammu-limits.h>

/**
 * Find network name from given network code.
 *
 * \ingroup Info
 */
const unsigned char *GSM_GetNetworkName(const char *NetworkCode);

/**
 * Find country name from given country code.
 *
 * \ingroup Info
 */
const unsigned char *GSM_GetCountryName(const char *CountryCode);

/**
 * Structure for defining code-name mappings.
 *
 * \ingroup Info
 */
typedef struct {
	const char Code[8];
	const char Name[64];
} GSM_CodeName;

/**
 * List of network codes, terminated by empty name/code.
 *
 * \ingroup Info
 */
extern const GSM_CodeName GSM_Networks[];

/**
 * List of country codes, terminated by empty name/code.
 *
 * \ingroup Info
 */
extern const GSM_CodeName GSM_Countries[];

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
 * Status of GPRS connection.
 *
 * \ingroup Info
 */
typedef enum {
	/**
	 * GRPS is detached.
	 */
	GSM_GPRS_Detached = 1,
	/**
	 * GRPS is attached.
	 */
	GSM_GPRS_Attached
} GSM_GPRS_State;

/**
 * Structure for getting the current network info.
 *
 * \ingroup Info
 */
typedef struct {
	/**
	 * Cell ID (CID)
	 */
	char CID[10];
	/**
	 * GSM network code.
	 */
	char NetworkCode[10];
	/**
	 * Status of network logging. If phone is not logged into any network,
         * some values are not filled
	 */
	GSM_NetworkInfo_State State;
	/**
	 * LAC (Local Area Code).
	 */
	char LAC[10];
	/**
	 * Name of current network like returned from phone (or empty).
	 */
	unsigned char NetworkName[15 * 2];
	/**
	 * GPRS state.
	 */
	GSM_GPRS_State GPRS;
	/**
	 * Cell ID (CID) for packet network
	 */
	char PacketCID[10];
	/**
	 * Status of network logging for packet data. If phone is not logged into any network,
         * some values are not filled
	 */
	GSM_NetworkInfo_State PacketState;
	/**
	 * LAC (Local Area Code) for packet data.
	 */
	char PacketLAC[10];
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
	int SignalStrength;
	/**
	 * Signal strength in percent.
	 */
	int SignalPercent;
	/**
	 * Bit error rate in percent.
	 */
	int BitErrorRate;
} GSM_SignalQuality;

/**
 * Power source
 *
 * \ingroup Info
 */
typedef enum {
	/**
	 * Powered from battery
	 */
	GSM_BatteryPowered = 1,
	/**
	 * Powered from AC, battery connected
	 */
	GSM_BatteryConnected,
	/**
	 * Powered from AC, battery is charging
	 */
	GSM_BatteryCharging,
	/**
	 * Powered from AC, no battery
	 */
	GSM_BatteryNotConnected,
	/**
	 * Powered from AC, battery is fully charged
	 */
	GSM_BatteryFull,
	/**
	 * Power failure
	 */
	GSM_PowerFault
} GSM_ChargeState;

/**
 * Power source
 *
 * \ingroup Info
 */
typedef enum {
	/**
	 * Unknown battery
	 */
	GSM_BatteryUnknown = 0,
	/**
	 * NiMH battery
	 */
	GSM_BatteryNiMH = 1,
	/**
	 * Lithium Ion battery
	 */
	GSM_BatteryLiIon,
	/**
	 * Lithium Polymer battery
	 */
	GSM_BatteryLiPol
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
	int Number;
	GSM_DisplayFeature Feature[7];
} GSM_DisplayFeatures;

/**
 * Phone features definition. This is usually used for things, which can not
 * be determined on run time.
 *
 * \ingroup Info
 */
typedef enum {
	/* n6110.c */
	/**
	 * Calendar,3310 style - 10 reminders, Unicode, 3 coding types
	 */
	F_CAL33 = 1,
	/**
	 * Calendar,5210 style - full Unicode, etc.
	 */
	F_CAL52,
	/**
	 * Calendar,8250 style - "normal", but with Unicode
	 */
	F_CAL82,
	/**
	 * Ringtones returned in SM format - 33xx
	 */
	F_RING_SM,
	/**
	 * No ringtones
	 */
	F_NORING,
	/**
	 * No phonebook in Unicode
	 */
	F_NOPBKUNICODE,
	/**
	 * No WAP
	 */
	F_NOWAP,
	/**
	 * No caller groups
	 */
	F_NOCALLER,
	/**
	 * No Picture Images
	 */
	F_NOPICTURE,
	/**
	 * No Picture Images text in Unicode
	 */
	F_NOPICTUREUNI,
	/**
	 * No startup logo
	 */
	F_NOSTARTUP,
	/**
	 * No calendar
	 */
	F_NOCALENDAR,
	/**
	 * Startup logo is not animated
	 */
	F_NOSTARTANI,
	/**
	 * Network and battery level get from netmonitor
	 */
	F_POWER_BATT,
	/**
	 * Phone profiles in 3310 style
	 */
	F_PROFILES33,
	/**
	 * Phone profiles in 5110 style
	 */
	F_PROFILES51,
	/**
	 * Phone can make authentication with magic bytes
	 */
	F_MAGICBYTES,
	/**
	 * Phone can't send DTMF
	 */
	F_NODTMF,
	/**
	 * Phone return display status
	 */
	F_DISPSTATUS,
	/**
	 * Phone does not return call info
	 */
	F_NOCALLINFO,

	/* n3320.c */
	/**
	 * Day and month reversed in pbk, when compare to GSM models
	 */
	F_DAYMONTH,

	/* n6510.c */
	/**
	 * Phonebook in 3510 style with ringtones ID
	 */
	F_PBK35,
	/**
	 * Phonebook in 7250 style with picture ID
	 */
	F_PBKIMG,
	/**
	 * Phonebook with selecting ringtones from gallery
	 */
	F_PBKTONEGAL,
	/**
	 * Phonebook with SMS list
	 */
	F_PBKSMSLIST,
	/**
	 * Phonebook with user ID
	 */
	F_PBKUSER,
	/**
	 * Caller groups like in 6230i
	 */
	F_6230iCALLER,
	/**
	 * Phone with FM radio
	 */
	F_RADIO,
	/**
	 * ToDo in 6310 style - 0x55 msg type
	 */
	F_TODO63,
	/**
	 * ToDo in 6610 style - like calendar, with date and other
	 */
	F_TODO66,
	/**
	 * No ringtones in MIDI
	 */
	F_NOMIDI,
	/**
	 * Bluetooth support
	 */
	F_BLUETOOTH,
	/**
	 * No images, ringtones, java saved in special filesystem
	 */
	F_NOFILESYSTEM,
	/**
	 * No MMS sets in phone
	 */
	F_NOMMS,
	/**
	 * GPRS point are not useable
	 */
	F_NOGPRSPOINT,
	/**
	 * Calendar,3510 style - Reminder,Call,Birthday
	 */
	F_CAL35,
	/**
	 * Calendar,6510 style - CBMM, method 3
	 */
	F_CAL65,
	/**
	 * WAP & MMS settings contains first & second proxy
	 */
	F_WAPMMSPROXY,
	/**
	 * Phone with Chat settings
	 */
	F_CHAT,
	/**
	 * Phone with SyncML settings
	 */
	F_SYNCML,
	/**
	 * Filesystem version 2
	 */
	F_FILES2,
	/**
	 * No filesystem version 1
	 */
	F_NOFILE1,
	/**
	 * WAP, MMS, etc. settings like in 6230i - unknown now
	 */
	F_6230iWAP,
	/**
	 * Profiles support available
	 */
	F_PROFILES,
	/**
	 * Series 40 3.0
	 */
	F_SERIES40_30,
	/**
	 * SMS are read from filesystem files like in Series 40 3.0
	 */
	F_SMS_FILES,
	/**
	 * MMS storage as in 3320
	 */
	F_3220_MMS,

	/* n6510.c && n7110.c */
	/**
	 * Voice tags available
	 */
	F_VOICETAGS,
	/**
	 * Calendar,6210 style - Call,Birthday,Memo,Meeting
	 */
	F_CAL62,
	/**
	 * Notes supported
	 */
	F_NOTES,

	/* AT modules */
	/**
	 * Phone supports only sent/unsent messages
	 */
	F_SMSONLYSENT,
	/**
	 * CPBS on some memories can hang phone
	 */
	F_BROKENCPBS,
	/**
	 * Siemens M20 like SMS handling
	 */
	F_M20SMS,
	/**
	 * Use slower writing which some phone need
	 */
	F_SLOWWRITE,
	/**
	 * SMS in ME start from location 900 - case of Sagem
	 */
	F_SMSME900,
	/**
	 * Phone supports Alcatel protocol
	 */
	F_ALCATEL,
	/**
	 * Phone can switch to OBEX protocol from AT mode
	 */
	F_OBEX,
	/**
	 * Phone supports IrMC level 2 even if it doesn't report it
	 */
	F_IRMC_LEVEL_2,
	/**
	 * Switching to OBEX mode is done using AT+MODE=22
	 */
	F_MODE22,
	/**
	 * Locations of SMS memories start from 0
	 */
	F_SMS_LOCATION_0,
	/**
	 * Phone does not support UCS2 even if it reports it.
	 */
	F_NO_UCS2,
	/**
	 * Phone returns strings in utf-8 even if it reports GSM.
	 */
	F_FORCE_UTF8,
	/**
	 * Phone supports SM storage for SMS even if it does not report
	 * so.
	 */
	F_SMS_SM,
	/**
	 * Phone supports ME storage for SMS even if it does not report
	 * so.
	 */
	F_SMS_ME,
	/**
	 * Switching to OBEX mode is done using AT+XLNK.
	 */
	F_XLNK,
	/**
	 * Submit messages can be saved on SM memory only.
	 */
	F_SUBMIT_SIM_ONLY,
	/**
	 * Prefer Unicode for phone book manipulations.
	 */
	F_PBK_UNICODE,
	/**
	 * Switching to OBEX mode using AT^SQWE=3.
	 */
	F_SQWE,
	/**
	 * Do not use OBEX/AT switching even if available.
	 */
	F_NO_ATOBEX,
	/**
	 * Length of text for contact is in bytes and not chars.
	 */
	F_LENGTH_BYTES,
	/**
	 * CMGL does not list real locations for CMGR, these should be
	 * sequential.
	 */
	F_BROKEN_CMGL,
	/**
	 * Phonebook has extra numeric field at the end.
	 */
	F_EXTRA_PBK_FIELD,
	/**
	 * Key presses can not be in unicode.
	 */
	F_CKPD_NO_UNICODE,
	/**
	 * OBEX switching using AT+CPROT even if phone does not report
	 * it properly.
	 */
	F_CPROT,
	/**
	 * Phonebook with favorite messaging numbers
	 */
	F_PBKFAVORITEMESSAGE,
	/**
	 * No support for postal entry in phonebook.
	 */
	F_PBKNOPOSTAL,
	/**
	 * Encode number in HEX charset.
	 */
	F_PBK_ENCODENUMBER,
	/**
	 * Do not use CLIP (phone hangs on it).
	 */
	F_NO_CLIP,
	/**
	 * USSD propmts and responses are encoded like PDU in SMS
	 * (packed 7-bit GSM encoding).
	 */
	F_ENCODED_USSD,
	/**
	 * Phone has better support for SMS text mode (rather than PDU mode)
	 */
	F_USE_SMSTEXTMODE,
	/**
	 * Phone does not end CPIN reply with OK/ERROR.
	 */
	F_CPIN_NO_OK,
	/**
	 * Phone require four digit year in time.
	 */
	F_FOUR_DIGIT_YEAR,
	/**
	 * Phone does not have a phone SMS memory even if it reports so.
	 */
	F_SMS_NO_ME,
	/**
	 * Phone does not have a SIM SMS memory even if it reports so.
	 */
	F_SMS_NO_SM,
	/**
	 * Phone supports Siemens style phonebook even if it does not
	 * tell so.
	 */
	F_SIEMENS_PBK,
	/**
	 * Disable AT+SYNCML probing.
	 */
	F_NO_ATSYNCML,
	/**
	 * Phone supports m-obex (usually Samsung phones).
	 */
	F_MOBEX,
	/**
	 * Phone supports m-obex (usually Samsung phones) using AT$TSSPCSW=1.
	 */
	F_TSSPCSW,
	/**
	 * Disable GetNext* operations on the dummy phone.
	 */
	F_DISABLE_GETNEXT,
	/**
	 * Disable GetNextSMS operations on the dummy phone.
	 */
	F_DISABLE_GETNEXTSMS,
	/**
	 * CMGL hangs, so should not be used.
	 */
	F_DISABLE_CMGL,
	/**
	 * Phone does not support UTF8 even if it reports it.
	 */
	F_NO_UTF8,
	/**
	 * Samsung B2100 in UCS-2 mode provides a garbled UTF-8 instead.
	 */
	F_SAMSUNG_UTF8,
	/**
	 * SMS text is always UTF-8 encoded.
	 */
	F_SMS_UTF8_ENCODED,
	/**
	 * Avoid forcibly stopping CUSD session.
	 */
	F_NO_STOP_CUSD,
	/**
	 * Reading og SMSes in text mode.
	 */
	F_READ_SMSTEXTMODE,

	/**
	 * Just marker of highest feature code, should not be used.
	 */
	F_LAST_VALUE
} GSM_Feature;

/**
 * Converts feature value to string.
 *
 * \ingroup Info
 *
 * \param feature GSM_Feature to convert.
 *
 * \return Pointer to static string with string for specified feature,
 * NULL on failure.
 */
const char *GSM_FeatureToString(GSM_Feature feature);

/**
 * Converts feature string to value.
 *
 * \ingroup Info
 *
 * \param feature GSM_Feature string to convert.
 *
 * \return GSM_Feature value, 0 on failure.
 */
GSM_Feature GSM_FeatureFromString(const char *feature);

/**
 * Model identification, used for finding phone features.
 *
 * \ingroup Info
 */
typedef struct {
	/**
	 * Model as returned by phone
	 */
	const char *model;
	/**
	 * Identification by Gammu
	 */
	const char *number;
	/**
	 * Model as used over IrDA
	 */
	const char *irdamodel;
	/**
	 * List of supported features
	 */
	GSM_Feature features[GSM_MAX_PHONE_FEATURES + 1];
} GSM_PhoneModel;

/**
 * Checks whether phone supports features.
 *
 * \param model Model information (you can get it using \ref GSM_GetModelInfo).
 * \param feature GSM_Feature to check for.
 *
 * \return True if phone has defined this feature.
 *
 * \ingroup Info
 */
gboolean GSM_IsPhoneFeatureAvailable(GSM_PhoneModel * model, GSM_Feature feature);

/**
 * Adds feature to phone configuration.
 *
 * \param model Model information (you can get it using \ref GSM_GetModelInfo).
 * \param feature GSM_Feature to check for.
 *
 * \return True if phone has defined this feature.
 *
 * \ingroup Info
 */
gboolean GSM_AddPhoneFeature(GSM_PhoneModel * model, GSM_Feature feature);

#include <gammu-statemachine.h>

/**
 * Reads manufacturer from phone.
 *
 * \param s State machine pointer.
 * \param value Pointer where to store manufacturer name
 *
 * \return Error code.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetManufacturer(GSM_StateMachine * s, char *value);

/**
 * Reads model from phone.
 *
 * \param s State machine pointer.
 * \param value Pointer where to store model name
 *
 * \return Error code.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetModel(GSM_StateMachine * s, char *value);

/**
 * Reads model info from state machine.
 *
 * \param s State machine pointer.
 *
 * \return Pointer to phone information structure.
 *
 * \ingroup Info
 */
GSM_PhoneModel *GSM_GetModelInfo(GSM_StateMachine * s);

/**
 * Reads firmware information from phone.
 *
 * \param s State machine pointer.
 * \param value Pointer where to store revision text
 * \param date Pointer where to store revision date
 * \param num Pointer where to store revision number
 *
 * \return Error code.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetFirmware(GSM_StateMachine * s, char *value, char *date,
			  double *num);

/**
 * Reads IMEI/serial number from phone.
 *
 * \param s State machine pointer.
 * \param value Pointer where to store IMEI, NULL to ignore.
 *
 * \return Error code.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetIMEI(GSM_StateMachine * s, char *value);

/**
 * Gets date and time from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetOriginalIMEI(GSM_StateMachine * s, char *value);

/**
 * Gets month when device was manufactured.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetManufactureMonth(GSM_StateMachine * s, char *value);

/**
 * Gets product code of device.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetProductCode(GSM_StateMachine * s, char *value);

/**
 * Gets hardware information about device.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetHardware(GSM_StateMachine * s, char *value);

/**
 * Gets PPM (Post Programmable Memory) info from phone
 * (in other words for Nokia get, which language pack is in phone)
 *
 * \ingroup Info
 */
GSM_Error GSM_GetPPM(GSM_StateMachine * s, char *value);

/**
 * Gets SIM IMSI from phone.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetSIMIMSI(GSM_StateMachine * s, char *IMSI);

/**
 * Gets information about batery charge and phone charging state.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetBatteryCharge(GSM_StateMachine * s, GSM_BatteryCharge * bat);

/**
 * Reads signal quality (strength and error rate).
 *
 * \ingroup Info
 */
GSM_Error GSM_GetSignalQuality(GSM_StateMachine * s, GSM_SignalQuality * sig);

/**
 * Gets network information.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetNetworkInfo(GSM_StateMachine * s, GSM_NetworkInfo * netinfo);

/**
 * Acquired display status.
 *
 * \ingroup Info
 */
GSM_Error GSM_GetDisplayStatus(GSM_StateMachine * s,
			       GSM_DisplayFeatures * features);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
