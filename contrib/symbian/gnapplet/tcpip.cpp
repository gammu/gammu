/*
 * $Id: tcpip.cpp,v 1.1 2005/09/01 22:28:50 bozo Exp $
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

#include "tcpip.h"


TCPIP::~TCPIP(void)
{
	CloseL();
	listen.Close();
	rserv.Close();
}


void TCPIP::ConstructL(TUint16 port)
{
	TInt error;
	TInetAddr addr;

	User::LeaveIfError(rserv.Connect());

	if ((error = listen.Open(rserv, KAfInet, KSockStream, KProtocolInetTcp)) != KErrNone) {
		User::Leave(error);
	}

	addr.SetPort(port);
	if ((error = listen.Bind(addr)) != KErrNone) {
		User::Leave(error);
	}

	listen.Listen(1);
}


void TCPIP::AcceptL(TRequestStatus &rs)
{
	User::LeaveIfError(conn.Open(rserv));

	listen.Accept(conn, rs);
}


void TCPIP::CloseL(void)
{
	conn.Close();
}


void TCPIP::ReadL(TRequestStatus &rs, TDes8 &buf)
{
	conn.Read(buf, rs);
}


void TCPIP::WriteL(TRequestStatus &rs, const TDes8 &buf)
{
	conn.Write(buf, rs);
}
