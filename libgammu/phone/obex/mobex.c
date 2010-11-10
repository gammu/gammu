/* (c) 2010 by Michal Cihar */

/**
 * \file obexgen.c
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * \defgroup MOBEXPhone m-obex phones communication
 * Implementation of m-obex protocol used by Samsung phones.
 *
 * @author Michal Cihar
 * @{
 */

#include <gammu-config.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../../gsmphones.h"
#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"
#include "../../protocol/obex/obex.h"
#include "obexfunc.h"
#include "obexgen.h"
#include "mobex.h"

#include <string.h>

#ifdef GSM_ENABLE_OBEXGEN

/**
 * How many read attempts will happen.
 */
#define MOBEX_TIMEOUT 10

GSM_Error MOBEX_GetStatus(GSM_StateMachine *s, const char *path, int *free_records, int *used)
{
    GSM_Error error;
    unsigned char *buffer = NULL;
    int len = 0, total;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
    char appdata[] = {'\x01'};

    Priv->m_obex_appdata = appdata;
    Priv->m_obex_appdata_len = 1;

    error = OBEXGEN_GetBinaryFile(s, path, &buffer, &len);

    Priv->m_obex_appdata = NULL;
    Priv->m_obex_appdata_len = 0;

    if (error != ERR_NONE) {
        free(buffer);
        return error;
    }

    if (len < 2) {
        smprintf(s, "Unknown length of data file: %d\n", len);
        free(buffer);
        return ERR_UNKNOWNRESPONSE;
    }

    total = (buffer[0] << 8) + buffer[1];

    *used = (buffer[1] << 8) + buffer[3];
    *free_records = total - *used;

    free(buffer);
    return ERR_NONE;
}

GSM_Error MOBEX_CreateEntry(GSM_StateMachine *s, const char *path, const char *data)
{
    GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
    char appdata[] = {'\x01'};

    Priv->m_obex_appdata = appdata;
    Priv->m_obex_appdata_len = 1;

    error = OBEXGEN_SetFile(s, path, data, strlen(data), FALSE);

    Priv->m_obex_appdata = NULL;
    Priv->m_obex_appdata_len = 0;

    if (error != ERR_NONE) {
        return error;
    }

    return ERR_NONE;
}

#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
