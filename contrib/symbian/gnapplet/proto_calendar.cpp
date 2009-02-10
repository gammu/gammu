/*
 * $Id: proto_calendar.cpp,v 1.6 2006/01/05 00:20:02 bozo Exp $
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
 * This file contains the calendar and todo list message handler.
 *
 */

#include "proto.h"


static CAgnEntry *GetEntryL(TUint32 location, TBool istodo)
{
	CAgnEntry *entry;
	TBool available, eistodo;

	available = g->agendaserv->CreateEntryIterator();

	while (available && location > 0) {
		entry = g->agendamodel->FetchEntryL(g->agendaserv->EntryIteratorPosition());

		eistodo = (entry->Type() == CAgnEntry::ETodo);
		if (eistodo && istodo || !eistodo && !istodo) {
			if (--location < 1) return entry;
		}

		delete entry;
		available = g->agendaserv->EntryIteratorNext();
	}

	return NULL;
}


static void ReadCalendarNoteL(PktBuf &in, PktBuf &out)
{
	TUint32 location;
	CAgnEntry *entry;
	TBuf<GN_CALNOTE_MAX_LENGTH - 1> text;
	TBuf<GN_CALNOTE_NUMBER_MAX_LENGTH - 1> phone_number;
	TBuf<GN_CALNOTE_MAX_LENGTH - 1> mlocation;
	TTime start_time, end_time, alarm_time;
	TUint16 recurrence;
	TUint8 type;
	CAgnAppt *appt;
	CAgnEvent *event;
	CAgnAnniv *anniv;
	const CAgnRptDef *rpt;

	g->InitCalendarL();

	in >> location;
	in.FinishL();

	if ((entry = GetEntryL(location, false)) == NULL) {
		out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_READ_RESP);
		out << TUint16(GN_ERR_INVALIDLOCATION);
		return;
	}
	CleanupStack::PushL(entry);

	entry->RichTextL()->Extract(text, 0);

	switch (entry->Type()) {
	case CAgnEntry::EAppt:
		appt = entry->CastToAppt();
		type = GN_CALNOTE_MEETING;
		start_time = appt->StartDateTime();
		end_time = appt->EndDateTime();
		mlocation = appt->Location();
		break;

	case CAgnEntry::EEvent:
		event = entry->CastToEvent();
		type = GN_CALNOTE_REMINDER;
		start_time = event->StartDate();
		end_time = event->EndDate();
		break;

	case CAgnEntry::EAnniv:
		anniv = entry->CastToAnniv();
		type = GN_CALNOTE_BIRTHDAY;
		start_time = anniv->StartDate();
		end_time = 0;
		break;

	default: User::Leave(entry->Type());
	}

	if (entry->HasAlarm()) {
		alarm_time = entry->AlarmInstanceDateTime();
	} else {
		alarm_time = 0;
	}

	if ((rpt = entry->RptDef()) != NULL) {
		switch (rpt->Type()) {
		case CAgnRptDef::EDaily:
			recurrence = GN_CALNOTE_DAILY;
			break;
		case CAgnRptDef::EWeekly:
			recurrence = (rpt->Interval() == 1) ?
				GN_CALNOTE_WEEKLY : GN_CALNOTE_2WEEKLY;
			break;
		case CAgnRptDef::EMonthlyByDates:
		case CAgnRptDef::EMonthlyByDays:
			recurrence = GN_CALNOTE_MONTHLY;
			break;
		case CAgnRptDef::EYearlyByDate:
		case CAgnRptDef::EYearlyByDay:
			recurrence = GN_CALNOTE_YEARLY;
			break;
		default:
			recurrence = GN_CALNOTE_NEVER;
			break;
		}
	} else
		recurrence = GN_CALNOTE_NEVER;

	out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_READ_RESP);
	out << TUint16(GN_ERR_NONE);
	out << location;
	out << type;
	out << toUTC(start_time);
	out << toUTC(end_time);
	out << toUTC(alarm_time);
	out << text;
	out << phone_number;
	out << mlocation;
	out << recurrence;

	CleanupStack::PopAndDestroy(entry);
}


