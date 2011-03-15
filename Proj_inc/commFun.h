#ifndef _COMMONFUN_H
#define _COMMONFUN_H
#include "mbglobals.h"
#include <string.h>
#include <stdio.h>
#define my_sleep(x) ((x)<1000 ? Sleep(1) : Sleep((x)/1000))
#define MAX_LINES 100

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
typedef struct {
	char	Specialtext[MAX_PATH];
	int		Anwserlines;
} GSM_ATMultiAnwser;

typedef struct {
	unsigned char	msgtype[20];
	int				subtypechar;
	unsigned char	subtype;
} GSM_Reply_MsgCheckInfo;

typedef struct {
	int nCount;
	GSM_Reply_MsgCheckInfo  CheckInfo[50];
} GSM_Reply_MsgType;


typedef struct {
	int 		Length;
	int 		Count;
	unsigned char 	Type;
	unsigned char 	Source;
	unsigned char 	Destination;
	unsigned char	*Buffer;
	int		BufferUsed;
	unsigned char 	CheckSum[2];
} GSM_Protocol_Message;
typedef enum {
	RX_Sync,
	RX_GetDestination,
	RX_GetSource,
	RX_GetType,
	RX_GetLength1,
	RX_GetLength2,
	RX_GetMessage
} GSM_Protocol_RXState;

typedef enum {
	DL_NONE = 0,		
	DL_BINARY = 1,		/* Binary transmission dump 	*/
	DL_TEXT,		/* Text transmission dump	*/
	DL_TEXTALL,		/* Everything			*/
	DL_TEXTERROR,		/* Only errors			*/
	DL_TEXTDATE,		/* Text transmission dump	*/
	DL_TEXTALLDATE,		/* Everything			*/
	DL_TEXTERRORDATE	/* Only errors			*/
} Debug_Level;
typedef enum {
	GCT_ERROR=0,
	GCT_MBUS2=1,
	GCT_FBUS2,
	GCT_FBUS2DLR3,
	GCT_FBUS2DKU5,
	GCT_FBUS2PL2303,
	GCT_FBUS2BLUE,
	GCT_FBUS2IRDA,
	GCT_PHONETBLUE,
	GCT_AT,
	GCT_MROUTERBLUE,

	GCT_IRDAOBEX,
	GCT_IRDAAT,
	GCT_IRDAPHONET,
	GCT_BLUEFBUS2,
	GCT_BLUEAT,
	GCT_BLUEPHONET,
	GCT_BLUEOBEX,
	GCT_PHONEAT,
	GCT_SYNCMLAT,
	GCT_SYNCMLBT,
	GCT_SYNCMLUSB,

	GCT_CABLEAT,
	GCT_CABLEPHONEAT,
	GCT_FTPAT,
	GCT_PCSC,

} GSM_ConnectionType;
/* ---------------------- Phone features ----------------------------------- */

