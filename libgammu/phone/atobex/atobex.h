/* (c) 2006 by Michal Cihar */

/**
 * \file atobex.h
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

#ifndef atobex_h
#define atobex_h

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
	ATOBEX_ModeAT = 1,
	/**
	 * OBEX mode using proprietary protocol.
	 */
	ATOBEX_ModeOBEX
} GSM_ATOBEX_Mode;

/**
 * Do we support switch to OBEX?
 */
typedef enum {
	ATOBEX_OBEX_None = 1, /**< No OBEX switch command */
	ATOBEX_OBEX_EOBEX, /**< Switch using AT*EOBEX */
	ATOBEX_OBEX_CPROT0, /**< Switch using AT+CPROT=0 */
	ATOBEX_OBEX_MODE22, /**< Switch using AT+MODE=22 */
	ATOBEX_OBEX_XLNK, /**< Switch using AT+XLNK */
	ATOBEX_OBEX_SQWE, /**< Switch using AT^SQWE */
} GSM_ATOBEX_OBEX;
/**
 * ATOBEX driver private data.
 */
typedef struct {
	/***********************************/
	/* Communication state information */
	/***********************************/
	/**
	 * Mode of current communication.
	 */
	GSM_ATOBEX_Mode	Mode;
	/**
	 * Whether phone support AT*EOBEX
	 */
	GSM_ATOBEX_OBEX	HasOBEX;
	/***
	 * Whether AT*EBCA failed.
	 */
	gboolean EBCAFailed;
} GSM_Phone_ATOBEXData;
#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
