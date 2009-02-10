/*
 * $Id: proto_phonebook.cpp,v 1.8 2004/09/13 22:08:54 bozo Exp $
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
 * Copyright (C) 2007 Matthias Lechner <matthias@lmme.de>
 *
 * This file contains the phonebook message handler.
 *
 */

#include "proto.h"
_LIT(KTsyName,"phonetsy.tsy");


static void LogFilterL(TUint16 memtype)
{
	CLogFilter *filter;
	TLogString direction;
	TRequestStatus status;

	filter = CLogFilter::NewL();
	CleanupStack::PushL(filter);
	switch (memtype) {
	case GN_MT_DC:	// dialled calls
		User::LeaveIfError(g->logclient->GetString(direction, R_LOG_DIR_OUT));
		break;
	case GN_MT_RC:	// received calls
		User::LeaveIfError(g->logclient->GetString(direction, R_LOG_DIR_IN));
		break;
	case GN_MT_MC:	// missed calls
		User::LeaveIfError(g->logclient->GetString(direction, R_LOG_DIR_MISSED));
		break;
	default:
		User::Invariant();
	}
	filter->SetDirection(direction);

	WaitIfReqL(g->logview->SetFilterL(*filter, status), status);
	CleanupStack::PopAndDestroy(filter);
}


static void PhonebookReadNextReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint16 ns;
	CContactItem *item;
	TBuf<64> family_name, given_name, name;
	TContactItemId location;
	int i, n;
	TUint16 start;

	g->InitPhoneBookL();
	in >> start;
	in.FinishL();

	if( start == 1 )
		g->contactIter->Reset();	

	location = g->contactIter->NextL();
	if( location == KNullContactId ) {
		cons->Printf(_L("eof\n"));
		g->contactIter->Reset();
		out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READNEXT_RESP;
		out << (TUint16)GN_ERR_EMPTYLOCATION;
	}

	TRAPD(error, item = g->cdb->ReadContactL(location));
	if (error != KErrNone) {
		out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READNEXT_RESP;
		out << (TUint16)GN_ERR_EMPTYLOCATION;
		return;
	}
	cons->Printf(_L("read: %d\n"), location);

	CleanupStack::PushL(item);
	CContactItemFieldSet &fieldset = item->CardFields();
	n = fieldset.Count();
	ns = 1;

	for (i = 0; i < n; i++) {
		const CContactItemField &field = fieldset[i];

		if (field.StorageType() == KStorageTypeText && field.TextStorage()->Text().Length() <= 0) {
			continue;
		}

		if (field.ContentType().ContainsFieldType(KUidContactFieldGivenName)) {
			given_name.Copy(field.TextStorage()->Text());
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldFamilyName)) {
			family_name.Copy(field.TextStorage()->Text());
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldFax)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldPhoneNumber)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldEMail)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldNote)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldRingTone)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldUrl)) {
			ns++;
		}
	}
	if (family_name.Length() > 0 && given_name.Length() > 0) {
		name.Copy(family_name);
		name.Append(_L(" "));
		name.Append(given_name);
	} else if (family_name.Length() > 0) {
		name.Copy(family_name);
	} else {
		name.Copy(given_name);
	}

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READNEXT_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint16) location;
	out << ns;
	out << (TUint16)GN_PHONEBOOK_ENTRY_Name << (TUint16)0 << name;
	for (i = 0; i < n; i++) {
		const CContactItemField &field = fieldset[i];

		if (field.StorageType() == KStorageTypeText && field.TextStorage()->Text().Length() <= 0) {
			continue;
		}

		if (field.ContentType().ContainsFieldType(KUidContactFieldFax)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Number << (TUint16)GN_PHONEBOOK_NUMBER_Fax << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldPhoneNumber)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Number;
			if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapFAX)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Fax;
			} else if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapHOME)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Home;
			} else if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapWORK)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Work;
			} else if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapCELL)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Mobile;
			} else {
				out << (TUint16)GN_PHONEBOOK_NUMBER_General;
			}
			out << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldEMail)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Email << (TUint16)0 << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldNote)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Note << (TUint16)0 << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldRingTone)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Ringtone << (TUint16)0 << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldUrl)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_URL << (TUint16)0 << field.TextStorage()->Text();
		}
	}
	CleanupStack::PopAndDestroy(item);
}

