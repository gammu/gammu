/* (c) 2002-2004 by Marcin Wiacek */

void DCT3SelfTests	 (int argc, char *argv[]);
void DCT3netmonitor	 (int argc, char *argv[]);
void DCT3Info		 (int argc, char *argv[]);
void DCT3ResetTest36	 (int argc, char *argv[]);
void DCT3SetPhoneMenus	 (int argc, char *argv[]);
void DCT3GetSecurityCode (int argc, char *argv[]);
void DCT3GetADC		 (int argc, char *argv[]);
void DCT3DisplayTest	 (int argc, char *argv[]);
void DCT3VibraTest	 (int argc, char *argv[]);
void DCT3GetT9		 (int argc, char *argv[]);

GSM_Error CheckDCT3Only(void);

#ifdef GSM_ENABLE_NOKIA6110
void DCT3GetOperatorName (int argc, char *argv[]);
void DCT3SetOperatorName (int argc, char *argv[]);
void DCT3DisplayOutput	 (int argc, char *argv[]);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
