/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <stdio.h>
#include <gtk/gtk.h>
#include "misc.h"
#include "xgnokii_common.h"
#include "xgnokii.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii_dtmf.h"
#include "xpm/Send.xpm"
#include "xpm/Delete.xpm"

#define MAX_DTMF_LENGTH		50

static GtkWidget *GUI_DTMFWindow;
static GtkWidget *displayEntry;
static ErrorDialog errorDialog = {NULL, NULL};
static gchar *saveFileName;


static inline void Help1 (GtkWidget *w, gpointer data)
{
  gchar *indx = g_strdup_printf ("/%s/gnokii/xgnokii/dtmf/index.htm", xgnokiiConfig.locale);
  Help (w, indx);
  g_free (indx);
}


static inline void ButtonCB (GtkWidget *widget,
                      gpointer   data )
{
  gchar buf[2] = " ";

  *buf = GPOINTER_TO_INT (data);
  gtk_entry_append_text (GTK_ENTRY (displayEntry), buf);
}


static inline void ClearCB (GtkWidget *widget,
                            gpointer   data )
{
  gtk_entry_set_text (GTK_ENTRY (displayEntry), "");
}


static inline void SendCB (GtkWidget *widget,
                            gpointer   data )
{
  PhoneEvent *e;
  gchar *buf = g_strdup (gtk_entry_get_text (GTK_ENTRY (displayEntry)));

  if (*buf == '\0')
  {
    g_free (buf);
    return;
  }

  e = g_malloc (sizeof (PhoneEvent));
  e->event = Event_SendDTMF;
  e->data = buf;
  GUI_InsertEvent (e);
}


static void OkLoadDialog (GtkWidget *w, GtkFileSelection *fs)
{
  FILE *f;
  gchar *fileName;
  gchar line[MAX_DTMF_LENGTH + 1];
  gchar buf[80];

  fileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  gtk_widget_hide (GTK_WIDGET (fs));

  if ((f = fopen (fileName, "r")) == NULL)
  {
    g_snprintf (buf, 80, _("Can't open file %s for reading!"), fileName);
    gtk_label_set_text (GTK_LABEL(errorDialog.text), buf);
    gtk_widget_show (errorDialog.dialog);
    return;
  }

  fgets (line, MAX_DTMF_LENGTH + 1, f);
  gtk_entry_set_text (GTK_ENTRY (displayEntry), line);
  fclose (f);
}


static void LoadFileDialog ()
{  
  static GtkWidget *fileDialog = NULL;

  if (fileDialog == NULL)
  {
    fileDialog = gtk_file_selection_new (_("Load"));
    gtk_signal_connect (GTK_OBJECT (fileDialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (OkLoadDialog), (gpointer) fileDialog);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->cancel_button),
                        "clicked", GTK_SIGNAL_FUNC (CancelDialog), (gpointer) fileDialog);
  }

  gtk_widget_show (fileDialog);
}


static void SaveDTMF ()
{
  FILE *f;
  gchar buf[80];

  if ((f = fopen (saveFileName, "w")) == NULL)
  {
    g_snprintf (buf, 80, _("Can't open file %s for writing!"), saveFileName);
    gtk_label_set_text (GTK_LABEL(errorDialog.text), buf);
    gtk_widget_show (errorDialog.dialog);
    return;
  }

  fputs (gtk_entry_get_text (GTK_ENTRY (displayEntry)), f);
  fclose (f);
}


static inline void YesSaveDialog (GtkWidget *w, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  SaveDTMF();
}


static void OkSaveDialog (GtkWidget *w, GtkFileSelection *fs)
{
  FILE *f;
  static YesNoDialog dialog = { NULL, NULL};
  gchar err[80]; 

  saveFileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  gtk_widget_hide (GTK_WIDGET (fs));

  if ((f = fopen (saveFileName, "r")) != NULL)
  {
    fclose (f);
    if (dialog.dialog == NULL)
    {
      CreateYesNoDialog (&dialog, YesSaveDialog, CancelDialog, GUI_DTMFWindow);
      gtk_window_set_title (GTK_WINDOW (dialog.dialog), _("Overwrite file?"));
      g_snprintf ( err, 80, _("File %s already exist.\nOverwrite?"), saveFileName); 
      gtk_label_set_text (GTK_LABEL(dialog.text), err);
    }
    gtk_widget_show (dialog.dialog);
  }
  else
    SaveDTMF ();
}


static void SaveFileDialog ()
{
  static GtkWidget *fileDialog = NULL;

  if (fileDialog == NULL)
  {
    fileDialog = gtk_file_selection_new (_("Save"));
    gtk_signal_connect (GTK_OBJECT (fileDialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (OkSaveDialog), (gpointer) fileDialog);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->cancel_button),
                        "clicked", GTK_SIGNAL_FUNC (CancelDialog), (gpointer) fileDialog);
  }

  gtk_widget_show (fileDialog);
}


