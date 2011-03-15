#include "stdafx.h"
#include "SIM_Coding.h"
#include "Coding.h"

int ParserLength(unsigned char *pIntput)
{
    int uint = 0;
    for (int i = 0; i < 5; i++)
	{
        uint = (uint << 7) | (pIntput[i] & 0x7F);
        if (!(pIntput[i] & 0x80)) 
            return uint;
    }
    return uint;

}
void Encode_OPAQUE(int nLength , unsigned char *pOutput)
{
	unsigned char octets[5];
    int i = 0, start = 0;
    octets[4] = (unsigned char) (nLength & 0x7f);
    nLength >>= 7;

    for (i = 3; nLength > 0 && i >= 0; i--) 
	{
        octets[i] = (unsigned char) (0x80 | (nLength & 0x7f));
        nLength >>= 7;
    }
    start = i + 1;
	memcpy(pOutput , octets + start,5 - start);
	pOutput[5 - start] = '\0';

}
BOOL SIM_DecodeText(unsigned char *pIntput,int nInputLength,unsigned char *pOutputUnicode)
{
	if(pIntput[0] < 0x80)
	{
		DecodeDefault(pOutputUnicode, pIntput, nInputLength, true, NULL); 
//		unsigned char *pSACIIText = new unsigned char [nInputLength+2];
//		memcpy(pSACIIText,pIntput,nInputLength);
//		pSACIIText[nInputLength] = 0x00;
//		pSACIIText[nInputLength+1] = 0x00;
//		EncodeUnicode(pOutputUnicode,pSACIIText,nInputLength+2);
//		delete pSACIIText;
		return true;
	}
		CString s;
	switch(pIntput[0])
	{
	case 0x80:
		memcpy(pOutputUnicode,pIntput+1,nInputLength-1);
		pOutputUnicode[nInputLength-1] = 0x00;
		pOutputUnicode[nInputLength] = 0x00;
		break;
	case 0x81:
		{
			int nCur = 0;
//			unsigned char szUnicodeText[10];
			unsigned char szOPAQUESize[5];
			unsigned char FirstWidecharByte[2];
			ZeroMemory(szOPAQUESize,5);
			szOPAQUESize[0] = pIntput[0];
			szOPAQUESize[1] =pIntput[2]; 
			FirstWidecharByte[0] = ParserLength(szOPAQUESize)/256;
			FirstWidecharByte[1] = ParserLength(szOPAQUESize)%256;
			for(int i = 3 ; i<nInputLength ; i++)
			{
				if(pIntput[i] <0x80 && pIntput[i] !=0x00)
				{
					DecodeDefault(pOutputUnicode+nCur, pIntput+i, 1, true, NULL);
//					unsigned char temp[2];
//					temp[0] = pIntput[i];
//					temp[1] = 0x00;
//					EncodeUnicode(szUnicodeText,temp,2);
//					memcpy(pOutputUnicode+nCur,szUnicodeText,2);
					nCur += 2;
				}
				else
				{
					pOutputUnicode[nCur++] = FirstWidecharByte[0];
					pOutputUnicode[nCur++] = pIntput[i] +FirstWidecharByte[1]- 0x80;

				}
			}
			pOutputUnicode[nCur++] = 0x00;
			pOutputUnicode[nCur++] = 0x00;
		}
		break;
	default:
		return false;
	}
	return true;

}

