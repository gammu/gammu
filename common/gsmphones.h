/**
 * \file gsmphones.h
 *
 * Phone features database.
 */
#ifndef __gsmphones_h
#define __gsmphones_h

#include <gammu-info.h>

extern OnePhoneModel allmodels[];

OnePhoneModel 	*GetModelData		(char *model, char *number, char *irdamodel);

#endif