static inline void CloseDTMF (GtkWidget *w, gpointer data)
{
  gtk_widget_hide (GUI_DTMFWindow);
}


inline void GUI_ShowDTMF ()
{
  gtk_widget_show (GUI_DTMFWindow);
}


static GtkItemFactoryEntry menu_items[] = {
  { NULL,		NULL,		NULL, 0, "<Branch>"},
  { NULL,		"<control>O",	LoadFileDialog, 0, NULL},
  { NULL,		"<control>S",	SaveFileDialog, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Separator>"},
  { NULL,		"<control>W",	CloseDTMF, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<LastBranch>"},
  { NULL,		NULL,		Help1, 0, NULL},
  { NULL,		NULL,		GUI_ShowAbout, 0, NULL},
};


static void InitMainMenu (void)
{
  menu_items[0].path = g_strdup (_("/_File"));
  menu_items[1].path = g_strdup (_("/File/_Open"));
  menu_items[2].path = g_strdup (_("/File/_Save"));
  menu_items[3].path = g_strdup (_("/File/Sep1"));
  menu_items[4].path = g_strdup (_("/File/_Close"));
  menu_items[5].path = g_strdup (_("/_Help"));
  menu_items[6].path = g_strdup (_("/Help/_Help"));
  menu_items[7].path = g_strdup (_("/Help/_About"));
}


void GUI_CreateDTMFWindow ()
{
  int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  GtkWidget *menubar;
  GtkWidget *main_vbox;
  GtkWidget *table;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *pixmap;
  gchar buf[2];
  gint i,j;


  InitMainMenu ();
  GUI_DTMFWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (GUI_DTMFWindow), _("Dial Tone"));
  //gtk_widget_set_usize (GTK_WIDGET (GUI_DTMFWindow), 436, 220);
  gtk_signal_connect (GTK_OBJECT (GUI_DTMFWindow), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);
  gtk_widget_realize (GUI_DTMFWindow);

  accel_group = gtk_accel_group_new ();
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
                                       accel_group);

  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  gtk_accel_group_attach (accel_group, GTK_OBJECT (GUI_DTMFWindow));

  /* Finally, return the actual menu bar created by the item factory. */ 
  menubar = gtk_item_factory_get_widget (item_factory, "<main>");

  main_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
  gtk_container_add (GTK_CONTAINER (GUI_DTMFWindow), main_vbox);
  gtk_widget_show (main_vbox);

  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, FALSE, 0);
  gtk_widget_show (menubar);

  table = gtk_table_new (8, 3, TRUE);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_border_width (GTK_CONTAINER (table), 3);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);

  displayEntry = gtk_entry_new_with_max_length (MAX_DTMF_LENGTH);
  gtk_table_attach_defaults (GTK_TABLE (table), displayEntry, 0, 3, 0, 1);
  gtk_widget_set_usize (displayEntry, 110, 22);

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
    {
      g_snprintf (buf, 2, "%d", j * 3 + i + 1);
      button = gtk_button_new_with_label (buf);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (ButtonCB), (gpointer) ((gint) *buf));
      gtk_table_attach_defaults (GTK_TABLE (table), button, i, i + 1, j + 2, j + 3);
    }

  button = gtk_button_new_with_label ("*");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (ButtonCB), (gpointer) '*');
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 5, 6);

  button = gtk_button_new_with_label ("0");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (ButtonCB), (gpointer) '0');
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, 5, 6);

  button = gtk_button_new_with_label ("#");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (ButtonCB), (gpointer) '#');
  gtk_table_attach_defaults (GTK_TABLE (table), button, 2, 3, 5, 6);


  button = gtk_button_new_with_label ("Pause");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (ButtonCB), (gpointer) 'p');
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 6, 7);

  button = gtk_button_new_with_label ("Wait");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (ButtonCB), (gpointer) 'w');
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, 6, 7);

  button = gtk_button_new ();
  pixmap = NewPixmap(Send_xpm, GUI_DTMFWindow->window,
                     &GUI_DTMFWindow->style->bg[GTK_STATE_NORMAL]);
  gtk_container_add (GTK_CONTAINER (button), pixmap);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (SendCB), (gpointer) NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 3, 7, 8);

  button = gtk_button_new ();
  pixmap = NewPixmap(Delete_xpm, GUI_DTMFWindow->window,
                     &GUI_DTMFWindow->style->bg[GTK_STATE_NORMAL]);
  gtk_container_add (GTK_CONTAINER (button), pixmap);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (ClearCB), (gpointer) NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 2, 3, 6, 7);

  gtk_widget_show_all (table);

  CreateErrorDialog (&errorDialog, GUI_DTMFWindow);
}
