/*
 * This file part of smscgi
 *
 * Copyright (C) 2007  Kamanashis Roy (kamanashisroy@gmail.com)
 *
 * smscgi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * smscgi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with smscgi.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "sms_cgi.h"

static GSM_StateMachine *s;
static INI_Section *cfg;

/* Function to handle errors */
void error_handler()
{
	if (error != ERR_NONE) {
		printf("ERROR: %s\n", GSM_ErrorString(error));
		if (GSM_IsConnected(s))
			GSM_TerminateConnection(s);
		exit(error);
	}
}


int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	const char*tmp;

	/* Enable global debugging to stderr */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL)
		return 3;

	/* Enable state machine debugging to stderr */
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(FALSE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Find configuration file */
	error = GSM_FindGammuRC(&cfg);
	error_handler();

	/* Read it */
	error = GSM_ReadConfig(cfg, GSM_GetConfig(s, 0), 0);
	error_handler();
	
	/* We have one valid configuration */
	GSM_SetConfigNum(s, 1);
	
	
	/* ---------------------------------------- get cgi script search directory */
	tmp = (const char*)INI_GetValue(cfg, (const char*)cfg->SectionName, "cgi-bin", 0);
	if(!tmp) {
		printf("could not get the cgi_path, try to add \"cgi-bin=/your/cgi-bin/path\" in .gammurc file \n");
		return -1;
	}
	strcpy(cgi_path, tmp);
	smprintf_level(s, D_TEXT, "CGI search path : %s\n", cgi_path);
	strcat(cgi_path, "/");

	error = -1;
	while(1) {
		
		/* ---------------------------------------------------- continuously read */
		if(error == ERR_NONE) {
			GSM_ReadDevice(s, TRUE);
			cgi_process(s);
			continue; /* go on */
		}
		
		/* ------------------------------------------ when error close connection */
		sleep(3); /* wait 30 seconds before retry */
		GSM_TerminateConnection(s); /* we do not care if shutdown fails */
		cgi_reset();
		
		/* ------------------------------------------------------------ reconnect */
		sleep(3); /* wait 30 seconds before retry */
		/* try to reconnect */
		error = GSM_InitConnection(s, 3);
		if(error != ERR_NONE) {
			continue; /* retry */
		}
		
		
		/* ---------------------------------------------- enable sms notification */
		GSM_SetIncomingSMS(s, 1	);
		if(error != ERR_NONE) {
			continue; /* retry */
		}
		
		/* ---------------------------------------- set the SMS callback function */
		GSM_SetIncomingSMSCallback(s, cgi_enqueue);
		smprintf_level(s, D_TEXT, "registered sms callback\n");
	}

	/* Terminate connection */
	error = GSM_TerminateConnection(s);
	error_handler();
	return 0;
}


/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