static void WriteCalendarNoteL(PktBuf &in, PktBuf &out)
{
	TUint32 location;
	CAgnEntry *entry;
	TBuf<GN_CALNOTE_MAX_LENGTH - 1> text;
	TBuf<GN_CALNOTE_NUMBER_MAX_LENGTH - 1> phone_number;
	TBuf<GN_CALNOTE_MAX_LENGTH - 1> mlocation;
	TTime start_time, end_time, alarm_time;
	TUint16 recurrence;
	TUint8 type;
	CAgnAppt *appt;
	CAgnEvent *event;
	CAgnAnniv *anniv;
	CAgnRptDef *rpt;
	TAgnEntryId eid;
	TAgnUniqueId uid;
	TBool overwrite;

	g->InitCalendarL();

	in >> location;
	in >> type;
	in >> start_time;
	in >> end_time;
	in >> alarm_time;
	in >> text;
	in >> phone_number;
	in >> mlocation;
	in >> recurrence;
	in.FinishL();

	if (location != 0) {
		if ((entry = GetEntryL(location, false)) == NULL) {
			out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_WRITE_RESP);
			out << TUint16(GN_ERR_INVALIDLOCATION);
			return;
		}
		eid = entry->EntryId();
		uid = entry->UniqueId();
		delete entry;
		overwrite = true;
	} else {
		overwrite = false;
	}

	const CParaFormatLayer *paraFormatLayer = g->agendamodel->ParaFormatLayer();
	const CCharFormatLayer *charFormatLayer = g->agendamodel->CharFormatLayer();

	switch (type) {
	case GN_CALNOTE_MEETING:
		appt = CAgnAppt::NewLC(paraFormatLayer, charFormatLayer);
		if (toLOCAL(end_time).DateTime().Year() != 0) {
			appt->SetStartAndEndDateTime(toLOCAL(start_time), toLOCAL(end_time));
		} else {
			appt->SetStartAndEndDateTime(toLOCAL(start_time));
		}
		appt->SetLocationL(mlocation);
		entry = appt;
		break;
	case GN_CALNOTE_CALL:
		appt = CAgnAppt::NewLC(paraFormatLayer, charFormatLayer);
		if (toLOCAL(end_time).DateTime().Year() != 0) {
			appt->SetStartAndEndDateTime(toLOCAL(start_time), toLOCAL(end_time));
		} else {
			appt->SetStartAndEndDateTime(toLOCAL(start_time));
		}
		appt->SetLocationL(phone_number);
		entry = appt;
		break;
	case GN_CALNOTE_REMINDER:
		event = CAgnEvent::NewLC(paraFormatLayer, charFormatLayer);
		if (toLOCAL(end_time).DateTime().Year() != 0) {
			event->SetStartAndEndDate(toLOCAL(start_time), toLOCAL(end_time));
		} else {
			event->SetStartAndEndDate(toLOCAL(start_time));
		}
		entry = event;
		break;
	case GN_CALNOTE_BIRTHDAY:
		anniv = CAgnAnniv::NewLC(paraFormatLayer, charFormatLayer);
		if (toLOCAL(end_time).DateTime().Year() != 0) {
			anniv->SetStartAndEndDate(toLOCAL(start_time), toLOCAL(end_time));
		} else {
			anniv->SetStartAndEndDate(toLOCAL(start_time));
		}
		entry = anniv;
		break;
	default:
		out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_WRITE_RESP);
		out << TUint16(GN_ERR_UNKNOWN);
		return;
	}

	entry->RichTextL()->InsertL(0, text);

	if (toLOCAL(alarm_time).DateTime().Year() != 0) {
		const TDateTime &d = toLOCAL(alarm_time).DateTime();
		entry->SetAlarm(alarm_time.DaysFrom(start_time), TTimeIntervalMinutes(d.Hour() * 60 + d.Minute()));
		entry->SetHasAlarm(true);
	} else {
		entry->SetHasAlarm(false);
	}

	if (recurrence != GN_CALNOTE_NEVER) {
		rpt = CAgnRptDef::NewL();
		CleanupStack::PushL(rpt);

		if (recurrence == GN_CALNOTE_DAILY) {
			TAgnDailyRpt daily;
			rpt->SetDaily(daily);
			rpt->SetInterval(1);
		} else if (recurrence == GN_CALNOTE_WEEKLY) {
			TAgnWeeklyRpt weekly;
			weekly.ClearDays();
			weekly.SetDay(toLOCAL(start_time).DayNoInWeek());
			rpt->SetWeekly(weekly);
			rpt->SetInterval(1);
		} else if (recurrence == GN_CALNOTE_2WEEKLY) {
			TAgnWeeklyRpt weekly;
			weekly.ClearDays();
			weekly.SetDay(toLOCAL(start_time).DayNoInWeek());
			rpt->SetWeekly(weekly);
			rpt->SetInterval(2);
		} else if (recurrence == GN_CALNOTE_MONTHLY) {
			TAgnMonthlyByDatesRpt monthly;
			monthly.ClearAllDates();
			monthly.SetDate(toLOCAL(start_time).DayNoInMonth());
			rpt->SetMonthlyByDates(monthly);
			rpt->SetInterval(1);
		} else if (recurrence == GN_CALNOTE_YEARLY) {
			TAgnYearlyByDateRpt yearly;
			rpt->SetYearlyByDate(yearly);
			rpt->SetInterval(1);
		}

		rpt->SetStartDate(toLOCAL(start_time));
		rpt->SetRepeatForever(true);
		entry->SetRptDefL(rpt);
		CleanupStack::PopAndDestroy(rpt);
	}

	if (overwrite) {
		entry->SetId(eid);
		entry->SetUniqueId(uid);
		g->agendamodel->UpdateEntryL(entry);
	} else {
		g->agendamodel->AddEntryL(entry);
	}

	out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_WRITE_RESP);
	out << TUint16(GN_ERR_NONE);
	out << location;

	CleanupStack::PopAndDestroy(entry);
}


