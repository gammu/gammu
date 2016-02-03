/* Sony Ericsson specific functions
 * Copyright (C) 2011 Márton Németh <nm127@freemail.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <gammu-config.h>

#include <string.h>

#ifdef GSM_ENABLE_ATGEN

//#include "atgen.h"
#include "sonyericsson.h"

// Tokens
#define TOKEN_DATA_0			0x00
#define TOKEN_DATA_1			0x01
#define TOKEN_DATA_2			0x02
#define TOKEN_DATA_3			0x03
#define TOKEN_DATA_4			0x04
#define TOKEN_DATA_5			0x05
#define TOKEN_DATA_6			0x06
#define TOKEN_DATA_7			0x07
#define TOKEN_DATA_8			0x08
#define TOKEN_DATA_9			0x09
#define TOKEN_DATA_A			0x0A
#define TOKEN_DATA_B			0x0B
#define TOKEN_DATA_C			0x0C
#define TOKEN_DATA_D			0x0D
#define TOKEN_DATA_E			0x0E
#define TOKEN_DATA_F			0x0F
#define TOKEN_COMMAND_ZISI_ECHO	0x10
#define TOKEN_RESPONSE_ZISI		0x11
#define TOKEN_RESPONSE_OK		0x12
#define TOKEN_WHITESPACE		0x13
#define TOKEN_UNKNOWN			0x14

#define STATE_ALPHA0		0
#define STATE_ALPHA1		1
#define STATE_RED0		2
#define STATE_RED1		3
#define STATE_GREEN0		4
#define STATE_GREEN1		5
#define STATE_BLUE0		6
#define STATE_BLUE1		7

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

GSM_Error SONYERICSSON_GetScreenshot(GSM_StateMachine *s, GSM_BinaryPicture *picture)
{
	GSM_Error error;

	s->Phone.Data.Picture = picture;

	smprintf(s, "Get Sony Ericsson screenshot\n");

	/* Fetch screen dimensions */
	error = GSM_WaitForAutoLen(s, "AT*ZISI=?\r", 0, SONYERICSSON_TIMEOUT, ID_Screenshot);

	if (error == ERR_NONE) {
		/* Fetch screen data */
		error = GSM_WaitForAutoLen(s, "AT*ZISI\r", 0, SONYERICSSON_TIMEOUT, ID_Screenshot);
	}

	s->Phone.Data.Picture = NULL;

	return error;
}

GSM_Error SONYERICSSON_Reply_Screenshot(GSM_Protocol_Message *msg, GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	const char *string;
	int line;
	int h, w, param3, param4;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Screenshot size received\n");

		/* Parse the lines looking for "*ZISI:" */
		line = 1;
		h = 0;
		w = 0;
		param3 = 0;
		param4 = 0;
		do {
			string = GetLineString(msg->Buffer, &Priv->Lines, line+1);

			/*
			 * *ZISI: 320, 240, 16, 0
			 */
			error = ATGEN_ParseReply(s, string,
				"*ZISI: @i, @i, @i, @i",
				&h,
				&w,
				&param3,
				&param4);

			if (error != ERR_NONE) {
				/* Try again. Some phones like Sony Ericsson
				 * K508i returns data in different format:
				 *
				 * *ZISI: 160, 128, 16
				 *
				 */
				error = ATGEN_ParseReply(s, string,
					"*ZISI: @i, @i, @i",
					&h,
					&w,
					&param3);
			}

			if (error == ERR_NONE) {
				smprintf(s, "Screen size is %ix%i\n", w, h);
				/* Remember the screen size */
				Priv->ScreenWidth = w;
				Priv->ScreenHeigth = h;
			}

			line++;

		} while (strcmp("OK", string) != 0);

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
        s->Phone.Data.Picture->Type = PICTURE_BMP;
        s->Phone.Data.Picture->Buffer = NULL;
        s->Phone.Data.Picture->Length = 0;
		return ERR_UNKNOWNRESPONSE;
	}
}

static void u32_store(u8 *p, u32 data) {
	u32 tmp;

	tmp = data;
	*p = tmp & 0xFF;
	tmp >>= 8;
	*(p+1) = tmp & 0xFF;
	tmp >>= 8;
	*(p+2) = tmp & 0xFF;
	tmp >>= 8;
	*(p+3) = tmp & 0xFF;
}

static void u16_store(u8 *p, u16 data) {
	u16 tmp;

	tmp = data;
	*p = tmp & 0xFF;
	tmp >>= 8;
	*(p+1) = tmp & 0xFF;
}

