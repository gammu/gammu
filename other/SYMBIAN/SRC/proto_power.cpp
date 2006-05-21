/*
 * $Id: proto_power.cpp,v 1.1 2004/04/08 23:29:06 bozo Exp $
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
 * This file contains the power status message handler.
 *
 */

#include "proto.h"


static void PowerInfoReqL(PktBuf &in, PktBuf &out)
{
	MBasicGsmPhoneBatteryAndPower::TBatteryInfo batt_info;
	TUint8 percent, source;

	g->InitPhoneInfoL();

	User::LeaveIfError(g->phone->GetBatteryInfo(batt_info));
	percent = batt_info.iChargeLevel;
	switch (batt_info.iStatus) {
	case MBasicGsmPhoneBatteryAndPower::EPoweredByBattery:
		source = GN_PS_BATTERY;
		break;
	case MBasicGsmPhoneBatteryAndPower::EBatteryConnectedButExternallyPowered:
	case MBasicGsmPhoneBatteryAndPower::ENoBatteryConnected:
	case MBasicGsmPhoneBatteryAndPower::EPowerFault:
	default:
		source = GN_PS_ACDC;
		break;
	}

	out << (TUint16)GNAPPLET_MSG_POWER_INFO_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint8)percent;
	out << (TUint8)source;
}


void HandlePowerMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_POWER_INFO_REQ:
		in.FinishL();
		PowerInfoReqL(in, out);
		break;

	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
