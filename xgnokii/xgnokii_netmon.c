/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef WIN32
# include <unistd.h>
#endif
#include <pthread.h>
#include <stdlib.h>  /* for atoi */
#include <string.h>
#include <gtk/gtk.h>
#include "misc.h"
#include "xgnokii_common.h"
#include "xgnokii.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii_netmon.h"
#include "xpm/Display.xpm"

static GtkWidget *GUI_NetmonWindow;
static DisplayData displayData = {NULL, 0};
static GtkWidget *tableLabels[4][7];
static GtkWidget *tableProgress[7];


static inline void Help1 (GtkWidget *w, gpointer data)
{
  gchar *indx = g_strdup_printf ("/%s/gnokii/xgnokii/netmon/index.htm", xgnokiiConfig.locale);
  Help (w, indx);
  g_free (indx);
}


static inline void CloseNetmon (GtkWidget *w, gpointer data)
{
  PhoneEvent *e;

  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_NetMonitor;
  e->data = (gpointer) 0;
  GUI_InsertEvent (e);
  gtk_widget_hide (GUI_NetmonWindow);
}


static inline void NetmonOnOff (GtkWidget *w, gpointer data)
{
  PhoneEvent *e;

  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_NetMonitorOnOff;
  e->data = data;
  GUI_InsertEvent (e);
}


static inline void RefreshDisplay ()
{
  static gchar number[3];

  if (!GTK_WIDGET_VISIBLE(GUI_NetmonWindow))
    return;

  g_snprintf (number, 3, "%.2d", displayData.curDisp);
  if (displayData.number)
    gtk_label_set_text (GTK_LABEL (displayData.number), number);

  pthread_mutex_lock (&netMonMutex);
  if (phoneMonitor.netmonitor.screen && displayData.label)
    gtk_label_set_text (GTK_LABEL (displayData.label),
                        g_strchomp (phoneMonitor.netmonitor.screen));
  pthread_mutex_unlock (&netMonMutex);
}


static void SetDisplay (GtkWidget *widget, gpointer data)
{
  PhoneEvent *e;

  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_NetMonitor;
  e->data = data;
  GUI_InsertEvent (e);
  displayData.curDisp = GPOINTER_TO_INT (data);

  RefreshDisplay ();
}

static void ChangedSpinner (GtkWidget *widget, GtkSpinButton *spin)
{
  PhoneEvent *e;

  displayData.curDisp = gtk_spin_button_get_value_as_int (spin);
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_NetMonitor;
  e->data = (gpointer) displayData.curDisp;
  GUI_InsertEvent (e);

  RefreshDisplay ();
}


static void ParseScreen (gchar *screen, gint i)
{
  gchar buf[5];
  gchar *ptr = screen;

  if (ptr == NULL || *ptr == '\0')
    return;

  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[0][i]), buf);
  ptr += 3;
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[2][i]), buf);
  ptr += 3;
  if (*ptr != '-')
  {
    buf[0] = '-';
    strncpy (buf + 1, ptr, 3);
    buf[4] = '\0';
  }
  else
  {
    strncpy (buf, ptr, 3);
    buf[3] = '\0';
  }
  gtk_label_set_text (GTK_LABEL (tableLabels[1][i]), buf);
  ptr += 3;
  gtk_progress_set_value (GTK_PROGRESS (tableProgress[i]), atoi (buf));
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[3][i]), buf);
  ptr += 4;

  if (i == 6)
    return;

  i++;
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[0][i]), buf);
  ptr += 3;
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[2][i]), buf);
  ptr += 3;
  if (*ptr != '-')
  {
    buf[0] = '-';
    strncpy (buf + 1, ptr, 3);
    buf[4] = '\0';
  }
  else
  {
    strncpy (buf, ptr, 3);
    buf[3] = '\0';
  }
  gtk_label_set_text (GTK_LABEL (tableLabels[1][i]), buf);
  ptr += 3;
  gtk_progress_set_value (GTK_PROGRESS (tableProgress[i]), atoi (buf));
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[3][i]), buf);
  ptr += 4;

  i++;
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[0][i]), buf);
  ptr += 3;
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[2][i]), buf);
  ptr += 3;
  if (*ptr != '-')
  {
    buf[0] = '-';
    strncpy (buf + 1, ptr, 3);
    buf[4] = '\0';
  }
  else
  {
    strncpy (buf, ptr, 3);
    buf[3] = '\0';
  }
  gtk_label_set_text (GTK_LABEL (tableLabels[1][i]), buf);
  ptr += 3;
  gtk_progress_set_value (GTK_PROGRESS (tableProgress[i]), atoi (buf));
  strncpy (buf, ptr, 3);
  buf[3] = '\0';
  gtk_label_set_text (GTK_LABEL (tableLabels[3][i]), buf);
}


void GUI_RefreshNetmon ()
{
  if (!GTK_WIDGET_VISIBLE (GUI_NetmonWindow))
    return;

  pthread_mutex_lock (&netMonMutex);
  ParseScreen (phoneMonitor.netmonitor.screen3, 0);
  ParseScreen (phoneMonitor.netmonitor.screen4, 3);
  ParseScreen (phoneMonitor.netmonitor.screen5, 6);
  pthread_mutex_unlock (&netMonMutex);

  RefreshDisplay ();
}


