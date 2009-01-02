/**
 * \file gsmphones.h
 *
 * Phone features database.
 */
#ifndef __gsmphones_h
#define __gsmphones_h

#include <gammu-debug.h>
#include <gammu-info.h>
#include <gammu-statemachine.h>

/**
 * Converts model string to model record record describing it's
 * features. If state machine structure is provided, phone features
 * can be overrided from current state machine configuration.
 *
 * \param s Pointer to state machine structure, can be NULL.
 * \param model Model name string, NULL if not to be searched.
 * \param number Model number string, NULL if not to be searched.
 * \param irdamodel IrDA model name string, NULL if not to be searched.
 *
 * \return Pointer to static structure containing phone information.
 */
GSM_PhoneModel *GetModelData(GSM_StateMachine *s, const char *model, const char *number, const char *irdamodel);

/**
 * Converts string to list of features.
 *
 * \param list Storage where features will be stored, needs to be long
 * enough (GSM_MAX_PHONE_FEATURES + 1).
 * \param string String with features to be parsed, will be parsed using
 * GSM_FeatureFromString.
 *
 * \return Error code.
 */
GSM_Error GSM_SetFeatureString(Feature *list, const char *string);
#endif
