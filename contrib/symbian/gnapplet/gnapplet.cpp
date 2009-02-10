/*
 * $Id: gnapplet.cpp,v 1.17 2005/09/01 22:28:50 bozo Exp $
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
#include "tcpip.h"
#include "pkt.h"
#include "proto.h"
#include "gnapplet.h"


static bool tdiff_ok = false;
static TTimeIntervalMicroSeconds tdiff;

CConsoleBase *cons;
GNApplet *g;


static void LoadIni(GNApplet *g, const char *file)
{
	FILE *f;
	char buf[64], key[64], value[64];
	TBuf<64> Key, Value;
	TLex lexer;
	TInt32 val;

	if ((f = fopen(file, "rt")) == NULL) return;

	while (fgets(buf, sizeof(buf), f) != NULL) {
		if (sscanf(buf, " %[a-zA-Z0-9_] = %[a-zA-Z0-9_-]", key, value) != 2) {
			continue;
		}
		if (key[0] == '#') {
			continue;
		}
		TPtrC8 k((TUint8 *)key), v((TUint8 *)value);
		Key.Copy(k);
		Value.Copy(v);
		lexer.Assign(Value);
		lexer.Val(val);
		if (Key == _L("irda_support")) {
			g->irda_support = val;
		} else if (Key == _L("bt_support")) {
			g->bt_support = val;
		} else if (Key == _L("bt_rfchannel")) {
			g->bt_rfchannel = val;
		} else if (Key == _L("init_timeout")) {
			g->init_timeout = val;
		} else if (Key == _L("reinit_timeout")) {
			g->reinit_timeout = val;
		} else if (Key == _L("tcp_support")) {
			g->tcp_support = val;
		} else if (Key == _L("tcp_port")) {
			g->tcp_port = val;
		}
	}
	fclose(f);
}


GNApplet::GNApplet(void)
{
	current_device = 0;
	irda_support = true;
	bt_support = true;
	bt_rfchannel = 14;
	tcp_support = false;
	tcp_port = 1912;
	init_timeout = 30;
	reinit_timeout = 5;
	LoadIni(this, "C:\\system\\apps\\gnapplet\\gnapplet.ini");

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

	agendaserv = 0;
	agendamodel = 0;

	alarmserv = 0;

	profile = 0;
	profilefs = 0;
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

	// Calendar
	if (agendamodel) {
		delete agendamodel;
	}
	if (agendaserv) {
		agendaserv->Close();
		delete agendaserv;
	}

	// Clock & alarm
	if (alarmserv) {
		alarmserv->Close();
		delete alarmserv;
	}

	// Profile
	if (profile) {
		delete profile;
	}
	if (profilefs) {
		profilefs->Close();
		delete profilefs;
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
	contactIter = new TContactIter( *cdb );
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


void GNApplet::InitCalendarL(void)
{
	if (agendaserv) return;

	agendaserv = RAgendaServ::NewL();
	User::LeaveIfError(agendaserv->Connect());

	agendamodel = CAgnEntryModel::NewL();
	agendamodel->SetServer(agendaserv);
	agendamodel->OpenL(_L("C:\\System\\Data\\Calendar"));
}


void GNApplet::InitClockL(void)
{
	if (alarmserv) return;

	alarmserv = new (ELeave)RAlarmServer;
	User::LeaveIfError(alarmserv->Connect());
}


void GNApplet::InitProfileL(void)
{
	if (profile) return;

	profile = CProfileAPI::NewL(EFalse);

	profilefs = new (ELeave)RFs;
	User::LeaveIfError(profilefs->Connect());
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


TTime toUTC(const TTime t)
{
	if (!tdiff_ok) {
		TTime tlocal, tutc;

		tlocal.HomeTime();
		tutc.UniversalTime();
		tdiff = tlocal.Int64() - tutc.Int64();
		tdiff_ok = true;
	}

	return t - tdiff;
}


TTime toLOCAL(const TTime t)
{
	if (!tdiff_ok) {
		TTime tlocal, tutc;

		tlocal.HomeTime();
		tutc.UniversalTime();
		tdiff = tlocal.Int64() - tutc.Int64();
		tdiff_ok = true;
	}

	return t + tdiff;
}


static void HandleConnectionL(Device *dev)
{
	HBufC8 *b_in, *b_out;
	PktBuf pkt_in, pkt_out;
	int type;

	g->current_device = dev;
	b_in = HBufC8::NewLC(512);
	b_out = HBufC8::NewLC(512);

	do {
		b_in->Des().Zero();
		b_out->Des().Zero();
		if (!ReadPacketL(dev, type, b_in->Des())) break;
		pkt_in.Set((void *)b_in->Des().Ptr(), b_in->Des().Length());
		pkt_out.Set(b_out->Des().Ptr(), 512);

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
		case GNAPPLET_MSG_CALENDAR:
			HandleCalendarMsg(pkt_in, pkt_out);
			break;
		case GNAPPLET_MSG_CLOCK:
			HandleClockMsg(pkt_in, pkt_out);
			break;
		case GNAPPLET_MSG_PROFILE:
			HandleProfileMsg(pkt_in, pkt_out);
			break;
		}

		b_out->Des().SetLength(pkt_out.GetOffs());
		WritePacketL(dev, type, b_out->Des());
	} while (1);

	CleanupStack::PopAndDestroy(b_out);
	CleanupStack::PopAndDestroy(b_in);
	g->current_device = 0;
}


static void MainL(void)
{
	IrCOMM *ircomm;
	RfCOMM *rfcomm;
	TCPIP *tcpip;
	RTimer timer;
	TRequestStatus ircomm_rs, rfcomm_rs, tcpip_rs, timer_rs;

	g = new GNApplet;
	CleanupStack::PushL(g);

	if (g->irda_support) {
		cons->Printf(_L("IrCOMM: on\r\n"));
		ircomm = new IrCOMM;
		CleanupStack::PushL(ircomm);
		ircomm->ConstructL();
		ircomm->AcceptL(ircomm_rs);
	}

	if (g->bt_support) {
		cons->Printf(_L("RfCOMM: on, Ch#%d\r\n"), g->bt_rfchannel);
		rfcomm = new RfCOMM;
		CleanupStack::PushL(rfcomm);
		rfcomm->ConstructL(g->bt_rfchannel);
		rfcomm->AcceptL(rfcomm_rs);
	}

	if (g->tcp_support) {
		cons->Printf(_L("TCP/IP: on, Port#%u\r\n"), (int)g->tcp_port);
		tcpip = new TCPIP;
		CleanupStack::PushL(tcpip);
		tcpip->ConstructL(g->tcp_port);
		tcpip->AcceptL(tcpip_rs);
	}

	cons->Printf(_L("timeout: "));
	if (g->init_timeout >= 0) {
		cons->Printf(_L("%d sec/"), g->init_timeout);
	} else {
		cons->Printf(_L("off/"));
	}
	if (g->reinit_timeout >= 0) {
		cons->Printf(_L("%d sec\r\n"), g->reinit_timeout);
	} else {
		cons->Printf(_L("off\r\n"));
	}

	timer.CreateLocal();
	if (g->init_timeout >= 0) {
		timer.After(timer_rs, TTimeIntervalMicroSeconds32(g->init_timeout * 1000000));
	} else {
		timer_rs = KRequestPending;
	}

	while (timer_rs == KRequestPending) {
		User::WaitForAnyRequest();
		if (g->irda_support && ircomm_rs == KErrNone) {
			cons->Printf(_L("IrCOMM connection\r\n"));
			HandleConnectionL(ircomm);
			ircomm->CloseL();
			ircomm->AcceptL(ircomm_rs);
			timer.Cancel();
			if (g->reinit_timeout >= 0) {
				timer.After(timer_rs, TTimeIntervalMicroSeconds32(g->reinit_timeout * 1000000));
			} else {
				timer_rs = KRequestPending;
			}
		}
		if (g->bt_support && rfcomm_rs == KErrNone) {
			cons->Printf(_L("RfCOMM connection\r\n"));
			HandleConnectionL(rfcomm);
			rfcomm->CloseL();
			rfcomm->AcceptL(rfcomm_rs);
			timer.Cancel();
			if (g->reinit_timeout >= 0) {
				timer.After(timer_rs, TTimeIntervalMicroSeconds32(g->reinit_timeout * 1000000));
			} else {
				timer_rs = KRequestPending;
			}
		}
		if (g->tcp_support && tcpip_rs == KErrNone) {
			cons->Printf(_L("TCP/IP connection\r\n"));
			HandleConnectionL(tcpip);
			tcpip->CloseL();
			tcpip->AcceptL(tcpip_rs);
			timer.Cancel();
			if (g->reinit_timeout >= 0) {
				timer.After(timer_rs, TTimeIntervalMicroSeconds32(g->reinit_timeout * 1000000));
			} else {
				timer_rs = KRequestPending;
			}
		}
	};

	if (g->tcp_support) {
		CleanupStack::PopAndDestroy(tcpip);
	}
	if (g->bt_support) {
		CleanupStack::PopAndDestroy(rfcomm);
	}
	if (g->irda_support) {
		CleanupStack::PopAndDestroy(ircomm);
	}
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
