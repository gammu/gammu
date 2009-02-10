/*
 * $Id: proto_sms.cpp,v 1.5 2004/07/11 22:53:22 bozo Exp $
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
 * This file contains the SMS message handler.
 *
 */

#include "proto.h"

const TMsvId MyFolders = 0x1008;
const TMsvId Templates = 0x1009;

class SMSObserver: public MMsvSessionObserver {
public:
	SMSObserver(void);
	int folder_count;
	TMsvId folders[GN_MT_XX];
	void RefreshL(void);
	TUint16 GetMT(TMsvId id);

private:
	int next_folder;
	void FetchL(void);

	virtual void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1,TAny* aArg2, TAny* aArg3);
};


SMSObserver sms_observer;


SMSObserver::SMSObserver(void)
{
	next_folder = 0;
	folder_count = 0;
}


void SMSObserver::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1,TAny* aArg2, TAny* aArg3)
{
}


void SMSObserver::FetchL(void)
{
	CMsvEntry *entry;
	int i, n;

	for (i = 0; i <= GN_MT_XX; i++)
		folders[i] = KMsvNullIndexEntryId;

	folders[GN_MT_IN] = KMsvGlobalInBoxIndexEntryId;
	folders[GN_MT_OU] = KMsvGlobalOutBoxIndexEntryId;
	folders[GN_MT_AR] = KMsvSentEntryId;
	next_folder = GN_MT_F1;
	folder_count = 3;

	entry = g->msv_session->GetEntryL(MyFolders);
	CleanupStack::PushL(entry);
	entry->SetSortTypeL(TMsvSelectionOrdering(KMsvNoGrouping, EMsvSortByNone, ETrue));
	n = entry->Count();

	for (i = 0; i < n && next_folder < GN_MT_XX; i++) {
		const TMsvEntry &child = (*entry)[i];

		if (child.Id() == Templates) {
			folders[GN_MT_TE] = Templates;
			folder_count++;
		} else if (child.Id() >= KFirstFreeEntryId) {
			folders[next_folder++] = child.Id();
			folder_count++;
		}
	}

	CleanupStack::PopAndDestroy(entry);
}


void SMSObserver::RefreshL(void)
{
	if (next_folder == 0) FetchL();
}


TUint16 SMSObserver::GetMT(TMsvId id)
{
	if (next_folder >= GN_MT_XX) return GN_MT_XX;

	folders[next_folder] = id;
	return next_folder++;
}


static void GetStatusL(PktBuf &in, PktBuf &out)
{
	TUint32 number, unread;
	int i, j, n;
	CMsvEntry *entry;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in.FinishL();

	number = 0;
	unread = 0;
	for (i = 0; i < GN_MT_XX; i++) {
		if (sms_observer.folders[i] == KMsvNullIndexEntryId) continue;

		entry = g->msv_session->GetEntryL(sms_observer.folders[i]);
		CleanupStack::PushL(entry);
		entry->SetSortTypeL(TMsvSelectionOrdering(KMsvNoGrouping, EMsvSortByNone, ETrue));
		n = entry->Count();

		for (j = 0; j < n; j++) {
			const TMsvEntry &f = (*entry)[j];
			if (f.Unread()) {
				unread++;
			} else {
				number++;
			}
		}

		CleanupStack::PopAndDestroy(entry);
	}

	out << (TUint16)GNAPPLET_MSG_SMS_STATUS_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << number;
	out << unread;
	out << (TBool)false;
	out << (TUint16)sms_observer.folder_count;
}


static void FolderListL(PktBuf &in, PktBuf &out)
{
	CMsvEntry *entry;
	TUint32 i;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in.FinishL();

	out << (TUint16)GNAPPLET_MSG_SMS_FOLDER_LIST_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << (TUint16)sms_observer.folder_count;

	for (i = 0; i < GN_MT_XX; i++) {
		if (sms_observer.folders[i] == KMsvNullIndexEntryId) continue;

		entry = g->msv_session->GetEntryL(sms_observer.folders[i]);
		CleanupStack::PushL(entry);

		const TMsvEntry &f = entry->Entry();
		out << (TUint16)i << f.iDetails;

		CleanupStack::PopAndDestroy(entry);
	}
}


