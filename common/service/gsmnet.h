/* (c) 2002-2003 by Marcin Wiacek & Michal Cihar */

#ifndef __gsm_net_h
#define __gsm_net_h


void NOKIA_EncodeNetworkCode (unsigned char* buffer, const char* input);
void NOKIA_DecodeNetworkCode (const unsigned char* buffer, char* output);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