static void PhonebookReadReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint16 ns;
	TUint32 location;
	CContactItem *item;
	TBuf<64> family_name, given_name, name;
	int i, n;

	g->InitPhoneBookL();

	in >> location;
	in.FinishL();

	TRAPD(error, item = g->cdb->ReadContactL(location));
	if (error != KErrNone) {
		out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READ_RESP;
		out << (TUint16)GN_ERR_EMPTYLOCATION;
		return;
	}
	cons->Printf(_L("read: %d\n"), location);

	CleanupStack::PushL(item);
	CContactItemFieldSet &fieldset = item->CardFields();
	n = fieldset.Count();
	ns = 1;

	for (i = 0; i < n; i++) {
		const CContactItemField &field = fieldset[i];

		if (field.StorageType() == KStorageTypeText && field.TextStorage()->Text().Length() <= 0) {
			continue;
		}

		if (field.ContentType().ContainsFieldType(KUidContactFieldGivenName)) {
			given_name.Copy(field.TextStorage()->Text());
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldFamilyName)) {
			family_name.Copy(field.TextStorage()->Text());
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldFax)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldPhoneNumber)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldEMail)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldNote)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldRingTone)) {
			ns++;
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldUrl)) {
			ns++;
		}
	}
	if (family_name.Length() > 0 && given_name.Length() > 0) {
		name.Copy(family_name);
		name.Append(_L(" "));
		name.Append(given_name);
	} else if (family_name.Length() > 0) {
		name.Copy(family_name);
	} else {
		name.Copy(given_name);
	}

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READ_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << ns;
	out << (TUint16)GN_PHONEBOOK_ENTRY_Name << (TUint16)0 << name;
	for (i = 0; i < n; i++) {
		const CContactItemField &field = fieldset[i];

		if (field.StorageType() == KStorageTypeText && field.TextStorage()->Text().Length() <= 0) {
			continue;
		}

		if (field.ContentType().ContainsFieldType(KUidContactFieldFax)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Number << (TUint16)GN_PHONEBOOK_NUMBER_Fax << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldPhoneNumber)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Number;
			if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapFAX)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Fax;
			} else if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapHOME)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Home;
			} else if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapWORK)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Work;
			} else if (field.ContentType().ContainsFieldType(KUidContactFieldVCardMapCELL)) {
				out << (TUint16)GN_PHONEBOOK_NUMBER_Mobile;
			} else {
				out << (TUint16)GN_PHONEBOOK_NUMBER_General;
			}
			out << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldEMail)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Email << (TUint16)0 << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldNote)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Note << (TUint16)0 << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldRingTone)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_Ringtone << (TUint16)0 << field.TextStorage()->Text();
		} else if (field.ContentType().ContainsFieldType(KUidContactFieldUrl)) {
			out << (TUint16)GN_PHONEBOOK_ENTRY_URL << (TUint16)0 << field.TextStorage()->Text();
		}
	}
	CleanupStack::PopAndDestroy(item);
}


