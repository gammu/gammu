/*
 * $Id: pkt.h,v 1.4 2004/10/11 20:29:43 bozo Exp $
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

#ifndef __GNAPPLET_PKT_H
#define	__GNAPPLET_PKT_H

#include <e32std.h>
#include <sys/time.h>
typedef TUint8 uint8_t;
#include <phones/gnapplet.h>

#include "gnapplet.h" /* toUTC, toLOCAL */


class PktBuf {
private:
	TUint8 *addr;
	TInt32 size;
	TInt32 offs;
public:
	PktBuf(void);
	PktBuf(void *a, TInt32 len);
	void Set(void *a, TInt32 len);
	TInt32 GetOffs(void) { return offs; }
	TUint8 *ExpandL(TInt32 len);
	void FinishL(void) { if (offs != size) User::Leave(KErrOverflow); }
};

PktBuf &operator<<(PktBuf &buf, TInt8 x);
PktBuf &operator<<(PktBuf &buf, TInt16 x);
PktBuf &operator<<(PktBuf &buf, TInt32 x);
PktBuf &operator<<(PktBuf &buf, TUint8 x);
PktBuf &operator<<(PktBuf &buf, TUint16 x);
PktBuf &operator<<(PktBuf &buf, TUint32 x);
PktBuf &operator<<(PktBuf &buf, const TDesC &x);
PktBuf &operator<<(PktBuf &buf, const TTime &x);
PktBuf &operator<<(PktBuf &buf, TBool x);
PktBuf &operator<<(PktBuf &buf, const TDesC8 &x);

PktBuf &operator>>(PktBuf &buf, TInt8 &x);
PktBuf &operator>>(PktBuf &buf, TInt16 &x);
PktBuf &operator>>(PktBuf &buf, TInt32 &x);
PktBuf &operator>>(PktBuf &buf, TUint8 &x);
PktBuf &operator>>(PktBuf &buf, TUint16 &x);
PktBuf &operator>>(PktBuf &buf, TUint32 &x);
PktBuf &operator>>(PktBuf &buf, TDes &x);
PktBuf &operator>>(PktBuf &buf, TTime &x);
PktBuf &operator>>(PktBuf &buf, TBool &x);
PktBuf &operator>>(PktBuf &buf, TDes8 &x);

#endif
