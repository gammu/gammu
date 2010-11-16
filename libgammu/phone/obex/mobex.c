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

GSM_Error MOBEX_CreateEntry(GSM_StateMachine *s, const char *path, int *location, const char *data)
{
	GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	char appdata[] = {'\x01'};

	Priv->m_obex_newid = -1;
	Priv->m_obex_appdata = appdata;
	Priv->m_obex_appdata_len = 1;

	error = OBEXGEN_SetFile(s, path, data, strlen(data), FALSE);

	Priv->m_obex_appdata = NULL;
	Priv->m_obex_appdata_len = 0;

	if (error != ERR_NONE) {
		return error;
	}

	*location = Priv->m_obex_newid;

	return ERR_NONE;
}

GSM_Error MOBEX_UpdateEntry(GSM_StateMachine *s, const char *path, const int location, const char *data)
{
	GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	char appdata[] = {'\x01', 0, 0};

	appdata[1] = (location  && 0xff00) >> 8;
	appdata[2] = (location  && 0xff);

	Priv->m_obex_appdata = appdata;
	Priv->m_obex_appdata_len = 3;

	error = OBEXGEN_SetFile(s, path, data, strlen(data), FALSE);

	Priv->m_obex_appdata = NULL;
	Priv->m_obex_appdata_len = 0;

	if (error != ERR_NONE) {
		return error;
	}

	return ERR_NONE;
}

GSM_Error MOBEX_GetEntry(GSM_StateMachine *s, const char *path, const int location, char **data)
{
	GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	char appdata[] = {'\x01', 0, 0};

	appdata[1] = (location  && 0xff00) >> 8;
	appdata[2] = (location  && 0xff);

	Priv->m_obex_appdata = appdata;
	Priv->m_obex_appdata_len = 3;

	error = OBEXGEN_GetTextFile(s, path, data);

	Priv->m_obex_appdata = NULL;
	Priv->m_obex_appdata_len = 0;

	if (error != ERR_NONE) {
		return error;
	}

	return ERR_NONE;
}

GSM_Error MOBEX_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry)
{
	GSM_Error error;
	char *data = NULL;
	size_t pos = 0;


	error = MOBEX_GetEntry(s, "m-obex/contacts/read", Entry->Location, &data);
	if (error != ERR_NONE) {
		free(data);
		return error;
	}

	error = GSM_DecodeVCARD(&(s->di), data, &pos, Entry, SonyEricsson_VCard21_Phone);
	free(data);
	data = NULL;
	if (error != ERR_NONE) {
		return error;
	}

	return ERR_NONE;
}

GSM_Error MOBEX_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry)
{
	GSM_Error error;
	char *data = NULL;
	size_t pos = 0;
	GSM_ToDoEntry	ToDo;


	error = MOBEX_GetEntry(s, "m-obex/calendar/read", Entry->Location, &data);
	if (error != ERR_NONE) {
		free(data);
		return error;
	}

	error = GSM_DecodeVCALENDAR_VTODO(&(s->di), data, &pos, Entry, &ToDo, SonyEricsson_VCalendar, SonyEricsson_VToDo);
	free(data);
	data = NULL;
	if (error != ERR_NONE) {
		return error;
	}

	return ERR_NONE;
}

GSM_Error MOBEX_GetNextEntry(GSM_StateMachine *s, const char *path, const gboolean start, int *nextid, int *nexterror, unsigned char **data, int *pos, int *size, char **entry, int *location)
{
	GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	char appdata[] = {'\x01', 0, 0};

	if (start) {
		*nextid = 0;
		*nexterror = 2;
		free(*data);
		*data = NULL;
		*pos = 0;
		*size = 0;
	} else {
		*nextid += 1;
	}

	/* Increment to next */
	if (*data != NULL) {
		*pos += (*data[*pos + 2] << 8) + *data[*pos + 3];
	}

	/* Need to fetch new data */
	if (*pos >= *size) {
		if (*nexterror == 0) {
			return ERR_EMPTY;
		}
		appdata[1] = (*nextid && 0xff00) >> 8;
		appdata[2] = (*nextid && 0xff);

		Priv->m_obex_appdata = appdata;
		Priv->m_obex_appdata_len = 3;

		error = OBEXGEN_GetBinaryFile(s, path, data, size);

		Priv->m_obex_appdata = NULL;
		Priv->m_obex_appdata_len = 0;

		*nexterror = Priv->m_obex_error;

		if (error != ERR_NONE) {
			return error;
		}
	}

	/* Nothing to return */
	if (*pos + 4 > *size) {
		return ERR_EMPTY;
	}

	/* Return values */
	*entry = *data + *pos + 4;
	*location = (*data[*pos + 0] << 8) + *data[*pos + 1];

	return ERR_NONE;
}

GSM_Error MOBEX_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry, gboolean start)
{
	GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	char *data = NULL;
	size_t pos = 0;


	error = MOBEX_GetNextEntry(s, "m-obex/contacts/load", start, &Priv->m_obex_contacts_nextid, &Priv->m_obex_contacts_nexterror, &Priv->m_obex_contacts_buffer, &Priv->m_obex_contacts_buffer_pos, &Priv->m_obex_contacts_buffer_size, &data, &(Entry->Location));
	if (error != ERR_NONE) {
		return error;
	}

	error = GSM_DecodeVCARD(&(s->di), data, &pos, Entry, SonyEricsson_VCard21_Phone);
	if (error != ERR_NONE) {
		return error;
	}

	return ERR_NONE;
}

GSM_Error MOBEX_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry, gboolean start)
{
	GSM_Error error;
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	char *data = NULL;
	size_t pos = 0;
	GSM_ToDoEntry	ToDo;

	error = MOBEX_GetNextEntry(s, "m-obex/calendar/load", start, &Priv->m_obex_calendar_nextid, &Priv->m_obex_calendar_nexterror, &Priv->m_obex_calendar_buffer, &Priv->m_obex_calendar_buffer_pos, &Priv->m_obex_calendar_buffer_size, &data, &(Entry->Location));
	if (error != ERR_NONE) {
		return error;
	}

	error = GSM_DecodeVCALENDAR_VTODO(&(s->di), data, &pos, Entry, &ToDo, SonyEricsson_VCalendar, SonyEricsson_VToDo);
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