void AddToBorder (GtkWidget* table, gchar *text, gint i, gint j, gint k, gint l)
{
  GtkWidget *label = gtk_label_new (text);
  GtkWidget *frame = gtk_frame_new (NULL);

  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (frame), label);
  gtk_table_attach_defaults (GTK_TABLE (table), frame, i, j, k, l);
}


inline void GUI_ShowNetmon ()
{
  PhoneEvent *e;

  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_NetMonitor;
  e->data = (gpointer) displayData.curDisp;;
  GUI_InsertEvent (e);
  gtk_widget_show (GUI_NetmonWindow);
}

static GtkItemFactoryEntry menu_items[] = {
  { NULL, NULL,		NULL, 0, "<Branch>"},
  { NULL, "<control>W",	CloseNetmon, 0, NULL},
  { NULL, NULL,		NULL, 0, "<Branch>"},
  { NULL, NULL,		NetmonOnOff, 1, NULL},
  { NULL, NULL,		NetmonOnOff, 0, NULL},
  { NULL, NULL,		NULL, 0, "<LastBranch>"},
  { NULL, NULL,		Help1, 0, NULL},
  { NULL, NULL,		GUI_ShowAbout, 0, NULL},
};


static void InitMainMenu (void)
{
  menu_items[0].path = g_strdup (_("/_File"));
  menu_items[1].path = g_strdup (_("/File/_Close"));
  menu_items[2].path = g_strdup (_("/_Tools"));
  menu_items[3].path = g_strdup (_("/Tools/Net monitor o_n"));
  menu_items[4].path = g_strdup (_("/Tools/Net monitor o_ff"));
  menu_items[5].path = g_strdup (_("/_Help"));
  menu_items[6].path = g_strdup (_("/Help/_Help"));
  menu_items[7].path = g_strdup (_("/Help/_About"));
}


#define DISPLAY_X	12
#define DISPLAY_Y	15
  
static inline gint ExposeDisplay (GtkWidget *widget, GdkEventExpose *event)
{
  GdkRectangle rec;
  GtkRequisition r;

  gtk_widget_size_request (displayData.number, &r);
  rec.x = DISPLAY_X;
  rec.y = DISPLAY_Y;
  rec.width = r.width;
  rec.height = r.height;
  gtk_widget_draw (displayData.number, &rec);

  gtk_widget_size_request (displayData.label, &r);
  rec.x += 3;
  rec.y += 12;
  rec.width = r.width;
  rec.height = r.height;
  gtk_widget_draw (displayData.label, &rec);

  return FALSE;
}


