/* (c) 2002-2003 by Marcin Wiacek */

#ifndef phone_nokia_dct3_h
#define phone_nokia_dct3_h

#include "../ncommon.h"

GSM_Error DCT3_ReplyPressKey		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyPlayTone		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyEnableSecurity	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetIMEI		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetSMSC		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyWaitSMSC		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySIMLogin		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySIMLogout		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetDateTime		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetAlarm		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySetDateTime		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySetAlarm		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyDialCommand		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetWAPBookmark	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetNetworkInfo	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySendSMSMessage	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySetSMSC		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyGetWAPSettings	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplySetWAPSettings	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyNetmonitor		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyDeleteSMSMessage	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_92_ReplyGetSignalQuality	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_92_ReplyGetBatteryCharge	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_92_ReplyPhoneSetting	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N61_71_ReplyResetPhoneSettings(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N61_91_ReplySetOpLogo		(GSM_Protocol_Message msg, GSM_StateMachine *s);
#ifdef GSM_ENABLE_CELLBROADCAST
GSM_Error DCT3_ReplySetIncomingCB	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3_ReplyIncomingCB		(GSM_Protocol_Message msg, GSM_StateMachine *s);
#endif

GSM_Error DCT3_DeleteWAPBookmark	(GSM_StateMachine *s, GSM_WAPBookmark *bookmark);
GSM_Error DCT3_GetWAPBookmark		(GSM_StateMachine *s, GSM_WAPBookmark *bookmark);
GSM_Error DCT3_PressKey			(GSM_StateMachine *s, GSM_KeyCode Key, bool Press);
GSM_Error DCT3_PlayTone			(GSM_StateMachine *s, int Herz, unsigned char Volume, bool start);
GSM_Error DCT3_EnableSecurity		(GSM_StateMachine *s, unsigned char	  status	);
GSM_Error DCT3_GetIMEI			(GSM_StateMachine *s);
GSM_Error DCT3_GetSMSC			(GSM_StateMachine *s, GSM_SMSC		  *smsc		);
GSM_Error DCT3_GetNetworkInfo		(GSM_StateMachine *s, GSM_NetworkInfo	  *netinfo	);
GSM_Error DCT3_DialVoice		(GSM_StateMachine *s, char		  *number, 	GSM_CallShowNumber ShowNumber);
GSM_Error DCT3_Reset			(GSM_StateMachine *s, bool		  hard		);
GSM_Error DCT3_CancelCall		(GSM_StateMachine *s, int ID, bool all);
GSM_Error DCT3_AnswerAllCalls		(GSM_StateMachine *s);
GSM_Error DCT3_SendSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage 	  *sms		);
GSM_Error DCT3_GetAlarm			(GSM_StateMachine *s, GSM_Alarm		  *Alarm,	unsigned char msgtype);
GSM_Error DCT3_GetDateTime		(GSM_StateMachine *s, GSM_DateTime	  *date_time, 	unsigned char 	msgtype	);
GSM_Error DCT3_SetAlarm			(GSM_StateMachine *s, GSM_Alarm		  *Alarm, 	unsigned char msgtype);
GSM_Error DCT3_SetDateTime		(GSM_StateMachine *s, GSM_DateTime	  *date_time, 	unsigned char 	msgtype	);
GSM_Error DCT3_SetIncomingCB		(GSM_StateMachine *s, bool		  enable	);
GSM_Error DCT3_GetSMSStatus		(GSM_StateMachine *s, GSM_SMSMemoryStatus *status	);
GSM_Error DCT3_SetSMSC			(GSM_StateMachine *s, GSM_SMSC		  *smsc		);
GSM_Error DCT3_GetWAPSettings		(GSM_StateMachine *s, GSM_MultiWAPSettings	  *settings	);
GSM_Error DCT3_SetWAPSettings		(GSM_StateMachine *s, GSM_MultiWAPSettings *settings);
GSM_Error DCT3_SetWAPBookmark		(GSM_StateMachine *s, GSM_WAPBookmark	  *bookmark);
GSM_Error DCT3_Netmonitor		(GSM_StateMachine *s, int 		  testnumber, 	char		*value	);
GSM_Error DCT3_GetManufactureMonth	(GSM_StateMachine *s, char		  *value	);
GSM_Error DCT3_GetProductCode		(GSM_StateMachine *s, char *value);
GSM_Error DCT3_GetOriginalIMEI		(GSM_StateMachine *s, char *value);
GSM_Error DCT3_GetHardware		(GSM_StateMachine *s, char *value);
GSM_Error DCT3_GetPPM			(GSM_StateMachine *s, char *value);
GSM_Error N61_71_ResetPhoneSettings	(GSM_StateMachine *s, GSM_ResetSettingsType Type);
GSM_Error N71_92_GetBatteryCharge	(GSM_StateMachine *s, GSM_BatteryCharge *bat);
GSM_Error N71_92_GetSignalQuality	(GSM_StateMachine *s, GSM_SignalQuality *sig);
GSM_Error N71_92_GetPhoneSetting	(GSM_StateMachine *s, int Request, int Setting);
GSM_Error N71_92_GetDateTime		(GSM_StateMachine *s, GSM_DateTime	  *date_time	);
GSM_Error N71_92_SetDateTime		(GSM_StateMachine *s, GSM_DateTime	  *date_time	);

GSM_Error DCT3_DecodeSMSFrame		(GSM_StateMachine *s, GSM_SMSMessage *SMS, unsigned char *buffer);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
