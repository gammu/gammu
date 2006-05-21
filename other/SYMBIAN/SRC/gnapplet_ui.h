/*
 * $Id: gnapplet_ui.h,v 1.1 2004/04/12 23:44:14 bozo Exp $
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

#ifndef __GNAPPLET_UI_H
#define __GNAPPLET_UI_H

#include <eikdll.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <eikappui.h>


class CGNAppletApplication: public CEikApplication {
public:
	CApaDocument *CreateDocumentL(void);
	TUid AppDllUid(void) const;
};

class CGNAppletDocument: public CEikDocument {
public:
	CGNAppletDocument(CEikApplication &aApp): CEikDocument(aApp) {}
	CEikAppUi *CreateAppUiL(void);
};

class CGNAppletAppUi: public CEikAppUi {
public:
	void ConstructL(void);
	void HandleCommandL(int aCommand) {}
};

#endif
