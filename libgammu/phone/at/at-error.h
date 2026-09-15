/* Internal AT error decoding. See at-error-sources.md for references. */
#ifndef GAMMU_AT_ERROR_H
#define GAMMU_AT_ERROR_H

#include "atgen.h"

void ATGEN_ParseError(GSM_Phone_ATGENData *priv, const char *text);
GSM_Error ATGEN_ErrorResult(const GSM_Phone_ATGENData *priv);

#endif