static void PhonebookWriteReqL(PktBuf &in, PktBuf &out)
{
	TUint16 memtype, ns, type, subtype;
	TUint32 location;
	CContactItem *item;
	CContactItemFieldSet *fieldset;
	CContactItemField *field;
	TBuf<64> name;
	TPtrC family_name, given_name;
	int i, s;

	g->InitPhoneBookL();

	in >> memtype;
	in >> location;

	if (memtype != GN_MT_ME) {
		out << (TUint16)GNAPPLET_MSG_PHONEBOOK_WRITE_RESP;
		out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
		return;
	}

	fieldset = CContactItemFieldSet::NewLC();
	in >> ns;
	for (i = 0; i < ns; i++) {
		in >> type >> subtype >> name;
		switch (type) {
		case GN_PHONEBOOK_ENTRY_Name:
			for (s = 0; s < name.Length() && name[s] != ' '; s++) ;
			if (s >= 1 && s < name.Length()) {
				family_name.Set(name.Ptr(), s);
				given_name.Set(name.Ptr() + s + 1, name.Length() - s - 1);
			} else {
				family_name.Set(name);
			}
			if (family_name.Length() > 0) {
				field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldFamilyName);
				//field->SetLabelL(_L("alma"));
				field->SetMapping(KUidContactFieldVCardMapUnusedN);
				field->TextStorage()->SetTextL(family_name);
				fieldset->AddL(*field);
			}
			if (given_name.Length() > 0) {
				field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldGivenName);
				//field->SetLabelL(_L("alma"));
				field->SetMapping(KUidContactFieldVCardMapUnusedN);
				field->TextStorage()->SetTextL(given_name);
				fieldset->AddL(*field);
			}
			break;
		case GN_PHONEBOOK_ENTRY_Email:
			field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldEMail);
			field->SetMapping(KUidContactFieldVCardMapEMAILINTERNET);
			field->TextStorage()->SetTextL(name);
			fieldset->AddL(*field);
			break;
		case GN_PHONEBOOK_ENTRY_Note:
			field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldNote);
			field->SetMapping(KUidContactFieldVCardMapNOTE);
			field->TextStorage()->SetTextL(name);
			fieldset->AddL(*field);
			break;
		case GN_PHONEBOOK_ENTRY_Number:
			if (subtype == GN_PHONEBOOK_NUMBER_Fax) {
				field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldFax);
				field->SetMapping(KUidContactFieldVCardMapTEL);
				field->AddFieldTypeL(KUidContactFieldVCardMapFAX);
			} else {
				field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldPhoneNumber);
				field->SetMapping(KUidContactFieldVCardMapTEL);
				switch (subtype) {
				case GN_PHONEBOOK_NUMBER_Home:
					field->AddFieldTypeL(KUidContactFieldVCardMapHOME);
					break;
				case GN_PHONEBOOK_NUMBER_Mobile:
					field->AddFieldTypeL(KUidContactFieldVCardMapCELL);
					break;
				case GN_PHONEBOOK_NUMBER_Work:
					field->AddFieldTypeL(KUidContactFieldVCardMapWORK);
					break;
				}
			}
			field->TextStorage()->SetTextL(name);
			fieldset->AddL(*field);
			break;
		case GN_PHONEBOOK_ENTRY_Ringtone:
			//field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldRingTone);
			//field->TextStorage()->SetTextL(name);
			//fieldset->AddL(*field);
			break;
		case GN_PHONEBOOK_ENTRY_Group:
			//field = CContactItemField::NewL(KStorageTypeText,
			//field->TextStorage()->SetTextL(name);
			//fieldset->AddL(*field);
			break;
		case GN_PHONEBOOK_ENTRY_URL:
			field = CContactItemField::NewL(KStorageTypeText, KUidContactFieldUrl);
			field->SetMapping(KUidContactFieldVCardMapURL);
			field->TextStorage()->SetTextL(name);
			fieldset->AddL(*field);
			break;
		default:
			break;
		}
	}
	in.FinishL();

	TRAPD(error, item = g->cdb->OpenContactL(location));
	if (error == KErrNone) {
		item->UpdateFieldSet(fieldset);
		CleanupStack::Pop(fieldset);
		CleanupStack::PushL(item);
		g->cdb->CommitContactL(*item);
	} else {
		item = CContactCard::NewL();
		item->UpdateFieldSet(fieldset);
		CleanupStack::Pop(fieldset);
		CleanupStack::PushL(item);
		location = g->cdb->AddNewContactL(*item);
	}
	CleanupStack::Pop(item);

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_WRITE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	out << location;
	cons->Printf(_L("write: %d\n"), location);
}


