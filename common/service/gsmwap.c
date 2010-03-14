
#include <string.h>

#include "gsmwap.h"
#include "../misc/coding.h"

void NOKIA_EncodeWAPBookmarkSMSText(char *Buffer, int *Length, GSM_WAPBookmark bookmark)
{
	unsigned char	buffer[100];
	bool		UnicodeCoding = false;

	EncodeUTF8(buffer,bookmark.Title);
	if (strlen(DecodeUnicodeString(bookmark.Title))!=strlen(buffer)) UnicodeCoding = true;

	strcpy(Buffer+(*Length),"\x01\x06-\x1F");				(*Length)=(*Length)+4;
	strcpy(Buffer+(*Length),"+application/x-wap-prov.browser-bookmarks");	(*Length)=(*Length)+12+29;
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x81\xEA");					(*Length)=(*Length)+2;

	if (!UnicodeCoding) {
		strcpy(Buffer+(*Length),"\x00");				(*Length)=(*Length)+1;
		strcpy(Buffer+(*Length),"\x01\x00");				(*Length)=(*Length)+2;
	} else {
		strcpy(Buffer+(*Length),"\x01\x01\x87\x68\x00");		(*Length)=(*Length)+5;
	}

	strcpy(Buffer+(*Length),"\x45\xC6\x7F");				(*Length)=(*Length)+3;

	/* The same to "title" block in WAP settings */
	strcpy(Buffer+(*Length),"\x01\x87\x15\x11\x03");			(*Length)=(*Length)+5;
	if (!UnicodeCoding) {
		memcpy(Buffer+(*Length),DecodeUnicodeString(bookmark.Title),strlen(DecodeUnicodeString(bookmark.Title)));
		(*Length)=(*Length)+strlen(DecodeUnicodeString(bookmark.Title));
	} else {
		CopyUnicodeString(Buffer+(*Length), bookmark.Title);
		(*Length)=(*Length)+2*strlen(DecodeUnicodeString(bookmark.Title));
		Buffer[(*Length)++]=0x00;
	}
	Buffer[(*Length)++]=0x00;

	/* The same to "homepage" block in WAP settings */
	strcpy(Buffer+(*Length),"\x01\x87\x17\x11\x03");			(*Length)=(*Length)+5;
	if (!UnicodeCoding) {
		memcpy(Buffer+(*Length),DecodeUnicodeString(bookmark.Address),strlen(DecodeUnicodeString(bookmark.Address)));
		(*Length)=(*Length)+strlen(DecodeUnicodeString(bookmark.Address));
	} else {
		CopyUnicodeString(Buffer+(*Length), bookmark.Address);
		(*Length)=(*Length)+2*strlen(DecodeUnicodeString(bookmark.Address));
		Buffer[(*Length)++]=0x00;
	}
	Buffer[(*Length)++]=0x00;

	/* Mark of end */
	strcpy(Buffer+(*Length),"\x01\x01\x01");				(*Length)=(*Length)+3;
}

