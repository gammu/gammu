/* (c) 2002-2003 by Marcin Wiacek & Michal Cihar */

#ifndef __gsm_net_h
#define __gsm_net_h

/**
 * Find network name from given network code.
 */
char *GSM_GetNetworkName(char *NetworkCode);

/**
 * Find country name from given country code.
 */
char *GSM_GetCountryName(char *CountryCode);

/**
 * Status of network logging
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

void NOKIA_EncodeNetworkCode (unsigned char* buffer, unsigned char* output);
void NOKIA_DecodeNetworkCode (unsigned char* buffer, unsigned char* output);

/**
 * Information about signal quality, all these should be -1 when unknown.
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

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
