/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef WIN32
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
#endif
#include <string.h>
#include <gtk/gtk.h>
#include "misc.h"  /* for _() */
#include "xgnokii_common.h"
#include "xgnokii.h"
#include "xpm/quest.xpm"
#include "xpm/stop.xpm"
#include "xpm/info.xpm"

typedef struct {
  GUIEventType type;
  void (*func)(void);
} GUIEvent;

static GSList *guiEvents = NULL;


inline void DeleteEvent (const GtkWidget *widget, const GdkEvent *event, const gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (widget));
}


inline void CancelDialog (const GtkWidget *widget, const gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
}


void CreateErrorDialog (ErrorDialog *errorDialog, GtkWidget *window)
{
  GtkWidget *button, *hbox, *pixmap;

  errorDialog->dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (errorDialog->dialog), _("Error"));
  gtk_window_set_modal (GTK_WINDOW (errorDialog->dialog), TRUE);
  gtk_window_position (GTK_WINDOW (errorDialog->dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width (GTK_CONTAINER (errorDialog->dialog), 5);
  gtk_signal_connect (GTK_OBJECT (errorDialog->dialog), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (errorDialog->dialog)->action_area),
                      button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (CancelDialog), (gpointer) errorDialog->dialog);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (errorDialog->dialog)->vbox), hbox);
  gtk_widget_show (hbox);

  if (window)
  {
    pixmap = NewPixmap (stop_xpm, window->window,
                        &window->style->bg[GTK_STATE_NORMAL]);
    gtk_box_pack_start (GTK_BOX(hbox), pixmap, FALSE, FALSE, 10);
    gtk_widget_show (pixmap);
  }

  errorDialog->text = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX(hbox), errorDialog->text, FALSE, FALSE, 10);
  gtk_widget_show (errorDialog->text);
}


void CreateInfoDialog (InfoDialog *infoDialog, GtkWidget *window)
{
  GtkWidget *hbox, *pixmap;

  infoDialog->dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (infoDialog->dialog), _("Info"));
  gtk_window_set_modal (GTK_WINDOW (infoDialog->dialog), TRUE);
  gtk_window_position (GTK_WINDOW (infoDialog->dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width (GTK_CONTAINER (infoDialog->dialog), 5);
  gtk_signal_connect (GTK_OBJECT (infoDialog->dialog), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (infoDialog->dialog)->vbox), hbox);
  gtk_widget_show_now (hbox);
  
  if (window)
  {
    pixmap = NewPixmap (info_xpm, window->window,
                        &window->style->bg[GTK_STATE_NORMAL]);
    gtk_box_pack_start (GTK_BOX(hbox), pixmap, FALSE, FALSE, 10);
    gtk_widget_show_now (pixmap);
  }

  infoDialog->text = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX(hbox), infoDialog->text, FALSE, FALSE, 10);
  gtk_widget_show_now (infoDialog->text);
}


void CreateYesNoDialog (YesNoDialog *yesNoDialog, const GtkSignalFunc yesFunc,
                        const GtkSignalFunc noFunc, GtkWidget *window)
{
  GtkWidget *button, *hbox, *pixmap;

  yesNoDialog->dialog = gtk_dialog_new ();
  gtk_window_position (GTK_WINDOW (yesNoDialog->dialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (yesNoDialog->dialog), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (yesNoDialog->dialog), 5);
  gtk_signal_connect (GTK_OBJECT (yesNoDialog->dialog), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);


  button = gtk_button_new_with_label (_("Yes"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesNoDialog->dialog)->action_area),
                      button, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (yesFunc), (gpointer) yesNoDialog->dialog);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);                               
  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("No"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesNoDialog->dialog)->action_area),
                      button, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (noFunc), (gpointer) yesNoDialog->dialog);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesNoDialog->dialog)->action_area),
                      button, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (CancelDialog), (gpointer) yesNoDialog->dialog);
  gtk_widget_show (button);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (yesNoDialog->dialog)->vbox), hbox);
  gtk_widget_show (hbox);
  
  if (window)
  {
    pixmap = NewPixmap (quest_xpm, window->window,
                        &window->style->bg[GTK_STATE_NORMAL]);
    gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 10);
    gtk_widget_show (pixmap);
  }

  yesNoDialog->text = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), yesNoDialog->text, FALSE, FALSE, 10);
  gtk_widget_show (yesNoDialog->text);
}


