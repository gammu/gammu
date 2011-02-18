/**
 * \file gammu-security.h
 * \author Michal Čihař
 *
 * Security functions.
 */
#ifndef __gammu_security_h
#define __gammu_security_h

/**
 * \defgroup Security Security
 * Security related operations with phone.
 */

#include <gammu-limits.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>

/**
 * Definition of security codes.
 *
 * \ingroup Security
 */
typedef enum {
	/**
	 * Security code.
	 */
	SEC_SecurityCode = 0x01,
	/**
	 * PIN.
	 */
	SEC_Pin,
	/**
	 * PIN 2.
	 */
	SEC_Pin2,
	/**
	 * PUK.
	 */
	SEC_Puk,
	/**
	 * PUK 2.
	 */
	SEC_Puk2,
	/**
	 * Code not needed.
	 */
	SEC_None,
	/**
	 * Phone code needed.
	 */
	SEC_Phone,
	/**
	 * Network code needed.
	 */
	SEC_Network,
} GSM_SecurityCodeType;

/**
 * Security code definition.
 *
 * \ingroup Security
 */
typedef struct {
	/**
	 * Type of the code.
	 */
	GSM_SecurityCodeType Type;
	/**
	 * Actual code.
	 */
	char Code[GSM_SECURITY_CODE_LEN + 1];
	/**
	 * New PIN code.
	 *
	 * Some phones require to set PIN on entering PUK, you can
	 * provide it here.
	 */
	char NewPIN[GSM_SECURITY_CODE_LEN + 1];
} GSM_SecurityCode;

/**
 * Enters security code (PIN, PUK,...) .
 *
 * \ingroup Security
 */
GSM_Error GSM_EnterSecurityCode(GSM_StateMachine * s, GSM_SecurityCode * Code);

/**
 * Queries whether some security code needs to be entered.
 *
 * \ingroup Security
 */
GSM_Error GSM_GetSecurityStatus(GSM_StateMachine * s,
				GSM_SecurityCodeType * Status);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
