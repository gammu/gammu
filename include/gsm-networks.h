/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Header file for GSM networks.

*/

#ifndef __gsm_networks_h
#define __gsm_networks_h

/* Define enums for RF units.  GRF_CSQ asks for units in form used
   in AT+CSQ command as defined by GSM 07.07 */
typedef enum {
  GRF_Arbitrary,
  GRF_dBm,
  GRF_mV,
  GRF_uV,
  GRF_CSQ,
  GRF_Percentage
} GSM_RFUnits;

/* This structure is used to get the current network status */

typedef struct {
  char NetworkCode[10]; /* GSM network code */
  char CellID[10];      /* CellID */
  char LAC[10];         /* LAC */
} GSM_NetworkInfo;

/* This type is used to hold information about various GSM networks. */

typedef struct {
  char Code[9+1]; /* GSM network code */
  char Name[50]; /* GSM network name */
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

void EncodeNetworkCode(unsigned char* buffer, unsigned char* output);
void DecodeNetworkCode(unsigned char* buffer, unsigned char* output);

#endif	/* __gsm_networks_h */