typedef enum {
	/* n6110.c */
	F_NONE = 0,	
//	F_CAL33 = 1,	/* Calendar,3310 style - 10 reminders, Unicode, 3 coding types	*/
//	F_CAL52,	/* Calendar,5210 style - full Unicode, etc.			*/
//	F_CAL82,	/* Calendar,8250 style - "normal", but with Unicode		*/
//	F_RING_SM,	/* Ringtones returned in SM format - 33xx			*/
//	F_NORING,	/* No ringtones							*/
//	F_NOPBKUNICODE,	/* No phonebook in Unicode					*/
//	F_NOWAP,      	/* No WAP							*/
//	F_NOCALLER,	/* No caller groups						*/
//	F_NOPICTURE,	/* No Picture Images						*/
//	F_NOPICTUREUNI,	/* No Picture Images text in Unicode				*/
//	F_NOSTARTUP,	/* No startup logo						*/
//	F_NOCALENDAR,	/* No calendar							*/
//	F_NOSTARTANI,	/* Startup logo is not animated 				*/
//	F_POWER_BATT,	/* Network and battery level get from netmonitor		*/
//	F_PROFILES33,	/* Phone profiles in 3310 style					*/
//	F_PROFILES51,	/* Phone profiles in 5110 style					*/
//	F_MAGICBYTES,	/* Phone can make authentication with magic bytes		*/
//	F_NODTMF,	/* Phone can't send DTMF					*/
//	F_DISPSTATUS,	/* Phone return display status					*/
//	F_NOCALLINFO,

	/* n3320.c */
//	F_DAYMONTH,	/* Day and month reversed in pbk, when compare to GSM models	*/

	/* n6510.c */
	F_PBK35,	/* Phonebook in 3510 style with ringtones ID			*/
	F_PBKIMG,	/* Phonebook in 7250 style with picture ID			*/
//	F_PBKTONEGAL,	/* Phonebook with selecting ringtones from gallery              */
//	F_PBKSMSLIST,	/* Phonebook with SMS list					*/
	F_PBKUSER,	/* Phonebook with user ID					*/
//	F_RADIO,	/* Phone with FM radio						*/
	F_TODO63,	/* ToDo in 6310 style - 0x55 msg type				*/
	F_TODO66,	/* ToDo in 6610 style - like calendar, with date and other	*/
//	F_NOMIDI,	/* No ringtones in MIDI						*/
//	F_BLUETOOTH,	/* Bluetooth support						*/
	F_NOFILESYSTEM,	/* No images, ringtones, java saved in special filesystem	*/
	F_NOMMS,	/* No MMS sets in phone						*/
	F_NOGPRSPOINT,	/* GPRS point are not useable					*/
	F_CAL35,	/* Calendar,3510 style - Reminder,Call,Birthday			*/
	F_CAL65,	/* Calendar,6510 style - CBMM, method 3				*/
	F_WAPMMSPROXY,	/* WAP & MMS settings contains first & second proxy		*/

	/* n6510.c && n7110.c */
	F_VOICETAGS,	/* Voice tags available						*/
	F_CAL62,	/* Calendar,6210 style - Call,Birthday,Memo,Meeting		*/
//	F_NOTES,
	F_PBKPUSHTALK,

	/* AT modules */
	F_SMSONLYSENT,	/* Phone supports only sent/unsent messages			*/
//	F_BROKENCPBS, 	/* CPBS on some memories can hang phone				*/
	F_M20SMS,	/* Siemens M20 like SMS handling				*/
	F_SLOWWRITE,	/* Use slower writing which some phone need			*/
	F_SMSME900,	/* SMS in ME start from location 900 - case of Sagem		*/
//	F_ALCATEL,	/* Phone supports Alcatel protocol				*/
	F_SETSERIAL, /* Phonebook not support vcf --only for SE (example : SE T serial)*/
	F_ATPHONE_SMS, /* User AtPhone and support SMS*/
	F_ATPHONE_PHONEBOOK_ME, /* Use AtPhone and support Phonebook ME*/
	F_ATPHONE_PHONEBOOK_SM, /* Use AtPhone and support Phonebook SM*/
	F_NOKIA_PHONEBOOK_NOSM, /* Use Nokia and unsupport Phonebook SM*/
	F_MOTO_PHONEBOOK_2, /* Use Moto and Phonebook data include Address ,birthday*/
	F_MOTO_PHONEBOOK_3,
	F_ATPHONE_PHONEBOOK_SYNCML, /* Use AtPhone and get Phonebook from database */
	F_NEWSERIAL40,
	F_SLOWOBEX,
	F_PIM_SYNCML_SERVER,
	F_CDMA_PHONE,
	F_SAMSUNG_PHONEBOOK_D,
	F_ATPHONE_PHONEBOOK_WITHOUTSPACE, /* Use AtPhone and setmemory without space*/
	F_CAL_DESCRIPTION,
	F_SMS_SETMAXMEINDEX,
	F_SMS_SHIFTMEINDEX,
	F_ATPHONE_PHONEBOOK_WITHOUTINDEX,
	F_MOTO_CALENDAR_2,
	F_MOTO_CALENDAR_3,
	F_SERIAL40_SYNCML,
	F_TODO6500,
	F_CAL6500,
	F_MOTO_E6,
	F_SHARPS_PBK_Postal,
	F_LG_KESeries,
	F_NOKIAS60_SMS_TYPE1,
	F_MOTO_SMSSTARTINDEX0,
	F_S40_PBKSYNCML,
	F_SAMSUNG_SMS2

} Feature;

typedef struct 
 {
	char		model[MAX_PATH];
	char		number[MAX_PATH];
	char		irdamodel[MAX_PATH];
	char		Manufacturer[MAX_PATH];
	char        PhoneDllName[MAX_PATH];
	char        ProtocolDllName[MAX_PATH];
	char        DeviceDllName[MAX_PATH];
	Feature		features[12];
}OnePhoneModel;

