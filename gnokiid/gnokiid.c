/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Mainline code for gnokiid daemon. Handles command line parsing and
  various daemon functions.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

#include "misc.h"
#include "files/cfgreader.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "data/virtmodem.h"

/* Global variables */
bool		DebugMode;	/* When true, run in debug mode */
char		*Model;		/* Model from .gnokiirc file. */
char		*Port;		/* Port from .gnokiirc file */
char		*Initlength;	/* Init length from .gnokiirc file */
char		*Connection;	/* Connection type from .gnokiirc file */
char            *SynchronizeTime;
char		*BinDir;	/* Directory of the mgnokiidev command */

bool  TerminateThread;

void version(void)
{

  fprintf(stdout, _("gnokiid Version %s\n"
"Copyright (C) Hugh Blemings <hugh@linuxcare.com>, 1999\n"
"Copyright (C) Pavel Janík ml. <Pavel.Janik@linux.cz>, 1999\n"
"Built %s %s for %s on %s \n"), VERSION, __TIME__, __DATE__, Model, Port);
}

/* The function usage is only informative - it prints this program's usage and
   command-line options.*/

void usage(void)
{

  fprintf(stdout, _("   usage: gnokiid {--help|--version}\n"
"          --help            display usage information."
"          --version         displays version and copyright information."
"          --debug           uses stdin/stdout for virtual modem comms.\n"));
}

/* Main function - handles command line arguments, passes them to separate
   functions accordingly. */

int main(int argc, char *argv[])
{

    GSM_ConnectionType connection;

		/* For GNU gettext */

	#ifdef USE_NLS
  		textdomain("gnokii");
	#endif

#ifndef WIN32
	if (strcmp(GetMygnokiiVersion(),VERSION)!=0)
	  fprintf(stderr,_("WARNING: version of installed libmygnokii.so (%s) is different to version of gnokiid (%s)\n"),GetMygnokiiVersion(),VERSION);
#endif

        if (CFG_ReadConfig(&Model, &Port, &Initlength, &Connection, &BinDir, &SynchronizeTime,true) < 0) {
                exit(-1);
        }

  		/* Handle command line arguments. */

	if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
		usage();
    	exit(0);
  	}

		/* Display version, copyright and build information. */

	if (argc >= 2 && strcmp(argv[1], "--version") == 0) {
    	version();
	    exit(0);
	}

	if (argc >= 2 && strcmp(argv[1], "--debug") == 0) {
		DebugMode = true;	
	}
	else {
		DebugMode = false;	
	}

        connection=GetConnectionTypeFromString(Connection);
	
	/* MBUS wasn't tested */
	if (connection!=GCT_FBUS && connection!=GCT_MBUS &&
	    connection!=GCT_Infrared && connection!=GCT_Tekram) {
	  fprintf(stdout,_("Incorrect connection type!\n"));    
	  exit(0);
	}
	
	TerminateThread=false;

	if (VM_Initialise(Model, Port, Initlength, connection, BinDir, DebugMode, true, SynchronizeTime) == false) 
	  exit (-1);

	while (1) {
	  if (TerminateThread==true) {
	    VM_Terminate();
	    exit(1);
	  }
	  sleep (1);
	}
	exit (0);
}