static void FolderStatusL(PktBuf &in, PktBuf &out)
{
	CMsvEntry *entry;
	TUint32 i, n;
	TUint16 memtype;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> memtype;
	in.FinishL();

	if (memtype >= GN_MT_XX || sms_observer.folders[memtype] == KMsvNullIndexEntryId) {
		out << (TUint16)GNAPPLET_MSG_SMS_FOLDER_STATUS_RESP;
		out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
		return;
	}

	entry = g->msv_session->GetEntryL(sms_observer.folders[memtype]);
	CleanupStack::PushL(entry);
	entry->SetSortTypeL(TMsvSelectionOrdering(KMsvNoGrouping, EMsvSortByNone, ETrue));
	n = entry->Count();

	out << (TUint16)GNAPPLET_MSG_SMS_FOLDER_STATUS_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	/// @todo erm, this is a waste of resources :o) better replace this when I have time

	// count sms entries
	TUint32 count=0;
	for( i=0; i<n; i++ ) {
		const TMsvEntry &child = (*entry)[i];
		if( child.iMtm == KUidMsgTypeSMS )
			count++;
	}

	out << count;

	for (i = 0; i < n; i++) {
		// only sms are supported atm
		const TMsvEntry &child = (*entry)[i];
		if( child.iMtm == KUidMsgTypeSMS )
			out << (TUint32)child.Id();
	}

	CleanupStack::PopAndDestroy(entry);
}


static void CreateFolderL(PktBuf &in, PktBuf &out)
{
	CMsvEntry *entry;
	TMsvEntry f;
	TBuf<64> name;
	TUint16 memtype;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> name;
	in.FinishL();

	entry = g->msv_session->GetEntryL(MyFolders);
	CleanupStack::PushL(entry);

	f.iType = KUidMsvFolderEntry;
	f.iMtm = entry->Entry().iMtm;
	f.iDetails.Set(name);
	f.iServiceId = KMsvLocalServiceIndexEntryId;

	entry->CreateL(f);
	memtype = sms_observer.GetMT(f.Id());

	CleanupStack::PopAndDestroy(entry);

	out << (TUint16)GNAPPLET_MSG_SMS_FOLDER_CREATE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype << name;
}


static void DeleteFolderL(PktBuf &in, PktBuf &out)
{
	CMsvEntry *entry;
	TUint16 memtype;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> memtype;
	in.FinishL();

	if (memtype >= GN_MT_XX || sms_observer.folders[memtype] == KMsvNullIndexEntryId) {
		out << (TUint16)GNAPPLET_MSG_SMS_FOLDER_DELETE_RESP;
		out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
		return;
	}

	entry = g->msv_session->GetEntryL(MyFolders);
	CleanupStack::PushL(entry);

	entry->DeleteL(sms_observer.folders[memtype]);
	sms_observer.folders[memtype] = KMsvNullIndexEntryId;

	CleanupStack::PopAndDestroy(entry);

	out << (TUint16)GNAPPLET_MSG_SMS_FOLDER_DELETE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
}


static void ReadMessageL(PktBuf &in, PktBuf &out)
{
	TUint16 memtype;
	TUint32 location;
	CMsvEntry *entry;
	CArrayFixFlat<TSms> *sms_array;
	TUint8 status;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> memtype;
	in >> location;
	in.FinishL();

	if (memtype >= GN_MT_XX || sms_observer.folders[memtype] == KMsvNullIndexEntryId) {
		out << (TUint16)GNAPPLET_MSG_SMS_MESSAGE_READ_RESP;
		out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
		return;
	}

	entry = g->msv_session->GetEntryL(location);
	CleanupStack::PushL(entry);

	g->smsmtm->SwitchCurrentEntryL(entry->EntryId());
	g->smsmtm->LoadMessageL();
	const TMsvEntry &f = entry->Entry();
	const CSmsHeader &header = g->smsmtm->SmsHeader();
	const CSmsMessage &msg = header.Message();

	out << (TUint16)GNAPPLET_MSG_SMS_MESSAGE_READ_RESP;
	out << (TUint16)GN_ERR_NONE;

	sms_array = new (ELeave)CArrayFixFlat<TSms>(5);
	CleanupStack::PushL(sms_array);
	msg.EncodeMessagePDUsL(*sms_array);
	out << (TUint16) sms_array->Count();
	for( int i=0; i<sms_array->Count(); i++ )
		out << (*sms_array)[i].PduL();
	CleanupStack::PopAndDestroy(sms_array);

	if (f.SendingState() == KMsvSendStateSent) {
		status = GN_SMS_Sent;
	} else if (f.SendingState() != KMsvSendStateNotApplicable && f.SendingState() != KMsvSendStateUnknown) {
		status = GN_SMS_Unsent;
	} else if (f.Unread()) {
		status = GN_SMS_Unread;
	} else {
		status = GN_SMS_Read;
	}

	out << status;

	CleanupStack::PopAndDestroy(entry);
}


