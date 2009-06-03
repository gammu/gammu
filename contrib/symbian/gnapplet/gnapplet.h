/*
 * $Id: gnapplet.h,v 1.15 2005/09/01 22:28:50 bozo Exp $
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

#ifndef	__GNAPPLET_H
#define	__GNAPPLET_H

#include <e32cons.h>
#include <etel.h>
#include <etelbgsm.h>
#include <etelagsm.h>
#include <cntdef.h>
#include <cntdb.h>
#include <cntitem.h>
#include <cntfldst.h>
#include <f32file.h>
#include <logcli.h>
#include <logview.h>
#include <logwrap.rsg>
#include <saclient.h>
#include <msvapi.h>
#include <msvids.h>
#include <mtclreg.h>
#include <smsclnt.h>
#include <smut.h>
#include <smuthdr.h>
#include <smutset.h>
#include <smutsimparam.h>
#include <gsmupdu.h>
#include <gsmuelem.h>
#include <agclient.h>
#include <t32alm.h>
#include <profileapi.h>

#include "device.h"


class GNApplet {
public:
	GNApplet(void);
	~GNApplet(void);

	// gnapplet specific
	Device *current_device;
	gboolean irda_support;
	gboolean bt_support;
	gboolean tcp_support;
	int bt_rfchannel;
	TUint16 tcp_port;
	int init_timeout;
	int reinit_timeout;

	// Phone info
	RTelServer *telserver;
	RBasicGsmPhone *phone;
	RSystemAgent *sysagent;
	void InitPhoneInfoL(void);

	// Phonebook
	CContactDatabase *cdb;
	TContactIter *contactIter;
	void InitPhoneBookL(void);

	// Log client
	RFs *logfs;
	CLogClient *logclient;
	CLogViewEvent *logview;
	void InitLogClientL(void);

	// SMS
	CMsvSession *msv_session;
	CClientMtmRegistry *mtmreg;
	CSmsClientMtm *smsmtm;
	void InitSmsL(MMsvSessionObserver &observer);

	// Calendar
	RAgendaServ *agendaserv;
	CAgnEntryModel *agendamodel;
	void InitCalendarL(void);

	// Clock & alarm
	RAlarmServer *alarmserv;
	void InitClockL(void);

	// Profile
	CProfileAPI *profile;
	RFs *profilefs;
	void InitProfileL(void);
};


extern CConsoleBase *cons;
extern GNApplet *g;


void WaitIfReqL(TBool wait, TRequestStatus &status, int msec = 3000);
TTime toUTC(const TTime t);
TTime toLOCAL(const TTime t);

#endif
