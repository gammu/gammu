/**
 * \file array.h
 * \author Michal Čihař
 *
 * String arrays handling code.
 */

#include "array.h"

#include <stdlib.h>
#include <string.h>

void GSM_StringArray_New(GSM_StringArray *array)
{
	array->used = 0;
	array->allocated = 0;
	array->data = NULL;
}

void GSM_StringArray_Free(GSM_StringArray *array)
{
	size_t i;
	for (i = 0; i < array->used; i++) {
		free(array->data[i]);
	}
	free(array->data);
	GSM_StringArray_New(array);
}

gboolean GSM_StringArray_Add(GSM_StringArray *array, const char *string)
{
	char **newdata;

	/* Allocate extra space if needed */
	if (array->used + 1 > array->allocated) {
		newdata = realloc(array->data, (array->allocated + 10) * sizeof(char **));
		if (newdata == NULL) return FALSE;
		array->allocated += 10;
		array->data = newdata;
	}

	array->data[array->used] = strdup(string);
	if (array->data[array->used] == NULL) return FALSE;

	array->used++;

	return TRUE;
}

gboolean GSM_StringArray_Find(GSM_StringArray *array, const char *string)
{
	size_t i;
	for (i = 0; i < array->used; i++) {
		if (strcmp(array->data[i], string) == 0) return TRUE;
	}
	return FALSE;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

