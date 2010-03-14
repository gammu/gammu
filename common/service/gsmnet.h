#ifndef __gsm_networks_h
#define __gsm_networks_h

/**
 * This type is used to hold information about various GSM networks.
 */
typedef struct {
	/**
	 * GSM network code.
	 */
	char *Code;
	/**
	 * GSM network name.
	 */
	char *Name;
} GSM_Network;

/**
 * This type is used to hold information about various GSM countries.
 */
typedef struct {
	/**
	 * GSM country code.
	 */
	char *Code;
	/**
	 * GSM country name.
	 */
	char *Name;
} GSM_Country;

/* These functions are used to search the structure defined above.*/

/**
 * Lookup GSM network by code.
 */
char *GSM_GetNetworkName(char *NetworkCode);
/**
 * Lookup GSM network by name.
 */
char *GSM_GetNetworkCode(char *NetworkName);

/**
 * Lookup GSM country by code.
 */
char *GSM_GetCountryName(char *CountryCode);
/**
 * Lookup GSM country by name.
 */
char *GSM_GetCountryCode(char *CountryName);

/**
 * State of network.
 */
typedef enum {
	/**
	 * Phone home network.
	 */
	GSM_HomeNetwork = 1,
	/**
	 * Using roaming.
	 */
	GSM_RoamingNetwork,
	/**
	 * Network explicitely requested by user.
	 */
	GSM_RequestingNetwork,
	/**
	 * No network available.
	 */
	GSM_NoNetwork
} GSM_NetworkInfo_State;

/**
 * This structure is used to get the current network status.
 */
typedef struct {
	/**
	 * Name of current network.
	 */
	unsigned char		   NetworkName[15*2];
	/**
	 * State of current network.
	 */
	GSM_NetworkInfo_State	   State;
	/**
	 * GSM network code.
	 */
	char			   NetworkCode[10];
	/**
	 * Cell ID
	 */
	unsigned char		   CellID[10];
	/**
	 * LAC (Local Area Code).
	 */
	unsigned char		   LAC[10];
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

#endif  /* __gsm_networks_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
