/**
 * Definition of security codes.
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
	SEC_None
} GSM_SecurityCodeType;

#define GSM_SECURITY_CODE_LEN	15

/**
 * Security code definition.
 */
typedef struct {
	/**
	 * Actual code.
	 */
	char 			Code[GSM_SECURITY_CODE_LEN+1];
	/**
	 * Type of the code.
	 */
	GSM_SecurityCodeType 	Type;
} GSM_SecurityCode;

/**
 * Enters security code (PIN, PUK,...) .
 */
GSM_Error GSM_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code);
/**
 * Queries whether some security code needs to be entered./
 */
GSM_Error GSM_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status);