static void PhonebookDeleteReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint32 location;

	g->InitPhoneBookL();

	in >> location;
	in.FinishL();

	TRAPD(error, g->cdb->DeleteContactL(location));

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_DELETE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	out << location;
}


static void PhonebookStatusReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint32 used, free;

	g->InitPhoneBookL();

	used = g->cdb->CountL();
	//free = 0x7fffffff - used;
	free = 0x10000 - used;	//FIXME: better limit

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_STATUS_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	out << used;
	out << free;
}


static void LogReadReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint32 location;
	TRequestStatus status;

	g->InitLogClientL();

	in >> location;
	in.FinishL();

	cons->Printf(_L("log(%d) read: %d\n"), (int)memtype, location);

	LogFilterL(memtype);
	if (location < 1 || location > static_cast<TUint32>(g->logview->CountL())) {
		out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READ_RESP;
		out << (TUint16)GN_ERR_EMPTYLOCATION;
		return;
	}

	WaitIfReqL(g->logview->FirstL(status), status);
	while (--location > 0) {
		WaitIfReqL(g->logview->NextL(status), status);
	}
	const CLogEvent &event = g->logview->Event();

	cons->Printf(_L("c: %d, n: %S\n"), (int)event.Contact(), &event.Number());

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READ_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint16)3;

	out << (TUint16)GN_PHONEBOOK_ENTRY_Name;
	out << (TUint16)0;
	out << _L("");

	out << (TUint16)GN_PHONEBOOK_ENTRY_Number;
	out << (TUint16)GN_PHONEBOOK_NUMBER_General;
	out << event.Number();

	out << (TUint16)GN_PHONEBOOK_ENTRY_Date;
	out << (TUint16)0;
	out << event.Time();
}


static void LogDeleteReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint32 i, location;
	TRequestStatus status;

	g->InitLogClientL();

	in >> location;
	in.FinishL();

	LogFilterL(memtype);
	if (location < 1 || location > static_cast<TUint32>(g->logview->CountL())) {
		out << (TUint16)GNAPPLET_MSG_PHONEBOOK_DELETE_RESP;
		out << (TUint16)GN_ERR_EMPTYLOCATION;
		return;
	}

	WaitIfReqL(g->logview->FirstL(status), status);
	for (i = 1; i < location; i++) {
		WaitIfReqL(g->logview->NextL(status), status);
	}
	g->logclient->DeleteEvent(g->logview->Event().Id(), status);
	WaitIfReqL(true, status);

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_DELETE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	out << location;
}


static void LogStatusReqL(TUint16 memtype, PktBuf &in, PktBuf &out)
{
	TUint32 used, free;

	g->InitLogClientL();

	LogFilterL(memtype);

	used = g->logview->CountL();
	free = 0;

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_STATUS_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	out << used;
	out << free;
}