static int SONYERICSSON_Screenshot_createBMPHeader(unsigned int w, unsigned int h, u8 *p) {
	u32 data;
	u16 data16;

	// BMP magic "BM"
	*p = 0x42;
	*(p+1) = 0x4D;

	// size of the BMP file in bytes
	data = 14 + 40 + w * h * 4;
	u32_store(p+2, data);

	// reserved
	data = 0;
	u32_store(p+6, data);

	// start of image data
	data = 14 + 40;
	u32_store(p+10, data);

	// size of this header
	data = 40;
	u32_store(p+14, data);

	// width in pixels (240)
	data = w;
	u32_store(p+18, data);

	// height in pixels (-320)
	data = (u32)(-h);
	u32_store(p+22, data);

	// color planes (1)
	data16 = 1;
	u16_store(p+26, data16);

	// bits per pixel (32)
	data16 = 32;
	u16_store(p+28, data16);

	// compression method
	data = 0;
	u32_store(p+30, data);

	// image size (320*240*4)
	data = w * h * 4;
	u32_store(p+34, data);

	// horizontal resolution (2835 pixel per meter = 72 dpi)
	data = 2835;
	u32_store(p+38, data);

	// vertical resolution (2835 pixel per meter = 72 dpi)
	data = 2835;
	u32_store(p+42, data);

	// number of colors in the color palette
	data = 0;
	u32_store(p+46, data);

	// number of important colors used
	data = 0;
	u32_store(p+50, data);

	return 54;
}

static int SONYERICSSON_Screenshot_addPixel(u8 alpha, u8 red, u8 green, u8 blue, u8 *p) {

	*p = blue;
	*(p+1) = green;
	*(p+2) = red;
	*(p+3) = alpha;

	return 4;
}


