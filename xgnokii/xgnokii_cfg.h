/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef XGNOKII_CFG_H
#define XGNOKII_CFG_H

#include <gtk/gtk.h>
#include "xgnokii.h"

#define		HTMLVIEWER_LENGTH	200
#define		MAILBOX_LENGTH		200

typedef struct {
  gchar key[10];
  gchar **value;
} ConfigEntry;

extern void GUI_ReadXConfig();
extern gint GUI_SaveXConfig();

#endif
