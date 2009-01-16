/* © 2007 - 2009 Michal Čihař */

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
	{"+CGMI", 2},
	{"+CGMM", 2},
	{"+CGMR", 2},
	{"+CGSN", 2},
	{"+CSCS", 2},
	{"+CIMI", 2},
	{"+CNUM", 2},
	{"+CVHU", 2},
	{"+CHUP", 2},
	{"+CIND", 2},
	{"+CLCK", 2},
	{"D", 2},  /* We want voice call */
	{"H", 2},  /* We want voice call */
	{"A", 2},  /* We want voice call */
	{"+CRING", 2},
	{"+CLIP", 2},
	{"+CLIR", 2},
	{"+CCFC", 2},
	{"+CHLD", 2},
	{"+COLP", 2},
	{"+CCWA", 2},
	{"+CLCC", 2},
	{"+CPBS", 2},
	{"+CPBR", 2},
	{"+CPBF", 2},
	{"+CPBW", 2},
	{"+CCLK", 2},
	{"+CNMI", 2},
	{"+CMGD", 2},
	{"+CMSS", 2},
	{"+CSMS", 2},
	{"+CPMS", 2},
	{"+CMGF", 2},
	{"+CSDH", 2},
	{"+CMTI", 2},
	{"+CMGL", 2},
	{"+CMGR", 2},
	{"+CMGW", 2},
	{"+CSCA", 2},
	{"+COPS", 2},
	{"+CBC", 2},
	{"+CRTT", 2},
	{"+CMEE", 2},
	{"+CKPD", 2},

	{"+CHV", 0},
	{"+CDV", 0},
	{"+CPAS", 0},
	{"+CREG", 0},
	{"+CSQ", 0},
	{"+GCAP", 0},
	{"+CMUT", 0},
	{"+CIMSI", 0},

	{"", 0},
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
	error = GSM_WaitFor(s, buffer, strlen(buffer), 0x00, 20, ID_ModeSwitch);

	/* On succes we remember it */
	if (error == ERR_NONE) {
		/* We might need to restore charset as phone resets it */
		if (cmd->Mode == 2) {
			smprintf(s, "Waiting for banner...\n");

			/* Wait for banner */
			error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 40);
			if (error != ERR_NONE) return error;

			/* Check for banner result */
			if (Priv->CurrentMode != 2) {
				smprintf(s, "Failed to set mode 2!\n");
				return ERR_BUG;
			}

			/* Now we can work with phone */
			error = ATGEN_SetCharset(s, AT_PREF_CHARSET_RESET);
		} else {
			Priv->CurrentMode = cmd->Mode;
		}
	}

	return error;
}

/**
 * Catches +MBAN: reply.
 */
GSM_Error MOTOROLA_Banner(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	s->Phone.Data.Priv.ATGEN.CurrentMode = 2;
	return ERR_NONE;
}

#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
