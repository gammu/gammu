/*
 * $Id: device.cpp,v 1.3 2005/09/01 22:28:50 bozo Exp $
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
 * This file provides device independent I/O support.
 *
 */

#include "device.h"

#define	GNBUS_MAGIC_BYTE	0x5a


void WaitFor(TRequestStatus &rs, int msec)
{
	TRequestStatus trs;
	RTimer timer;

	timer.CreateLocal();
	timer.After(trs, TTimeIntervalMicroSeconds32(msec * 1000));

	while (rs == KRequestPending && trs == KRequestPending) {
		User::WaitForAnyRequest();
	}

	timer.Cancel();
	timer.Close();
}


void ReadL(Device *dev, TDes8 &buf)
{
	TRequestStatus rs;

	dev->ReadL(rs, buf);
	User::WaitForRequest(rs);
	switch (rs.Int()) {
	case KErrNone:
	case KErrDisconnected:
	case KErrEof:
		break;
	default:
		User::Leave(rs.Int());
		break;
	}
}


void WriteL(Device *dev, const TDes8 &buf)
{
	TRequestStatus rs;

	dev->WriteL(rs, buf);
	WaitFor(rs, 10000);
	if (rs != KErrNone) {
		User::Leave(rs.Int());
	}
}


TBool ReadPacketL(Device *dev, int &type, TDes8 &msg)
{
	TBuf8<1> b;
	TBuf8<8> header;
	TUint8 checksum[2];
	int len, c;

	checksum[0] = checksum[1] = 0;
	while (header.Length() < 6) {
		ReadL(dev, b);
		if (!b.Length()) return false;
		checksum[header.Length() & 1] ^= b[0];
		header.Append(b);
	}

	type = header[4];
	c = 0;
	len = (header[2] << 8) + header[3];
	msg.Zero();
	while (len > 0) {
		ReadL(dev, b);
		checksum[c] ^= b[0];
		msg.Append(b);
		c ^= 1;
		len--;
	}
	for (len = 2 + c; len > 0; c ^= 1, len--) {
		ReadL(dev, b);
		checksum[c] ^= b[0];
	}
	if (checksum[0] != 0x00 || checksum[1] != 0x00) {
		User::Leave(666);
	}

	return true;
}


void WritePacketL(Device *dev, int type, const TDes8 &msg)
{
	TBuf8<2048> pkt;
	int i;
	TUint8 checksum[2];

	pkt.Append(GNBUS_MAGIC_BYTE);	/* magic */
	pkt.Append(0);			/* sequence */
	pkt.Append(msg.Length() >> 8);	/* length MSB */
	pkt.Append(msg.Length() & 0xff);/* length LSB */
	pkt.Append(type);		/* type */
	pkt.Append(0);			/* reserved */
	pkt.Append(msg);
	if (pkt.Length() & 1) pkt.Append(0);

	checksum[0] = checksum[1] = 0;
	for (i = 0; i < pkt.Length(); i++) {
		checksum[i & 1] ^= pkt[i];
	}

	pkt.Append(checksum[0]);
	pkt.Append(checksum[1]);

	WriteL(dev, pkt);
}
