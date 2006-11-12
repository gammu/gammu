/* (c) 2006 by Michal Cihar */

/**
 * High level functions for communication with Sony-Ericsson phones.
 */

#ifndef sonyericsson_h
#define sonyericsson_h

#include "../../gsmcomon.h"

#ifndef GSM_USED_AT
# define GSM_USED_AT
#endif
#ifndef GSM_USED_OBEX
# define GSM_USED_OBEX
#endif

/**
 * Determines which mode is phone currently using.
 */
typedef enum {
	/**
	 * Phone accepts normal AT commands.
	 */
	SONYERICSSON_ModeAT = 1,
	/**
	 * OBEX mode using proprietary protocol.
	 */
	SONYERICSSON_ModeOBEX
} GSM_SONYERICSSON_Mode;

/**
 * SONYERICSSON driver private data.
 */
typedef struct {
	/***********************************/
	/* Communication state information */
	/***********************************/
	/**
	 * Mode of current communication.
	 */
	GSM_SONYERICSSON_Mode	Mode;
	/**
	 * Whether phone support AT*EOBEX
	 */
	bool			HasOBEX;
} GSM_Phone_SONYERICSSONData;
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