static void DialNumberL(PktBuf &in, PktBuf &out) {
	/// @todo maybe use a CActive object to allow the call to be cancelled from gammu
	TBuf<100> aPhoneNumber;

	in >> aPhoneNumber;

	cons->Printf( _L("dialing... number:\n") );
	cons->Printf( aPhoneNumber );
	cons->Printf( _L("\n") );

	TName newCallName = _L("call");

	//Create a connection to the tel server
	RTelServer server;
	CleanupClosePushL(server);
	User::LeaveIfError(server.Connect());

	//Load in the phone device driver
	User::LeaveIfError(server.LoadPhoneModule(KTsyName));
	
	//Find the number of phones available from the tel server
	TInt numberPhones;
	User::LeaveIfError(server.EnumeratePhones(numberPhones));

	//Check there are available phones
	if (numberPhones < 1)
	{
		User::Leave(KErrNotFound);
	}

	//Get info about the first available phone
	RTelServer::TPhoneInfo info;
	User::LeaveIfError(server.GetPhoneInfo(0, info));

	//Use this info to open a connection to the phone, the phone is identified by its name
	RPhone phone;
	CleanupClosePushL(phone);
	User::LeaveIfError(phone.Open(server, info.iName));

	//Get info about the first line from the phone
	RPhone::TLineInfo lineInfo;
	User::LeaveIfError(phone.GetLineInfo(0, lineInfo));

	//Use this to open a line
	RLine line;
	CleanupClosePushL(line);
	User::LeaveIfError(line.Open(phone, lineInfo.iName));

	//Open a new call on this line
	RCall call;
	CleanupClosePushL(call);
	User::LeaveIfError(call.OpenNewCall(line, newCallName));

	if ((call.Dial(aPhoneNumber) == KErrNone))
	{
		TRequestStatus iStatus;
		RCall::TStatus iLineStatus;
		line.NotifyStatusChange(iStatus, iLineStatus);
		User::WaitForRequest(iStatus);
	}

	CleanupStack::PopAndDestroy(4);//phone, line, call

	//Unload the phone device driver
	User::LeaveIfError(server.UnloadPhoneModule(KTsyName));

	out << (TUint16)GNAPPLET_MSG_PHONEBOOK_DIAL_RESP;
	out << (TUint16)GN_ERR_NONE;
}


void HandlePhonebookMsg(PktBuf &in, PktBuf &out) {
	TUint16 code, memtype;

	in >> code;
	switch (code) {
		case GNAPPLET_MSG_PHONEBOOK_READ_REQ:
			in >> memtype;
			switch (memtype) {
				case GN_MT_ME:
					PhonebookReadReqL(memtype, in, out);
					break;
				case GN_MT_DC:
				case GN_MT_RC:
				case GN_MT_MC:
					LogReadReqL(memtype, in, out);
					break;
				default:
					out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READ_RESP;
					out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
					break;
			}
			break;

		case GNAPPLET_MSG_PHONEBOOK_WRITE_REQ:
			PhonebookWriteReqL(in, out);
			break;

		case GNAPPLET_MSG_PHONEBOOK_DELETE_REQ:
			in >> memtype;
			switch (memtype) {
				case GN_MT_ME:
					PhonebookDeleteReqL(memtype, in, out);
					break;
				case GN_MT_DC:
				case GN_MT_RC:
				case GN_MT_MC:
					LogDeleteReqL(memtype, in, out);
					break;
				default:
					out << (TUint16)GNAPPLET_MSG_PHONEBOOK_DELETE_RESP;
					out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
					break;
			}
			break;

		case GNAPPLET_MSG_PHONEBOOK_STATUS_REQ:
			in >> memtype;
			in.FinishL();
			switch (memtype) {
				case GN_MT_ME:
					PhonebookStatusReqL(memtype, in, out);
					break;
				case GN_MT_DC:
				case GN_MT_RC:
				case GN_MT_MC:
					LogStatusReqL(memtype, in, out);
					break;
				default:
					out << (TUint16)GNAPPLET_MSG_PHONEBOOK_STATUS_RESP;
					out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
					break;
			}
			break;

		case GNAPPLET_MSG_PHONEBOOK_DIAL_REQ:
			DialNumberL(in, out);
			break;

		case GNAPPLET_MSG_PHONEBOOK_READNEXT_REQ:
			in >> memtype;
			switch (memtype) {
				case GN_MT_ME:
					PhonebookReadNextReqL(memtype, in, out);
					break;
				default:
					out << (TUint16)GNAPPLET_MSG_PHONEBOOK_READNEXT_RESP;
					out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
			}
			break;

		default:
			out << (TUint16)((code + 1) & ~1);
			out << (TUint16)GN_ERR_NOTSUPPORTED;
			break;
	}
}
