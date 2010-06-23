/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Mainline code for mgnetd daemon.

  Last modification: 07.02.2002
  Ralf Thelen <ralf@mythelen.de>	

*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
 
#if defined(__svr4__) || defined(__FreeBSD__)
  #  include <strings.h>        /* for bzero */
#endif
 
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <getopt.h>

#include "misc.h"
#include "files/cfgreader.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "mgnetd.h"

#ifdef USE_NLS
  #include <locale.h>
#endif

/* Global variables */
char		*Model;		/* Model from .gnokiirc file. */
char		*Port;		/* Port from .gnokiirc file */
char		*Initlength;	/* Init length from .gnokiirc file */
char		*Connection;	/* Connection type from .gnokiirc file */
char            *SynchronizeTime;
char            *BinDir;        /* Directory of the mgnokiidev command */ 


void usage(void)
{

  fprintf(stdout, _("mgnetd Version %d.%d, a udp server for mygnokii\n based on Version %s\n"
 	"Built %s %s for %s on %s \n")
	,MG_NET_API_MAJOR_VERSION,MG_NET_API_MINOR_VERSION, VERSION, __TIME__, __DATE__, Model, Port);
  fprintf(stdout, _("   usage: mgnetd passwd\n"));
  exit(1);
}

/* fbusinit is the generic function which waits for the FBUS link. The limit
   is 10 seconds. After 10 seconds we quit. */
 
void fbusinit(void (*rlp_handler)(RLP_F96Frame *frame))
{
 
  int count=0;
  GSM_Error error;

#ifndef WIN32
  if (strcmp(GetMygnokiiVersion(),VERSION)!=0)
    fprintf(stderr,_("WARNING: version of installed libmygnokii.so (%s) is different to version of mgnetd (%s)\n"),GetMygnokiiVersion(),VERSION);
#endif
 
  /* Initialise the code for the GSM interface. */
  error = GSM_Initialise(Model, Port, Initlength, GetConnectionTypeFromString(Connection), rlp_handler, SynchronizeTime);
 
  if (error != GE_NONE) {
    fprintf(stderr, _("GSM/FBUS init failed! (Unknown model ?). Quitting.\n"));
    exit(-1);
  }
 
  /* First (and important!) wait for GSM link to be active. We allow 10
     seconds... */
  while (count++ < 200 && *GSM_LinkOK == false)
    usleep(50000);
 
  if (*GSM_LinkOK == false) {
    fprintf (stderr, _("Hmmm... GSM_LinkOK never went true. Quitting.\n"));
    exit(-1);
  }
}


/* 
	main
	simple udp-based server
 */
