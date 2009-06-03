#include <gammu.h>

extern volatile gboolean wasincomingsms;
extern void IncomingSMS(GSM_StateMachine * s, GSM_SMSMessage sms, void *user_data);
extern void IncomingCB(GSM_StateMachine * s, GSM_CBMessage CB, void *user_data);
extern void IncomingUSSD(GSM_StateMachine * s, GSM_USSDMessage ussd, void *user_data);
extern void IncomingUSSD2(GSM_StateMachine * s, GSM_USSDMessage ussd, void *user_data);
extern void DisplayIncomingSMS(void);
extern void GetSMSC(int argc, char *argv[]);
extern void GetSMS(int argc, char *argv[]);
extern void DeleteSMS(int argc, char *argv[]);
extern void GetAllSMS(int argc, char *argv[]);
extern void GetEachSMS(int argc, char *argv[]);
extern void GetSMSFolders(int argc, char *argv[]);
extern void GetMMSFolders(int argc, char *argv[]);
extern void SendSaveDisplaySMS(int argc, char *argv[]);
extern void GetEachMMS(int argc, char *argv[]);
extern void GetUSSD(int argc, char *argv[]);
extern void ReadMMSFile(int argc, char *argv[]);
extern void AddSMSFolder(int argc, char *argv[]);
extern void DeleteAllSMS(int argc, char *argv[]);
