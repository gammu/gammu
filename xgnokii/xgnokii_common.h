/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef XGNOKII_COMMON_H
#define XGNOKII_COMMON_H

#include <gtk/gtk.h>
#include <stdlib.h>  /* for size_t */

typedef struct {
  GtkWidget *clist;
  gint       column;
} SortColumn;

typedef struct {
  GtkWidget *dialog;
  GtkWidget *text;
} ErrorDialog;

typedef struct {
  GtkWidget *dialog;
  GtkWidget *text;
} InfoDialog;

typedef struct {
  GtkWidget *dialog;
  GtkWidget *text;
} YesNoDialog;

typedef struct {
  GdkPixmap *pixmap;
  GdkBitmap *mask;
} QuestMark;

typedef enum {
  GUI_EVENT_CONTACTS_CHANGED,
  GUI_EVENT_CALLERS_GROUPS_CHANGED,
  GUI_EVENT_SMS_NUMBER_CHANGED,
  GUI_EVENT_SMS_CENTERS_CHANGED,
  GUI_EVENT_NETMON_CHANGED,
  GUI_EVENT_SMS_FOLDERS_CHANGED
} GUIEventType;

extern void CancelDialog (const GtkWidget *, const gpointer);
extern void CreateErrorDialog (ErrorDialog *, GtkWidget *);
extern void CreateInfoDialog (InfoDialog *, GtkWidget *);
extern void CreateYesNoDialog (YesNoDialog *, const GtkSignalFunc,
                               const GtkSignalFunc, GtkWidget *);
extern GtkWidget* NewPixmap (gchar **, GdkWindow *, GdkColor *);
extern void DeleteEvent (const GtkWidget *, const GdkEvent *, const gpointer );
extern gint LaunchProcess (const gchar *, const gchar *, const gint,
                           const gint, const gint);
extern void RemoveZombie (const gint);
extern void Help (const GtkWidget *, const gpointer);
extern gint strrncmp (const gchar * const, const gchar * const, size_t);
extern gchar *GetModel (const gchar *);
extern bool CallerGroupSupported (const gchar *);
extern bool NetmonitorSupported (const gchar *);
extern bool KeyboardSupported (const gchar *);
extern bool SMSSupported (const gchar *);
extern bool CalendarSupported (const gchar *);
extern bool DTMFSupported (const gchar *);
extern bool DataSupported (const gchar *);
extern bool SpeedDialSupported (const gchar *);
extern void GUI_Refresh (void);
extern void SetSortColumn (GtkWidget *, SortColumn *);
extern void GUIEventAdd (GUIEventType, void (*)(void));
extern bool GUIEventRemove (GUIEventType, void (*)(void));
extern void GUIEventSend (GUIEventType);
#endif