int main(int argc, char *argv[])
{

MG_api_request rq;
MG_rq_version version;
MG_rq_identify ident;
MG_rq_memorystatus status;
MG_rq_phonebook_location ph;
MG_rq_send_sms sms;

GSM_PhonebookEntry entry;
GSM_MemoryStatus stats;
GSM_RFUnits rf_units = GRF_Arbitrary;
GSM_BatteryUnits batt_units = GBU_Arbitrary;

GSM_MultiSMSMessage MultiSMS;
GSM_SMSMessageType SMSType=GST_SMS;
int SMSClass = -1;
bool SMSReply = false;
GSM_Coding_Type SMSCoding = GSM_Coding_Default;
GSM_UDH SMSUDHType = GSM_NoUDH;

char password[8];

char memory_type_string[20];

float rflevel=-1, batterylevel=-1;

time_t curr_t, last_t;

int s;				/* socket descriptor */
int cc;				/* contains the number of bytes read */

struct servent *sp;		/* pointer to service information */

struct sockaddr_in myaddr;	/* for local socket address */
struct sockaddr_in clientaddr;	/* for client's socket address */

extern int errno;               /* for unblocking io */  

int addrlen;
int error_cnt = 0;
int i;

/* For GNU gettext */
#ifdef USE_NLS
  textdomain("gnokii");
  setlocale(LC_ALL, "pl_PL"); //here is string for Polish localisation
#endif
 
    /* Read config file */
    if (CFG_ReadConfig(&Model, &Port, &Initlength, &Connection, &BinDir, &SynchronizeTime,false) < 0) {
        exit(-1);
    }
 
/* Handle command line arguments. */
if (argc != 2) usage();

/* get password from commandline */
strncpy(password,argv[1],8);


/* init timevars */
curr_t = last_t = time(NULL);

/* clear out address structures */
memset ((char *)&myaddr, 0, sizeof(struct sockaddr_in));
memset ((char *)&clientaddr, 0, sizeof(struct sockaddr_in));

/* Set up address structure for the socket. */
myaddr.sin_family = AF_INET;
myaddr.sin_addr.s_addr = INADDR_ANY;

sp = getservbyname ("mygnokii", "udp");
	if (sp == NULL) {
#ifdef DEBUG
		printf("mygnokii not found in /etc/services\n");
		printf("using default port 50963\n");
#endif
	 myaddr.sin_port = htons(50963);
	}
	else myaddr.sin_port = sp->s_port;


	/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket\n", argv[0]);
		exit(1);
	}

	/* Bind the server's address to the socket. */
	if (bind(s, (struct sockaddr *) &myaddr, sizeof(struct sockaddr)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address\n", argv[0]);
		exit(1);
	}

	/* Make socket unblocking */
	if ( fcntl(s, F_SETFL, O_NONBLOCK) == -1) {
		perror(argv[0]);
		printf("%s: unable to make socket unblocking\n", argv[0]);
		exit(1);
	}


	setpgrp();

	switch (fork()) {
	case -1:		/* Unable to fork, for some reason. */
		perror(argv[0]);
		printf("%s: unable to fork daemon\n", argv[0]);
		exit(1);

	case 0:	

		/* init GSM Interface */
		fbusinit(NULL);
		printf("init done, daemonizing\n");

#ifndef DEBUG
		close(0);
		close(1);
		close(2);
#endif

		for(;;) {
			addrlen = sizeof(struct sockaddr_in);

			cc = recvfrom(s, &rq, sizeof(rq), 0,
						(struct sockaddr *) &clientaddr, &addrlen);

			if ( cc == -1)
			 {
			  if ( errno != EAGAIN) exit(1);
			  else
			   {	/* no data -> do some usefull things */
				/* required for infrared connections to prevent timeouts */
				curr_t = time(NULL);
				if ( difftime(curr_t, last_t) > 5 )
				  {
    				   if (GSM->GetRFLevel(&rf_units, &rflevel) != GE_NONE)
					 error_cnt++;
				   else
					 error_cnt=0;
				   if (GSM->GetBatteryLevel(&batt_units, &batterylevel) != GE_NONE)
					 error_cnt++;
				   else
					 error_cnt=0;
				   last_t = curr_t = time(NULL);
				  }
				continue;
				}
			   }/* if( cc == -1) */

			/* don't answer if we see wrong password */
			if (strncmp(password, rq.passwd, 8) != 0) continue;

#ifdef DEBUG
	printf("server: received cmd Nr. %d\n",rq.cmd);
#endif
			switch (rq.cmd) 
				{
					case MG_INIT:	
							/* RTH FIXME: for Version 0.1, init is done at startup */
							break;
					case MG_EXIT:	
							GSM->Terminate();
							break;
					case MG_VERSION:
							version.major = MG_NET_API_MAJOR_VERSION;
							version.minor = MG_NET_API_MINOR_VERSION;
      							memcpy(rq.buffer, &version, sizeof(version));
							break;
					case MG_IDENTIFY:
							/* RTH FIXME: check errors */
							while (GSM->GetIMEI(ident.imei)    != GE_NONE) sleep(1);
							while (GSM->GetRevision(ident.rev) != GE_NONE) sleep(1);
							while (GSM->GetModel(ident.model)  != GE_NONE) sleep(1);
      							memcpy(rq.buffer, &ident, sizeof(ident));
							break;

					case MG_MEMORYSTATUS:
      							memcpy(&status, rq.buffer, sizeof(status));
#ifdef DEBUG
	printf("server: getmemorystaus for %s requested\n",status.memtype);
#endif
  							if (!GetMemoryTypeID(status.memtype, &stats.MemoryType))
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
    							if ( GSM->GetMemoryStatus( &stats) != GE_NONE)
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							status.free = stats.Free;
							status.used = stats.Used;
      							memcpy(rq.buffer, &status, sizeof(status));
							break;
					case MG_GET_MEMORY_LOCATION:
      							memcpy(&ph, rq.buffer, sizeof(ph));
#ifdef DEBUG
	printf("server: getmemorylocation for %s, Location: %d requested\n",ph.memtype,ph.location);
#endif
							ph.name[0] = ph.group[0] = ph.nr_general[0] = ph.nr_mobile[0] = ph.nr_work[0] = '\0';
							ph.nr_fax[0] = ph.nr_home[0] = ph.note[0] = ph.postal[0] = ph.email[0] = '\0';
  							if (!GetMemoryTypeID(ph.memtype, &entry.MemoryType))
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							entry.Location = ph.location;
							GetMemoryTypeString(memory_type_string, &entry.MemoryType);
    							if ( GSM->GetMemoryLocation(&entry) != GE_NONE)
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							strcpy(ph.name,entry.Name);
							strcpy(ph.nr_general,entry.Number);
							//strcpy(ph.group,NAMEOF:entry.group);
        						for( i = 0; i <  entry.SubEntriesCount; i++ )
       							{
							 switch(entry.SubEntries[i].EntryType)
							 {
							  case GSM_Number:
								switch(entry.SubEntries[i].NumberType)
								{
								 case GSM_Mobile:
									strcpy(ph.nr_mobile,entry.SubEntries[i].data.Number);
									break;
								 case GSM_Work:
									strcpy(ph.nr_work,entry.SubEntries[i].data.Number);
									break;
								 case GSM_Fax:
									strcpy(ph.nr_fax,entry.SubEntries[i].data.Number);
									break;
								 case GSM_Home:
									strcpy(ph.nr_home,entry.SubEntries[i].data.Number);
									break;
								 default:
									break;
								}
								break;
							  case GSM_Note:
								strcpy(ph.note,entry.SubEntries[i].data.Number);
								break;
							  case GSM_Postal:
								strcpy(ph.postal,entry.SubEntries[i].data.Number);
								break;
							  case GSM_Email:
								strcpy(ph.email,entry.SubEntries[i].data.Number);
								break;
							  default:
								break;
							 }
							}//for
      							memcpy(rq.buffer, &ph, sizeof(ph));
							break;
					case MG_WRITE_PHONEBOOK_LOCATION:
      							memcpy(&ph, rq.buffer, sizeof(ph));
#ifdef DEBUG
	printf("server: writememorylocation for Location: %d requested\n",ph.location);
#endif
  							if (!GetMemoryTypeID(ph.memtype, &entry.MemoryType))
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							GetMemoryTypeString(memory_type_string, &entry.MemoryType);
    							if ( GSM->GetMemoryLocation(&entry) != GE_NONE)
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							entry.Location = ph.location;
							entry.SubEntriesCount = 0;
							strcpy(entry.Name,ph.name);
							strcpy(entry.Number,ph.nr_general);
							//if ( ph.group[0] != '\0') Version 0.3 !!
							if ( ph.nr_mobile[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Number;
							 entry.SubEntries[entry.SubEntriesCount].NumberType = GSM_Mobile;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.nr_mobile);
							 entry.SubEntriesCount++;
							}
							if ( ph.nr_work[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Number;
							 entry.SubEntries[entry.SubEntriesCount].NumberType = GSM_Work;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.nr_work);
							 entry.SubEntriesCount++;
							}
							if ( ph.nr_fax[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Number;
							 entry.SubEntries[entry.SubEntriesCount].NumberType = GSM_Fax;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.nr_fax);
							 entry.SubEntriesCount++;
							}
							if ( ph.nr_home[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Number;
							 entry.SubEntries[entry.SubEntriesCount].NumberType = GSM_Home;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.nr_home);
							 entry.SubEntriesCount++;
							}
							if ( ph.note[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Note;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.note);
							 entry.SubEntriesCount++;
							}
							if ( ph.postal[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Postal;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.postal);
							 entry.SubEntriesCount++;
							}
							if ( ph.email[0]!= '\0')
							{  
							 entry.SubEntries[entry.SubEntriesCount].EntryType = GSM_Email;
						 	 strcpy(entry.SubEntries[entry.SubEntriesCount].data.Number,ph.email);
							 entry.SubEntriesCount++;
							}

							/* This is to send other exports (like from 6110) to 7110 */
							if (!entry.SubEntriesCount) {
								entry.SubEntriesCount = 1;
								entry.SubEntries[0].EntryType   = GSM_Number;
								entry.SubEntries[0].NumberType  = GSM_General;
								entry.SubEntries[0].BlockNumber = 2;
								strcpy(entry.SubEntries[0].data.Number, entry.Number);
							}

    							if (GSM->WritePhonebookLocation(&entry) != GE_NONE)
 							{
							  rq.cmd = -1; /* error */
							  break;
  							}
      							memcpy(rq.buffer, &ph, sizeof(ph));
							break;
					case MG_DELETE_PHONEBOOK_LOCATION:
      							memcpy(&ph, rq.buffer, sizeof(ph));
#ifdef DEBUG
	printf("server: deletememorylocation for Location: %d requested\n",ph.location);
#endif
  							if (!GetMemoryTypeID(ph.memtype, &entry.MemoryType))
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							GetMemoryTypeString(memory_type_string, &entry.MemoryType);
    							if ( GSM->GetMemoryLocation(&entry) != GE_NONE)
 								 {
								  rq.cmd = -1; /* error */
								  break;
  								 }
							entry.SubEntriesCount = 0;
							entry.Name[0] = '\0';
							entry.Number[0] = '\0';
							entry.Location = ph.location;
    							if (GSM->WritePhonebookLocation(&entry) != GE_NONE)
 							{
							  rq.cmd = -1; /* error */
							  break;
  							}
      							memcpy(rq.buffer, &ph, sizeof(ph));
							break;
					case MG_SEND_SMS:
      							memcpy(&sms, rq.buffer, sizeof(sms));
#ifdef DEBUG
	printf("server: sendsms to %s requested\n",sms.destination);
	printf("server: message:%s\n",sms.message);
	printf("server: Centerindex = %d\n",sms.SMSCenterIndex);
#endif
							/* options */
							if(sms.enablevoice) SMSUDHType=GSM_EnableVoice;
							if(sms.disablevoice) SMSUDHType=GSM_DisableVoice;
							if(sms.enableemail) SMSUDHType=GSM_EnableEmail;
							if(sms.disableemail) SMSUDHType=GSM_DisableEmail;
							if(sms.enablefax) SMSUDHType=GSM_EnableFax;
							if(sms.disablefax) SMSUDHType=GSM_DisableFax;
							if(sms.unicode) SMSCoding=GSM_Coding_Unicode;

							/* message */
							GSM_MakeMultiPartSMS2(&MultiSMS,sms.message,strlen(sms.message),SMSUDHType,SMSCoding);
							MultiSMS.SMS[0].Class=SMSClass;
							MultiSMS.SMS[0].ReplyViaSameSMSC=SMSReply;
							MultiSMS.SMS[0].Type=SMSType;


							/* Message Center index */
							if (sms.SMSCenterIndex == 0 ) 
    								MultiSMS.SMS[0].MessageCenter.No = 1;
							else
							   {
								if (sms.SMSCenterIndex < 0 || sms.SMSCenterIndex > 5)
 								{
							 	 rq.cmd = -2; /* error */
							   	 break;
  								}
								MultiSMS.SMS[0].MessageCenter.No = sms.SMSCenterIndex;
							   }

							/* Message Center number */
							if(strlen(sms.SMSCNumber) > 0)
								{
								 MultiSMS.SMS[0].MessageCenter.No = 0;
								 strcpy(MultiSMS.SMS[0].MessageCenter.Number,sms.SMSCNumber);
								}

							/* destination  */
							strcpy(MultiSMS.SMS[0].Destination,sms.destination);

							/* delivery report */
							if(sms.delivery)  MultiSMS.SMS[0].Type=GST_DR;

							/* validity  */
							if (sms.SMSValidity != 0)
							   MultiSMS.SMS[0].Validity=sms.SMSValidity;
							else
							   MultiSMS.SMS[0].Validity=4320; /* 3 days */

							if(  GSM->SendSMSMessage(&MultiSMS.SMS[0]) != GE_SMSSENDOK )
 							{
							  rq.cmd = -3; /* error */
							  break;
  							}

      							memcpy(rq.buffer, &sms, sizeof(sms));
							break;
					default:	
							rq.cmd = -1;
							break;

				}

			sendto (s, &rq, sizeof(rq), 0, (struct sockaddr *) &clientaddr, addrlen);
			if (rq.cmd == MG_EXIT) exit(0);
		}

	default:		/* Parent process comes here. */
		exit(0);
	}
}
