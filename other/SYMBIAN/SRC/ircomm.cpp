/*
 * $Id: ircomm.cpp,v 1.1.1.1 2004/04/03 21:29:46 bozo Exp $
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
 * This file provides IrCOMM support.
 *
 */

#include "ircomm.h"


IrCOMM::~IrCOMM(void)
{
	CloseL();
	rserv.UnloadCommModule(_L("IrCOMM"));
}


void IrCOMM::ConstructL(void)
{
	TInt error;

	error = User::LoadPhysicalDevice(_L("EUART1"));
	if (error != KErrNone && error != KErrAlreadyExists) {
		User::Leave(error);
	}
	error = User::LoadLogicalDevice(_L("ECOMM"));
	if (error != KErrNone && error != KErrAlreadyExists) {
		User::Leave(error);
	}

	rserv.Connect();
	rserv.LoadCommModule(_L("IrCOMM"));
}


void IrCOMM::AcceptL(TRequestStatus &rs)
{
	TBuf8<0> b;
	TInt error;

	if ((error = rcomm.Open(rserv, _L("IrCOMM::0"), ECommExclusive)) != KErrNone) {
		User::Leave(error);
	}

	rcomm.Write(rs, b);
}


void IrCOMM::CloseL(void)
{
	rcomm.Close();
}


void IrCOMM::ReadL(TRequestStatus &rs, TDes8 &buf)
{
	rcomm.Read(rs, buf);
}


void IrCOMM::WriteL(TRequestStatus &rs, const TDes8 &buf)
{
	rcomm.Write(rs, buf);
}