static void WriteMessageL(PktBuf &in, PktBuf &out)
{
	TUint16 memtype;
	TUint32 location;
	CMsvEntry *entry;
	TSms::TPdu pdu;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> memtype;
	in >> location;
	in >> pdu;
	in.FinishL();

	if (memtype >= GN_MT_XX || sms_observer.folders[memtype] == KMsvNullIndexEntryId) {
		out << (TUint16)GNAPPLET_MSG_SMS_MESSAGE_WRITE_RESP;
		out << (TUint16)GN_ERR_INVALIDMEMORYTYPE;
		return;
	}

	if (1) {
		CMsvOperationWait *wait;
		CMsvOperation *operation;
		TMsvEntry f;
		TInt error;

		f.iMtm = KUidMsgTypeSMS;
		f.iType = KUidMsvMessageEntry;
		f.iServiceId = KMsvLocalServiceIndexEntryId;
		f.iDate.HomeTime();
		f.SetInPreparation(true);

		entry = CMsvEntry::NewL(*g->msv_session, sms_observer.folders[memtype], TMsvSelectionOrdering());
		CleanupStack::PushL(entry);
		
		wait = CMsvOperationWait::NewLC();
		wait->Start();
		operation = entry->CreateL(f, wait->iStatus);
		CleanupStack::PushL(operation);

		while (CActiveScheduler::RunIfReady(error, CActive::EPriorityIdle)) ;
		TMsvLocalOperationProgress progress = McliUtils::GetLocalProgressL(*operation);
		User::LeaveIfError(progress.iError);

		entry->SetEntryL(progress.iId);
		location = progress.iId;

		CleanupStack::PopAndDestroy(2);
	} else {
		//!!!FIXME: modify existing entry
		entry = g->msv_session->GetEntryL(location);
		CleanupStack::PushL(entry);
	}

	g->smsmtm->SwitchCurrentEntryL(entry->EntryId());
	g->smsmtm->RestoreServiceAndSettingsL();

	TMsvEntry f;
	f = entry->Entry();
	//f.iDate = msg_time;
	//f.iDetails.Set(remote_number);
	f.SetInPreparation(false);

	CSmsHeader &header = g->smsmtm->SmsHeader();
	CSmsMessage &msg = header.Message();

	CArrayFixFlat<TSms> *sms_array;
	TSms sms(pdu);
	sms.SetUseDefaultSca(false);
	sms_array = new (ELeave)CArrayFixFlat<TSms>(1);
	CleanupStack::PushL(sms_array);
	sms_array->AppendL(sms);

#if 1
	msg.DecodeMessagePDUsL(*sms_array, true);
#endif
#if 0
	CSmsMessage *m = CSmsMessage::NewL(sms, CSmsBuffer::NewL(), true);
	TSmsUtilities smsut;
	g->smsmtm->SaveMessageL();
	TSmsUtilities::PopulateMsgEntry(f, *m, f.iServiceId);
#endif

	CleanupStack::PopAndDestroy(sms_array);
	
	entry->ChangeL(f);

	g->smsmtm->SaveMessageL();

	out << (TUint16)GNAPPLET_MSG_SMS_MESSAGE_WRITE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << memtype;
	out << location;

	CleanupStack::PopAndDestroy(entry);
}


static void SendMessageL(PktBuf &in, PktBuf &out)
{
	TSms::TPdu pdu;
	RAdvGsmSmsMessaging messaging;
	RAdvGsmSmsMessaging::TSmsSendResult result;
	TRequestStatus status;

	g->InitPhoneInfoL();
	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> pdu;
	in.FinishL();

	TSms sms(pdu);
	sms.SetUseDefaultSca(false);

	messaging.Open(*g->phone);
	CleanupClosePushL(messaging);
	messaging.SendAdvSmsMessage(status, sms, result);
	User::WaitForRequest(status);

	out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_SEND_RESP);
	out << TUint16(status == KErrNone ? GN_ERR_NONE : GN_ERR_FAILED);

	CleanupStack::PopAndDestroy(1);
}


static void DeleteMessageL(PktBuf &in, PktBuf &out)
{
	TUint16 memtype;
	TUint32 location;
	CMsvEntry *entry;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> memtype;
	in >> location;
	in.FinishL();

	if (memtype >= GN_MT_XX || sms_observer.folders[memtype] == KMsvNullIndexEntryId) {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_DELETE_RESP);
		out << TUint16(GN_ERR_INVALIDMEMORYTYPE);
		return;
	}

	entry = g->msv_session->GetEntryL(sms_observer.folders[memtype]);
	CleanupStack::PushL(entry);

	TRAPD(error, entry->DeleteL(location));
	if (error != KErrNone) {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_DELETE_RESP);
		out << TUint16(GN_ERR_INVALIDLOCATION);
	} else {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_DELETE_RESP);
		out << TUint16(GN_ERR_NONE);
	}

	CleanupStack::PopAndDestroy(entry);
}


