/* (c) 2007 Michal Cihar */

/**
 * @file motorola.c
 * @author Michal Čihař
 */
/**
 * @ingroup Phone
 * @{
 */
/**
 * @addtogroup ATPhone
 * @{
 */

#include <gammu-config.h>

#ifdef GSM_ENABLE_ATGEN

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "atgen.h"
#include <strings.h>
#include <string.h>
#include "../../misc/coding/coding.h"

typedef struct {
	char Command[20];
	int Mode;
} MOTOROLA_CommandInfo;

/**
 * AT Commands which can be issued only in one mode.
 *
 * List is based on c18 AT Commands document revision C.
 */
MOTOROLA_CommandInfo Commands[] = {
	{ .Command = "+CGMI", .Mode = 2},
	{ .Command = "+CGMM", .Mode = 2},
	{ .Command = "+CGMR", .Mode = 2},
	{ .Command = "+CGSN", .Mode = 2},
	{ .Command = "+CSCS", .Mode = 2},
	{ .Command = "+CIMI", .Mode = 2},
	{ .Command = "+CNUM", .Mode = 2},
	{ .Command = "+CVHU", .Mode = 2},
	{ .Command = "+CHUP", .Mode = 2},
	{ .Command = "+CIND", .Mode = 2},
	{ .Command = "+CLCK", .Mode = 2},
	{ .Command = "D", .Mode = 2},  /* We want voice call */
	{ .Command = "H", .Mode = 2},  /* We want voice call */
	{ .Command = "A", .Mode = 2},  /* We want voice call */
	{ .Command = "+CRING", .Mode = 2},
	{ .Command = "+CLIP", .Mode = 2},
	{ .Command = "+CLIR", .Mode = 2},
	{ .Command = "+CCFC", .Mode = 2},
	{ .Command = "+CHLD", .Mode = 2},
	{ .Command = "+COLP", .Mode = 2},
	{ .Command = "+CCWA", .Mode = 2},
	{ .Command = "+CLCC", .Mode = 2},
	{ .Command = "+CPBS", .Mode = 2},
	{ .Command = "+CPBR", .Mode = 2},
	{ .Command = "+CPBF", .Mode = 2},
	{ .Command = "+CPBW", .Mode = 2},
	{ .Command = "+CCLK", .Mode = 2},
	{ .Command = "+CNMI", .Mode = 2},
	{ .Command = "+CMGD", .Mode = 2},
	{ .Command = "+CMSS", .Mode = 2},
	{ .Command = "+CSMS", .Mode = 2},
	{ .Command = "+CPMS", .Mode = 2},
	{ .Command = "+CMGF", .Mode = 2},
	{ .Command = "+CSDH", .Mode = 2},
	{ .Command = "+CMTI", .Mode = 2},
	{ .Command = "+CMGL", .Mode = 2},
	{ .Command = "+CMGR", .Mode = 2},
	{ .Command = "+CMGW", .Mode = 2},
	{ .Command = "+CSCA", .Mode = 2},
	{ .Command = "+COPS", .Mode = 2},
	{ .Command = "+CBC", .Mode = 2},
	{ .Command = "+CRTT", .Mode = 2},
	{ .Command = "+CMEE", .Mode = 2},

	{ .Command = "+CHV", .Mode = 0},
	{ .Command = "+CDV", .Mode = 0},
	{ .Command = "+CPAS", .Mode = 0},
	{ .Command = "+CREG", .Mode = 0},
	{ .Command = "+CSQ", .Mode = 0},
	{ .Command = "+GCAP", .Mode = 0},
	{ .Command = "+CMUT", .Mode = 0},
	{ .Command = "+CIMSI", .Mode = 0},

	{ .Command = ""},
};

GSM_Error MOTOROLA_SetMode(GSM_StateMachine *s, const char *command)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	MOTOROLA_CommandInfo *cmd;
	const char *realcmd;
	char buffer[30];
	GSM_Error error = ERR_NONE;

	/* Do we need any mode switching? */
	if (!Priv->Mode) return ERR_NONE;

	/* We don't care about non AT commands */
	if (strncasecmp(command, "AT", 2) != 0) return ERR_NONE;

	/* Skip AT prefix */
	realcmd = command + 2;

	/* Do we have it in our list? */
	for (cmd = Commands; cmd->Command[0] != 0; cmd++) {
		if (strncasecmp(realcmd, cmd->Command, strlen(cmd->Command)) == 0) {
			break;
		}
	}

	/* Not found? */
	if (cmd->Command[0] == 0) {
		smprintf(s, "Nothing known about %s command, using current mode\n", command);
		return ERR_NONE;
	}

	/* Compare current mode */
	if (cmd->Mode == Priv->CurrentMode) {
		smprintf(s, "Already in mode %d\n", cmd->Mode);
		return ERR_NONE;
	}

	/* Switch mode */
	smprintf(s, "Switching to mode %d\n", cmd->Mode);
	sprintf(buffer, "AT+MODE=%d\r", cmd->Mode);
	error = GSM_WaitFor(s, buffer, strlen(buffer), 0x00, 3, ID_ModeSwitch);

	/* On succes we remember it */
	if (error == ERR_NONE) {
		Priv->CurrentMode = cmd->Mode;
	}

	return error;
}

#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