typedef struct {
	Debug_Level	dl;
	FILE		*df;
//	bool        	use_global;
	char		*coding;
} Debug_Info;
typedef struct {
	int numbers[MAX_LINES*2];
} GSM_Lines;

void WINAPI ReadVCALDateTime_GMT(char *Buffer, GSM_DateTime *dt);
int WINAPI smprintf(Debug_Info *debugInfo, const char *format, ...);
void WINAPI GSM_OSErrorInfo(Debug_Info *debugInfo, char *description);
int WINAPI smfprintf(FILE *f, Debug_Level dl, const char *format, ...);
void WINAPI GSM_GetCurrentDateTime 	(GSM_DateTime *Date);
char *WINAPI DayOfWeek 		(int year, int month, int day);
GSM_Error WINAPI CheckReplyFunctions(GSM_Reply_MsgType ReplyCheckType,GSM_Protocol_Message		*msg);
void WINAPI Fill_GSM_DateTime(GSM_DateTime *Date, time_t timet);
int WINAPI dbgprintf(const char *format, ...);
void WINAPI CopyLineString(unsigned char *dest, unsigned char *src, GSM_Lines lines, int start);
void WINAPI SplitLines(unsigned char *message, int messagesize, GSM_Lines *lines, unsigned char *whitespaces, int spaceslen, bool eot);
char *WINAPI GetLineString(unsigned char *message, GSM_Lines lines, int start);
BOOL WINAPI GetModelData(char *model, char *number, char *irdamodel,OnePhoneModel *pMobileInfo);
BOOL WINAPI GetModelDataEx(char *model, char *number, char *irdamodel,OnePhoneModel *pMobileInfo,int nCompanyID);
bool WINAPI IsPhoneFeatureAvailable(OnePhoneModel *model, Feature feature);
bool WINAPI ReadVCALText(unsigned char *Buffer, char *Start,unsigned char *Value,BOOL bSkipFirst = true);
bool WINAPI ReadVCALText_ADR(unsigned char *Buffer, char *Start,unsigned char *Value);
void WINAPI SaveVCALText(char *Buffer, int *Length,unsigned char *Text, char *Start);
void WINAPI SiemensSaveVCARDText(char *Buffer, int *Length, char *Text, char *Start,char *beforetext);
int WINAPI ATGEN_ExtractOneParameter(unsigned char *input, unsigned char *output);
int WINAPI ATGEN_ExtractOneParameterEx(unsigned char *input, unsigned char *output);
int WINAPI ATGEN_ExtractOneParameterEx2(unsigned char *input, unsigned char *output);
bool WINAPI HHMM_Get_DateTime(GSM_DateTime *Date, char * timet);
bool WINAPI MMDDYY_Get_DateTime(GSM_DateTime *Date, char * timet);
int WINAPI GetDurationDifferenceTime(GSM_DateTime *end_DT, GSM_DateTime *start_DT);
void WINAPI GetFilePath_Unicode(unsigned char* wFileFullName,unsigned char *pFilePath);
void WINAPI GetFileName_Unicode(unsigned char* wFileFullName,unsigned char *pFileName);
int WINAPI VCARD_ParseAddress(unsigned char* buffer, unsigned char* postal, unsigned char* street, unsigned char* city, unsigned char* state, unsigned char* zipcode, unsigned char* country);
void WINAPI VCARD_AddressCat(GSM_MemoryEntry* pbk, unsigned char* value);
void WINAPI SharpSaveVCARDText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext, char* endtext);
void WINAPI GSM_DecodeSharpDevInfo(unsigned char* Buffer, char* pszManufacturer, char* pszModel, char* pszIMEI);
bool WINAPI SAGEM_ReadVCALText(unsigned char *Buffer, char *Start,unsigned char *Value,BOOL bSkipFirst = true);
BOOL WINAPI IsCheckLimitedTrial(bool* checkKey ,bool* checkStack);

void SplitString(char *pString ,int  Split,char* pStr1,char* pStr2);
//#ifdef _DEBUG
	int WINAPI dbgprintf(const char *format, ...);
//#else
//  #  define dbgprintf
//#endif

#endif
