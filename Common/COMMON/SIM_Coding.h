#ifndef _SIM_CODING_H
#define  _SIM_CODING_H
int ParserLength(unsigned char *pIntput);
void Encode_OPAQUE(int nLength , unsigned char *pOutput);
BOOL SIM_DecodeText(unsigned char *pIntput,int nInputLebgth,unsigned char *pOutputUnicode);
BOOL SIM_EncodeText(unsigned char *pIntputUnicode,unsigned char *pOutput,int* nOutputLengthWithoutNULL);
#endif 