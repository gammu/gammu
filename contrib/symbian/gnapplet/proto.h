/*
 * $Id: proto.h,v 1.7 2005/03/08 10:30:12 bozo Exp $
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
 * This file contains the prototypes of the message handlers.
 *
 */

#ifndef __GNAPPLET_PROTO_H
#define __GNAPPLET_PROTO_H

#include "gnapplet.h"
#include "pkt.h"


void Debug(TRefByValue<const TDesC> fmt, ...);

void HandleInfoMsg(PktBuf &in, PktBuf &out);
void HandlePhonebookMsg(PktBuf &in, PktBuf &out);
void HandleNetinfoMsg(PktBuf &in, PktBuf &out);
void HandlePowerMsg(PktBuf &in, PktBuf &out);
void HandleDebugMsg(PktBuf &in, PktBuf &out);
void HandleSMSMsg(PktBuf &in, PktBuf &out);
void HandleCalendarMsg(PktBuf &in, PktBuf &out);
void HandleClockMsg(PktBuf &in, PktBuf &out);
void HandleProfileMsg(PktBuf &in, PktBuf &out);

#endif