GtkWidget* NewPixmap (gchar **data, GdkWindow *window, GdkColor *background)
{
  GtkWidget *wpixmap;
  GdkPixmap *pixmap;
  GdkBitmap *mask;

  pixmap = gdk_pixmap_create_from_xpm_d (window, &mask, background, data);

  wpixmap = gtk_pixmap_new (pixmap, mask);

  return wpixmap;
}


gint LaunchProcess (const gchar *p, const gchar *arg, const gint infile,
                    const gint outfile, const gint errfile)
{
  pid_t pid;

  if (p == 0)
    return (1);
  pid = fork ();
  if (pid == -1)
    return (-1);
  if (pid == 0)
  {
    pid = getpid ();
    setpgid (pid, pid);
    if (getuid () != geteuid ())
      seteuid (getuid ());

    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_DFL);
    signal (SIGTSTP, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
    signal (SIGTTOU, SIG_DFL);
    signal (SIGCHLD, SIG_DFL);

    if (infile != STDIN_FILENO)
    {
      dup2 (infile, STDIN_FILENO);
      close (infile);
    }
    if (outfile != STDOUT_FILENO)
    {
      dup2 (outfile, STDOUT_FILENO);
      close (outfile);
    }
    if (errfile != STDERR_FILENO)
    {
      dup2 (errfile, STDERR_FILENO);
      close (errfile);
    }

    execlp (p, p, arg, NULL);
    g_print (_("Can't exec %s\n"), p);
    execlp ("/bin/false", p, NULL);
    return (-1);
  }

  setpgid (pid, pid);
  return (0);
}


void RemoveZombie (const gint sign)
{
  gint status;

  wait (&status);
}


void Help (const GtkWidget *w, const gpointer data)
{
  gchar buf[255] = "file:";

  strncat (buf, xgnokiiConfig.xgnokiidocsdir, 255 - strlen (buf));
  buf[254] = '\0';
  strncat (buf, (gchar *) data, 255 - strlen (buf));
  buf[254] = '\0';
  LaunchProcess (xgnokiiConfig.helpviewer, buf, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
}


inline gint strrncmp (const gchar * const s1, const gchar * const s2, size_t n)
{
  gint l1 = strlen (s1);
  gint l2 = strlen (s2);

  if (l1 == 0 && l2 != 0)
    return (-1);
  else if (l1 != 0 && l2 == 0)
    return (1);

  while (l1-- > 0 && l2-- > 0 && n-- > 0)
  {
    if (s1[l1] < s2[l2])
      return (-1);
    else if (s1[l1] > s2[l2])
      return (1);
  }

  return (0);
}


inline void GUI_Refresh (void)
{
  while (gtk_events_pending())
    gtk_main_iteration();
}


inline void SetSortColumn (GtkWidget *widget, SortColumn *data)
{
  gtk_clist_set_sort_column (GTK_CLIST (data->clist), data->column);
  gtk_clist_sort (GTK_CLIST (data->clist));
}


inline void GUIEventAdd (GUIEventType type, void (*func)(void))
{
  GUIEvent *event = g_malloc (sizeof (GUIEvent));
  
  event->type = type;
  event->func = func;
  
  guiEvents = g_slist_append (guiEvents, event);
}


bool GUIEventRemove (GUIEventType type, void (*func)(void))
{
  GUIEvent event;
  GSList *list;
  
  event.type = type;
  event.func = func;
  
  list = g_slist_find (guiEvents, &event);
  if (list)
  {
    g_print ("Nasiel som\n");
    guiEvents = g_slist_remove_link (guiEvents, list);
    g_slist_free_1 (list);
    return (TRUE);
  }
  
  return (FALSE);
}

static inline void CallEvent (gpointer data, gpointer user_data)
{
  GUIEvent *event = (GUIEvent *) data;
  
  if (event->type == GPOINTER_TO_INT (user_data))
    event->func ();
}

inline void GUIEventSend (GUIEventType type)
{
  g_slist_foreach (guiEvents, CallEvent, GINT_TO_POINTER (type));
}
