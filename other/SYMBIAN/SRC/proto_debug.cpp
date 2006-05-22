/*
 * $Id: proto_debug.cpp,v 1.1 2004/04/12 11:30:04 bozo Exp $
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
 * This file contains the debugging message handler.
 *
 */

#include "proto.h"


void Debug(TRefByValue<const TDesC> fmt, ...)
{
	TBuf8<1100> buf;
	TBuf<512> msg;
	PktBuf pkt;
	VA_LIST ap;

	VA_START(ap, fmt);
	msg.FormatList(fmt, ap);
	VA_END(ap);

	buf.Zero();
	pkt.Set(buf.Ptr(), 1100);
	
	pkt << (TUint16)GNAPPLET_MSG_DEBUG_NOTIFICATION;
	pkt << (TUint16)GN_ERR_NONE;
	pkt << msg;

	buf.SetLength(pkt.GetOffs());
	WritePacketL(g->current_device, GNAPPLET_MSG_DEBUG, buf);
}


void HandleDebugMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {

	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
