/**
 * \info gammu-category.h
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

#define GSM_MAX_CATEGORY_NAME_LENGTH 60

/**
 * Type of category
 * \ingroup Category
 */
typedef enum {
	Category_ToDo = 1, /**< Todo entry category */
	Category_Phonebook /**< Phonebook entry category */
} GSM_CategoryType;

/**
 * Category entry.
 * \ingroup Category
 */
typedef struct {
	GSM_CategoryType    Type; /**< Type of category */
	int                 Location; /**< Location of category */
	unsigned char       Name[(GSM_MAX_CATEGORY_NAME_LENGTH + 1)*2]; /**< Name of category */
} GSM_Category;

/**
 * Status of categories.
 * \ingroup Category
 */
typedef struct {
	GSM_CategoryType    Type; /**< Type of category */
	int                 Used; /**< Number of used category names */
} GSM_CategoryStatus;

/**
 * Reads category from phone.
 *
 * \ingroup Category
 */
GSM_Error GAMMU_GetCategory(GSM_StateMachine *s, GSM_Category *Category);
/**
 * Adds category to phone.
 *
 * \ingroup Category
 */
GSM_Error GAMMU_AddCategory(GSM_StateMachine *s, GSM_Category *Category);
/**
 * Reads category status (number of used entries) from phone.
 *
 * \ingroup Category
 */
GSM_Error GAMMU_GetCategoryStatus(GSM_StateMachine *s, GSM_CategoryStatus *Status);
#endif
