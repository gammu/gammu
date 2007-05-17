#define GSM_MAX_CATEGORY_NAME_LENGTH 60

typedef enum {
	Category_ToDo = 1,
	Category_Phonebook
} GSM_CategoryType;

typedef struct {
	GSM_CategoryType    Type;
	int                 Location;
	unsigned char       Name[(GSM_MAX_CATEGORY_NAME_LENGTH + 1)*2];
} GSM_Category;

typedef struct {
	GSM_CategoryType    Type;
	int                 Used;
} GSM_CategoryStatus;

/**
 * Reads category from phone.
 */
GSM_Error GAMMU_GetCategory(GSM_StateMachine *s, GSM_Category *Category);
/**
 * Adds category to phone.
 */
GSM_Error GAMMU_AddCategory(GSM_StateMachine *s, GSM_Category *Category);
/**
 * Reads category status (number of used entries) from phone.
 */
GSM_Error GAMMU_GetCategoryStatus(GSM_StateMachine *s, GSM_CategoryStatus *Status);
