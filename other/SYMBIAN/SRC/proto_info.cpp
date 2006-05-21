/*
 * $Id: proto_info.cpp,v 1.5 2004/04/12 12:55:05 bozo Exp $
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
 * This file contains the phone info message handler.
 *
 */

#include "proto.h"

#include <sysutil.h>
#include <hal.h>
#include <hal_data.h>


void HandleInfoMsg(PktBuf &in, PktBuf &out)
{
	TUint16 code;
	MBasicGsmPhoneId::TId id;
	TBuf<KSysUtilVersionTextLength> v;
	TPtrC model, sw_ver;
	TBuf<32> hw_ver;
	TInt i, s, manuf;

	in >> code;
	switch (code) {
	case GNAPPLET_MSG_INFO_ID_REQ:
		g->InitPhoneInfoL();
		User::LeaveIfError(g->phone->GetGsmPhoneId(id));
		SysUtil::GetSWVersion(v);
		User::LeaveIfError(HAL::Get(HALData::EManufacturer, manuf));
		Debug(_L("sw version:\n%S"), &v);
		for (s = 0; v[s] < '0' || v[s] > '9'; s++) ;
		for (i = s + 1; v[i] != '\n'; i++) ;
		sw_ver.Set(v.Ptr() + s, i - s);
		if (manuf == HALData::EManufacturer_Nokia) {
			for (s = i + 1; v[s] != '\n'; s++) ;
			for (i = ++s; v[i] != '\n'; i++) ;
			model.Set(v.Ptr() + s, i - s);
		} else {
			model.Set(id.iModelId);
		}
		/*
		User::LeaveIfError(HAL::Get(HALData::EDeviceFamily, i));
		hw_ver.AppendNum(i);
		hw_ver.Append(_L("/"));
		User::LeaveIfError(HAL::Get(HALData::EDeviceFamilyRev, i));
		hw_ver.AppendNum(i);
		hw_ver.Append(_L("/"));
		*/
		User::LeaveIfError(HAL::Get(HALData::EManufacturerHardwareRev, i));
		hw_ver.AppendNum(i);

		out << (TUint16)GNAPPLET_MSG_INFO_ID_RESP;
		out << (TUint16)GN_ERR_NONE;
		out << (TUint16)GNAPPLET_MAJOR_VERSION;
		out << (TUint16)GNAPPLET_MINOR_VERSION;
		out << id.iManufacturerId;
		out << model;
		out << id.iSerialNumber;
		out << sw_ver;
		out << hw_ver;
		in.FinishL();
		break;

	default:
		out << (TUint16)((code + 1) & ~1);
		out << (TUint16)GN_ERR_NOTSUPPORTED;
		break;
	}
}
