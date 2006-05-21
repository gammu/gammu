/*
 * $Id: ircomm.h,v 1.1.1.1 2004/04/03 21:29:46 bozo Exp $
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
 * This file provides IrCOMM support.
 *
 */

#ifndef	__GNAPPLET_IRCOMM_H
#define	__GNAPPLET_IRCOMM_H

#include "device.h"


class IrCOMM: public Device {
	RCommServ rserv;
	RComm rcomm;
public:
	IrCOMM(void) {};
	~IrCOMM(void);
	void ConstructL(void);
	void AcceptL(TRequestStatus &rs);
	void CloseL(void);
	void ReadL(TRequestStatus &rs, TDes8 &buf);
	void WriteL(TRequestStatus &rs, const TDes8 &buf);
};

#endif
