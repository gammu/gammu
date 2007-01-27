/*
 * $Id: proto_profile.cpp,v 1.2 2005/03/24 00:08:12 bozo Exp $
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
 * This file contains the profile message handler.
 *
 */

#include "proto.h"


static void ReadProfileL(PktBuf &in, PktBuf &out)
{
	TUint16 id;
	TBuf<256> ringtone1, ringtone2;
	int call_alert, volume, vibra, key_tone, i;

	g->InitProfileL();

	in >> id;
	in.FinishL();

	if (id >= 5) {
		out << (TUint16)GNAPPLET_MSG_PROFILE_READ_RESP;
		out << (TUint16)GN_ERR_INVALIDLOCATION;
		return;
	}

	int i5;
	CArrayFixFlat<TContactItemId> *foo = new (ELeave)CArrayFixFlat<long>(10);
	g->profile->GetProfileMultiData(ringtone1, ringtone2, call_alert, volume, vibra, key_tone, foo, i5, id);
	delete foo;
	//Debug(_L("%S|%S|%d|%d|%d|%d|%d"), &ringtone1, &ringtone2, call_alert, volume, vibra, key_tone, i5);

	CPermanentFileStore *fstore =  CPermanentFileStore::OpenLC(
			    *g->profilefs,
			    _L("c:\\system\\Apps\\profileApp\\dbProfile.db"),
			    EFileRead | EFileWrite);
	RDbStoreDatabase database;    
	database.OpenL(fstore, fstore->Root());
	RDbTable table;
	// tableId, profileName, profileUID, remove, modify, active, visible, rename
	table.Open(database, _L("ProfileLookup"));
	table.FirstL();
	for (i = 0; i < id; i++)
		table.NextL();
	table.GetL();
	TBuf<16> name = table.ColDes16(2);
	table.Close();
	database.Close();
	CleanupStack::PopAndDestroy(fstore);

	out << (TUint16)GNAPPLET_MSG_PROFILE_READ_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << id;
	if (name.Length() == 0 || name == _L(" ")) {
		switch (id) {
		case 0: out << _L("General"); break;
		case 1: out << _L("Silent"); break;
		case 2: out << _L("Meeting"); break;
		case 3: out << _L("Outdoor"); break;
		case 4: out << _L("Pager"); break;
		}
		out << (TUint16)id;
	} else {
		out << name;
		out << (TUint16)0xffff;
	}
	switch (key_tone) {
	case 0x00: out << (TUint8)GN_PROFILE_KEYVOL_Off; break;
	case 0x01: out << (TUint8)GN_PROFILE_KEYVOL_Level1; break;
	case 0x02: out << (TUint8)GN_PROFILE_KEYVOL_Level2; break;
	case 0x03: out << (TUint8)GN_PROFILE_KEYVOL_Level3; break;
	default: User::Leave(1);
	}
	// lights
	switch (call_alert) {
	case 0x00: out << (TUint8)GN_PROFILE_CALLALERT_Ringing; break;
	case 0x01: out << (TUint8)GN_PROFILE_CALLALERT_Ascending; break;
	case 0x02: out << (TUint8)GN_PROFILE_CALLALERT_RingOnce; break;
	case 0x03: out << (TUint8)GN_PROFILE_CALLALERT_BeepOnce; break;
	case 0x04: out << (TUint8)GN_PROFILE_CALLALERT_Off; break;
	default: User::Leave(1);
	}
	//out << ringtone; // ringtone
	out << (TUint8)volume;
	// message tone FIXME: mar nem enum!!!
	out << (TUint8)GN_PROFILE_WARNING_On; // warning tone
	out << (TUint8)(vibra ? GN_PROFILE_VIBRATION_On : GN_PROFILE_VIBRATION_Off);
	// caller groups
	// automatic answer
}


static void GetActiveProfileL(PktBuf &in, PktBuf &out)
{
	int id;

	g->InitProfileL();

	in.FinishL();

	CPermanentFileStore *fstore =  CPermanentFileStore::OpenLC(
			    *g->profilefs,
			    _L("c:\\system\\Apps\\profileApp\\dbProfile.db"),
			    EFileRead | EFileWrite);
	RDbStoreDatabase database;    
	database.OpenL(fstore, fstore->Root());
	RDbTable table;
	// tableId, profileName, profileUID, remove, modify, active, visible, rename
	table.Open(database, _L("ProfileLookup"));
	table.FirstL();
	for (id = 0; table.AtRow(); id++) {
		table.GetL();
		if (table.ColInt(6) != 0) break;
		table.NextL();
	}
	table.Close();
	database.Close();
	CleanupStack::PopAndDestroy(fstore);

	out << (TUint16)GNAPPLET_MSG_PROFILE_GET_ACTIVE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint16)id;
}


void HandleProfileMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_PROFILE_READ_REQ:
		ReadProfileL(in, out);
		break;

	case GNAPPLET_MSG_PROFILE_GET_ACTIVE_REQ:
		GetActiveProfileL(in, out);
		break;

	case GNAPPLET_MSG_PROFILE_WRITE_REQ:
	case GNAPPLET_MSG_PROFILE_SET_ACTIVE_REQ:
	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
