/*
 * $Id: proto_clock.cpp,v 1.1 2004/10/11 20:29:43 bozo Exp $
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
 * This file contains the clock and alarm message handler.
 *
 */

#include "proto.h"


static void ReadClockL(PktBuf &in, PktBuf &out)
{
	TTime t;

	in.FinishL();

	t.UniversalTime();

	out << (TUint16)GNAPPLET_MSG_CLOCK_DATETIME_READ_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << t;
}


static void WriteClockL(PktBuf &in, PktBuf &out)
{
	TTime t;

	in >> t;
	in.FinishL();

	if (User::SetHomeTime(toLOCAL(t)) != KErrNone) {
		out << (TUint16)GNAPPLET_MSG_CLOCK_DATETIME_WRITE_RESP;
		out << (TUint16)GN_ERR_FAILED;
		return;
	}

	out << (TUint16)GNAPPLET_MSG_CLOCK_DATETIME_WRITE_RESP;
	out << (TUint16)GN_ERR_NONE;
}


static void ReadAlarmL(PktBuf &in, PktBuf &out)
{
	TAlarmInfo info;
	TBool enabled;

	g->InitClockL();

	in.FinishL();

	enabled = false;
	while (g->alarmserv->AlarmInfo(info, RAlarmServer::EInfoNext) == KErrNone) {
		if (info.iType == EAlarmTypeClock) {
			enabled = true;
			break;
		}
	}

	out << (TUint16)GNAPPLET_MSG_CLOCK_ALARM_READ_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << enabled;
	out << (enabled ? toUTC(info.iAlarmTime) : TTime(0));
}


static void WriteAlarmL(PktBuf &in, PktBuf &out)
{
	TAlarmInfo info;
	TBool enabled;
	TTime alarm;

	g->InitClockL();

	in >> enabled;
	in >> alarm;
	in.FinishL();

	if (enabled) {
		g->alarmserv->SetClockAlarm(0, toLOCAL(alarm), _L(""), _L("a"), EAlarmClockNext24hours);
	} else {
		while (g->alarmserv->AlarmInfo(info, RAlarmServer::EInfoNext) == KErrNone) {
			if (info.iType == EAlarmTypeClock) {
				g->alarmserv->AlarmDelete(info.iAlarmId);
				break;
			}
		}
	}

	out << (TUint16)GNAPPLET_MSG_CLOCK_ALARM_WRITE_RESP;
	out << (TUint16)GN_ERR_NONE;
}


void HandleClockMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_CLOCK_DATETIME_READ_REQ:
		ReadClockL(in, out);
		break;

	case GNAPPLET_MSG_CLOCK_DATETIME_WRITE_REQ:
		WriteClockL(in, out);
		break;

	case GNAPPLET_MSG_CLOCK_ALARM_READ_REQ:
		ReadAlarmL(in, out);
		break;

	case GNAPPLET_MSG_CLOCK_ALARM_WRITE_REQ:
		WriteAlarmL(in, out);
		break;

	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
