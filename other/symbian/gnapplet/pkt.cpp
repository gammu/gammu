/*
 * $Id: pkt.cpp,v 1.4 2004/10/11 20:29:43 bozo Exp $
 *
 * G N A P P L E T
 *
 * gnapplet is a gnbus protocol driver for symbian phones.
 *
 * This file is part of gnokii.
 *
 * Gnokii is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Gnokii is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gnokii; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright (C) 2004 BORBELY Zoltan
 *
 * This file provides encoding/decoding support for network communication.
 *
 */

#include "pkt.h"


PktBuf::PktBuf(void)
{
	addr = 0;
	size = 0;
	offs = 0;
}


PktBuf::PktBuf(void *a, TInt32 len)
{
	addr = (TUint8 *)a;
	size = len;
	offs = 0;
}


void PktBuf::Set(void *a, TInt32 len)
{
	addr = (TUint8 *)a;
	size = len;
	offs = 0;
}


TUint8 *PktBuf::ExpandL(TInt32 len)
{
	TUint8 *a;

	if (size < offs + len) User::Leave(KErrOverflow);

	a = addr + offs;
	offs += len;

	return a;
}


PktBuf &operator<<(PktBuf &buf, TInt8 x)
{
	TUint8 *b = buf.ExpandL(1);

	b[0] = (TUint8)x;

	return buf;
}


PktBuf &operator<<(PktBuf &buf, TInt16 x)
{
	TUint8 *b = buf.ExpandL(2);

	b[0] = (TUint8)((x >> 8) & 0xff);
	b[1] = (TUint8)(x & 0xff);

	return buf;
}


PktBuf &operator<<(PktBuf &buf, TInt32 x)
{
	TUint8 *b = buf.ExpandL(4);

	b[0] = (TUint8)((x >> 24) & 0xff);
	b[1] = (TUint8)((x >> 16) & 0xff);
	b[2] = (TUint8)((x >> 8) & 0xff);
	b[3] = (TUint8)(x & 0xff);

	return buf;
}


PktBuf &operator<<(PktBuf &buf, TUint8 x)
{
	TUint8 *b = buf.ExpandL(1);

	b[0] = x;

	return buf;
}


PktBuf &operator<<(PktBuf &buf, TUint16 x)
{
	TUint8 *b = buf.ExpandL(2);

	b[0] = ((x >> 8) & 0xff);
	b[1] = (x & 0xff);

	return buf;
}


PktBuf &operator<<(PktBuf &buf, TUint32 x)
{
	TUint8 *b = buf.ExpandL(4);

	b[0] = ((x >> 24) & 0xff);
	b[1] = ((x >> 16) & 0xff);
	b[2] = ((x >> 8) & 0xff);
	b[3] = (x & 0xff);

	return buf;
}


PktBuf &operator<<(PktBuf &buf, const TDesC &x)
{
	TUint16 n = x.Length();
	TUint8 *b;
	int i;

	buf << n;

	b = buf.ExpandL(2 * n);
	for(i = 0; i < n; i++) {
		b[2 * i] = (x[i] >> 8);
		b[2 * i + 1] = (x[i] & 0xff);
	}

	return buf;
}


PktBuf &operator<<(PktBuf &buf, const TTime &x)
{
	TTime t;
	TDateTime dt;
	TUint8 month, day;

	t = toLOCAL(x);

	dt = t.DateTime();
	month = dt.Month() + 1;
	day = dt.Day() + 1;

	buf << (TUint16)dt.Year() << month << day;
	buf << (TUint8)dt.Hour() << (TUint8)dt.Minute() << (TUint8)dt.Second();

	return buf;
}


PktBuf &operator<<(PktBuf &buf, const TBool x)
{
	buf << static_cast<TUint8>(x ? 1 : 0);

	return buf;
}


PktBuf &operator<<(PktBuf &buf, const TDesC8 &x)
{
	TUint16 n = x.Length();
	TUint8 *b;
	int i;

	buf << n;

	b = buf.ExpandL(n);
	for(i = 0; i < n; i++) {
		b[i] = x[i];
	}

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TInt8 &x)
{
	TUint8 *b = buf.ExpandL(1);

	x = (TInt8)b[0];

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TInt16 &x)
{
	TUint8 *b = buf.ExpandL(2);

	x = (TInt16)((b[0] << 8) + b[1]);

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TInt32 &x)
{
	TUint8 *b = buf.ExpandL(4);

	x = (TInt32)((b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3]);

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TUint8 &x)
{
	TUint8 *b = buf.ExpandL(1);

	x = b[0];

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TUint16 &x)
{
	TUint8 *b = buf.ExpandL(2);

	x = (b[0] << 8) + b[1];

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TUint32 &x)
{
	TUint8 *b = buf.ExpandL(4);

	x = (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TDes &x)
{
	TUint16 l;
	TUint8 *b;
	int i;

	buf >> l;
	b = buf.ExpandL(2 * l);

	x.SetLength(l);
	for(i = 0; i < l; i++)
		x[i] = (b[2 * i] << 8) + b[2 * i + 1];
	x.ZeroTerminate();

	return buf;
}

PktBuf &operator>>(PktBuf &buf, TTime &x)
{
	TUint16 year;
	TUint8 month, day, hour, minute, second;
	TDateTime dt;

	buf >> year >> month >> day >> hour >> minute >> second;
	dt.Set(year, TMonth(month - 1), day - 1, hour, minute, second, 0);
	x = toUTC(TTime(dt));

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TBool &x)
{
	TUint8 b;

	buf >> b;

	x = b ? true : false;

	return buf;
}


PktBuf &operator>>(PktBuf &buf, TDes8 &x)
{
	TUint16 l;
	TUint8 *b;
	int i;

	buf >> l;
	b = buf.ExpandL(l);

	x.SetLength(l);
	for(i = 0; i < l; i++)
		x[i] = b[i];

	return buf;
}
