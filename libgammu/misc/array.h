/**
 * \file array.h
 * \author Michal Čihař
 *
 * String arrays handling code.
 */
#ifndef __gammu_array_h
#define __gammu_array_h

#include <gammu-types.h>
#include <string.h>

typedef struct {
	/**
	 * Number of used elements.
	 */
	size_t used;
	/**
	 * Number of allocated elements.
	 */
	size_t allocated;
	/**
	 * The elements.
	 */
	char **data;
} GSM_StringArray;

/**
 * Initiates new array structures.
 */
void GSM_StringArray_New(GSM_StringArray *array);

/**
 * Frees all memory allocated by array.
 */
void GSM_StringArray_Free(GSM_StringArray *array);

/**
 * Appends string to the array.
 */
gboolean GSM_StringArray_Add(GSM_StringArray *array, const char *string);

/**
 * Checks whether string is present in array.
 */
gboolean GSM_StringArray_Find(GSM_StringArray *array, const char *string);

#endif
/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
