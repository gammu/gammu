#include "stdafx.h"
/* ================================================================ */
/*
File:   ConvertUTF7.c
Author: David B. Goldsmith
Copyright (C) 1994, 1996 IBM Corporation All rights reserved.
Revisions: Header update only July, 2001.

This code is copyrighted. Under the copyright laws, this code may not
be copied, in whole or part, without prior written consent of IBM Corporation. 

IBM Corporation grants the right to use this code as long as this ENTIRE
copyright notice is reproduced in the code.  The code is provided
AS-IS, AND IBM CORPORATION DISCLAIMS ALL WARRANTIES, EITHER EXPRESS OR
IMPLIED, INCLUDING, BUT NOT LIMITED TO IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT
WILL IBM CORPORATION BE LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING,
WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS
INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY
LOSS) ARISING OUT OF THE USE OR INABILITY TO USE THIS CODE, EVEN
IF IBM CORPORATION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
BECAUSE SOME STATES DO NOT ALLOW THE EXCLUSION OR LIMITATION OF
LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE
LIMITATION MAY NOT APPLY TO YOU.

RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the
government is subject to restrictions as set forth in subparagraph
(c)(l)(ii) of the Rights in Technical Data and Computer Software
clause at DFARS 252.227-7013 and FAR 52.227-19.

This code may be protected by one or more U.S. and International
Patents.

*/

#include "CVTUTF7.H"

static char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static short invbase64[128];

static char direct[] = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'(),-./:?";
static char optional[] = "!\"#$%&*;<=>@[]^_`{|}";
static char spaces[] = " \011\015\012";		/* space, tab, return, line feed */
static char mustshiftsafe[128];
static char mustshiftopt[128];

static int needtables = 1;

#define SHIFT_IN '+'
#define SHIFT_OUT '-'

static void
tabinit()
{
	int i, limit;

	for (i = 0; i < 128; ++i)
	{
		mustshiftopt[i] = mustshiftsafe[i] = 1;
		invbase64[i] = -1;
	}
	limit = strlen(direct);
	for (i = 0; i < limit; ++i)
		mustshiftopt[direct[i]] = mustshiftsafe[direct[i]] = 0;
	limit = strlen(spaces);
	for (i = 0; i < limit; ++i)
		mustshiftopt[spaces[i]] = mustshiftsafe[spaces[i]] = 0;
	limit = strlen(optional);
	for (i = 0; i < limit; ++i)
		mustshiftopt[optional[i]] = 0;
	limit = strlen(base64);
	for (i = 0; i < limit; ++i)
		invbase64[base64[i]] = i;

	needtables = 0;
}

#define DECLARE_BIT_BUFFER register unsigned long BITbuffer = 0, buffertemp = 0; int bufferbits = 0
#define BITS_IN_BUFFER bufferbits
#define WRITE_N_BITS(x, n) ((BITbuffer |= ( ((x) & ~(-1L<<(n))) << (32-(n)-bufferbits) ) ), bufferbits += (n) )
#define READ_N_BITS(n) ((buffertemp = (BITbuffer >> (32-(n)))), (BITbuffer <<= (n)), (bufferbits -= (n)), buffertemp)
#define TARGETCHECK  {if (target >= targetEnd) {result = targetExhausted; break;}}

ConversionResult1 ConvertUCS2toUTF7(
                UCS2** sourceStart, UCS2* sourceEnd, 
                char** targetStart, char* targetEnd,
                int optional, int verbose)
{
	ConversionResult1 result = ok;
	DECLARE_BIT_BUFFER;
	int shifted = 0, needshift = 0, done = 0;
	register UCS2 *source = *sourceStart;
	register char *target = *targetStart;
	char *mustshift;

	if (needtables)
		tabinit();

	if (optional)
		mustshift = mustshiftopt;
	else
		mustshift = mustshiftsafe;

	do
	{
		register UCS2 r;

		if (!(done = (source >= sourceEnd)))
			r = *source++;
		needshift = (!done && ((r > 0x7f) || mustshift[r]));

		if (needshift && !shifted)
		{
		//	TARGETCHECK;
			if (target >= targetEnd) 
			{
				result = targetExhausted; break;
			}
			*target++ = SHIFT_IN;
			/* Special case handling of the SHIFT_IN character */
			if (r == (UCS2)SHIFT_IN) {
				TARGETCHECK;
				*target++ = SHIFT_OUT;
			}
			else
				shifted = 1;
		}

		if (shifted)
		{
			/* Either write the character to the bit buffer, or pad
			   the bit buffer out to a full base64 character.
			 */
			if (needshift)
				WRITE_N_BITS(r, 16);
			else
				WRITE_N_BITS(0, (6 - (BITS_IN_BUFFER % 6))%6);

			/* Flush out as many full base64 characters as possible
			   from the bit buffer.
			 */
			while ((target < targetEnd) && BITS_IN_BUFFER >= 6)
			{
				*target++ = base64[READ_N_BITS(6)];
			}

			if (BITS_IN_BUFFER >= 6)
				TARGETCHECK;

			if (!needshift)
			{
				/* Write the explicit shift out character if
				   1) The caller has requested we always do it, or
				   2) The directly encoded character is in the
				   base64 set, or
				   3) The directly encoded character is SHIFT_OUT.
				 */
				if (verbose || ((!done) && (invbase64[r] >=0 || r == SHIFT_OUT)))
				{
//					TARGETCHECK1;
					*target++ = SHIFT_OUT;
				}
				shifted = 0;
			}
		}

		/* The character can be directly encoded as ASCII. */
		if (!needshift && !done)
		{
			TARGETCHECK;
			*target++ = (char) r;
		}

	}
	while (!done);
	
    *sourceStart = source;
    *targetStart = target;
	*target++ ='\0';
	*target++ ='\0';
    return result;
}

