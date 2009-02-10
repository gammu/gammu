/*
 * $Id: tcpip.h,v 1.1 2005/09/01 22:28:50 bozo Exp $
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
 * Copyright (C) 2005 BORBELY Zoltan
 *
 * This file provides TCP/IP support.
 *
 */

#ifndef	__GNAPPLET_TCPIP_H
#define	__GNAPPLET_TCPIP_H

#include <in_sock.h>

#include "device.h"


class TCPIP: public Device {
	RSocketServ rserv;
	RSocket listen;
	RSocket conn;
public:
	TCPIP(void) {};
	~TCPIP(void);
	void ConstructL(TUint16 port);
	void AcceptL(TRequestStatus &rs);
	void CloseL(void);
	void ReadL(TRequestStatus &rs, TDes8 &buf);
	void WriteL(TRequestStatus &rs, const TDes8 &buf);
};

#endif