void GUI_CreateNetmonWindow ()
{
  int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  GtkWidget *menubar;
  GtkWidget *main_vbox;
  GtkWidget *hbox, *vbox;
  GtkWidget *toolbar;
  GtkWidget *table;
  GtkWidget *display;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkAdjustment *adj;
  gint i,j;


  InitMainMenu ();
  GUI_NetmonWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (GUI_NetmonWindow), _("Net Monitor"));
  //gtk_widget_set_usize (GTK_WIDGET (GUI_NetmonWindow), 436, 220);
  gtk_signal_connect (GTK_OBJECT (GUI_NetmonWindow), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);
  gtk_widget_realize (GUI_NetmonWindow);

  accel_group = gtk_accel_group_new ();
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
                                       accel_group);

  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  gtk_accel_group_attach (accel_group, GTK_OBJECT (GUI_NetmonWindow));
  
  /* Finally, return the actual menu bar created by the item factory. */ 
  menubar = gtk_item_factory_get_widget (item_factory, "<main>");

  main_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
  gtk_container_add (GTK_CONTAINER (GUI_NetmonWindow), main_vbox);
  gtk_widget_show (main_vbox);

  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, FALSE, 0);
  gtk_widget_show (menubar);

  frame = gtk_frame_new (NULL);
  gtk_container_border_width (GTK_CONTAINER (frame), 10);
  gtk_widget_show (frame);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (hbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  display = gtk_fixed_new ();
  gtk_box_pack_start (GTK_BOX (hbox), display, FALSE, FALSE, 10);
  gtk_widget_show (display);

  pixmap = NewPixmap (Display_xpm, GUI_NetmonWindow->window,
                      &GUI_NetmonWindow->style->bg[GTK_STATE_NORMAL]);

  gtk_signal_connect_after (GTK_OBJECT (pixmap), "expose_event",
                      (GtkSignalFunc) ExposeDisplay, NULL);

  gtk_fixed_put (GTK_FIXED (display), pixmap, 0 , 0);
  gtk_widget_show (pixmap);

  displayData.number = gtk_label_new ("");
  gtk_fixed_put (GTK_FIXED (display), displayData.number, DISPLAY_X, DISPLAY_Y);
  gtk_widget_show (displayData.number);
  displayData.label = gtk_label_new ("");
  gtk_fixed_put (GTK_FIXED (display), displayData.label, DISPLAY_X + 3, DISPLAY_Y + 12);
  gtk_widget_show (displayData.label);
  displayData.curDisp = 0x01;

  vbox = gtk_vbox_new (TRUE, 0);

  /* 1. line */
  toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar), GTK_RELIEF_NORMAL);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "1", _("Active cell"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 1);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "2", _("Active cell"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 2);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "3", _("NCELL list I"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 3);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "4", _("NCELL list II"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 4);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "5", _("NCELL list III"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 5);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "6", _("Prefered/Denied networks"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 6);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "7", _("System information bits"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 7);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "10", _("TMSI, Paging, PLU"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 10);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "11", _("Cells info"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 11);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "12", _("DTX, Cipher, Hopping"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 12);

  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);

  /* 2. line */
  toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar), GTK_RELIEF_NORMAL);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "13", _("Uplink DTX"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 13);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "17", _("BTS TEST"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 17);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "19", _("CELL BARR-Flag"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 19);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "20", _("Accumulator, Charge status"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 20);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "21", _("?Constant voltage charging display"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 21);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "22", _("?Battery full detection"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 22);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "23", _("Accumulator"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 23);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "35", _("SW-Resets"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 35);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "36", _("Reset-Counter"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 36);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "39", _("Cause codes for last connection abortion"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 39);

  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);

  /* 3. line */
  toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar), GTK_RELIEF_NORMAL);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "40", _("Reset handover counters"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 40);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "41", _("Handover Counter"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 41);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "42", _("Handover Counter (Dual)"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 42);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "43", _("L2-Timeouts"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 43);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "51", _("SIM"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 51);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "54", _("?Block display 1"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 54);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "56", NULL,
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 56);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "57", _("Memory status before reset"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 57);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "60", _("Reset Counters"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 60);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "61", _("Counter for PLMN Search and Cell reselection (Singleband)"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 61);

  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);

  /* 4. line */
  toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar), GTK_RELIEF_NORMAL);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "62", _("Neighbourhood measurement"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 62);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "63", _("Calls"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 63);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "70", _("Temporary counters of DSP"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 70);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "83", _("Control of task information displays"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 83);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "84", _("Information about task numbers 0-7"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 84);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "85", _("Information about task numbers 8-15"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 85);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "86", _("Information about task numbers 16-23"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 86);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "87", _("Information about OS_SYSTEM_STACK"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 87);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "88", _("Information about current MCU and DSP software versions"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 88);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), "89", _("Hardware version"),
                           NULL, NULL, (GtkSignalFunc) SetDisplay, (gpointer) 89);

  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);

  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 10);
  gtk_widget_show (vbox);

  table = gtk_table_new (6, 8, FALSE);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_border_width (GTK_CONTAINER (table), 3);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  AddToBorder (table, "", 0, 1, 0, 1);
  AddToBorder (table, _("Chan"), 1, 2, 0, 1);
  AddToBorder (table, _("RxLv"), 2, 3, 0, 1);  
  AddToBorder (table, _("C1"), 3, 4, 0, 1);  
  AddToBorder (table, _("C2"), 4, 5, 0, 1);  
  AddToBorder (table, _("ACT"), 0, 1, 1, 2);  
  AddToBorder (table, _("NC2"), 0, 1, 2, 3);  
  AddToBorder (table, _("NC3"), 0, 1, 3, 4);  
  AddToBorder (table, _("NC4"), 0, 1, 4, 5);  
  AddToBorder (table, _("NC5"), 0, 1, 5, 6);  
  AddToBorder (table, _("NC6"), 0, 1, 6, 7);  
  AddToBorder (table, _("NC7"), 0, 1, 7, 8);  

  for (i = 0; i < 4; i++)
    for (j = 0; j < 7; j++)
    {
      tableLabels[i][j] = gtk_label_new ("");
      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
      gtk_container_add (GTK_CONTAINER (frame), tableLabels[i][j]);
      gtk_table_attach_defaults (GTK_TABLE (table), frame, i + 1, i + 2, j + 1, j + 2);
    }

  for (i = 0; i < 7; i++)
  {
    adj = (GtkAdjustment *) gtk_adjustment_new (-114, -114, -40, 0, 0, 0);
    tableProgress[i] = gtk_progress_bar_new_with_adjustment (adj);
    gtk_table_attach_defaults (GTK_TABLE (table),  tableProgress[i], 5, 6, i + 1, i + 2);
  }

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 5, 6, 0, 1);

  adj = (GtkAdjustment *) gtk_adjustment_new (1, 1, 89, 1, 10, 0);
  button = gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (button), TRUE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (button), TRUE);
  gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (button), GTK_SHADOW_OUT);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (ChangedSpinner), (gpointer) button);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  label = gtk_label_new (_("Page:"));
  gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 5);

  gtk_table_set_col_spacing (GTK_TABLE (table), 4, 10);
  gtk_widget_show_all (table);
  GUIEventAdd (GUI_EVENT_NETMON_CHANGED, GUI_RefreshNetmon);
}