BOOL SIM_EncodeText(unsigned char *pIntputUnicode,unsigned char *pOutput,int* nOutputLengthWithoutNULL)
{
	int nUnicodeLength = UnicodeLength(pIntputUnicode);

//	unsigned char* szASCIIText = new unsigned char[(nUnicodeLength+1)*2];
//	DecodeUnicode(pIntputUnicode,szASCIIText);
//	int nASCIILength = strlen((char*)szASCIIText);
	unsigned char* buffer = new unsigned char[(nUnicodeLength+1)*3];
	EncodeUnicode2UTF8(pIntputUnicode,(unsigned char *)buffer);// for _UNICODE
	int nASCIILength = strlen((char*)buffer);
	if (nUnicodeLength==nASCIILength)
//	if(nUnicodeLength == nASCIILength)
	{
		if(nASCIILength >12)
		{
			nASCIILength = 12;
			pIntputUnicode[12*2] = '\0';
			pIntputUnicode[12*2+1] = '\0';
		}
		*nOutputLengthWithoutNULL=nASCIILength;
		EncodeDefault(pOutput, pIntputUnicode, nOutputLengthWithoutNULL, true, NULL);
//		memcpy(pOutput,szASCIIText,nASCIILength);
//		pOutput[nASCIILength] = '\0';
//		*nOutputLengthWithoutNULL = nASCIILength;
	//	delete szASCIIText;
		delete[] buffer;
		return true;
	}

	int nDBCScharCount =0;
	unsigned char nFirstWidechar[4];
	nFirstWidechar[3]=nFirstWidechar[4] = 0;
	unsigned char nFirstUnicode[4];

	for(int i = 0 ; i<nUnicodeLength; i++)
	{
		if(pIntputUnicode[i*2] !=0)
//		if (::IsDBCSLeadByte((BYTE) szASCIIText[i]))
		{
			if(nDBCScharCount == 0)
			{
				nFirstWidechar[0] = pIntputUnicode[i*2];
				nFirstWidechar[1] = pIntputUnicode[(i*2)+1];
				nDBCScharCount =1;
			}
			else
			{
				if((pIntputUnicode[i*2]!= nFirstWidechar[0]) || 
					(pIntputUnicode[(i*2)+1]!=nFirstWidechar[1]))
				{
					nDBCScharCount++;
					break;
				}
			}
	//		i++;

		}
	}

	if(nDBCScharCount >1)
	{
		pOutput[0] = 0x80;
		if(nUnicodeLength > 5)
		{
			nUnicodeLength = 5;
			pIntputUnicode[5*2] = 0x00;
			pIntputUnicode[5*2+1] = 0x00;
		}
		memcpy(pOutput+1,pIntputUnicode,(nUnicodeLength	+1)*2);
		*nOutputLengthWithoutNULL = 1+(nUnicodeLength*2);
	}
	else if(nDBCScharCount == 1)
	{
	//	EncodeUnicode(nFirstUnicode,nFirstWidechar,3);
		CopyUnicodeString(nFirstUnicode,nFirstWidechar);
		unsigned char uLength[5];
		Encode_OPAQUE(nFirstUnicode[0]*256+nFirstUnicode[1],uLength);
		if(uLength[0] != 0x81)
		{
			pOutput[0] = 0x80;
			if(nUnicodeLength > 5)
			{
				nUnicodeLength = 5;
				pIntputUnicode[5*2] = 0x00;
				pIntputUnicode[5*2+1] = 0x00;
			}

			memcpy(pOutput+1,pIntputUnicode,(nUnicodeLength	+1)*2);
			*nOutputLengthWithoutNULL = 1+(nUnicodeLength*2);
		}
		else
		{
			int nPos=0;
			pOutput[nPos++] = uLength[0];
			pOutput[nPos++] = nUnicodeLength;
			pOutput[nPos++] = uLength[1];

			for(int i = 0 ; i<nUnicodeLength; i++)
			{
				if(nPos >= 9)
				{
			//		szASCIIText[i] = '\0';
			//		EncodeUnicode(pIntputUnicode,szASCIIText,strlen((char*)szASCIIText)+1);
			//		pOutput[1] = UnicodeLength(pIntputUnicode);
					pIntputUnicode[i*2] = 0;
					pIntputUnicode[i*2+1] = 0;
					pOutput[1] = UnicodeLength(pIntputUnicode);
					break;
				}
			//	if (::IsDBCSLeadByte((BYTE) szASCIIText[i]))
				if(pIntputUnicode[i*2] !=0)
				{
					pOutput[nPos++] = uLength[2]+0x80;
			//		i++;
				}
				else{
					int len=2;
					unsigned char in[2], out[2];
				//	in[0]=0x00;
				//	in[1]=szASCIIText[i];
					in[0]=pIntputUnicode[i*2];
					in[1]=pIntputUnicode[i*2+1];
					EncodeDefault(out, in, &len, true, NULL);
					memcpy(pOutput+nPos, out, 1);
					nPos++;
//					pOutput[nPos++] = szASCIIText[i];
				}
			}
			*nOutputLengthWithoutNULL = nPos;
			pOutput[nPos++] = 0;
			pOutput[nPos++] = 0;
		}
	}
	else
	{
//		delete szASCIIText;
		delete[] buffer;
		return false;
	}

//	delete szASCIIText;
	delete[] buffer;
	return true;
}

