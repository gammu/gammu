#ifndef _gsm_call_h
#define _gsm_call_h

typedef enum {
        GN_CALL_IncomingCall,	 /* Somebody calls to us 	 			*/
        GN_CALL_OutgoingCall,	 /* We call somewhere 		 			*/
        GN_CALL_CallStart,	 /* Call started 	 				*/
        GN_CALL_CallEnd,	 /* End of call from unknown side 			*/
        GN_CALL_CallRemoteEnd,	 /* End of call from remote side  			*/
        GN_CALL_CallLocalEnd, 	 /* End of call from our side	 			*/
	GN_CALL_CallEstablished, /* Call established. Waiting for answer or dropping 	*/
	GN_CALL_OutgoingFreeCall /* Outgoing call will be for free			*/
} GSM_CallStatus;

typedef struct {
        GSM_CallStatus 		Status;
        char 			PhoneNumber [(GSM_MAX_NUMBER_LENGTH+1)*2];
	int			CallID;
	int			Code;
} GSM_Call;

#endif /* _gsm_call_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
