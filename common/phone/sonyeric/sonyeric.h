/* (c) 2006 by Michal Cihar */

/**
 * \file sonyeric.h
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @addtogroup SEPhone
 * @{
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
 * Do we support switch to OBEX?
 */
typedef enum {
	SONYERICSSON_OBEX_None = 1, /**< No OBEX switch command */
	SONYERICSSON_OBEX_EOBEX, /**< Switch using AT*EOBEX */
	SONYERICSSON_OBEX_CPROT0, /**< Switch using AT+CPROT=0 */
	SONYERICSSON_OBEX_MODE22, /**< Switch using AT+MODE=22 */
} GSM_SONYERICSSON_OBEX;
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
	GSM_SONYERICSSON_OBEX	HasOBEX;
} GSM_Phone_SONYERICSSONData;
#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