BOOL SIM_EncodeText2(unsigned char *pIntputUnicode, int nOutputMaxLength,unsigned char *pOutput,int* nOutputLengthWithoutNULL)
{
	int nUnicodeLength = UnicodeLength(pIntputUnicode);
	unsigned char* szASCIIText = new unsigned char[(nUnicodeLength+1)*2];
	DecodeUnicode(pIntputUnicode,szASCIIText);
	int nASCIILength = strlen((char*)szASCIIText);
	if(nUnicodeLength == nASCIILength)
	{
		if(nASCIILength >nOutputMaxLength)
		{
			nASCIILength = nOutputMaxLength;
			pIntputUnicode[nOutputMaxLength*2] = '\0';
			pIntputUnicode[nOutputMaxLength*2+1] = '\0';
		}
		*nOutputLengthWithoutNULL=nASCIILength;
		EncodeDefault(pOutput, pIntputUnicode, nOutputLengthWithoutNULL, true, NULL);
		delete szASCIIText;
		return true;
	}

	int nDBCScharCount =0;
	unsigned char nFirstWidechar[4];
	nFirstWidechar[3]=nFirstWidechar[4] = 0;
	unsigned char nFirstUnicode[4];

	for(int i = 0 ; i<nASCIILength; i++)
	{
		if (::IsDBCSLeadByte((BYTE) szASCIIText[i]))
		{
			if(nDBCScharCount == 0)
			{
				nFirstWidechar[0] = szASCIIText[i];
				nFirstWidechar[1] = szASCIIText[i+1];
				nDBCScharCount =1;
			}
			else
			{
				if((szASCIIText[i]!= nFirstWidechar[0]) || 
					(szASCIIText[i+1]!=nFirstWidechar[1]))
				{
					nDBCScharCount++;
					break;
				}
			}
			i++;

		}
	}

	int nLength;

	if(nDBCScharCount >1)
	{
		pOutput[0] = 0x80;
		nLength=(nOutputMaxLength-1)/2;
		if(nUnicodeLength > nLength)
		{
			nUnicodeLength = nLength;
			pIntputUnicode[nLength*2] = 0x00;
			pIntputUnicode[nLength*2+1] = 0x00;
		}
		memcpy(pOutput+1,pIntputUnicode,(nUnicodeLength	+1)*2);
		*nOutputLengthWithoutNULL = 1+(nUnicodeLength*2);
	}
	else if(nDBCScharCount == 1)
	{
		EncodeUnicode(nFirstUnicode,nFirstWidechar,3);

		unsigned char uLength[5];
		Encode_OPAQUE(nFirstUnicode[0]*256+nFirstUnicode[1],uLength);
		if(uLength[0] != 0x81)
		{
			pOutput[0] = 0x80;
			nLength=(nOutputMaxLength-1)/2;
			if(nUnicodeLength > nLength)
			{
				nUnicodeLength = nLength;
				pIntputUnicode[nLength*2] = 0x00;
				pIntputUnicode[nLength*2+1] = 0x00;
			}

			memcpy(pOutput+1,pIntputUnicode,(nUnicodeLength	+1)*2);
			*nOutputLengthWithoutNULL = 1+(nUnicodeLength*2);
		}
		else
		{
			int nPos=0;
			pOutput[nPos++] = uLength[0];
			pOutput[nPos++] = nUnicodeLength;
			pOutput[nPos++] = uLength[1];
			nLength=nOutputMaxLength-3;

			for(int i = 0 ; i<nASCIILength; i++)
			{
				if(nPos >= nLength)
				{
					szASCIIText[i] = '\0';
					EncodeUnicode(pIntputUnicode,szASCIIText,strlen((char*)szASCIIText)+1);
					pOutput[1] = UnicodeLength(pIntputUnicode);
					break;
				}
				if (::IsDBCSLeadByte((BYTE) szASCIIText[i]))
				{
					pOutput[nPos++] = uLength[2]+0x80;
					i++;
				}
				else{
					int len=2;
					unsigned char in[2], out[2];
					in[0]=0x00;
					in[1]=szASCIIText[i];
					EncodeDefault(out, in, &len, true, NULL);
					memcpy(pOutput+nPos, out, 1);
					nPos++;
				}
			}
			*nOutputLengthWithoutNULL = nPos;
			pOutput[nPos++] = 0;
			pOutput[nPos++] = 0;
		}
	}
	else
	{
		delete szASCIIText;
		return false;
	}

	delete szASCIIText;
	return true;
}