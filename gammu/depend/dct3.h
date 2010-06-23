
void DCT3tests		 (int argc, char *argv[]);
void DCT3netmonitor	 (int argc, char *argv[]);
void DCT3Info		 (int argc, char *argv[]);
void DCT3ResetTest36	 (int argc, char *argv[]);
void DCT3SetPhoneMenus	 (int argc, char *argv[]);
void DCT3GetSecurityCode (int argc, char *argv[]);

#include "../../common/gsmstate.h"

#ifdef GSM_ENABLE_NOKIA6110
void DCT3GetOperatorName (int argc, char *argv[]);
void DCT3SetOperatorName (int argc, char *argv[]);
#endif
