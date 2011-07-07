//#include "DeCodeBase.h"
char Base64Alphabet[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

unsigned char GetBase64Index(char ch)
{
	unsigned char i, v_Return = -1;
	for(i = 0; i < 64; i++)
	{
		if(Base64Alphabet[(int)i] == ch)
		{
			v_Return = i;
			break;
		}
	}
	return v_Return;
}
int Decode(const char * szDecoding, char * szOutput)
{
	char			Ch;
	char			chCurrentIndex;
	unsigned int	i,nCurrentBits;
	unsigned char	chTargetChar;
	char			*pl_DeData;
	nCurrentBits = 0;
	chTargetChar = '\0';
	pl_DeData = szOutput;
	for(i = 0; i < strlen(szDecoding); i++)
	{
		Ch = szDecoding[i];
		if( Ch == '=')
		{
			nCurrentBits = 0;
	continue;
		}
		chCurrentIndex = GetBase64Index(Ch);
		if(chCurrentIndex < 0) 	return 0;
		if((nCurrentBits+6) >= 8)
		{
			chTargetChar |= (chCurrentIndex >> (6-(8-nCurrentBits)));
			*pl_DeData = chTargetChar;
			pl_DeData++;
			nCurrentBits = nCurrentBits - 2;
			chTargetChar = chCurrentIndex << (8 - nCurrentBits);
		}
		else
		{
			chTargetChar |= (chCurrentIndex << (2-nCurrentBits));
			nCurrentBits += 6;
		}
	}
	return pl_DeData-szOutput;
}

int Encode(const unsigned char *pp_Data, int v_DataLen,unsigned char *pp_EncodeData)
{
	unsigned char	v_FirstByte,v_SecondByte,v_ThirdByte, v_FourthByte;
	unsigned char	*pl_EnData;
	int 			i,v_Left;	
	pl_EnData = (unsigned char *)pp_EncodeData;
	for(i = 0; i < v_DataLen / 3; i++)
	{
		v_FirstByte	= pp_Data[0] >> 2;
		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | (pp_Data[1] >> 4) ;
		v_ThirdByte	= ((pp_Data[1] & 0x0F) << 2) | (pp_Data[2] >> 6);
		v_FourthByte	= pp_Data[2] & 0x3F;
		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
        pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
		pl_EnData[2] = Base64Alphabet[(int)v_ThirdByte];
		pl_EnData[3] = Base64Alphabet[(int)v_FourthByte];

		pp_Data += 3;
		pl_EnData += 4;	 
	}
	v_Left = v_DataLen % 3;
	if(v_Left == 1)
	{
		v_FirstByte = pp_Data[0] >> 2;
		v_SecondByte = (pp_Data[0] & 0x03) << 4;
		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
		pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
		pl_EnData[2] = '=';
		pl_EnData[3] = '=';
		pp_Data += 1;
		pl_EnData += 4;
	}
	else if(v_Left == 2)
	{
		v_FirstByte	= pp_Data[0] >> 2;
		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | ((pp_Data[1] & 0xF0) >> 4);
		v_ThirdByte	= (pp_Data[1] & 0x0F) << 2; 
		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
		pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
		pl_EnData[2] = Base64Alphabet[(int)v_ThirdByte];
		pl_EnData[3] = '=';
		pp_Data += 2;
		pl_EnData += 4;	 
	}
	
	return pl_EnData - pp_EncodeData ;
}
void bcd2ascii( char *p_bcd,char *p_ascii,int ascii_len)
{
	unsigned char i,j,len_char,temp_number;
	memset(p_ascii,'\0',ascii_len);
	len_char=*p_bcd;
	j=1;
	if(len_char%2)
	{
		for(i=0;i<len_char-1;i=i+2)
		{
				temp_number=*(p_bcd+j);
				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
				p_ascii[i+1]=(temp_number & 0x0f)+'0';
				j++;
		}
		temp_number=*(p_bcd+j);
		p_ascii[len_char-1]=temp_number;
	}
	else
	{
		for(i=0;i<len_char;i=i+2)
		{
				temp_number=*(p_bcd+j);
				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
				p_ascii[i+1]=(temp_number & 0x0f)+'0';
				j++;
		}
	}
	p_ascii[len_char]='\0';
	len_char=strlen(p_ascii);
	for(i=0;i<len_char;i++)
	{
		if(p_ascii[i]==(10+'0'))
		{
			p_ascii[i]='p';
		}
		if(p_ascii[i]==(11+'0'))
		{
			p_ascii[i]='w';
		}
		if(p_ascii[i]==(12+'0'))
		{
			p_ascii[i]='+';
		}
		if(p_ascii[i]==(13+'0'))
		{
			p_ascii[i]='*';
		}
		if(p_ascii[i]==(14+'0'))
		{
			p_ascii[i]='#';
		}
	}
}
void  ascii2bcd(char *p_ascii,char *p_bcdcode,int bcdcode_len)
{
	unsigned char i,j,len_char;
	memset(p_bcdcode,'\0',bcdcode_len);
	len_char=strlen(p_ascii);
	for(i=0;i<len_char;i++)
	{
		if((*(p_ascii+i))=='p')
		{
			*(p_ascii+i)=(10+'0');
		}
		else if((*(p_ascii+i))=='w')
		{
			*(p_ascii+i)=(11+'0');
		}
		else if((*(p_ascii+i))=='+')
		{
			*(p_ascii+i)=(12+'0');
		}
		else if((*(p_ascii+i))=='*')
		{
			*(p_ascii+i)=(13+'0');
		}
		else if((*(p_ascii+i))=='#')
		{
			*(p_ascii+i)=(14+'0');
		}
	}
	j=1;
	p_bcdcode[0]=len_char;

	if(len_char%2)
	{
		for(i=0;i<len_char-1;i=i+2)
		{
			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
			j++;
		}
		p_bcdcode[j]=*(p_ascii+i);

	}
	else
	{
		for(i=0;i<len_char-1;i=i+2)
		{
			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
			j++;
		}
	}
}
char CheckSum(char* pSrc, int nLen)
{

       if(nLen == 0)return 0;
       BYTE ch = *pSrc;
       for(int i = 1; i < nLen; i++)
       {
              ch = ch ^ (*(pSrc+i));
       }
       return ch;
}