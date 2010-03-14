/*
   MYGNOKII net_api 
   Version 0.03 07.02.2002
   <ralf@mythelen.de>
	*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include "mg_net_api.h"


/* globals */
static int mg_s;				/* socket descriptor */
static struct sockaddr_in mg_myaddr_in;		/* for local socket address */
static struct sockaddr_in mg_servaddr_in;	/* for server socket address */
static char mg_password[8];			/* passwd for udp-requsts */

/*
 *                      H A N D L E R
 *
 *      This routine is the signal handler for the alarm signal.
 *      It simply re-installs itself as the handler and returns.
 */
void handler()
{
        signal(SIGALRM, handler);
}

/* Send the request to the server. */
/* and wait for response           */
static int send_rq(MG_api_request *rq)
{
int ret;

strncpy(rq->passwd,mg_password,8);

if (sendto (mg_s, rq, sizeof(*rq), 0, (struct sockaddr *) &mg_servaddr_in,
			sizeof(struct sockaddr_in)) == -1) {
	return(-1);
	}


  /* Set up alarm signal handler. */
signal(SIGALRM, handler);

  /* Set up a timeout so I don't hang in case the packet
   * gets lost.  After all, UDP does not guarantee
   * delivery.
   */
alarm(5);

  /* Wait for the reply to come in.  We assume that
   * no messages will come from any other source,
   * so that we do not need to do a recvfrom nor
   * check the responder's address.
   */	

/* RTH FIXME: TIMEOUT DONT WORK !! */
ret = recv (mg_s, rq, sizeof(*rq), 0);
switch (ret) {
	case EINTR:
         /* Alarm went off and aborted the receive. */
         alarm(0);
	 return(MG_ERROR_TIMEOUT);
	 break;

	default:
         alarm(0);
	 return(ret);
	 break;
	}
}

int mg_exit(void)
{
int ret;
MG_api_request rq;

rq.cmd = MG_EXIT;

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else return(rq.cmd);

}

int mg_version(MG_rq_version *version)
{
int ret;
MG_api_request rq;

rq.cmd = MG_VERSION;

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(version, rq.buffer, sizeof(*version));
	return(rq.cmd);
	}
}

int mg_identify(MG_rq_identify *ident)
{
int ret;
MG_api_request rq;

rq.cmd = MG_IDENTIFY;

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(ident, rq.buffer, sizeof(*ident));
	return(rq.cmd);
	}
}

int mg_memorystatus(MG_rq_memorystatus *stats)
{
int ret;
MG_api_request rq;

rq.cmd = MG_MEMORYSTATUS;
memcpy(rq.buffer, stats, sizeof(*stats));

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(stats, rq.buffer, sizeof(*stats));
	return(rq.cmd);
	}
}

int mg_get_memory_location(MG_rq_phonebook_location *rph)
{
int ret;
MG_api_request rq;

rq.cmd = MG_GET_MEMORY_LOCATION;
memcpy(rq.buffer, rph, sizeof(*rph));

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(rph, rq.buffer, sizeof(*rph));
	return(rq.cmd);
	}
}

int mg_write_phonebook_location(MG_rq_phonebook_location *wph)
{
int ret;
MG_api_request rq;

rq.cmd = MG_WRITE_PHONEBOOK_LOCATION;
memcpy(rq.buffer, wph, sizeof(*wph));

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(wph, rq.buffer, sizeof(*wph));
	return(rq.cmd);
	}
}

int mg_delete_phonebook_location(MG_rq_phonebook_location *dph)
{
int ret;
MG_api_request rq;

rq.cmd = MG_DELETE_PHONEBOOK_LOCATION;
memcpy(rq.buffer, dph, sizeof(*dph));

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(dph, rq.buffer, sizeof(*dph));
	return(rq.cmd);
	}
}

int mg_send_sms(MG_rq_send_sms *sms)
{
int ret;
MG_api_request rq;

rq.cmd = MG_SEND_SMS;
memcpy(rq.buffer, sms, sizeof(*sms));

ret = send_rq(&rq);

if (ret < 0) 
	return(ret);
else
	{
	memcpy(sms, rq.buffer, sizeof(*sms));
	return(rq.cmd);
	}
}


/*
	mg_init
	Input: Hostname of mygnokii-server
	Output: connected udp-port on success, else -1 
*/
int mg_init(char *hostname, char *passwd)
{

struct hostent *hp;		/* pointer to host info for nameserver host */
struct servent *sp;		/* pointer to service information */

/* set password */
strncpy(mg_password,passwd,8);  

/* clear out address structures */
memset ((char *)&mg_myaddr_in, 0, sizeof(struct sockaddr_in));
memset ((char *)&mg_servaddr_in, 0, sizeof(struct sockaddr_in));

/* Set up the server address. */
mg_servaddr_in.sin_family = AF_INET;

/* Get the host information for the server's hostname that the
 * user passed in.
 */
hp = gethostbyname (hostname);
if (hp == NULL) {
	 return(1);
	}

mg_servaddr_in.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
/* Find the information for the "example" server
 * in order to get the needed port number.
 */
sp = getservbyname ("mygnokii", "udp");
if (sp == NULL) {
       /* not found -> use default port 50963 */
	mg_servaddr_in.sin_port = htons(50963);
	}
else mg_servaddr_in.sin_port = sp->s_port;
		
/* Create the socket. */
mg_s = socket (AF_INET, SOCK_DGRAM, 0);
if (mg_s == -1) {
	 return(-1);
	}

	/* Bind socket to some local address so that the
	 * server can send the reply back.  A port number
	 * of zero will be used so that the system will
	 * assign any available port number.  An address
	 * of INADDR_ANY will be used so we do not have to
	 * look up the internet address of the local host.
	 */
mg_myaddr_in.sin_family = AF_INET;
mg_myaddr_in.sin_port = 0;
mg_myaddr_in.sin_addr.s_addr = INADDR_ANY;
if (bind(mg_s, (struct sockaddr *) &mg_myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		return(-1);
	}

/* return portnumber */
return(mg_servaddr_in.sin_port);

}

