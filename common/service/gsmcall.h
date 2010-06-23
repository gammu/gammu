#ifndef _gsm_call_h
#define _gsm_call_h

typedef enum {
        GN_CALL_IncomingCall,	/* Somebody calls to us 	 */
        GN_CALL_OutgoingCall,	/* We call somewhere 		 */
        GN_CALL_CallStart,	/* Outgoing call established 	 */
        GN_CALL_CallEnd,	/* End of call from unknown side */
        GN_CALL_CallRemoteEnd,	/* End of call from remote side  */
        GN_CALL_CallLocalEnd	/* End of call from our side	 */
} GSM_CallStatus;

typedef struct {
        GSM_CallStatus 		Status;
        char 			PhoneNumber [(GSM_MAX_NUMBER_LENGTH+1)*2];
} GSM_Call;

#endif /* _gsm_call_h */
