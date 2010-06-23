/*
   Demo/test prg for MYGNOKII net_api 
	Ralf Thelen 20.01.2002
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mg_net_api.h"

/* usage */
void usage(void)
{
    fprintf(stderr, "\n\nUsage:  mg_demo_client <server> <passwd> <command> <option 1, option 2>\n"
"   where \n"
" <server> is hostname or ip-address with running 'gnokii --server'\n"
" <passwd> is the password for the running gnokii server\n"
" <command> is a number between 0 and 5 (see below)\n"
" <option 1> is command specific (see below)\n\n\n"

"   mg_demo_client <server> <passwd> 1\n"
"command = 1(MG_EXIT) -> let server die\n\n"

"   mg_demo_client <server> <passwd> 2\n"
"command = 2	(MG_VERSION) -> ask for server version of mg_net_api\n\n"

"   mg_demo_client <server> <passwd> 3\n"
"command = 3	(MG_IDENTIFY) -> get identify-string from phone\n\n"

"   mg_demo_client <server> <passwd> 4 < SM | ME> \n"
"command = 4	(MG_MEMORYSTATUS) -> get memory status from phone\n\n"

"   mg_demo_client <server> <passwd> 5 < SM | ME> <location-nr> \n"
"command = 5	(MG_GET_MEMORY_LOCATION) -> get phonebookentry at location from phone \n\n"

"   mg_demo_client <server> <passwd> 6 < SM | ME> <location-nr> <name> <number>\n"
"command = 6	(MG_WRITE_PHONEBOOK_LOCATION) -> write phonebookentry to location at phone\n\n"

"   mg_demo_client <server> <passwd> 7 < SM | ME> <location-nr>\n"
"command = 7	(MG_DELETE_PHONEBOOK_LOCATION) -> delete phonebookentry at location at phone\n\n"

"   mg_demo_client <server> <passwd> 8 <destination> <message>\n"
"command = 8	(MG_SEND_SMS) -> send sms 'message to destionation\n\n");

    exit(1);
}


int main(argc, argv)
int argc;
char *argv[];
{
	int user_cmd;
	int ret;
	MG_rq_version version;
	MG_rq_identify ident;
	MG_rq_memorystatus status;
	MG_rq_phonebook_location ph;
	MG_rq_send_sms sms;


	if (argc < 3) {
		usage();
	}
        if ( mg_init(argv[1], argv[2])  < 0 )
	  {
		printf("init failed\n");
		exit(1);
		}

	user_cmd = atoi(argv[3]);
	switch(user_cmd)
	{
	 case MG_EXIT:
		if (argc != 4) usage();
		if ( (ret = mg_exit()) < 0)
		  printf("exit failed (%d)\n",ret);
		else
		  printf("server is exiting\n");
		break;
	 case MG_VERSION:
		if (argc != 4) usage();
		if ( (ret = mg_version(&version)) < 0)
		  printf("version failed (%d)\n",ret);
		else
		{
		  printf("Version is %d.%d\n",version.major, version.minor);
		}
		break;
	 case MG_IDENTIFY:
		if (argc != 4) usage();
		if ( (ret = mg_identify(&ident)) < 0)
		  printf("identify failed(%d)\n",ret);
		else
		{
		  printf("IMEI: %s, Model: %s, Revision: %s\n",ident.imei, ident.model, ident.rev);
		}
		break;
	 case MG_MEMORYSTATUS:
		if (argc != 5) usage();
		strcpy(status.memtype,argv[4]);
		if ( (ret = mg_memorystatus(&status)) < 0)
		  printf("status failed(%d)\n",ret);
		else
		{
		  printf("Status for %s: %d Free, %d Used\n",status.memtype,status.free,status.used);
		}
		break;
	 case MG_GET_MEMORY_LOCATION:
		if (argc != 6) usage();
		strcpy(ph.memtype,argv[4]);
		ph.location = atoi(argv[5]);
		if ( (ret = mg_get_memory_location(&ph)) < 0)
		  printf("get memory location failed(%d)\n",ret);
		else
		{
		  printf("Location Nr: %d:\n",ph.location);
		  printf("Name: %s\nGroup: %s\nnr_general: %s\nnr_mobile: %s\nnr_work: %s\nnr_fax: %s\nnr_home: %s\nnote: %s\npostal: %s\nemail: %s\n",
				ph.name,
			        ph.group,
			        ph.nr_general,
			        ph.nr_mobile,
			        ph.nr_work,
			        ph.nr_fax,
			        ph.nr_home,
			        ph.note,
			        ph.postal,
			        ph.email);
		}
		break;
	 case MG_WRITE_PHONEBOOK_LOCATION:
		if (argc != 8) usage();
		ph.group[0] = ph.nr_mobile[0] = ph.nr_work[0] = '\0';
                ph.nr_fax[0] = ph.nr_home[0] = ph.note[0] = ph.postal[0] = ph.email[0] = '\0';
		strcpy(ph.memtype,argv[4]);
		ph.location = atoi(argv[5]);
		strcpy(ph.name,argv[6]);
		strcpy(ph.nr_general,argv[7]);
		if ( (ret = mg_write_phonebook_location(&ph)) < 0)
		  printf("Write Location Nr: %d %s %s FAILED(%d)\n",ph.location,ph.name,ph.nr_general,ret);
		else
		{
		  printf("Write Location Nr: %d %s %s succeded\n",ph.location,ph.name,ph.nr_general);
		}
		break;
	 case MG_DELETE_PHONEBOOK_LOCATION:
		if (argc != 6) usage();
		strcpy(ph.memtype,argv[4]);
		ph.location = atoi(argv[5]);
		if ( (ret = mg_delete_phonebook_location(&ph)) < 0)
		  printf("Delete Location Nr: %d failed (%d)\n",ph.location,ret);
		else
		{
		  printf("Delete Location Nr: %d succeded\n",ph.location);
		}
		break;
	 case MG_SEND_SMS:
		if (argc != 6) usage();
		strcpy(sms.destination,argv[4]);
		strcpy(sms.message,argv[5]);
		/* defaults */
		sms.SMSCNumber[0] = '\0';                   /* SMSC number */
        	sms.SMSCenterIndex = 0;                          /* SMSC number index in phone memory */
        	sms.SMSValidity= 0;                               /* validity of sms */
        	sms.enablevoice=FALSE;                               /* SMS enables voice indicator */
        	sms.disablevoice=FALSE;                              /* SMS disables voice indicator */
        	sms.enableemail=FALSE;                               /* SMS enables email indicator */
        	sms.disableemail=FALSE;                              /* SMS disables email indicator */
        	sms.enablefax=FALSE;                                 /* SMS enables fax indicator */
        	sms.disablefax=FALSE;                                /* SMS disables fax indicator */
        	sms.unicode=FALSE;                                   /* SMS coding type */
        	sms.delivery=FALSE;
		if ( (ret = mg_send_sms(&sms)) < 0)
		  printf("send sms failed \n(%d)",ret);
		else
		{
		  printf("send sms succeded \n");
		}
		break;
	 default:
		printf("unknown command\n");
		usage();
	}

return(0);
}
