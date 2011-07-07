#include "stdAfx.h"
#include "commfun.h"

/* ASCII subsets */
static	const	unsigned char	base64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static	const	unsigned char	safe[] = "'(),-.:?";
static	const	unsigned char	optional[] = "!\"#$%&*;<=>@[]^_`{|}";
static	const	unsigned char	space[] = " \t\n\r";

#define	NUMBYTES 256
#define	CHARSIZE 16


#define	BASE64	0x01
#define	SAFE	0x02
#define	MYOPTIONAL 0x04
#define	SPACE	0x08

static	char	inv_base64[128];
static	char	char_type[NUMBYTES];

static	bool	initialized;

static void
invert()
{
	int	i;
	const	unsigned char	*s;

	initialized = TRUE;
	for (i = 0; i < NUMBYTES; i++)
		char_type[i] = 0;
	for (s = base64; *s != '\0'; s++) {
		char_type[*s] |= BASE64;
		inv_base64[*s] = s - base64;
	}
	for (s = safe; *s != '\0'; s++)
		char_type[*s] |= SAFE;
	for (s = optional; *s != '\0'; s++)
		char_type[*s] |= MYOPTIONAL;
	for (s = space; *s != '\0'; s++)
		char_type[*s] |= SPACE;
}

static	bool	in_base64 = false;
static	unsigned long	bit_buffer;
static	int	nbits;


void UnicodeToUTF7(unsigned char *dest, unsigned char *src, int len)
{in_base64 = false;
	unsigned char c;
	int i,j =0;
	if (! initialized)
		invert();
	for(i=0; i<len; i++) 
	{
		c = src[i];
		if (c <= 0x7f && (char_type[c] & (BASE64|SAFE|SPACE)))
		{
			if (in_base64) {
				if (nbits > 0)
				{
					dest[j++] = base64[(bit_buffer<<(6-nbits))&0x3f];
				}
				in_base64 = FALSE;
			}
			dest[j++] = c;

			if (c == '+')
				dest[j++] = '-';
		}
		else 
		{
			if (! in_base64) 
			{
				dest[j++] = '+';
				in_base64 = TRUE;
				nbits = 0;
			}
			bit_buffer <<= CHARSIZE;
			bit_buffer |= c;
			nbits += CHARSIZE;
			while (nbits >= 6)
			{
				nbits -= 6;
				dest[j++] = base64[(bit_buffer >> nbits)&0x3f];
			}
		}
	}//for
}



