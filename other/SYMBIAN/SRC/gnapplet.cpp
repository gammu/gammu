/*
 * $Id: gnapplet.cpp,v 1.9 2004/06/20 00:04:13 bozo Exp $
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
 * This file contains the controlling logic of the application.
 *
 */

#include "ircomm.h"
#include "rfcomm.h"
#include "pkt.h"
#include "proto.h"
#include "gnapplet.h"


CConsoleBase *cons;
GNApplet *g;


GNApplet::GNApplet(void)
{
	current_device = 0;

	telserver = 0;
	phone = 0;
	sysagent = 0;

	cdb = 0;

	logfs = 0;
	logclient = 0;
	logview = 0;

	msv_session = 0;
	mtmreg = 0;
	smsmtm = 0;
}


GNApplet::~GNApplet()
{
	// Phone info
	if (sysagent) {
		delete sysagent;
	}
	if (phone) {
		delete phone;
	}
	if (telserver) {
		telserver->Close();
		telserver->UnloadPhoneModule(_L("phonetsy.tsy"));
		delete telserver;
	}

	// Phonebook
	if (cdb) {
		delete cdb;
	}

	// Log client
	if (logview) {
		delete logview;
	}
	if (logclient) {
		delete logclient;
	}
	if (logfs) {
		logfs->Close();
		delete logfs;
	}

	// SMS
	if (smsmtm) {
		delete smsmtm;
	}
	if (mtmreg) {
		delete mtmreg;
	}
	if (msv_session) {
		delete msv_session;
	}
}


void GNApplet::InitPhoneInfoL(void)
{
	RTelServer::TPhoneInfo info;

	if (telserver) return;

	telserver = new (ELeave)RTelServer;
	User::LeaveIfError(telserver->Connect());
	User::LeaveIfError(telserver->LoadPhoneModule(_L("phonetsy.tsy")));

	User::LeaveIfError(telserver->GetPhoneInfo(0, info));
	phone = new (ELeave)RBasicGsmPhone;
	User::LeaveIfError(phone->Open(*telserver, info.iName));

	sysagent = new (ELeave)RSystemAgent;
	User::LeaveIfError(sysagent->Connect());
}


void GNApplet::InitPhoneBookL(void)
{
	if (cdb) return;

	cdb = CContactDatabase::OpenL();
}


void GNApplet::InitLogClientL(void)
{
	if (logview) return;

	logfs = new (ELeave)RFs;
	User::LeaveIfError(logfs->Connect());

	logclient = CLogClient::NewL(*logfs);

	logview = CLogViewEvent::NewL(*logclient);
}


void GNApplet::InitSmsL(MMsvSessionObserver &observer)
{
	if (msv_session) return;

	msv_session = CMsvSession::OpenSyncL(observer);
	mtmreg = CClientMtmRegistry::NewL(*msv_session);
	smsmtm = static_cast<CSmsClientMtm *>(mtmreg->NewMtmL(KUidMsgTypeSMS));
}


void WaitIfReqL(TBool wait, TRequestStatus &status, int msec = 3000)
{
	TRequestStatus timer_status;
	RTimer timer;
	TInt error;

	if (!wait) return;

	timer.CreateLocal();
	timer.After(timer_status, TTimeIntervalMicroSeconds32(msec * 1000));

	while (status == KRequestPending && timer_status == KRequestPending) {
		User::WaitForAnyRequest();
		while (CActiveScheduler::RunIfReady(error, CActive::EPriorityIdle)) ;
	}

	timer.Cancel();
	timer.Close();

	if (status != KErrNone) {
		User::Leave(status.Int());
	}
}


static void HandleConnectionL(Device *dev)
{
	TBuf8<512> b_in, b_out;
	PktBuf pkt_in, pkt_out;
	int type;

	g->current_device = dev;

	do {
		b_in.Zero();
		b_out.Zero();
		if (!ReadPacketL(dev, type, b_in)) break;
		pkt_in.Set((void *)b_in.Ptr(), b_in.Length());
		pkt_out.Set(b_out.Ptr(), 512);

		switch (type) {
		case GNAPPLET_MSG_INFO:
			HandleInfoMsg(pkt_in, pkt_out);
			break;
		case GNAPPLET_MSG_PHONEBOOK:
			HandlePhonebookMsg(pkt_in, pkt_out);
			break;
		case GNAPPLET_MSG_NETINFO:
			HandleNetinfoMsg(pkt_in, pkt_out);
			break;
		case GNAPPLET_MSG_POWER:
			HandlePowerMsg(pkt_in, pkt_out);
			break;
		case GNAPPLET_MSG_SMS:
			HandleSMSMsg(pkt_in, pkt_out);
			break;
		}

		b_out.SetLength(pkt_out.GetOffs());
		WritePacketL(dev, type, b_out);
	} while (1);

	g->current_device = 0;
}


static void MainL(void)
{
	IrCOMM *ircomm;
	RfCOMM *rfcomm;
	RTimer timer;
	TRequestStatus ircomm_rs, rfcomm_rs, timer_rs;

	g = new GNApplet;
	CleanupStack::PushL(g);

	ircomm = new IrCOMM;
	CleanupStack::PushL(ircomm);
	ircomm->ConstructL();

	rfcomm = new RfCOMM;
	CleanupStack::PushL(rfcomm);
	rfcomm->ConstructL(14);

	ircomm->AcceptL(ircomm_rs);
	rfcomm->AcceptL(rfcomm_rs);
	timer.CreateLocal();
	timer.After(timer_rs, TTimeIntervalMicroSeconds32(30000 * 1000));

	while (timer_rs == KRequestPending) {
		User::WaitForAnyRequest();
		if (ircomm_rs == KErrNone) {
			cons->Printf(_L("IrCOMM connection\r\n"));
			HandleConnectionL(ircomm);
			break;
		}
		if (rfcomm_rs == KErrNone) {
			cons->Printf(_L("RfCOMM connection\r\n"));
			HandleConnectionL(rfcomm);
			break;
		}
	};

	CleanupStack::PopAndDestroy(rfcomm);
	CleanupStack::PopAndDestroy(ircomm);
	CleanupStack::PopAndDestroy(g);
}


static void SpawnMainL(void)
{
	CActiveScheduler *sched;
	TInt error;

	error = StartC32();
	if (error != KErrNone && error != KErrAlreadyExists) {
		User::Leave(error);
	}

	sched = new (ELeave)CActiveScheduler;
	CleanupStack::PushL(sched);
	CActiveScheduler::Install(sched);

	cons = Console::NewL(_L("GNAPPLET"), TSize(KDefaultConsWidth, KDefaultConsHeight));
	CleanupStack::PushL(cons);

	cons->Printf(_L("gnapplet started\n"));
	MainL();
	//cons->Printf(_L("[press any key]\n"));
	//cons->Getch();

	CleanupStack::PopAndDestroy(cons);

	CleanupStack::PopAndDestroy(sched);
}


GLDEF_C int E32Main(void)
{
	CTrapCleanup *cleanup = CTrapCleanup::New();

	TRAPD(error, SpawnMainL());
	if (error != KErrNone) {
		User::Panic(_L("GNAPPLET"), error);
	}

	delete cleanup;

	return 0;
}