static void DeleteCalendarNoteL(PktBuf &in, PktBuf &out)
{
	TUint32 location;
	CAgnEntry *entry;

	g->InitCalendarL();

	in >> location;
	in.FinishL();

	if ((entry = GetEntryL(location, false)) == NULL) {
		out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_DELETE_RESP);
		out << TUint16(GN_ERR_INVALIDLOCATION);
		return;
	}
	delete entry;

	g->agendamodel->DeleteEntryL(g->agendaserv->EntryIteratorPosition());

	out << TUint16(GNAPPLET_MSG_CALENDAR_NOTE_DELETE_RESP);
	out << TUint16(GN_ERR_NONE);
	out << location;
}


static void ReadTodoL(PktBuf &in, PktBuf &out)
{
	TUint32 location;
	CAgnEntry *entry;
	TBuf<GN_TODO_MAX_LENGTH - 1> text;
	CAgnTodo *todo;

	g->InitCalendarL();

	in >> location;
	in.FinishL();

	if ((entry = GetEntryL(location, true)) == NULL) {
		out << TUint16(GNAPPLET_MSG_CALENDAR_TODO_READ_RESP);
		out << TUint16(GN_ERR_INVALIDLOCATION);
		return;
	}
	CleanupStack::PushL(entry);

	entry->RichTextL()->Extract(text, 0);
	todo = entry->CastToTodo();

	out << TUint16(GNAPPLET_MSG_CALENDAR_TODO_READ_RESP);
	out << TUint16(GN_ERR_NONE);
	out << location;
	out << text;
	out << TUint8(todo->Priority());

	CleanupStack::PopAndDestroy(entry);
}