GSM_Error SONYERICSSON_Reply_ScreenshotData(GSM_Protocol_Message *msg, GSM_StateMachine *s)
{
	GSM_Error error = ERR_NONE;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned int i;
	int state;
	int token;
	u8 alpha = 0, red = 0, green = 0, blue = 0;
	u8 *p;
	unsigned int length;
	unsigned int w, h;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Screenshot data received\n");

		w = Priv->ScreenWidth;
		h = Priv->ScreenHeigth;

		s->Phone.Data.Picture->Type = PICTURE_BMP;
		s->Phone.Data.Picture->Buffer = (unsigned char *)malloc(54 + w * h * 4);
		if (s->Phone.Data.Picture->Buffer == NULL) {
			return ERR_MOREMEMORY;
		}
		p = s->Phone.Data.Picture->Buffer;

		length = SONYERICSSON_Screenshot_createBMPHeader(w, h, p);
		s->Phone.Data.Picture->Length = length;
		p += length;

		i = 0;
		state = STATE_ALPHA0;
		while (i < msg->BufferUsed) {
			/* Tokenize. The tokens are:
			 *  - TOKEN_COMMAND_ZISI_ECHO: "AT*ZISI"
			 *  - TOKEN_RESPONSE_ZISI: "*ZISI:"
			 *  - TOKEN_RESPONSE_OK: "OK"
			 *  - TOKEN_WHITESPACE: " ", "\r", "\n"
			 *  - TOKEN_DATA_0: "0"
			 *  - TOKEN_DATA_1: "1"
			 *  - TOKEN_DATA_2: "2"
			 *  - TOKEN_DATA_3: "3"
			 *  - TOKEN_DATA_4: "4"
			 *  - TOKEN_DATA_5: "5"
			 *  - TOKEN_DATA_6: "6"
			 *  - TOKEN_DATA_7: "7"
			 *  - TOKEN_DATA_8: "8"
			 *  - TOKEN_DATA_9: "9"
			 *  - TOKEN_DATA_A: "A"
			 *  - TOKEN_DATA_B: "B"
			 *  - TOKEN_DATA_C: "C"
			 *  - TOKEN_DATA_D: "D"
			 *  - TOKEN_DATA_E: "E"
			 *  - TOKEN_DATA_F: "F"
			 */

			switch (msg->Buffer[i]) {
				case 'A':
					if (strlen("AT*ZISI") <= msg->BufferUsed - i && strncmp(&msg->Buffer[i], "AT*ZISI", strlen("AT*ZISI")) == 0) {
						token = TOKEN_COMMAND_ZISI_ECHO;
						i += strlen("AT*ZISI");
					} else {
						token = TOKEN_DATA_A;
						i++;
					}
					break;
				case '*':
					if (strlen("*ZISI:") <= msg->BufferUsed - i && strncmp(&msg->Buffer[i], "*ZISI:", strlen("*ZISI:")) == 0) {
						token = TOKEN_RESPONSE_ZISI;
						i += strlen("*ZISI:");
					} else {
						token = TOKEN_UNKNOWN;
						i++;
					}
					break;
				case 'O':
					if (strlen("OK") <= msg->BufferUsed - i && strncmp(&msg->Buffer[i], "OK", strlen("OK")) == 0) {
						token = TOKEN_RESPONSE_OK;
						i += strlen("OK");
					} else {
						token = TOKEN_UNKNOWN;
						i++;
					}
					break;
				case ' ':
				case '\r':
				case '\n':
					token = TOKEN_WHITESPACE;
					i++;
					break;
				case '0':
					token = TOKEN_DATA_0;
					i++;
					break;
				case '1':
					token = TOKEN_DATA_1;
					i++;
					break;
				case '2':
					token = TOKEN_DATA_2;
					i++;
					break;
				case '3':
					token = TOKEN_DATA_3;
					i++;
					break;
				case '4':
					token = TOKEN_DATA_4;
					i++;
					break;
				case '5':
					token = TOKEN_DATA_5;
					i++;
					break;
				case '6':
					token = TOKEN_DATA_6;
					i++;
					break;
				case '7':
					token = TOKEN_DATA_7;
					i++;
					break;
				case '8':
					token = TOKEN_DATA_8;
					i++;
					break;
				case '9':
					token = TOKEN_DATA_9;
					i++;
					break;
				case 'B':
					token = TOKEN_DATA_B;
					i++;
					break;
				case 'C':
					token = TOKEN_DATA_C;
					i++;
					break;
				case 'D':
					token = TOKEN_DATA_D;
					i++;
					break;
				case 'E':
					token = TOKEN_DATA_E;
					i++;
					break;
				case 'F':
					token = TOKEN_DATA_F;
					i++;
					break;
				default:
					token = TOKEN_UNKNOWN;
					i++;
			}

			switch (token) {
				case TOKEN_COMMAND_ZISI_ECHO:
				case TOKEN_RESPONSE_ZISI:
				case TOKEN_RESPONSE_OK:
				case TOKEN_WHITESPACE:
					/* nothing to do, just skip */
					break;
				case TOKEN_DATA_0:
				case TOKEN_DATA_1:
				case TOKEN_DATA_2:
				case TOKEN_DATA_3:
				case TOKEN_DATA_4:
				case TOKEN_DATA_5:
				case TOKEN_DATA_6:
				case TOKEN_DATA_7:
				case TOKEN_DATA_8:
				case TOKEN_DATA_9:
				case TOKEN_DATA_A:
				case TOKEN_DATA_B:
				case TOKEN_DATA_C:
				case TOKEN_DATA_D:
				case TOKEN_DATA_E:
				case TOKEN_DATA_F:
					switch (state) {
						case STATE_ALPHA0:
							alpha = token;
							state = STATE_ALPHA1;
							break;
						case STATE_ALPHA1:
							alpha = (alpha << 4) | token;
							state = STATE_RED0;
							break;
						case STATE_RED0:
							red = token;
							state = STATE_RED1;
							break;
						case STATE_RED1:
							red = (red << 4) | token;
							state = STATE_GREEN0;
							break;
						case STATE_GREEN0:
							green = token;
							state = STATE_GREEN1;
							break;
						case STATE_GREEN1:
							green = (green << 4) | token;
							state = STATE_BLUE0;
							break;
						case STATE_BLUE0:
							blue = token;
							state = STATE_BLUE1;
							break;
						case STATE_BLUE1:
							blue = (blue << 4) | token;
							length = SONYERICSSON_Screenshot_addPixel(alpha, red, green, blue, p);
							s->Phone.Data.Picture->Length += length;
							p += length;
							state = STATE_ALPHA0;
							break;
					}
					break;
				case TOKEN_UNKNOWN:
					break;

			}

		}
		break;

	case AT_Reply_Error:
		error = ERR_UNKNOWN;
		break;

	case AT_Reply_CMSError:
		error = ATGEN_HandleCMSError(s);
		break;

	case AT_Reply_CMEError:
		error = ATGEN_HandleCMEError(s);
		break;

	default:
		error = ERR_UNKNOWNRESPONSE;
	}

	return error;
}

#endif