static void MoveMessageL(PktBuf &in, PktBuf &out)
{
	TUint16 memtype, newfolder;
	TUint32 location;
	CMsvEntry *entry;
	TRequestStatus status;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> memtype;
	in >> location;
	in >> newfolder;
	in.FinishL();

	if (memtype >= GN_MT_XX || sms_observer.folders[memtype] == KMsvNullIndexEntryId) {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_MOVE_RESP);
		out << TUint16(GN_ERR_INVALIDMEMORYTYPE);
		return;
	}
	if (newfolder >= GN_MT_XX || sms_observer.folders[newfolder] == KMsvNullIndexEntryId) {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_MOVE_RESP);
		out << TUint16(GN_ERR_INVALIDMEMORYTYPE);
		return;
	}

	entry = g->msv_session->GetEntryL(sms_observer.folders[memtype]);
	CleanupStack::PushL(entry);

	TRAPD(error, entry->MoveL(location, newfolder, status));
	if (error == KErrNone) {
		WaitIfReqL(true, status);
		error = status.Int();
	}

	if (error != KErrNone) {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_MOVE_RESP);
		out << TUint16(GN_ERR_INVALIDLOCATION);
	} else {
		out << TUint16(GNAPPLET_MSG_SMS_MESSAGE_MOVE_RESP);
		out << TUint16(GN_ERR_NONE);
	}

	CleanupStack::PopAndDestroy(entry);
}


static void ReadSmscL(PktBuf &in, PktBuf &out)
{
	TUint16 index;
	TRequestStatus status;
	CSmsSimParamOperation *op;
	const CAdvGsmSmspList *sc;
	RAdvGsmSmsMessaging::TSmspEntry e;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> index;
	in.FinishL();

	op = g->smsmtm->ReadSimParamsL(status);
	CleanupStack::PushL(op);
	WaitIfReqL(true, status);
	if (status != KErrNone) {
		out << (TUint16)GNAPPLET_MSG_SMS_CENTER_READ_RESP;
		out << (TUint16)GN_ERR_FAILED;
		return;
	}
	sc = &op->SmspList();
	if (sc->GetEntry(index, e) != KErrNone) {
		CleanupStack::PopAndDestroy(1);
		out << (TUint16)GNAPPLET_MSG_SMS_CENTER_READ_RESP;
		out << (TUint16)GN_ERR_INVALIDLOCATION;
		return;
	}
	Debug(_L("OK: %02x %S/%S"), e.iValidParams, &e.iText, &e.iServiceCentre.iTelNumber);

	out << (TUint16)GNAPPLET_MSG_SMS_CENTER_READ_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << index;
	out << e.iText;
	out << TInt16(-1);
	out << TUint8(GN_SMS_MF_Text);	//!!!FIXME

	if (e.iValidParams & RAdvGsmSmsMessaging::KValidityPeriodIncluded) {
		out << e.iValidityPeriod;
	} else {
		out << TUint8(0xff);
	}

	if (e.iValidParams & RAdvGsmSmsMessaging::KServiceCentreAddressIncluded) {
		out << TUint8(e.iServiceCentre.iTypeOfNumber) << e.iServiceCentre.iTelNumber;
	} else {
		out << TUint8(0) << _L("");
	}

	if (e.iValidParams & RAdvGsmSmsMessaging::KDestinationIncluded) {
		out << TUint8(e.iDestination.iTypeOfNumber) << e.iDestination.iTelNumber;
	} else {
		out << TUint8(0) << _L("");
	}

	CleanupStack::PopAndDestroy(1);
}