static void WriteTodoL(PktBuf &in, PktBuf &out)
{
	TUint32 location;
	CAgnEntry *entry;
	TBuf<GN_TODO_MAX_LENGTH - 1> text;
	TUint8 priority;
	CAgnTodo *todo;
	TAgnEntryId eid;
	TAgnUniqueId uid;
	TBool overwrite;
	CAgnTodoListNames *todolist;

	g->InitCalendarL();

	in >> location;
	in >> text;
	in >> priority;
	in.FinishL();

	if (location != 0) {
		if ((entry = GetEntryL(location, true)) == NULL) {
			out << TUint16(GNAPPLET_MSG_CALENDAR_TODO_WRITE_RESP);
			out << TUint16(GN_ERR_INVALIDLOCATION);
			return;
		}
		eid = entry->EntryId();
		uid = entry->UniqueId();
		delete entry;
		overwrite = true;
	} else {
		overwrite = false;
	}

	const CParaFormatLayer *paraFormatLayer = g->agendamodel->ParaFormatLayer();
	const CCharFormatLayer *charFormatLayer = g->agendamodel->CharFormatLayer();

	todo = CAgnTodo::NewLC(paraFormatLayer, charFormatLayer);

	todolist = CAgnTodoListNames::NewL();
	CleanupStack::PushL(todolist);
	g->agendamodel->PopulateTodoListNamesL(todolist);

	todo->RichTextL()->InsertL(0, text);
	todo->SetPriority(priority);
	todo->SetTodoListId(todolist->TodoListId(0));

	if (overwrite) {
		todo->SetId(eid);
		todo->SetUniqueId(uid);
		g->agendamodel->UpdateEntryL(todo);
	} else {
		g->agendamodel->AddEntryL(todo);
	}

	out << TUint16(GNAPPLET_MSG_CALENDAR_TODO_WRITE_RESP);
	out << TUint16(GN_ERR_NONE);
	out << location;

	CleanupStack::PopAndDestroy(todolist);
	CleanupStack::PopAndDestroy(todo);
}


static void DeleteTodoL(PktBuf &in, PktBuf &out)
{
	TUint32 location;
	CAgnEntry *entry;

	g->InitCalendarL();

	in >> location;
	in.FinishL();

	if ((entry = GetEntryL(location, true)) == NULL) {
		out << TUint16(GNAPPLET_MSG_CALENDAR_TODO_DELETE_RESP);
		out << TUint16(GN_ERR_INVALIDLOCATION);
		return;
	}
	delete entry;

	g->agendamodel->DeleteEntryL(g->agendaserv->EntryIteratorPosition());

	out << TUint16(GNAPPLET_MSG_CALENDAR_TODO_DELETE_RESP);
	out << TUint16(GN_ERR_NONE);
	out << location;
}


void HandleCalendarMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_CALENDAR_NOTE_READ_REQ:
		ReadCalendarNoteL(in, out);
		break;

	case GNAPPLET_MSG_CALENDAR_NOTE_WRITE_REQ:
		WriteCalendarNoteL(in, out);
		break;

	case GNAPPLET_MSG_CALENDAR_NOTE_DELETE_REQ:
		DeleteCalendarNoteL(in, out);
		break;

	case GNAPPLET_MSG_CALENDAR_TODO_READ_REQ:
		ReadTodoL(in, out);
		break;

	case GNAPPLET_MSG_CALENDAR_TODO_WRITE_REQ:
		WriteTodoL(in, out);
		break;

	case GNAPPLET_MSG_CALENDAR_TODO_DELETE_REQ:
		DeleteTodoL(in, out);
		break;

	default:
		out << TUint16((code + 1) & ~1);
		out << TUint16(GN_ERR_NOTSUPPORTED);
		break;
	}
}
