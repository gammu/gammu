/*

  ( M Y  )G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  MyGnokii NET API

  Version 0.3 Ralf Thelen 07.02.2002

*/

#ifndef __mg_net_api_h
#define __mg_net_api_h

#define MG_NET_API_MAJOR_VERSION 0
#define MG_NET_API_MINOR_VERSION 3

#define MG_ERROR		(-1)
#define MG_ERROR_TIMEOUT	(-1001)

#define TRUE	1
#define FALSE	0

typedef enum {
   MG_INIT,
   MG_EXIT,
   MG_VERSION,
   MG_IDENTIFY,
   MG_MEMORYSTATUS,
   MG_GET_MEMORY_LOCATION,
   MG_WRITE_PHONEBOOK_LOCATION,
   MG_DELETE_PHONEBOOK_LOCATION,
   MG_SEND_SMS
} MG_api_cmd;
 
 
typedef struct {
        MG_api_cmd cmd;                 /* requestet command 	     */
	char passwd[8];			/* password for udp requests */
        char buffer[1024];	        /* parameter         	     */
} MG_api_request;

typedef struct {
        int major;
	int minor;
} MG_rq_version;

typedef struct {
        char imei[64];
	char model[64];
	char rev[64];
} MG_rq_identify;
 
typedef struct {
	char memtype[3];
	int used;
	int free;
} MG_rq_memorystatus;

typedef struct {
	char memtype[3];
	int location;
	char name[51];
	char group[49];
	char nr_general[49];
	char nr_mobile[49];
	char nr_work[49];
	char nr_fax[49];
	char nr_home[49];
	char note[51];
	char postal[51];
	char email[51];
} MG_rq_phonebook_location;

typedef struct {
	char destination[12];				/* destination */
        char message[161];			        /* the message */
        char SMSCNumber[100];	        		/* SMSC number */
	int  SMSCenterIndex;				/* SMSC number index in phone memory */
	int  SMSValidity;				/* validity of sms */
	char longsms;					/* long messages, NOT USED ! */
	char enablevoice;				/* SMS enables voice indicator */
	char disablevoice;				/* SMS disables voice indicator */
	char enableemail;				/* SMS enables email indicator */
	char disableemail;				/* SMS disables email indicator */
	char enablefax;					/* SMS enables fax indicator */
	char disablefax;				/* SMS disables fax indicator */
	char unicode;					/* SMS coding type */
	char delivery;					/* we want delivery report */
} MG_rq_send_sms;

/* Prototypes */
int mg_init(char *hostname, char *passwd);
int mg_exit(void);
int mg_version(MG_rq_version *version);
int mg_identify(MG_rq_identify *ident);
int mg_memorystatus(MG_rq_memorystatus *stats);
int mg_get_memory_location(MG_rq_phonebook_location *rph);
int mg_write_phonebook_location(MG_rq_phonebook_location *wph);
int mg_delete_phonebook_location(MG_rq_phonebook_location *dph);
int mg_send_sms(MG_rq_send_sms *sms);
 
#endif /* __mg_net_api_h */
