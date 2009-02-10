/*
 * $Id: rfcomm.cpp,v 1.2 2005/04/12 20:18:24 bozo Exp $
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
 * This file provides RfCOMM support.
 *
 */

#include <btmanclient.h>

#include "rfcomm.h"


RfCOMM::~RfCOMM(void)
{
	CloseL();
	listen.Close();
	rserv.Close();
}


void RfCOMM::ConstructL(int port)
{
	TInt error;
	TBTSockAddr addr;
	RBTMan btSecManager;
	RBTSecuritySettings btSecSettings;
	TBTServiceSecurity btServiceSecurity(TUid::Uid(0x107d4df2), KSolBtRFCOMM, 0);
	TRequestStatus rs;

	rserv.Connect();

	if ((error = listen.Open(rserv, _L("RFCOMM"))) != KErrNone) {
		User::Leave(error);
	}

	addr.SetPort(port);
	if ((error = listen.Bind(addr)) != KErrNone) {
		User::Leave(error);
	}

	listen.Listen(1);

	btSecManager.Connect();
	btSecSettings.Open(btSecManager);
	btServiceSecurity.SetAuthentication(TRUE);
	btServiceSecurity.SetEncryption(TRUE);
	btServiceSecurity.SetAuthorisation(TRUE);
	btServiceSecurity.SetChannelID(port);
	btSecSettings.RegisterService(btServiceSecurity, rs);
	User::WaitForRequest(rs);
}


void RfCOMM::AcceptL(TRequestStatus &rs)
{
	User::LeaveIfError(conn.Open(rserv));

	listen.Accept(conn, rs);
}


void RfCOMM::CloseL(void)
{
	conn.Close();
}


void RfCOMM::ReadL(TRequestStatus &rs, TDes8 &buf)
{
	conn.Read(buf, rs);
}


void RfCOMM::WriteL(TRequestStatus &rs, const TDes8 &buf)
{
	conn.Write(buf, rs);
}