/* Function created from software from http://www.tektonica.com/projects/gsmsms/ */
void NOKIA_EncodeWAPSettingsSMSText(char *Buffer, int *Length, GSM_WAPSettings settings)
{
	strcpy(Buffer+(*Length),"\x01\x06\x04\x03\x94");			(*Length)=(*Length)+5;

	strcpy(Buffer+(*Length),"\x81\xEA\x00");				(*Length)=(*Length)+3;

	strcpy(Buffer+(*Length),"\x01\x00");					(*Length)=(*Length)+2;
	strcpy(Buffer+(*Length),"\x45\xC6\x06");				(*Length)=(*Length)+3;

	strcpy(Buffer+(*Length),"\x01\x87\x12");				(*Length)=(*Length)+3;
	switch (settings.Bearer) {
		case WAPSETTINGS_BEARER_DATA : Buffer[(*Length)++]=0x45;	break;
		case WAPSETTINGS_BEARER_SMS  : Buffer[(*Length)++]=0x41;	break;
		default			     : 					break;
	}

	strcpy(Buffer+(*Length),"\x01\x87\x13\x11\x03");			(*Length)=(*Length)+5;
	memcpy(Buffer+(*Length),DecodeUnicodeString(settings.IPAddress),strlen(DecodeUnicodeString(settings.IPAddress)));
	(*Length)=(*Length)+strlen(DecodeUnicodeString(settings.IPAddress));
	Buffer[(*Length)++]=0x00;

	strcpy(Buffer+(*Length),"\x01\x87\x14");				(*Length)=(*Length)+3;
	if (settings.IsContinuous) {
		Buffer[(*Length)++]=0x61; 	/* continuous */
	} else {
		Buffer[(*Length)++]=0x60;	/* temporary  */
	}

  	strcpy(Buffer+(*Length),"\x01\x87\x21\x11\x03");			(*Length)=(*Length)+5;
	memcpy(Buffer+(*Length),DecodeUnicodeString(settings.DialUp),strlen(DecodeUnicodeString(settings.DialUp)));
	(*Length)=(*Length)+strlen(DecodeUnicodeString(settings.DialUp));
	Buffer[(*Length)++]=0x00;

	strcpy(Buffer+(*Length),"\x01\x87\x22");				(*Length)=(*Length)+3;
	if (settings.IsNormalAuthentication) {
		Buffer[(*Length)++]=0x70;	/* OTA_CSD_AUTHTYPE_PAP. Correct ?  */
	} else {
		Buffer[(*Length)++]=0x71;	/* OTA_CSD_AUTHTYPE_CHAP. Correct ? */
	}

	strcpy(Buffer+(*Length),"\x01\x87\x23\x11\x03");			(*Length)=(*Length)+5;
	memcpy(Buffer+(*Length),DecodeUnicodeString(settings.User),strlen(DecodeUnicodeString(settings.User)));
	(*Length)=(*Length)+strlen(DecodeUnicodeString(settings.User));
	Buffer[(*Length)++]=0x00;

	strcpy(Buffer+(*Length),"\x01\x87\x24\x11\x03");			(*Length)=(*Length)+5;
	memcpy(Buffer+(*Length),DecodeUnicodeString(settings.Password),strlen(DecodeUnicodeString(settings.Password)));
	(*Length)=(*Length)+strlen(DecodeUnicodeString(settings.Password));
	Buffer[(*Length)++]=0x00;

	strcpy(Buffer+(*Length),"\x01\x87\x28");				(*Length)=(*Length)+3;
	if (settings.IsISDNCall) {
		Buffer[(*Length)++]=0x73;	/* ISDN */
	} else {
		Buffer[(*Length)++]=0x72;	/* analogue */
	}

	strcpy(Buffer+(*Length),"\x01\x87\x29");				(*Length)=(*Length)+3;
	switch (settings.Speed) {
		case WAPSETTINGS_SPEED_AUTO : Buffer[(*Length)++]=0x6A; break;	
		case WAPSETTINGS_SPEED_9600 : Buffer[(*Length)++]=0x6B; break;
		case WAPSETTINGS_SPEED_14400: Buffer[(*Length)++]=0x6C; break;		
	}

	Buffer[(*Length)++]=0x01;
	strcpy(Buffer+(*Length),"\x01\x86\x07\x11\x03");			(*Length)=(*Length)+5;
	memcpy(Buffer+(*Length),DecodeUnicodeString(settings.HomePage),strlen(DecodeUnicodeString(settings.HomePage)));
	(*Length)=(*Length)+strlen(DecodeUnicodeString(settings.HomePage));
	Buffer[(*Length)++]=0x00;

	strcpy(Buffer+(*Length),"\x01\xC6\x08");				(*Length)=(*Length)+3;
	strcpy(Buffer+(*Length),"\x01\x87\x15\x11\x03");			(*Length)=(*Length)+5;
	memcpy(Buffer+(*Length),DecodeUnicodeString(settings.Title),strlen(DecodeUnicodeString(settings.Title)));
	(*Length)=(*Length)+strlen(DecodeUnicodeString(settings.Title));
	Buffer[(*Length)++]=0x00;

	/* Mark of end */
	strcpy(Buffer+(*Length),"\x01\x01\x01");				(*Length)=(*Length)+3;
}
