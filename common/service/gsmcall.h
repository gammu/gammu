#ifndef _gsm_call_h
#define _gsm_call_h

typedef enum {
        GN_CALL_Idle,
        GN_CALL_Ringing,
        GN_CALL_Dialing,
        GN_CALL_Established,
        GN_CALL_Held
} GSM_CallStatus;

typedef struct {
        int 			CallID;
        GSM_CallStatus 		Status;
        char 			RemoteNumber[GSM_MAX_NUMBER_LENGTH*2];
        char 			RemoteName  [GSM_MAX_NUMBER_LENGTH*2];
        bool  			LocalOriginated; /* true is local originated */
} GSM_Call;

#endif /* _gsm_call_h */
