/*
 * $Id: gnapplet_ui.cpp,v 1.2 2005/04/12 20:18:24 bozo Exp $
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
 * This file contains a frontent to the real application.
 *
 */

#include "gnapplet_ui.h"


GLDEF_C TInt E32Dll(TDllReason)
{
	return KErrNone;
}

EXPORT_C CApaApplication *NewApplication(void)
{
	return new CGNAppletApplication;
}

/* ------------------------------------------------------------------------- */

CApaDocument *CGNAppletApplication::CreateDocumentL(void)
{
	return new (ELeave)CGNAppletDocument(*this);
}

TUid CGNAppletApplication::AppDllUid(void) const
{
	TUid id = { 0x107d4df2 };	/* UID3 */
	
	return id;
}

/* ------------------------------------------------------------------------- */

CEikAppUi *CGNAppletDocument::CreateAppUiL(void)
{
	return new (ELeave)CGNAppletAppUi;
}

/* ------------------------------------------------------------------------- */

void CGNAppletAppUi::ConstructL(void)
{
	TFileName file;
	int pos;

	BaseConstructL(ENoAppResourceFile);

	file.Copy(Application()->AppFullName());
	pos = file.LocateReverse('\\');
	file.Replace(pos, file.Length() - pos, _L("\\gnapplet.exe"));

	EikDll::StartExeL(file);

	User::Exit(0);
}
