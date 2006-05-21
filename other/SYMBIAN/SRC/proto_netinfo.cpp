/*
 * $Id: proto_netinfo.cpp,v 1.1 2004/04/08 23:29:06 bozo Exp $
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
 * This file contains the network status message handler.
 *
 */

#include "proto.h"


static void NetinfoGetCurrentReqL(PktBuf &in, PktBuf &out)
{
	MBasicGsmPhoneNetwork::TCurrentNetworkInfo net_info;
	TBuf<8> net_code;

	g->InitPhoneInfoL();

	User::LeaveIfError(g->phone->GetCurrentNetworkInfo(net_info));
	net_code.Format(_L("%03d %02d"), net_info.iNetworkInfo.iId.iMCC, net_info.iNetworkInfo.iId.iMNC);

	out << (TUint16)GNAPPLET_MSG_NETINFO_GETCURRENT_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint16)net_info.iCellId;
	out << (TUint16)net_info.iLocationAreaCode;
	out << (TUint8)0;	// FIXME: registration status
	out << net_code;
}


static void NetinfoGetRFLevelReqL(PktBuf &in, PktBuf &out)
{
	TUint8 percent;

	g->InitPhoneInfoL();

	switch (g->sysagent->GetState(KUidNetworkStrength)) {
	case ESANetworkStrengthLow:
		percent = 33;
		break;
	case ESANetworkStrengthMedium:
		percent = 66;
		break;
	case ESANetworkStrengthHigh:
		percent = 100;
		break;
	case ESANetworkStrengthNone:
	case ESANetworkStrengthUnknown:
	default:
		percent = 0;
		break;
	}
	
	out << (TUint16)GNAPPLET_MSG_NETINFO_GETRFLEVEL_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint8)percent;
}


void HandleNetinfoMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_NETINFO_GETCURRENT_REQ:
		in.FinishL();
		NetinfoGetCurrentReqL(in, out);
		break;

	case GNAPPLET_MSG_NETINFO_GETRFLEVEL_REQ:
		in.FinishL();
		NetinfoGetRFLevelReqL(in, out);
		break;

	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
