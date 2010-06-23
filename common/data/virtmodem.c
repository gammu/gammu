/*
  $Id: virtmodem.c,v 1.4 2001/04/14 23:23:43 pkot Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides a virtual modem interface to the GSM phone by calling
  code in gsm-api.c, at-emulator.c and datapump.c. The code here provides
  the overall framework and coordinates switching between command mode
  (AT-emulator) and "online" mode where the data pump code translates data
  from/to the GSM handset and the modem data/fax stream.

*/

#define		__virtmodem_c

#include <config.h>

/* This is the right way to include stdlib with __USE_XOPEN defined */
#ifdef USE_UNIX98PTYS
# define _XOPEN_SOURCE 500
# include <features.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <grp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <pthread.h>
#include <unistd.h>

#include "misc.h"
#include "gsm-api.h"
#include "gsm-common.h"
#include "data/at-emulator.h"
#include "data/datapump.h"
#include "data/virtmodem.h"
#include "data/rlp-common.h"

	/* Global variables */

//extern bool TerminateThread;
int ConnectCount;

	/* Local variables */

int		PtyRDFD;	/* File descriptor for reading and writing to/from */
int		PtyWRFD;	/* pty interface - only different in debug mode. */ 

bool	UseSTDIO;	/* Use STDIO for debugging purposes instead of pty */
bool	CommandMode;

pthread_t		Thread;
bool			RequestTerminate;


	/* If initialised in debug mode, stdin/out is used instead
	   of ptys for interface. */
bool	VM_Initialise(char *model,char *port, char *initlength, GSM_ConnectionType connection, char *bindir, bool debug_mode, bool GSMInit,char *synchronizetime)
{
	int		rtn;

	CommandMode = true;

	RequestTerminate = false;

	if (debug_mode == true) {
		UseSTDIO = true;
	}
	else {
		UseSTDIO = false;
	}

	if (GSMInit) {
#ifdef DEBUG
	  fprintf (stderr , "Initialising GSM\n");
#endif /* DEBUG */
	  if ((VM_GSMInitialise(model, port, initlength, connection, synchronizetime) != GE_NONE)) {
		fprintf (stderr, _("VM_Initialise - VM_GSMInitialise failed!\n"));
		return (false);
		
	  }
	}
	GSMInit=false;

	if (VM_PtySetup(bindir) < 0) {
		fprintf (stderr, _("VM_Initialise - VM_PtySetup failed!\n"));
		return (false);
	}
    
	if (ATEM_Initialise(PtyRDFD, PtyWRFD, model, port) != true) {
		fprintf (stderr, _("VM_Initialise - ATEM_Initialise failed!\n"));
		return (false);
	}

	if (DP_Initialise(PtyRDFD, PtyWRFD) != true) {
		fprintf (stderr, _("VM_Initialise - DP_Initialise failed!\n"));
		return (false);
	}

		/* Create and start thread, */
	rtn = pthread_create(&Thread, NULL, (void *) VM_ThreadLoop, (void *)NULL);

    if (rtn == EAGAIN || rtn == EINVAL) {
        return (false);
    }
	return (true);
}

void	VM_ThreadLoop(void)
{
	int res;
	struct pollfd ufds;

		/* Note we can't use signals here as they are already used
		   in the FBUS code.  This may warrant changing the FBUS
		   code around sometime to use select instead to free up
		   the SIGIO handler for mainline code. */

	ufds.fd=PtyRDFD;
	ufds.events=POLLIN;

	while (!RequestTerminate) {
	  if (!CommandMode) {
	    sleep(1);
	  } else {  /* If we are in data mode, leave it to datapump to get the data */

		res=poll(&ufds,1,500);

		switch (res) {
		        case 0: /* Timeout */
				break;

			case -1:
				perror("VM_ThreadLoop - select");
				exit (-1);

			default:
			  if (ufds.revents==POLLIN) {
			    VM_CharHandler();
			  } else usleep(500); /* Probably the file has been closed */
			  break;
		}
	  }
	}
	
}

	/* Application should call VM_Terminate to shut down
	   the virtual modem thread */
void		VM_Terminate(void)
{
     
	/* Request termination of thread */
	RequestTerminate = true;

	/* Now wait for thread to terminate. */
	pthread_join(Thread, NULL);

	if (!UseSTDIO) {
		close (PtyRDFD);
		close (PtyWRFD);
	}
}

	/* Open pseudo tty interface and (in due course create a symlink
	   to be /dev/gnokii etc. ) */

