/**
 * \file gammu-category.h
 * \author Michal Čihař
 *
 * Categories handling.
 */
#ifndef __gammu_category_h
#define __gammu_category_h

/**
 * \defgroup Category Category
 * Categories handling.
 */

#include <gammu-limits.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>

/**
 * Type of category
 *
 * \ingroup Category
 */
typedef enum {
	/**
	 * Todo entry category
	 */
	Category_ToDo = 1,
	/**
	 * Phonebook entry category
	 */
	Category_Phonebook
} GSM_CategoryType;

/**
 * Category entry.
 * \ingroup Category
 */
typedef struct {
	/**
	 * Type of category
	 */
	GSM_CategoryType Type;
	/**
	 * Location of category
	 */
	int Location;
	/**
	 * Name of category
	 */
	unsigned char Name[(GSM_MAX_CATEGORY_NAME_LENGTH + 1) * 2];
} GSM_Category;

/**
 * Status of categories.
 * \ingroup Category
 */
typedef struct {
	/**
	 * Type of category.
	 */
	GSM_CategoryType Type;
	/**
	 * Number of used category names.
	 */
	int Used;
} GSM_CategoryStatus;

/**
 * Reads category from phone.
 *
 * \param s State machine pointer.
 * \param Category Storage for category, containing its type and location.
 *
 * \return Error code
 *
 * \ingroup Category
 */
GSM_Error GSM_GetCategory(GSM_StateMachine * s, GSM_Category * Category);

/**
 * Adds category to phone.
 *
 * \param s State machine pointer.
 * \param Category New category, containing its type and location.
 *
 * \return Error code
 *
 * \ingroup Category
 */
GSM_Error GSM_AddCategory(GSM_StateMachine * s, GSM_Category * Category);

/**
 * Reads category status (number of used entries) from phone.
 *
 * \param s State machine pointer.
 * \param Status Category status, fill in type before calling.
 *
 * \return Error code
 *
 * \ingroup Category
 */
GSM_Error GSM_GetCategoryStatus(GSM_StateMachine * s,
				GSM_CategoryStatus * Status);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
