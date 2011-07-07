// #include "stdafx.h"
/*#include "MTK.h"*/
unsigned char GetBase64Index(char ch);
int Decode(const char * szDecoding, char * szOutput);
int Encode(const unsigned char *pp_Data, int v_DataLen,unsigned char *pp_EncodeData);
void bcd2ascii( char *p_bcd,char *p_ascii,int ascii_len);
void  ascii2bcd(char *p_ascii,char *p_bcdcode,int bcdcode_len);
char CheckSum(char* pSrc, int nLen)