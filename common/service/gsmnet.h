#ifndef __gsm_networks_h
#define __gsm_networks_h

/* This type is used to hold information about various GSM networks. */
typedef struct {
	char *Code; /* GSM network code */
	char *Name; /* GSM network name */
} GSM_Network;

/* This type is used to hold information about various GSM countries. */
typedef struct {
	char *Code; /* GSM country code */
	char *Name; /* GSM country name */
} GSM_Country;

/* These functions are used to search the structure defined above.*/
char *GSM_GetNetworkName(char *NetworkCode);
char *GSM_GetNetworkCode(char *NetworkName);

char *GSM_GetCountryName(char *CountryCode);
char *GSM_GetCountryCode(char *CountryName);

typedef enum {
	GSM_HomeNetwork = 1,
	GSM_RoamingNetwork,
	GSM_RequestingNetwork,
	GSM_NoNetwork
} GSM_NetworkInfo_State;

/* This structure is used to get the current network status */
typedef struct {
	unsigned char			NetworkName[15*2];
	GSM_NetworkInfo_State 		State;
	char				NetworkCode[10];	/* GSM network code */
	unsigned char			CellID[10];		/* CellID */
	unsigned char			LAC[10];		/* LAC */
} GSM_NetworkInfo;

void NOKIA_EncodeNetworkCode (unsigned char* buffer, unsigned char* output);
void NOKIA_DecodeNetworkCode (unsigned char* buffer, unsigned char* output);

typedef struct {
    /* All these should be -1 when unknown */
    int 	SignalStrength; /* Signal strength in dBm 	*/
    int 	SignalPercent;  /* Signal strength in percent 	*/
    int 	BitErrorRate;   /* Bit error rate in percent 	*/
} GSM_SignalQuality;

#endif	/* __gsm_networks_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