int		VM_PtySetup(char *bindir)
{
	int			err;
	char		mgnokiidev[200];
	char		*slave_name;
	char		cmdline[200];

	if (bindir) {
		strncpy(mgnokiidev, bindir, 200);
		strcat(mgnokiidev, "/");
	}
	strncat(mgnokiidev, "mgnokiidev", 200 - strlen(bindir));

	if (UseSTDIO) {
		PtyRDFD = STDIN_FILENO;
		PtyWRFD = STDOUT_FILENO;
		return (0);
	}
	
	PtyRDFD = VM_GetMasterPty(&slave_name);
	if (PtyRDFD < 0) {
		fprintf (stderr, _("Couldn't open pty!\n"));
		return(-1);
	}
	PtyWRFD = PtyRDFD;

		/* Check we haven't been installed setuid root for some reason
		   if so, don't create /dev/gnokii */
	if (getuid() != geteuid()) {
		fprintf(stderr, _("gnokiid should not be installed setuid root!\n"));
		return (0);
	}

#ifdef DEBUG
	fprintf (stderr, _("Slave pty is %s, calling %s to create /dev/gnokii.\n"), slave_name, mgnokiidev);
#endif /* DEBUG */

		/* Create command line, something line ./mkgnokiidev ttyp0 */
	sprintf(cmdline, "%s %s", mgnokiidev, slave_name);

		/* And use system to call it. */	
	err = system (cmdline);
	
	return (err);

}

    /* Handler called when characters received from serial port.
       calls state machine code to process it. */
void    VM_CharHandler(void)
{
    unsigned char   buffer[255];
    int             res;


    /* If we are in command mode, get the character, otherwise leave it */

    if (CommandMode && ATEM_Initialised) {
      
      res = read(PtyRDFD, buffer, 255);
      
      /* A returned value of -1 means something serious has gone wrong - so quit!! */
      /* Note that file closure etc. should have been dealt with in ThreadLoop */
      
      if (res < 0) {	
//	    TerminateThread=true;
	    return;
      }
	
      ATEM_HandleIncomingData(buffer, res);
    }	
}     

	/* Initialise GSM interface, returning GSM_Error as appropriate  */
GSM_Error 	VM_GSMInitialise(char *model, char *port, char *initlength, GSM_ConnectionType connection, char *synchronizetime)
{
	int 		count=0;
	GSM_Error 	error;

		/* Initialise the code for the GSM interface. */     

	error = GSM_Initialise(model,port, initlength, connection, RLP_DisplayF96Frame, synchronizetime);

	if (error != GE_NONE) {
		fprintf(stderr, _("GSM/FBUS init failed! (Unknown model ?). Quitting.\n"));
    	return (error);
	}

		/* First (and important!) wait for GSM link to be active. We allow 10
		   seconds... */

	while (count++ < 200 && *GSM_LinkOK == false) {
    	usleep(50000);
	}

	if (*GSM_LinkOK == false) {
		fprintf (stderr, _("Hmmm... GSM_LinkOK never went true. Quitting. \n"));
   		return (GE_NOLINK); 
	}

	return (GE_NONE);
}

/* VM_GetMasterPty()
   Takes a double-indirect character pointer in which to put a slave
   name, and returns an integer file descriptor.  If it returns < 0, an
   error has occurred.  Otherwise, it has returned the master pty
   file descriptor, and fills in *name with the name of the
   corresponding slave pty.  Once the slave pty has been opened,
   you are responsible to free *name.  Code is from Developling Linux
   Applications by Troan and Johnson */


int	VM_GetMasterPty(char **name) { 

#ifdef USE_UNIX98PTYS
	int master, err;

	master = open("/dev/ptmx", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (master >= 0) {
		err = grantpt(master);
		err = err || unlockpt(master);
		if (!err) {
			*name = ptsname(master);
		} else {
			return(-1);
		}
	}
#else /* USE_UNIX98PTYS */
   int i = 0 , j = 0;
   /* default to returning error */
   int master = -1;

   /* create a dummy name to fill in */
   *name = strdup("/dev/ptyXX");

   /* search for an unused pty */
   for (i=0; i<16 && master <= 0; i++) {
      for (j=0; j<16 && master <= 0; j++) {
         (*name)[8] = "pqrstuvwxyzPQRST"[i];
         (*name)[9] = "0123456789abcdef"[j];
         /* open the master pty */
         if ((master = open(*name, O_RDWR | O_NOCTTY | O_NONBLOCK )) < 0) {
            if (errno == ENOENT) {
               /* we are out of pty devices */
               free (*name);
               return (master);
            }
         }
      }
   }
   if ((master < 0) && (i == 16) && (j == 16)) {
      /* must have tried every pty unsuccessfully */
      free (*name);
      return (master);
   }

   /* By substituting a letter, we change the master pty
    * name into the slave pty name.
    */
   (*name)[5] = 't';

#endif /* USE_UNIX98PTYS */

   return (master);
}