static void WriteSmscL(PktBuf &in, PktBuf &out)
{
	TUint16 index;
	TRequestStatus status;
	CSmsSimParamOperation *op;
	const CAdvGsmSmspList *sc;
	RAdvGsmSmsMessaging::TSmspEntry e;
	TInt16 defname;
	TUint8 type;
	TBuf<KGsmMaxTelNumberSize> number;

	g->InitSmsL(sms_observer);
	sms_observer.RefreshL();

	in >> index;
	in >> e.iText;
	in >> defname;

	in >> e.iProtocolId;
	e.iValidParams |= RAdvGsmSmsMessaging::KProtocolIdIncluded;

	//e.iValidParams |= RAdvGsmSmsMessaging::KDcsIncluded;
	e.iDcs = 0xf4;

	in >> e.iValidityPeriod;
	e.iValidParams |= RAdvGsmSmsMessaging::KValidityPeriodIncluded;

	in >> type >> number;
	if (type != 0 && number.Length() > 0) {
		e.iServiceCentre.iTypeOfNumber = type;
		e.iServiceCentre.iTelNumber = number;
		e.iValidParams |= RAdvGsmSmsMessaging::KServiceCentreAddressIncluded;
	}

	in >> type >> number;
	if (type != 0 && number.Length() > 0) {
		e.iDestination.iTypeOfNumber = type;
		e.iDestination.iTelNumber = number;
		e.iValidParams |= RAdvGsmSmsMessaging::KDestinationIncluded;
	}

	in.FinishL();

	op = g->smsmtm->ReadSimParamsL(status);
	CleanupStack::PushL(op);
	WaitIfReqL(true, status);
	if (status != KErrNone) {
		out << (TUint16)GNAPPLET_MSG_SMS_CENTER_WRITE_RESP;
		out << (TUint16)GN_ERR_FAILED;
		return;
	}
	//sc = &op->SmspList();
	sc = op->ServiceCentersLC();
	int err;
	/*
	if ((err = sc->ChangeEntry(index, e)) != KErrNone) {
	if (0) {
	*/
	if ((err = sc->AddEntryL(e)) != KErrNone) {
		Debug(_L("err: %d"), err);
		CleanupStack::PopAndDestroy(1);
		out << (TUint16)GNAPPLET_MSG_SMS_CENTER_WRITE_RESP;
		out << (TUint16)GN_ERR_INVALIDLOCATION;
		return;
	}
	//g->smsmtm->ServiceSettings().SetCommDbAction(CSmsSettings::EStoreToCommDb);
	//g->smsmtm->ServiceSettings().AddSCAddressL(_L("alma"), _L("1234"));

	g->InitPhoneInfoL();
	RAdvGsmSmsMessaging messaging;
	messaging.Open(*g->phone);
	CleanupClosePushL(messaging);
	
	//messaging.NotifySmsParameterListChanged(status);
	Debug(_L("OK: %02x %S/%S"), e.iValidParams, &e.iText, &e.iServiceCentre.iTelNumber);
	//sc = messaging.RetrieveSmsParameterListLC();
	Debug(_L("elek: %d"), g->smsmtm->ServiceSettings().NumSCAddresses());
	User::LeaveIfError(messaging.StoreSmsParameterListL(sc));
	Debug(_L("Elek: %d"), g->smsmtm->ServiceSettings().NumSCAddresses());
	CleanupStack::PopAndDestroy(1);

	out << (TUint16)GNAPPLET_MSG_SMS_CENTER_WRITE_RESP;
	out << (TUint16)GN_ERR_NONE;
	out << index;

	CleanupStack::PopAndDestroy(2);
}


void HandleSMSMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_SMS_FOLDER_LIST_REQ:
		FolderListL(in, out);
		break;

	case GNAPPLET_MSG_SMS_FOLDER_STATUS_REQ:
		FolderStatusL(in, out);
		break;

	case GNAPPLET_MSG_SMS_FOLDER_CREATE_REQ:
		CreateFolderL(in, out);
		break;

	case GNAPPLET_MSG_SMS_FOLDER_DELETE_REQ:
		DeleteFolderL(in, out);
		break;

	case GNAPPLET_MSG_SMS_STATUS_REQ:
		GetStatusL(in, out);
		break;

	case GNAPPLET_MSG_SMS_MESSAGE_READ_REQ:
		ReadMessageL(in, out);
		break;

	case GNAPPLET_MSG_SMS_MESSAGE_WRITE_REQ:
		WriteMessageL(in, out);
		break;

	case GNAPPLET_MSG_SMS_MESSAGE_SEND_REQ:
		SendMessageL(in, out);
		break;

	case GNAPPLET_MSG_SMS_MESSAGE_DELETE_REQ:
		DeleteMessageL(in, out);
		break;

	case GNAPPLET_MSG_SMS_MESSAGE_MOVE_REQ:
		MoveMessageL(in, out);
		break;

	case GNAPPLET_MSG_SMS_CENTER_READ_REQ:
		ReadSmscL(in, out);
		break;

	case GNAPPLET_MSG_SMS_CENTER_WRITE_REQ:
		WriteSmscL(in, out);
		break;

	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
