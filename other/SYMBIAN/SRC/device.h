/*
 * $Id: device.h,v 1.2 2004/04/07 23:46:40 bozo Exp $
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

#ifndef	__GNAPPLET_DEVICE_H
#define	__GNAPPLET_DEVICE_H

#include <c32comm.h>


class Device {
public:
	virtual ~Device() {};
	virtual void AcceptL(TRequestStatus &rs) = 0;
	virtual void CloseL(void) = 0;
	virtual void ReadL(TRequestStatus &rs, TDes8 &buf) = 0;
	virtual void WriteL(TRequestStatus &rs, const TDes8 &buf) = 0;
};

void WaitFor(TRequestStatus &rs, int msec);
void ReadL(Device *dev, TDes8 &buf);
void WriteL(Device *dev, const TDes8 &buf);
TBool ReadPacketL(Device *dev, int &type, TDes8 &msg);
void WritePacketL(Device *dev, int type, const TDes8 &msg);

#endif
