/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  $Id: xgnokii_contacts.c,v 1.29 2001/03/23 08:24:56 ja Exp $
  
*/


#include "misc.h"

#include <stdio.h>
#include <pthread.h>

#ifndef WIN32
# include <unistd.h>
#else
# include <windows.h>
# define sleep(x) Sleep((x) * 1000)
# define usleep(x) Sleep(((x) < 1000) ? 1 : ((x) / 1000))
# define index strchr
#endif

#include <stdlib.h>
#include <string.h>
#if __unices__
# include <strings.h>
#endif
#include <gtk/gtk.h>

#include "gsm-api.h"
#include "gsm-common.h"
#include "xgnokii_contacts.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii.h"
#include "xgnokii_common.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii_sms.h"
#include "xpm/Read.xpm"
#include "xpm/Send.xpm"
#include "xpm/Open.xpm"
#include "xpm/Save.xpm"
#include "xpm/New.xpm"
#include "xpm/Duplicate.xpm"
#include "xpm/Edit.xpm"
#include "xpm/Delete.xpm"
#include "xpm/Dial.xpm"
#include "xpm/sim.xpm"
#include "xpm/phone.xpm"
#include "xpm/quest.xpm"


typedef struct {
  GtkWidget *dialog;
  GtkWidget *entry;
} DialVoiceDialog;

typedef struct {
  GtkWidget *dialog;
  GtkWidget *entry;
  gint row;
  GtkWidget *gbutton;
  GtkWidget *mbutton;
  GtkWidget *wbutton;
  GtkWidget *fbutton;
  GtkWidget *hbutton;
  PhonebookEntry *pbEntry;
} ExtPbkDialog;

static GtkWidget *GUI_ContactsWindow;
static bool fbus3810;
static bool contactsMemoryInitialized;
static MemoryStatus memoryStatus;
static ContactsMemory contactsMemory;	/* Hold contacts. */
static GtkWidget *clist;
static StatusInfo statusInfo;
static ProgressDialog progressDialog = {NULL, NULL, NULL};
static ErrorDialog errorDialog = {NULL, NULL};
static FindEntryStruct findEntryStruct = { "", 0};
static ExportDialogData exportDialogData = {NULL};
static MemoryPixmaps memoryPixmaps;
static QuestMark questMark;
static EditEntryData newEditEntryData = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static EditEntryData editEditEntryData = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static EditEntryData duplicateEditEntryData = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static EditNumbersData editNumbersData = {NULL, NULL};

/* return != 0 if user has unsaved changes in contacts memory */
inline gint GUI_ContactsIsChanged (void)
{
  return statusInfo.ch_ME | statusInfo.ch_SM;
}


/* return TRUE if Contacts memory was read from phone or from file */
inline bool GUI_ContactsIsIntialized (void)
{
  return contactsMemoryInitialized;
}


static inline void Help1 (GtkWidget *w, gpointer data)
{
  gchar *indx = g_strdup_printf ("/help/%s/windows/contacts/index.html", xgnokiiConfig.locale);
  Help (w, indx);
  g_free (indx);
}


void RefreshStatusInfo (void)
{
  char p,s;
  
  if (statusInfo.ch_ME)
    p = '*';
  else
    p = ' ';
  
  if (statusInfo.ch_SM)
    s = '*';
  else
    s = ' ';
  g_snprintf(statusInfo.text, STATUS_INFO_LENGTH, "SIM: %d/%d%c  Phone: %d/%d%c",
              memoryStatus.UsedSM, memoryStatus.MaxSM, s,
              memoryStatus.UsedME, memoryStatus.MaxME, p);
  gtk_label_set_text(GTK_LABEL (statusInfo.label), statusInfo.text);
}


static inline void SetGroup0 (GtkWidget *item, gpointer data)
{
  ((EditEntryData *) data)->newGroup = 0;
}


static inline void SetGroup1 (GtkWidget *item, gpointer data)
{
  ((EditEntryData *) data)->newGroup = 1;
}


static inline void SetGroup2 (GtkWidget *item, gpointer data)
{
  ((EditEntryData *) data)->newGroup = 2;
}


static inline void SetGroup3 (GtkWidget *item, gpointer data)
{
  ((EditEntryData *) data)->newGroup = 3;
}


static inline void SetGroup4 (GtkWidget *item, gpointer data)
{
  ((EditEntryData *) data)->newGroup = 4;
}


static inline void SetGroup5 (GtkWidget *item, gpointer data)
{
  ((EditEntryData *) data)->newGroup = 5;
}


PhonebookEntry *FindFreeEntry (GSM_MemoryType type)
{
  PhonebookEntry *entry;
  gint start, end;
  register gint i;

  if (type == GMT_ME)
  {
    if (memoryStatus.FreeME == 0)
      return NULL;
    start = 0;
    end = memoryStatus.MaxME;
  }
  else
  {
    if (memoryStatus.FreeSM == 0)
      return NULL;
    start = memoryStatus.MaxME;
    end = memoryStatus.MaxME + memoryStatus.MaxSM;
  }

  for(i = start; i < end; i++)
  {
    entry = g_ptr_array_index(contactsMemory, i);
    if (entry->status == E_Empty || entry->status == E_Deleted)
      return entry;
  }

  return NULL;
}


inline PhonebookEntry *GUI_GetEntry (GSM_MemoryType type, gint nr)
{
  if ((type == GMT_ME && (nr < 1 || nr >= memoryStatus.MaxME)) ||
      (type == GMT_SM && (nr < 1 || nr >= memoryStatus.MaxSM)))
    return NULL;

  if (type == GMT_ME)
    return g_ptr_array_index (contactsMemory, nr - 1);
  else
    return g_ptr_array_index (contactsMemory, nr + memoryStatus.MaxME - 1);
}


static void CloseContacts (GtkWidget *w, gpointer data)
{
  gtk_widget_hide (GUI_ContactsWindow);
}


/* I don't want to allow window to close */
static void ProgressDialogDeleteEvent (GtkWidget *w, gpointer data)
{
  return;
}


static void CancelEditDialog (GtkWidget *widget, gpointer data )
{
  gtk_widget_hide(GTK_WIDGET(((EditEntryData*) data)->dialog));
}



static void OkEditEntryDialog (GtkWidget *widget, gpointer data)
{
  gchar *clist_row[4];
  PhonebookEntry *entry;

  if (GTK_TOGGLE_BUTTON(((EditEntryData*) data)->memoryTypePhone)->active &&
      ((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_SM)
  {
    if ((entry = FindFreeEntry(GMT_ME)) == NULL)
    {
      gtk_label_set_text(GTK_LABEL(errorDialog.text), _("Can't change memory type!"));  
      gtk_widget_show (errorDialog.dialog);
      return;
    }
    strncpy(entry->entry.Name, 
            gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
            max_phonebook_name_length);
    entry->entry.Name[max_phonebook_name_length] = '\0';

    if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    {
    }
    else
    {
      strncpy(entry->entry.Number,
              gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
              max_phonebook_number_length);
      entry->entry.Name[max_phonebook_number_length] = '\0';
    }

    entry->entry.Group = ((EditEntryData*) data)->newGroup;

    entry->status = E_Changed;

    ((EditEntryData*) data)->pbEntry->status = E_Deleted;

    memoryStatus.UsedME++;
    memoryStatus.FreeME--;
    memoryStatus.UsedSM--;
    memoryStatus.FreeSM++;
    statusInfo.ch_ME = statusInfo.ch_SM = 1;

    ((EditEntryData*) data)->pbEntry = entry;
  }
  else if (GTK_TOGGLE_BUTTON(((EditEntryData*) data)->memoryTypeSIM)->active &&
      ((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
  {
    if ((entry = FindFreeEntry(GMT_SM)) == NULL)
    {
      gtk_label_set_text(GTK_LABEL (errorDialog.text), _("Can't change memory type!"));
      gtk_widget_show (errorDialog.dialog);
      return;
    }
    strncpy(entry->entry.Name, 
            gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
            max_phonebook_sim_name_length);
    entry->entry.Name[max_phonebook_sim_name_length] = '\0';

    if (strlen(gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name))) > max_phonebook_sim_name_length)
    {
      gtk_label_set_text( GTK_LABEL (errorDialog.text), _("Sorry, phonebook name will be truncated,\n\
because you save it into SIM memory!"));
      gtk_widget_show (errorDialog.dialog);
    }

    if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    {
    }
    else
    {
      strncpy(entry->entry.Number,
              gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
              max_phonebook_sim_number_length);
      entry->entry.Name[max_phonebook_sim_number_length] = '\0';
    }

    entry->entry.Group = ((EditEntryData*) data)->newGroup;

    entry->status = E_Changed;    

    ((EditEntryData*) data)->pbEntry->status = E_Deleted;

    memoryStatus.UsedME--;
    memoryStatus.FreeME++;
    memoryStatus.UsedSM++;
    memoryStatus.FreeSM--;
    statusInfo.ch_ME = statusInfo.ch_SM = 1;    

    ((EditEntryData*) data)->pbEntry = entry;
  }
  else
  {
    if (GTK_TOGGLE_BUTTON(((EditEntryData*) data)->memoryTypePhone)->active)
    {
      strncpy(((EditEntryData*) data)->pbEntry->entry.Name, 
              gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
              max_phonebook_name_length);
      ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_name_length] = '\0';

      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
      {
      }
      else
      {
        strncpy(((EditEntryData*) data)->pbEntry->entry.Number,
                gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
                max_phonebook_number_length);
        ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_number_length] = '\0';
      }
    }
    else
    {
      strncpy(((EditEntryData*) data)->pbEntry->entry.Name, 
              gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
              max_phonebook_sim_name_length);
      ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_sim_name_length] = '\0';

      if (strlen(gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name))) > max_phonebook_sim_name_length)
      {
        gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Sorry, phonebook name will be truncated,\n\
because you save it into SIM memory!"));
        gtk_widget_show (errorDialog.dialog);
      }

      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
      {
      }
      else
      {
        strncpy(((EditEntryData*) data)->pbEntry->entry.Number,
                gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
                max_phonebook_sim_number_length);
        ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_sim_number_length] = '\0';
      }
    }

    ((EditEntryData*) data)->pbEntry->entry.Group = ((EditEntryData*) data)->newGroup;

    ((EditEntryData*) data)->pbEntry->status = E_Changed;

    if (((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
      statusInfo.ch_ME = 1;
    else
      statusInfo.ch_SM = 1;
  }

  gtk_widget_hide (GTK_WIDGET(((EditEntryData*) data)->dialog));

  RefreshStatusInfo ();

  gtk_clist_freeze (GTK_CLIST (clist));
  gtk_clist_remove (GTK_CLIST (clist), ((EditEntryData*) data)->row);

  clist_row[0] = ((EditEntryData*) data)->pbEntry->entry.Name;

  clist_row[1] = ((EditEntryData*) data)->pbEntry->entry.Number;
  
  if( ((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
    clist_row[2] = "P";
  else
    clist_row[2] = "S";
  if (GetModelFeature(FN_CALLERGROUPS)!=0)
    clist_row[3] = xgnokiiConfig.callerGroups[((EditEntryData*) data)->pbEntry->entry.Group];
  else
    clist_row[3] = "";
  gtk_clist_insert(GTK_CLIST (clist), ((EditEntryData*) data)->row, clist_row);
  if (((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
    gtk_clist_set_pixmap( GTK_CLIST (clist), ((EditEntryData*) data)->row, 2,
                          memoryPixmaps.phoneMemPix, memoryPixmaps.mask);
  else
    gtk_clist_set_pixmap( GTK_CLIST (clist), ((EditEntryData*) data)->row, 2,
                          memoryPixmaps.simMemPix, memoryPixmaps.mask);

  gtk_clist_set_row_data(GTK_CLIST (clist), ((EditEntryData*) data)->row, (gpointer) ((EditEntryData*) data)->pbEntry);

  gtk_clist_sort(GTK_CLIST (clist));
  gtk_clist_thaw(GTK_CLIST (clist));
}


static void OkDeleteEntryDialog(GtkWidget *widget, gpointer data )
{
  PhonebookEntry *pbEntry;
  GList *sel;
  gint row;

  sel = GTK_CLIST (clist)->selection;

  gtk_clist_freeze(GTK_CLIST (clist));

  while (sel != NULL)
  {
    row = GPOINTER_TO_INT(sel->data); 
    pbEntry = (PhonebookEntry *) gtk_clist_get_row_data(GTK_CLIST (clist), row);
    sel = sel->next;

    pbEntry->status = E_Deleted;

    if (pbEntry->entry.MemoryType == GMT_ME)
    {
      memoryStatus.UsedME--;
      memoryStatus.FreeME++;
      statusInfo.ch_ME = 1;
    }
    else
    {
      memoryStatus.UsedSM--;
      memoryStatus.FreeSM++;
      statusInfo.ch_SM = 1;
    }

    gtk_clist_remove(GTK_CLIST (clist), row);
  }

  RefreshStatusInfo ();

  gtk_widget_hide(GTK_WIDGET (data));

  gtk_clist_thaw(GTK_CLIST (clist));
}


static void OkNewEntryDialog(GtkWidget *widget, gpointer data )
{
  gchar *clist_row[4];
  PhonebookEntry *entry;

  if (GTK_TOGGLE_BUTTON(((EditEntryData*) data)->memoryTypePhone)->active &&
      ((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_SM)
  {
    if ((entry = FindFreeEntry (GMT_ME)) == NULL)
    {
      gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't change memory type!"));
      gtk_widget_show (errorDialog.dialog);
      return;
    }
    strncpy(entry->entry.Name, 
            gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
            max_phonebook_name_length);
    entry->entry.Name[max_phonebook_name_length] = '\0';

    if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    {
    }
    else
    {
      strncpy(entry->entry.Number,
              gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
              max_phonebook_number_length);
      entry->entry.Name[max_phonebook_number_length] = '\0';
    }

    entry->entry.Group = ((EditEntryData*) data)->newGroup;

    entry->status = E_Changed;

    ((EditEntryData*) data)->pbEntry = entry;
  }
  else if (GTK_TOGGLE_BUTTON(((EditEntryData*) data)->memoryTypeSIM)->active &&
      ((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
  {
    if ((entry = FindFreeEntry (GMT_SM)) == NULL)
    {
      gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't change memory type!"));
      gtk_widget_show (errorDialog.dialog);
      return;
    }
    strncpy(entry->entry.Name, 
            gtk_entry_get_text (GTK_ENTRY (((EditEntryData*) data)->name)),
            max_phonebook_sim_name_length);
    entry->entry.Name[max_phonebook_sim_name_length] = '\0';

    if (strlen(gtk_entry_get_text (GTK_ENTRY (((EditEntryData*) data)->name))) > max_phonebook_sim_name_length)
    {
      gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Sorry, phonebook name will be truncated\n\
because you save it into SIM memory!"));
      gtk_widget_show (errorDialog.dialog);
    }

    if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    {
    }
    else
    {
      strncpy(entry->entry.Number,
              gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
              max_phonebook_sim_number_length);
      entry->entry.Name[max_phonebook_sim_number_length] = '\0';
    }

    entry->entry.Group = ((EditEntryData*) data)->newGroup;

    entry->status = E_Changed;

    ((EditEntryData*) data)->pbEntry = entry;
  }
  else
  {
    if (GTK_TOGGLE_BUTTON(((EditEntryData*) data)->memoryTypePhone)->active)
    {
      strncpy(((EditEntryData*) data)->pbEntry->entry.Name, 
              gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
              max_phonebook_name_length);
      ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_name_length] = '\0';

      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
      {
      }
      else
      {
        strncpy(((EditEntryData*) data)->pbEntry->entry.Number,
                gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
                max_phonebook_number_length);
        ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_number_length] = '\0';
      }
    }
    else
    {
      strncpy (((EditEntryData*) data)->pbEntry->entry.Name, 
              gtk_entry_get_text(GTK_ENTRY (((EditEntryData*) data)->name)),
              max_phonebook_sim_name_length);
      ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_sim_name_length] = '\0';

      if (strlen(gtk_entry_get_text (GTK_ENTRY (((EditEntryData*) data)->name))) > max_phonebook_sim_name_length)
      {
        gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Sorry, phonebook name will be truncated\n\
because you save it into SIM memory!"));
        gtk_widget_show (errorDialog.dialog);
      }

      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
      {
      }
      else
      {
        strncpy (((EditEntryData*) data)->pbEntry->entry.Number,
                gtk_entry_get_text (GTK_ENTRY(((EditEntryData*) data)->number)),
                max_phonebook_sim_number_length);
        ((EditEntryData*) data)->pbEntry->entry.Name[max_phonebook_sim_number_length] = '\0';
      }
    }

    ((EditEntryData*) data)->pbEntry->entry.Group = ((EditEntryData*) data)->newGroup;

    ((EditEntryData*) data)->pbEntry->status = E_Changed;
  }

  if (((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
  {
    memoryStatus.UsedME++;
    memoryStatus.FreeME--;
    statusInfo.ch_ME = 1;
  }
  else
  {
    memoryStatus.UsedSM++;
    memoryStatus.FreeSM--;
    statusInfo.ch_SM = 1;
  }

  gtk_widget_hide(GTK_WIDGET(((EditEntryData*) data)->dialog));

  RefreshStatusInfo ();

  gtk_clist_freeze (GTK_CLIST (clist));
  clist_row[0] = ((EditEntryData*) data)->pbEntry->entry.Name;

  clist_row[1] = ((EditEntryData*) data)->pbEntry->entry.Number;

  if( ((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
    clist_row[2] = "P";
  else
    clist_row[2] = "S";
  if (GetModelFeature(FN_CALLERGROUPS)!=0)
    clist_row[3] = xgnokiiConfig.callerGroups[((EditEntryData*) data)->pbEntry->entry.Group];
  else
    clist_row[3] = "";
  gtk_clist_insert (GTK_CLIST (clist), 1, clist_row);

  if (((EditEntryData*) data)->pbEntry->entry.MemoryType == GMT_ME)
    gtk_clist_set_pixmap (GTK_CLIST (clist), 1, 2,
                          memoryPixmaps.phoneMemPix, memoryPixmaps.mask);
  else
    gtk_clist_set_pixmap (GTK_CLIST (clist), 1, 2,
                          memoryPixmaps.simMemPix, memoryPixmaps.mask);

  gtk_clist_set_row_data (GTK_CLIST (clist), 1, (gpointer) ((EditEntryData*) data)->pbEntry);

  gtk_clist_sort (GTK_CLIST (clist));
  gtk_clist_thaw (GTK_CLIST (clist));
}


static void OkChangeEntryDialog( GtkWidget *widget, gpointer data)
{
  gchar *clist_row[4];
  gint row;
  PhonebookEntry *oldPbEntry, *newPbEntry;
  GList *sel;
  gchar string[100];

  sel = GTK_CLIST (clist)->selection;

  gtk_widget_hide(GTK_WIDGET (data));

  gtk_clist_freeze(GTK_CLIST (clist));

  while (sel != NULL)
  {
    row = GPOINTER_TO_INT (sel->data);
    oldPbEntry = (PhonebookEntry *) gtk_clist_get_row_data(GTK_CLIST (clist), row);

    sel = sel->next;

    if (oldPbEntry->entry.MemoryType == GMT_SM)
    {
      if ((newPbEntry = FindFreeEntry (GMT_ME)) == NULL)
      {
        gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't change memory type!"));  
        gtk_widget_show (errorDialog.dialog);
        return;
      }

      newPbEntry->entry = oldPbEntry->entry;
      newPbEntry->entry.MemoryType = GMT_ME;

      newPbEntry->status = E_Changed;
      oldPbEntry->status = E_Deleted;

      memoryStatus.UsedME++;
      memoryStatus.FreeME--;
      memoryStatus.UsedSM--;
      memoryStatus.FreeSM++;
      statusInfo.ch_ME = statusInfo.ch_SM = 1;

    }
    else
    {
      if ((newPbEntry = FindFreeEntry(GMT_SM)) == NULL)
      {
        gtk_label_set_text(GTK_LABEL (errorDialog.text), _("Can't change memory type!"));
        gtk_widget_show (errorDialog.dialog);
        return;
      }

      newPbEntry->entry = oldPbEntry->entry;
      newPbEntry->entry.Name[max_phonebook_sim_name_length] = '\0';
      newPbEntry->entry.MemoryType = GMT_SM;

      newPbEntry->status = E_Changed;
      oldPbEntry->status = E_Deleted;

      memoryStatus.UsedME--;
      memoryStatus.FreeME++;
      memoryStatus.UsedSM++;
      memoryStatus.FreeSM--;
      statusInfo.ch_ME = statusInfo.ch_SM = 1;
    }

    gtk_clist_remove(GTK_CLIST (clist), row);

    clist_row[0] = newPbEntry->entry.Name;

    if (newPbEntry->entry.SubEntriesCount>0) {
      snprintf(string,100,"%s *",newPbEntry->entry.Number);
      clist_row[1]=string;
    } else clist_row[1] = newPbEntry->entry.Number;
    
    if( newPbEntry->entry.MemoryType == GMT_ME)
      clist_row[2] = "P";
    else
      clist_row[2] = "S";
    if (GetModelFeature(FN_CALLERGROUPS)!=0)
      clist_row[3] = xgnokiiConfig.callerGroups[newPbEntry->entry.Group];
    else
      clist_row[3] = "";
    gtk_clist_insert (GTK_CLIST (clist), row, clist_row);
    if (newPbEntry->entry.MemoryType == GMT_ME)
      gtk_clist_set_pixmap (GTK_CLIST (clist), row, 2,
                            memoryPixmaps.phoneMemPix, memoryPixmaps.mask);
    else
      gtk_clist_set_pixmap (GTK_CLIST (clist), row, 2,
                            memoryPixmaps.simMemPix, memoryPixmaps.mask);

    gtk_clist_set_row_data (GTK_CLIST (clist), row, (gpointer) newPbEntry);
  }

  RefreshStatusInfo ();  
  gtk_clist_sort (GTK_CLIST (clist));
  gtk_clist_thaw (GTK_CLIST (clist));
}


static void SearchEntry (void)
{
  gchar *buf;
  gchar *entry;
  gint i;

  if (!contactsMemoryInitialized || *findEntryStruct.pattern == '\0')
    return;

  gtk_clist_unselect_all( GTK_CLIST (clist));
  g_strup(findEntryStruct.pattern);

  gtk_clist_get_text( GTK_CLIST (clist), findEntryStruct.lastRow,
                      findEntryStruct.type, &entry);
  i = (findEntryStruct.lastRow + 1) % (memoryStatus.MaxME + memoryStatus.MaxSM);

  while (findEntryStruct.lastRow != i)
  {
    buf = g_strdup (entry);
    g_strup (buf);

    if (strstr( buf, findEntryStruct.pattern))
    {
      g_free(buf);
      findEntryStruct.lastRow = i;
      gtk_clist_select_row( GTK_CLIST (clist),
                            (i + memoryStatus.MaxME + memoryStatus.MaxSM - 1)
                            % (memoryStatus.MaxME + memoryStatus.MaxSM),
                            findEntryStruct.type);
      gtk_clist_moveto( GTK_CLIST (clist),
      			(i + memoryStatus.MaxME + memoryStatus.MaxSM - 1)
                        % (memoryStatus.MaxME + memoryStatus.MaxSM),
                        findEntryStruct.type, 0.5, 0.5);
      return;
    }
    g_free(buf);
    gtk_clist_get_text (GTK_CLIST (clist), i, findEntryStruct.type, &entry);

    i = (i + 1) % (memoryStatus.MaxME + memoryStatus.MaxSM);
  }

  gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't find pattern!"));
  gtk_widget_show (errorDialog.dialog);  
}


static void OkFindEntryDialog (GtkWidget *widget, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (((FindEntryData*) data)->nameB)->active)
    findEntryStruct.type = FIND_NAME;
  else
    findEntryStruct.type = FIND_NUMBER;

  strncpy(findEntryStruct.pattern, 
          gtk_entry_get_text (GTK_ENTRY (((FindEntryData*) data)->pattern)),
          max_phonebook_number_length);
  findEntryStruct.pattern[max_phonebook_number_length] = '\0';

  findEntryStruct.lastRow = 0;

  gtk_widget_hide( ((FindEntryData*) data)->dialog);

  SearchEntry ();
}


void CreateGroupMenu (EditEntryData *data)
{
  GtkWidget *item;

  if (data->groupMenu)
  {
    gtk_option_menu_remove_menu (GTK_OPTION_MENU (data->group));
    if (GTK_IS_WIDGET (data->groupMenu))
      gtk_widget_destroy (GTK_WIDGET (data->groupMenu));
    data->groupMenu = NULL;
  }

  data->groupMenu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label (xgnokiiConfig.callerGroups[0]);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (SetGroup0),
                      (gpointer) data);
  gtk_widget_show (item);
  gtk_menu_append (GTK_MENU (data->groupMenu), item);

  item = gtk_menu_item_new_with_label (xgnokiiConfig.callerGroups[1]);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (SetGroup1),
                      (gpointer) data);
  gtk_widget_show (item);
  gtk_menu_append (GTK_MENU (data->groupMenu), item);

  item = gtk_menu_item_new_with_label (xgnokiiConfig.callerGroups[2]);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (SetGroup2),
                      (gpointer) data);
  gtk_widget_show (item);
  gtk_menu_append (GTK_MENU (data->groupMenu), item);

  item = gtk_menu_item_new_with_label (xgnokiiConfig.callerGroups[3]);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (SetGroup3),
                      (gpointer) data);
  gtk_widget_show (item);
  gtk_menu_append (GTK_MENU (data->groupMenu), item);

  item = gtk_menu_item_new_with_label (xgnokiiConfig.callerGroups[4]);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (SetGroup4),
                      (gpointer) data);
  gtk_widget_show (item);
  gtk_menu_append (GTK_MENU (data->groupMenu), item);

  item = gtk_menu_item_new_with_label (xgnokiiConfig.callerGroups[5]);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (SetGroup5),
                      (gpointer) data);
  gtk_widget_show (item);
  gtk_menu_append (GTK_MENU (data->groupMenu), item);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (data->group), data->groupMenu);
}


inline void GUI_RefreshGroupMenu (void)
{
  if (newEditEntryData.group)
    CreateGroupMenu (&newEditEntryData);

  if (editEditEntryData.group)
    CreateGroupMenu (&editEditEntryData);

  if (duplicateEditEntryData.group)
    CreateGroupMenu (&duplicateEditEntryData);
}


static void EditSubEntries (GtkWidget      *clist,
                            gint            row,
                            gint            column,
                            GdkEventButton *event,
                            gpointer        data )
{
  if(event && event->type == GDK_2BUTTON_PRESS)
//    EditPbEntry((PhonebookEntry *) gtk_clist_get_row_data(GTK_CLIST (clist), row),
//                row);
  {
    gchar *buf;

    gtk_clist_get_text (GTK_CLIST (clist), row, 1, &buf);
    g_print (buf);
  }
}


static void EditNumbers (GtkWidget *widget, void *data)
{
  GtkWidget *button, *clistScrolledWindow;
  gchar *row[3];
  register gint i;
  
  
  if (editNumbersData.dialog == NULL)
  {
    editNumbersData.dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (editNumbersData.dialog), _("Numbers"));
    gtk_window_set_modal (GTK_WINDOW (editNumbersData.dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (editNumbersData.dialog), 10);
    gtk_signal_connect (GTK_OBJECT (editNumbersData.dialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);

    button = gtk_button_new_with_label (_("Ok"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editNumbersData.dialog)->action_area),
                        button, TRUE, TRUE, 10);
//    gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                        GTK_SIGNAL_FUNC (okFunc), (gpointer) editEntryData);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label (_("New"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editNumbersData.dialog)->action_area),
                        button, TRUE, TRUE, 10);
//    gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                        GTK_SIGNAL_FUNC (okFunc), (gpointer) editEntryData);
    gtk_widget_show (button);
    
    button = gtk_button_new_with_label (_("Cancel"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editNumbersData.dialog)->action_area),
                        button, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (CancelDialog), (gpointer) editNumbersData.dialog);
    gtk_widget_show (button);

    gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (editNumbersData.dialog)->vbox), 5);
    
    editNumbersData.clist = gtk_clist_new (3);
    gtk_clist_set_shadow_type (GTK_CLIST (editNumbersData.clist), GTK_SHADOW_OUT);

    gtk_clist_set_column_width (GTK_CLIST (editNumbersData.clist), 0, 4);
    gtk_clist_set_column_width (GTK_CLIST (editNumbersData.clist), 1, 115);
    gtk_clist_set_column_width (GTK_CLIST (editNumbersData.clist), 2, 10);
    //gtk_clist_set_column_justification (GTK_CLIST (editNumbersData.clist), 2, GTK_JUSTIFY_CENTER);

    gtk_signal_connect (GTK_OBJECT (editNumbersData.clist), "select_row",
                        GTK_SIGNAL_FUNC (EditSubEntries), NULL);

    clistScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (clistScrolledWindow), editNumbersData.clist);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (clistScrolledWindow),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (editNumbersData.dialog)->vbox),
                       clistScrolledWindow);
                       
    gtk_widget_show (editNumbersData.clist);
    gtk_widget_show (clistScrolledWindow);
  }

  gtk_clist_freeze (GTK_CLIST (editNumbersData.clist));
  gtk_clist_clear (GTK_CLIST (editNumbersData.clist));

  for(i = 0; i < editNumbersData.pbEntry->entry.SubEntriesCount; i++)
  {
    if (strcmp (editNumbersData.pbEntry->entry.Number,
                editNumbersData.pbEntry->entry.SubEntries[i].data.Number) == 0)
      row[0] = "*";
    else
      row[0] = "";
      
    row[1] = editNumbersData.pbEntry->entry.SubEntries[i].data.Number;

    switch (editNumbersData.pbEntry->entry.SubEntries[i].EntryType)
    {
      case GSM_Number:
        switch (editNumbersData.pbEntry->entry.SubEntries[i].NumberType)
        {
          case GSM_General:
            row[2] = "General";
            break;

          case GSM_Mobile:
            row[2] = "Mobile";
            break;

          case GSM_Work:
            row[2] = "Work";
            break;

          case GSM_Fax:
            row[2] = "Fax";
            break;

          case GSM_Home:
            row[2] = "Home";
            break;

          default:
            row[2] = "Unknown";
            break;
        }
        break;

      case GSM_Note:
        row[2] = "Note";
        break;

      case GSM_Postal:
        row[2] = "Postal";
        break;

      case GSM_Email:
        row[2] = "E-Mail";
        break;

      case GSM_Name:
        row[2] = "Name";
        break;

      default:
        row[2] = "Unknown";
        break;
    }
    
    gtk_clist_append (GTK_CLIST (editNumbersData.clist), row);
/*      if (pbEntry->entry.MemoryType == GMT_ME)
      gtk_clist_set_pixmap (GTK_CLIST (clist), row_i, 2,
                            memoryPixmaps.phoneMemPix, memoryPixmaps.mask);
    else
      gtk_clist_set_pixmap (GTK_CLIST (clist), row_i, 2,
                            memoryPixmaps.simMemPix, memoryPixmaps.mask);
*/
  }
    
  gtk_clist_thaw (GTK_CLIST (editNumbersData.clist));
  
  gtk_widget_show (editNumbersData.dialog);
}


static void CreateEditDialog (EditEntryData *editEntryData, gchar *title,
                              GtkSignalFunc okFunc)
{
  GtkWidget *button, *label, *hbox;

  editEntryData->dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (editEntryData->dialog), title);
  gtk_window_set_modal (GTK_WINDOW (editEntryData->dialog), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (editEntryData->dialog), 10);
  gtk_signal_connect (GTK_OBJECT (editEntryData->dialog), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);

  button = gtk_button_new_with_label (_("Ok"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editEntryData->dialog)->action_area),
                      button, TRUE, TRUE, 10);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (okFunc), (gpointer) editEntryData);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  button = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editEntryData->dialog)->action_area),
                      button, TRUE, TRUE, 10);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (CancelEditDialog), (gpointer) editEntryData);
  gtk_widget_show (button);

  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (editEntryData->dialog)->vbox), 5);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (editEntryData->dialog)->vbox), hbox);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Name:"));
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  editEntryData->name = gtk_entry_new_with_max_length (100);

  gtk_box_pack_end(GTK_BOX(hbox), editEntryData->name, FALSE, FALSE, 2);
  gtk_widget_show (editEntryData->name);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (editEntryData->dialog)->vbox), hbox);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Number:"));
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
  {
    button = gtk_button_new ();
    editEntryData->number = gtk_label_new ("");

    gtk_container_add (GTK_CONTAINER (button), editEntryData->number);

    gtk_box_pack_end(GTK_BOX(hbox), button, TRUE, TRUE, 2);

    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (EditNumbers), NULL);
    gtk_widget_show_all (button);
  }
  else
  {
    editEntryData->number = gtk_entry_new_with_max_length (max_phonebook_number_length);
    gtk_box_pack_end(GTK_BOX(hbox), editEntryData->number, FALSE, FALSE, 2);
    gtk_widget_show (editEntryData->number);
  }

/*  if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT) {

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (editEntryData->dialog)->vbox), hbox);
//    gtk_widget_show (hbox);

    label = gtk_label_new (_("Extended:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
//    gtk_widget_show (label);
    
    editEntryData->extended = gtk_combo_new();
    gtk_box_pack_end(GTK_BOX(hbox), editEntryData->extended, FALSE, FALSE, 2);
//    gtk_widget_show (editEntryData->extended);

    gtk_signal_connect (GTK_OBJECT (GTK_COMBO(editEntryData->extended)->entry), "activate",
			GTK_SIGNAL_FUNC (ExtPbkChanged), (gpointer) editEntryData);    
  }
*/
    
  editEntryData->memoryBox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (editEntryData->dialog)->vbox),
                     editEntryData->memoryBox);
    
  label = gtk_label_new (_("Memory:"));
  gtk_box_pack_start(GTK_BOX (editEntryData->memoryBox), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  editEntryData->memoryTypePhone = gtk_radio_button_new_with_label (NULL, _("phone"));
  gtk_box_pack_end (GTK_BOX (editEntryData->memoryBox), editEntryData->memoryTypePhone, TRUE, FALSE, 2);
  gtk_widget_show (editEntryData->memoryTypePhone);

  editEntryData->memoryTypeSIM = gtk_radio_button_new_with_label( 
        gtk_radio_button_group (GTK_RADIO_BUTTON (editEntryData->memoryTypePhone)), "SIM");
  gtk_box_pack_end(GTK_BOX (editEntryData->memoryBox), editEntryData->memoryTypeSIM, TRUE, FALSE, 2);
  gtk_widget_show (editEntryData->memoryTypeSIM);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (editEntryData->dialog)->vbox), hbox);
  gtk_widget_show (hbox);

  editEntryData->groupLabel = gtk_label_new (_("Caller group:"));
  gtk_box_pack_start (GTK_BOX (hbox), editEntryData->groupLabel, FALSE, FALSE, 2);

  editEntryData->group = gtk_option_menu_new ();

  CreateGroupMenu (editEntryData);

  gtk_box_pack_start (GTK_BOX (hbox), editEntryData->group, TRUE, TRUE, 0);
}


static void EditPbEntry(PhonebookEntry *pbEntry, gint row)
{
  if (editEditEntryData.dialog == NULL)
    CreateEditDialog(&editEditEntryData, _("Edit entry"), OkEditEntryDialog);

  editEditEntryData.pbEntry = pbEntry;
  editEditEntryData.newGroup = pbEntry->entry.Group;
  editEditEntryData.row = row;
  editNumbersData.pbEntry = pbEntry;

  gtk_entry_set_text (GTK_ENTRY (editEditEntryData.name), pbEntry->entry.Name);

  if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    gtk_label_set_text (GTK_LABEL (editEditEntryData.number), pbEntry->entry.Number);
  else
    gtk_entry_set_text (GTK_ENTRY (editEditEntryData.number), pbEntry->entry.Number);

  if (pbEntry->entry.MemoryType == GMT_ME)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editEditEntryData.memoryTypePhone), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editEditEntryData.memoryTypeSIM), TRUE);

  gtk_option_menu_set_history( GTK_OPTION_MENU (editEditEntryData.group),
                               pbEntry->entry.Group);

  if (GetModelFeature(FN_CALLERGROUPS)!=0)
  {
    gtk_widget_show (editEditEntryData.group);
    gtk_widget_show (editEditEntryData.groupLabel);
  }
  else
  {
    gtk_widget_hide (editEditEntryData.group);
    gtk_widget_hide (editEditEntryData.groupLabel);
  }

  if (memoryStatus.MaxME > 0)
    gtk_widget_show (GTK_WIDGET (editEditEntryData.memoryBox));
  else
    gtk_widget_hide (GTK_WIDGET (editEditEntryData.memoryBox));

  gtk_widget_show (GTK_WIDGET (editEditEntryData.dialog));
}


void DeletePbEntry (void)
{
  static GtkWidget *dialog = NULL;
  GtkWidget *button, *hbox, *label, *pixmap;

  if (dialog == NULL)
  {
    dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), _("Delete entries"));
    gtk_window_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
    gtk_window_set_modal(GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 10);
    gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);

    button = gtk_button_new_with_label (_("Ok"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                        button, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (OkDeleteEntryDialog), (gpointer) dialog);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);
    button = gtk_button_new_with_label (_("Cancel"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                        button, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (CancelDialog), (gpointer) dialog);
    gtk_widget_show (button);

    gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 5);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox);
    gtk_widget_show (hbox);

    pixmap = gtk_pixmap_new (questMark.pixmap, questMark.mask);
    gtk_box_pack_start(GTK_BOX(hbox), pixmap, FALSE, FALSE, 10);
    gtk_widget_show(pixmap);

    label = gtk_label_new(_("Do you want to delete selected entries?"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
    gtk_widget_show(label);
  }

  gtk_widget_show(GTK_WIDGET (dialog));
}


void NewPbEntry(PhonebookEntry *pbEntry)
{
  if (newEditEntryData.dialog == NULL)
    CreateEditDialog (&newEditEntryData, _("New entry"), OkNewEntryDialog);

  newEditEntryData.pbEntry = pbEntry;
  newEditEntryData.newGroup = 5;
  editNumbersData.pbEntry = pbEntry;

  gtk_entry_set_text (GTK_ENTRY (newEditEntryData.name), "");
  
  if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    gtk_label_set_text (GTK_LABEL (newEditEntryData.number), "");
  else
    gtk_entry_set_text (GTK_ENTRY (newEditEntryData.number), "");

  if (pbEntry->entry.MemoryType == GMT_ME)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (newEditEntryData.memoryTypePhone), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (newEditEntryData.memoryTypeSIM), TRUE);

  gtk_option_menu_set_history( GTK_OPTION_MENU (newEditEntryData.group),
                               pbEntry->entry.Group);

  if (GetModelFeature(FN_CALLERGROUPS)!=0)
  {
    gtk_widget_show (newEditEntryData.group);
    gtk_widget_show (newEditEntryData.groupLabel);
  }
  else
  {
    gtk_widget_hide (newEditEntryData.group);
    gtk_widget_hide (newEditEntryData.groupLabel);
  }

  if (memoryStatus.MaxME > 0)
    gtk_widget_show (GTK_WIDGET (newEditEntryData.memoryBox));
  else
    gtk_widget_hide (GTK_WIDGET (newEditEntryData.memoryBox));

  gtk_widget_show(GTK_WIDGET (newEditEntryData.dialog));
}


void DuplicatePbEntry (PhonebookEntry *pbEntry)
{
  if (duplicateEditEntryData.dialog == NULL)
    CreateEditDialog (&duplicateEditEntryData, _("Duplicate entry"), OkNewEntryDialog);

  duplicateEditEntryData.pbEntry = pbEntry;
  duplicateEditEntryData.newGroup = pbEntry->entry.Group;
  editNumbersData.pbEntry = pbEntry;

  gtk_entry_set_text (GTK_ENTRY (duplicateEditEntryData.name), pbEntry->entry.Name);

  if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
    gtk_label_set_text (GTK_LABEL (duplicateEditEntryData.number), pbEntry->entry.Number);
  else
    gtk_entry_set_text (GTK_ENTRY (duplicateEditEntryData.number), pbEntry->entry.Number);

  if (pbEntry->entry.MemoryType == GMT_ME)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (duplicateEditEntryData.memoryTypePhone), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (duplicateEditEntryData.memoryTypeSIM), TRUE);

  gtk_option_menu_set_history( GTK_OPTION_MENU (duplicateEditEntryData.group),
                               pbEntry->entry.Group);

  if (GetModelFeature(FN_CALLERGROUPS)!=0)
  {
    gtk_widget_show (duplicateEditEntryData.group);
    gtk_widget_show (duplicateEditEntryData.groupLabel);
  }
  else
  {
    gtk_widget_hide (duplicateEditEntryData.group);
    gtk_widget_hide (duplicateEditEntryData.groupLabel);
  }

  if (memoryStatus.MaxME > 0)
    gtk_widget_show (GTK_WIDGET (duplicateEditEntryData.memoryBox));
  else
    gtk_widget_hide (GTK_WIDGET (duplicateEditEntryData.memoryBox));

  gtk_widget_show(GTK_WIDGET (duplicateEditEntryData.dialog));
}


static void EditEntry (void)
{
  if (contactsMemoryInitialized)
  {
    if (GTK_CLIST (clist)->selection == NULL)
      return;

    EditPbEntry((PhonebookEntry *) gtk_clist_get_row_data(GTK_CLIST (clist),
                GPOINTER_TO_INT(GTK_CLIST (clist)->selection->data)),
                GPOINTER_TO_INT(GTK_CLIST (clist)->selection->data));
  }
}


static void DuplicateEntry (void)
{
  PhonebookEntry *new, *old;

  if (contactsMemoryInitialized)
  {
    if (GTK_CLIST (clist)->selection == NULL)
      return;

    old = (PhonebookEntry *) gtk_clist_get_row_data (GTK_CLIST (clist), GPOINTER_TO_INT(GTK_CLIST (clist)->selection->data));

    if ( old->entry.MemoryType == GMT_ME)
    {
      if ((new = FindFreeEntry (GMT_ME)) == NULL)
        if ((new = FindFreeEntry (GMT_SM)) == NULL)
        {
          gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't find free memory."));
          gtk_widget_show (errorDialog.dialog);
          return;
        }
    }
    else
    {
      if ((new = FindFreeEntry(GMT_SM)) == NULL)
        if ((new = FindFreeEntry(GMT_ME)) == NULL)
        {
          gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't find free memory."));
          gtk_widget_show (errorDialog.dialog);
          return;
        }
    }

    new->entry = old->entry;

    DuplicatePbEntry (new);
  }
}


static void NewEntry (void)
{
  PhonebookEntry *entry;

  if (contactsMemoryInitialized)
  {
    if ((entry = FindFreeEntry (GMT_SM)) != NULL)
      NewPbEntry(entry);
    else if ((entry = FindFreeEntry (GMT_ME)) != NULL)
      NewPbEntry(entry);
    else
    {
      gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't find free memory."));
      gtk_widget_show (errorDialog.dialog);
    }
  }
}


static inline void ClickEntry (GtkWidget      *clist,
                 gint            row,
                 gint            column,
                 GdkEventButton *event,
                 gpointer        data )
{
  if(event && event->type == GDK_2BUTTON_PRESS)
    EditPbEntry((PhonebookEntry *) gtk_clist_get_row_data(GTK_CLIST (clist), row),
                row);
}


static inline void DeleteEntry (void)
{
  if (contactsMemoryInitialized)
  {
    if (GTK_CLIST (clist)->selection == NULL)
      return;

    DeletePbEntry ();
  }
}


static void ChMemType (void)
{
  static GtkWidget *dialog = NULL;
  GtkWidget *button, *hbox, *label;

  if (contactsMemoryInitialized)
  {
    if (GTK_CLIST (clist)->selection == NULL)
      return;

    if (dialog == NULL)
    {
      dialog = gtk_dialog_new ();
      gtk_window_set_title (GTK_WINDOW (dialog), _("Changing memory type"));
      gtk_window_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
      gtk_window_set_modal(GTK_WINDOW (dialog), TRUE);
      gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
      gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
                          GTK_SIGNAL_FUNC (DeleteEvent), NULL);

      button = gtk_button_new_with_label (_("Continue"));
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                          button, FALSE, FALSE, 0);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (OkChangeEntryDialog), (gpointer) dialog);
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (button);
      gtk_widget_show (button);

      button = gtk_button_new_with_label (_("Cancel"));
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                          button, FALSE, FALSE, 0);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (CancelDialog), (gpointer) dialog);
      gtk_widget_show (button);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox);
      gtk_widget_show (hbox);

      label = gtk_label_new (_("If you change from phone memory to SIM memory\nsome entries may be truncated."));
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 10);
      gtk_widget_show (label);
    }

    gtk_widget_show (dialog);
  }
}


static void FindFirstEntry (void)
{
  static FindEntryData findEntryData = { NULL };
  GtkWidget *button, *label, *hbox;

  if (contactsMemoryInitialized)
  {
    if (findEntryData.dialog == NULL)
    {
      findEntryData.dialog = gtk_dialog_new ();
      gtk_window_set_title (GTK_WINDOW (findEntryData.dialog), _("Find"));
      gtk_window_set_modal (GTK_WINDOW (findEntryData.dialog), TRUE);
      gtk_container_set_border_width (GTK_CONTAINER (findEntryData.dialog), 10);
      gtk_signal_connect (GTK_OBJECT (findEntryData.dialog), "delete_event",
                          GTK_SIGNAL_FUNC (DeleteEvent), NULL);

      button = gtk_button_new_with_label (_("Find"));
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (findEntryData.dialog)->action_area),
                          button, TRUE, TRUE, 10);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (OkFindEntryDialog), (gpointer) &findEntryData);
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (button);
      gtk_widget_show (button);

      button = gtk_button_new_with_label (_("Cancel"));
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (findEntryData.dialog)->action_area),
                          button, TRUE, TRUE, 10);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (CancelEditDialog), (gpointer) &findEntryData);
      gtk_widget_show (button);

      gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (findEntryData.dialog)->vbox), 5);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (GTK_DIALOG (findEntryData.dialog)->vbox), hbox);
      gtk_widget_show (hbox);

      label = gtk_label_new (_("Pattern:"));
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
      gtk_widget_show (label);

      findEntryData.pattern = gtk_entry_new_with_max_length (max_phonebook_name_length);

      gtk_box_pack_end (GTK_BOX (hbox), findEntryData.pattern, FALSE, FALSE, 2);
      gtk_widget_show (findEntryData.pattern);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (GTK_DIALOG (findEntryData.dialog)->vbox), hbox);
      gtk_widget_show (hbox);

      findEntryData.nameB = gtk_radio_button_new_with_label (NULL, _("Name"));
      gtk_box_pack_start (GTK_BOX (hbox), findEntryData.nameB, TRUE, FALSE, 2);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (findEntryData.nameB), TRUE);
      gtk_widget_show (findEntryData.nameB);

      findEntryData.numberB = gtk_radio_button_new_with_label ( 
        gtk_radio_button_group (GTK_RADIO_BUTTON (findEntryData.nameB)), _("Number"));
      gtk_box_pack_start (GTK_BOX(hbox), findEntryData.numberB, TRUE, FALSE, 2);
      gtk_widget_show (findEntryData.numberB);
    }

    gtk_widget_show (findEntryData.dialog);
  }
}


static inline void SelectAll (void)
{
  gtk_clist_select_all (GTK_CLIST (clist));
}


static inline void OkDialVoiceDialog (GtkWidget *w, gpointer data)
{
  PhoneEvent *e;
  gchar *buf = g_strdup (gtk_entry_get_text (GTK_ENTRY (((DialVoiceDialog *) data)->entry)));

  if (strlen (buf) > 0)
  {
    e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
    e->event = Event_DialVoice;
    e->data = buf;
    GUI_InsertEvent (e);
  }
  else
    g_free (buf);

  gtk_widget_hide (((DialVoiceDialog *) data)->dialog);
}


static void DialVoice (void)
{
  static DialVoiceDialog dialVoiceDialog = {NULL, NULL};
  GtkWidget *button, *label;
  PhonebookEntry *pbEntry;

  if (dialVoiceDialog.dialog == NULL)
  {
    dialVoiceDialog.dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (dialVoiceDialog.dialog), _("Dial voice"));
    gtk_window_set_modal(GTK_WINDOW (dialVoiceDialog.dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialVoiceDialog.dialog), 10);
    gtk_signal_connect (GTK_OBJECT (dialVoiceDialog.dialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);

    button = gtk_button_new_with_label (_("Ok"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialVoiceDialog.dialog)->action_area),
                        button, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (OkDialVoiceDialog), (gpointer) &dialVoiceDialog);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);

    button = gtk_button_new_with_label (_("Cancel"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialVoiceDialog.dialog)->action_area),
                        button, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (CancelDialog), (gpointer) dialVoiceDialog.dialog);
    gtk_widget_show (button);

    gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialVoiceDialog.dialog)->vbox), 5);

    label = gtk_label_new (_("Number:"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialVoiceDialog.dialog)->vbox), label, FALSE, FALSE, 5);
    gtk_widget_show (label);

    dialVoiceDialog.entry = gtk_entry_new_with_max_length (GSM_MAX_PHONEBOOK_NUMBER_LENGTH);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialVoiceDialog.dialog)->vbox), dialVoiceDialog.entry, FALSE, FALSE, 5);
    gtk_widget_show (dialVoiceDialog.entry);
  }

  if (GTK_CLIST (clist)->selection != NULL)
  {
    pbEntry = (PhonebookEntry *) gtk_clist_get_row_data (GTK_CLIST (clist),
               GPOINTER_TO_INT (GTK_CLIST (clist)->selection->data));

    gtk_entry_set_text (GTK_ENTRY (dialVoiceDialog.entry), pbEntry->entry.Number);
  }

  gtk_widget_show (dialVoiceDialog.dialog);  
}


static gint CListCompareFunc (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  static gchar phoneText[] = "B";
  static gchar simText[] = "A";
  char *text1 = NULL;
  char *text2 = NULL;
  gint ret;

  GtkCListRow *row1 = (GtkCListRow *) ptr1;
  GtkCListRow *row2 = (GtkCListRow *) ptr2;

  switch (row1->cell[clist->sort_column].type)
  {
    case GTK_CELL_TEXT:
      text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text1 = GTK_CELL_PIXTEXT (row1->cell[clist->sort_column])->text;
      break;
    default:
      break;
  }
  switch (row2->cell[clist->sort_column].type)
  {
    case GTK_CELL_TEXT:
      text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;
      break;
    case GTK_CELL_PIXTEXT:
      text2 = GTK_CELL_PIXTEXT (row2->cell[clist->sort_column])->text;
      break;
    default:
      break;
  }

  if (clist->sort_column == 2)
  {
    if (((PhonebookEntry *) row1->data)->entry.MemoryType == GMT_ME)
      text1 = phoneText;
    else
      text1 = simText;
    if (((PhonebookEntry *) row2->data)->entry.MemoryType == GMT_ME)
      text2 = phoneText;
    else
      text2 = simText;
  }

  if (!text2)
    return (text1 != NULL);

  if (!text1)
    return -1;

  if ((ret = g_strcasecmp (text1, text2)) == 0)
  {
    if (((PhonebookEntry *) row1->data)->entry.MemoryType < ((PhonebookEntry *) row2->data)->entry.MemoryType)
      ret = -1;
    else if (((PhonebookEntry *) row1->data)->entry.MemoryType > ((PhonebookEntry *) row2->data)->entry.MemoryType)
      ret = 1;
  }

  return ret;      
}


static void CreateProgressDialog (gint maxME, gint maxSM)
{
  GtkWidget *vbox, *label;
  GtkAdjustment *adj;
  
  progressDialog.dialog = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_window_position (GTK_WINDOW (progressDialog.dialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (progressDialog.dialog), TRUE);
  gtk_signal_connect (GTK_OBJECT (progressDialog.dialog), "delete_event",
                      GTK_SIGNAL_FUNC (ProgressDialogDeleteEvent), NULL);

  vbox = gtk_vbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (progressDialog.dialog), vbox);

  gtk_widget_show (vbox);

  label = gtk_label_new (_("Phone memory..."));
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  gtk_widget_show (label);

  adj = (GtkAdjustment *) gtk_adjustment_new (0, 1, maxME, 0, 0, 0);
  progressDialog.pbarME = gtk_progress_bar_new_with_adjustment (adj);

  gtk_box_pack_start (GTK_BOX (vbox), progressDialog.pbarME, FALSE, FALSE, 0);
  gtk_widget_show (progressDialog.pbarME);

  label = gtk_label_new (_("SIM memory..."));
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  gtk_widget_show (label);

  adj = (GtkAdjustment *) gtk_adjustment_new (0, 1, maxSM, 0, 0, 0);
  progressDialog.pbarSM = gtk_progress_bar_new_with_adjustment (adj);

  gtk_box_pack_start (GTK_BOX (vbox), progressDialog.pbarSM, FALSE, FALSE, 0);
  gtk_widget_show (progressDialog.pbarSM);
}


static void SaveContacts (void)
{
  register gint i;
  PhonebookEntry *pbEntry;
  PhoneEvent *e;
  D_MemoryLocation *ml;

  if (progressDialog.dialog == NULL)
  {
    CreateProgressDialog (memoryStatus.MaxME, memoryStatus.MaxSM);
  }

  if (contactsMemoryInitialized && progressDialog.dialog)
  {
    gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarME), 0);
    gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarSM), 0);
    gtk_window_set_title (GTK_WINDOW (progressDialog.dialog), _("Saving entries"));  
    gtk_widget_show_now (progressDialog.dialog);

    /* Save Phone memory */
    for(i = 0; i < memoryStatus.MaxME; i++)
    {
      pbEntry = g_ptr_array_index(contactsMemory, i);
#ifdef XDEBUG    
      g_print ("%d;%s;%s;%d;%d;%d\n", pbEntry->entry.Empty, pbEntry->entry.Name,
               pbEntry->entry.Number, (int) pbEntry->entry.MemoryType, pbEntry->entry.Group,
               (int) pbEntry->status);
#endif
      if (pbEntry->status == E_Changed || pbEntry->status == E_Deleted)
      {
        if (pbEntry->status == E_Deleted)
        {
          pbEntry->entry.Name[0] = '\0';
          pbEntry->entry.Number[0] = '\0';
          pbEntry->entry.Group = 5;
        }

        pbEntry->entry.Location = i + 1;
        ml = (D_MemoryLocation *) g_malloc (sizeof (D_MemoryLocation));
        ml->entry = &(pbEntry->entry);
        e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
        e->event = Event_WriteMemoryLocation;
        e->data = ml;
        GUI_InsertEvent (e);
        pthread_mutex_lock (&memoryMutex);
        pthread_cond_wait (&memoryCond, &memoryMutex);
        pthread_mutex_unlock (&memoryMutex);

        if (ml->status != GE_NONE)
        {
          g_print (_("%s: line: %d:Can't write ME memory entry number %d! Error: %d\n"),
                   __FILE__, __LINE__, i + 1, ml->status);
        }
        g_free (ml);
      }
      gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarME), i + 1);
      GUI_Refresh ();
    }

    /* Save SIM memory */
    for (i = memoryStatus.MaxME; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
    {
      pbEntry = g_ptr_array_index (contactsMemory, i);

#ifdef XDEBUG    
      g_print ("%d;%s;%s;%d;%d;%d\n", pbEntry->entry.Empty, pbEntry->entry.Name,
               pbEntry->entry.Number, (int) pbEntry->entry.MemoryType, pbEntry->entry.Group,
               (int) pbEntry->status);
#endif

      if (pbEntry->status == E_Changed || pbEntry->status == E_Deleted)
      {
        if (pbEntry->status == E_Deleted)
        {
          pbEntry->entry.Name[0] = '\0';
          pbEntry->entry.Number[0] = '\0';
          pbEntry->entry.Group = 5;
        }

        pbEntry->entry.Location = i - memoryStatus.MaxME + 1;
        ml = (D_MemoryLocation *) g_malloc (sizeof (D_MemoryLocation));
        ml->entry = &(pbEntry->entry);
        e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
        e->event = Event_WriteMemoryLocation;
        e->data = ml;
        GUI_InsertEvent (e);
        pthread_mutex_lock (&memoryMutex);
        pthread_cond_wait (&memoryCond, &memoryMutex);
        pthread_mutex_unlock (&memoryMutex);

        if (ml->status != GE_NONE)
        {
          g_print(_("%s: line %d:Can't write SM memory entry number %d! Error: %d\n"),
                   __FILE__, __LINE__, i - memoryStatus.MaxME + 1, ml->status);
        }
        g_free (ml);
      }
      gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarSM), i - memoryStatus.MaxME + 1);
      GUI_Refresh ();
    }

    statusInfo.ch_ME = statusInfo.ch_SM = 0;
    RefreshStatusInfo ();
    gtk_widget_hide (progressDialog.dialog);
  }
}


static GtkWidget *CreateSaveQuestionDialog (GtkSignalFunc SaveFunc,
                                            GtkSignalFunc DontSaveFunc)
{
  GtkWidget *dialog;
  GtkWidget *button, *label, *hbox, *pixmap;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("Save changes?"));
  gtk_window_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 10);
  gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);

  button = gtk_button_new_with_label (_("Save"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                      button, TRUE, TRUE, 10);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (SaveFunc), (gpointer) dialog);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("Don't save"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
                      button, TRUE, TRUE, 10);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (DontSaveFunc), (gpointer) dialog);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog)->action_area),
                    button, TRUE, TRUE, 10);
  gtk_signal_connect(GTK_OBJECT (button), "clicked",
                     GTK_SIGNAL_FUNC (CancelDialog), (gpointer) dialog);
  gtk_widget_show (button);

  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 5);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox);
  gtk_widget_show (hbox);

  pixmap = gtk_pixmap_new (questMark.pixmap, questMark.mask);
  gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 10);
  gtk_widget_show (pixmap);

  label = gtk_label_new (_("You have made changes in your\ncontacts directory.\n\
\n\nDo you want save these changes into phone?\n"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  return dialog;
}


static void OkExtPbkDialog (GtkWidget *w, gpointer data) {

  ExtPbkDialog *d=(ExtPbkDialog*)data;
  GSM_NumberType numtype;
  gint i, found=999;
  gchar *chars;

  if (w==d->gbutton) numtype=GSM_General;
  else if (w==d->mbutton) numtype=GSM_Mobile;
  else if (w==d->wbutton) numtype=GSM_Work;
  else if (w==d->fbutton) numtype=GSM_Fax;
  else if (w==d->hbutton) numtype=GSM_Home;
  else numtype=GSM_General;

  /* Search through subentries */

  for (i=0;i<d->pbEntry->entry.SubEntriesCount;i++) {
     if (d->pbEntry->entry.SubEntries[i].EntryType==GSM_Number)
       if (d->pbEntry->entry.SubEntries[i].NumberType==numtype)
	 found=i;
  }

  if (found>i) { /* Not found */
    found=d->pbEntry->entry.SubEntriesCount;
    d->pbEntry->entry.SubEntriesCount++;
    d->pbEntry->entry.SubEntries[found].EntryType=GSM_Number;
    d->pbEntry->entry.SubEntries[found].NumberType=numtype;
  }

  chars=gtk_editable_get_chars(GTK_EDITABLE(d->entry),0,-1);
  strcpy(d->pbEntry->entry.SubEntries[found].data.Number,chars);
  g_free(chars);

  /* Update the entry dialog */
  EditPbEntry(d->pbEntry,d->row);

  gtk_widget_hide (d->dialog);
}


void ExtPbkChanged (GtkWidget *widget, gpointer data )
{
  static ExtPbkDialog dialog = {NULL,NULL,0,NULL,NULL,NULL,NULL,NULL};
  GtkWidget *button;
  static GtkWidget *label;
  gchar *chars;


  if (dialog.dialog==NULL) {

    dialog.dialog=gtk_dialog_new();
    
    gtk_window_set_title (GTK_WINDOW (dialog.dialog), _("Which Extended Phonebook Entry?"));
    gtk_window_set_modal(GTK_WINDOW (dialog.dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog.dialog), 10);
    gtk_signal_connect (GTK_OBJECT (dialog.dialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);

    dialog.gbutton = gtk_button_new_with_label (_("General"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->vbox),
                        dialog.gbutton, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (dialog.gbutton), "clicked",
                        GTK_SIGNAL_FUNC (OkExtPbkDialog), (gpointer)&dialog );
    gtk_widget_show (dialog.gbutton);
    dialog.mbutton = gtk_button_new_with_label (_("Mobile"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->vbox),
                        dialog.mbutton, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (dialog.mbutton), "clicked",
                        GTK_SIGNAL_FUNC (OkExtPbkDialog), (gpointer)&dialog );
    gtk_widget_show (dialog.mbutton);
    dialog.wbutton = gtk_button_new_with_label (_("Work"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->vbox),
                        dialog.wbutton, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (dialog.wbutton), "clicked",
                        GTK_SIGNAL_FUNC (OkExtPbkDialog), (gpointer)&dialog );
    gtk_widget_show (dialog.wbutton);
    dialog.fbutton = gtk_button_new_with_label (_("Fax"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->vbox),
                        dialog.fbutton, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (dialog.fbutton), "clicked",
                        GTK_SIGNAL_FUNC (OkExtPbkDialog), (gpointer)&dialog );
    gtk_widget_show (dialog.fbutton);
    dialog.hbutton = gtk_button_new_with_label (_("Home"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->vbox),
                        dialog.hbutton, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (dialog.hbutton), "clicked",
                        GTK_SIGNAL_FUNC (OkExtPbkDialog), (gpointer)&dialog );
    gtk_widget_show (dialog.hbutton);

    button = gtk_button_new_with_label (_("Cancel"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->action_area),
                        button, TRUE, TRUE, 10);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (CancelDialog), (gpointer) dialog.dialog);
    gtk_widget_show (button);

    label = gtk_label_new("");
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog.dialog)->vbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);
  }

  chars=gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
  gtk_label_set_text(GTK_LABEL(label),chars);
  g_free(chars);
  dialog.pbEntry=((EditEntryData*)data)->pbEntry;
  dialog.row=((EditEntryData*)data)->row;
  dialog.entry=widget;
  gtk_widget_show(dialog.dialog);
}



void GUI_RefreshContacts (void)
{
  PhonebookEntry *pbEntry;
  gint row_i = 0;
  register gint i;

  if (contactsMemoryInitialized == FALSE)
    return;

  gtk_clist_freeze (GTK_CLIST (clist));
  gtk_clist_clear (GTK_CLIST (clist));

  for (i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    gchar *row[4];

    pbEntry = g_ptr_array_index (contactsMemory, i);
    if (pbEntry->status != E_Empty && pbEntry->status != E_Deleted)
    {
      row[0] = pbEntry->entry.Name;

      row[1] = pbEntry->entry.Number;

      if (pbEntry->entry.MemoryType == GMT_ME)
        row[2] = "P";
      else
        row[2] = "S";
      if (GetModelFeature(FN_CALLERGROUPS)!=0)
        row[3] = xgnokiiConfig.callerGroups[pbEntry->entry.Group];
      else
        row[3] = "";
      gtk_clist_append (GTK_CLIST (clist), row);
      if (pbEntry->entry.MemoryType == GMT_ME)
        gtk_clist_set_pixmap (GTK_CLIST (clist), row_i, 2,
                              memoryPixmaps.phoneMemPix, memoryPixmaps.mask);
      else
        gtk_clist_set_pixmap (GTK_CLIST (clist), row_i, 2,
                              memoryPixmaps.simMemPix, memoryPixmaps.mask);

      gtk_clist_set_row_data (GTK_CLIST (clist), row_i++, (gpointer) pbEntry);
    }
#ifdef XDEBUG    
    g_print ("%d;%s;%s;%d;%d;%d\n", pbEntry->entry.Empty, pbEntry->entry.Name,
            pbEntry->entry.Number, (int) pbEntry->entry.MemoryType, pbEntry->entry.Group,
            (int) pbEntry->status);
#endif
  }

  gtk_clist_sort (GTK_CLIST (clist));
  gtk_clist_thaw (GTK_CLIST (clist));
}


static gint InsertPBEntryME (GSM_PhonebookEntry *entry)
{
  PhonebookEntry *pbEntry;

  if ((pbEntry = (PhonebookEntry *) g_malloc (sizeof (PhonebookEntry))) == NULL)
  {
    g_print (_("%s: line %d: Can't allocate memory!\n"), __FILE__, __LINE__);
    g_ptr_array_free (contactsMemory, TRUE);
    gtk_widget_hide (progressDialog.dialog);
    return (-1);
  }

  pbEntry->entry = *entry;

  if (*pbEntry->entry.Name == '\0' && *pbEntry->entry.Number == '\0')
    pbEntry->status = E_Empty;
  else
    pbEntry->status = E_Unchanged;

  g_ptr_array_add (contactsMemory, (gpointer) pbEntry);
  gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarME), entry->Location);
  GUI_Refresh ();

  return (GE_NONE);  
}


static gint InsertPBEntrySM (GSM_PhonebookEntry *entry)
{
  PhonebookEntry *pbEntry;

  if ((pbEntry = (PhonebookEntry *) g_malloc (sizeof (PhonebookEntry))) == NULL)
  {
    g_print (_("%s: line %d: Can't allocate memory!\n"), __FILE__, __LINE__);
    g_ptr_array_free (contactsMemory, TRUE);
    gtk_widget_hide (progressDialog.dialog);
    return (-1);
  }

  pbEntry->entry = *entry;

  if (*pbEntry->entry.Name == '\0' && *pbEntry->entry.Number == '\0')
    pbEntry->status = E_Empty;
  else
  {
    pbEntry->status = E_Unchanged;
    if (fbus3810)
    {
      memoryStatus.UsedSM++;
      memoryStatus.FreeSM--;
    }
  }

  g_ptr_array_add (contactsMemory, (gpointer) pbEntry);
  gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarSM), entry->Location);
  GUI_Refresh ();

  return (GE_NONE);  
}


static inline gint ReadFailedPBEntry (gint i)
{
  g_ptr_array_free (contactsMemory, TRUE);
  gtk_widget_hide (progressDialog.dialog);
  return (0);
}


static void ReadContactsMain (void)
{
  PhoneEvent *e;
  D_MemoryStatus *ms;
  D_MemoryLocationAll *mla;
  PhonebookEntry *pbEntry;
  register gint i;

  fbus3810 = FALSE;

  if (contactsMemoryInitialized == TRUE)
  {
    for (i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
    {
      pbEntry = g_ptr_array_index (contactsMemory, i);
      g_free (pbEntry);
    }
    g_ptr_array_free (contactsMemory, TRUE);
    contactsMemory = NULL;
    gtk_clist_clear (GTK_CLIST (clist));
    contactsMemoryInitialized = FALSE;
    memoryStatus.MaxME = memoryStatus.UsedME = memoryStatus.FreeME =
    memoryStatus.MaxSM = memoryStatus.UsedSM = memoryStatus.FreeSM = 0;
    statusInfo.ch_ME = statusInfo.ch_SM = 0;
    RefreshStatusInfo ();
  }

  ms = (D_MemoryStatus *) g_malloc (sizeof (D_MemoryStatus));
  ms->memoryStatus.MemoryType = GMT_ME;
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_GetMemoryStatus;
  e->data = ms;
  GUI_InsertEvent (e);
  pthread_mutex_lock (&memoryMutex);
  pthread_cond_wait (&memoryCond, &memoryMutex);
  pthread_mutex_unlock (&memoryMutex);

  if (ms->status != GE_NONE)
    /* Phone don't support ME (5110) */
    memoryStatus.MaxME = memoryStatus.UsedME = memoryStatus.FreeME = 0;
  else
  {
    memoryStatus.MaxME = ms->memoryStatus.Used + ms->memoryStatus.Free;
    memoryStatus.UsedME = ms->memoryStatus.Used;
    memoryStatus.FreeME = ms->memoryStatus.Free;
  }

  ms->memoryStatus.MemoryType = GMT_SM;
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_GetMemoryStatus;
  e->data = ms;
  GUI_InsertEvent (e);
  pthread_mutex_lock (&memoryMutex);
  pthread_cond_wait (&memoryCond, &memoryMutex);
  pthread_mutex_unlock (&memoryMutex);

  if (ms->status != GE_NONE)
  {
    fbus3810 = TRUE; /* I try to recognize memoryStatus.UsedSM while reading */
    gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't get SM memory status!\n\n\
Setting max SIM entries to 100!\n"));
    memoryStatus.MaxSM = memoryStatus.FreeSM = 100;
    memoryStatus.UsedSM = 0;
    gtk_widget_show (errorDialog.dialog);
  }
  else
  {
    memoryStatus.MaxSM = ms->memoryStatus.Used + ms->memoryStatus.Free;
    memoryStatus.UsedSM = ms->memoryStatus.Used;
    memoryStatus.FreeSM = ms->memoryStatus.Free;
  }
  g_free (ms);

  statusInfo.ch_ME = statusInfo.ch_SM = 0;

  RefreshStatusInfo ();

  if (progressDialog.dialog == NULL)
  {
    CreateProgressDialog (memoryStatus.MaxME, memoryStatus.MaxSM);
  }

  gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarME), 0);
  gtk_progress_set_value (GTK_PROGRESS (progressDialog.pbarSM), 0);
  gtk_window_set_title (GTK_WINDOW (progressDialog.dialog), _("Getting entries"));
  gtk_widget_show_now (progressDialog.dialog);

  contactsMemory = g_ptr_array_new ();

  mla = (D_MemoryLocationAll *) g_malloc (sizeof (D_MemoryLocationAll));
  if (memoryStatus.MaxME > 0)
  {
    mla->min = 1;
    mla->max = memoryStatus.MaxME;
    mla->type = GMT_ME;
    mla->used = memoryStatus.UsedME; //MW
    mla->InsertEntry = InsertPBEntryME;
    mla->ReadFailed = ReadFailedPBEntry;

    e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
    e->event = Event_GetMemoryLocationAll;
    e->data = mla;
    GUI_InsertEvent (e);
    pthread_mutex_lock (&memoryMutex);
    pthread_cond_wait (&memoryCond, &memoryMutex);
    pthread_mutex_unlock (&memoryMutex);

    if (mla->status != GE_NONE)
    {
      g_free (mla);
      gtk_widget_hide (progressDialog.dialog);
      return;
    }
  }

  mla->min = 1;
  mla->max = memoryStatus.MaxSM;
  mla->type = GMT_SM;
  mla->used = memoryStatus.UsedSM; //MW
  mla->InsertEntry = InsertPBEntrySM;
  mla->ReadFailed = ReadFailedPBEntry;

  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_GetMemoryLocationAll;
  e->data = mla;
  GUI_InsertEvent (e);
  pthread_mutex_lock (&memoryMutex);
  pthread_cond_wait (&memoryCond, &memoryMutex);
  pthread_mutex_unlock (&memoryMutex);

  if (mla->status != GE_NONE)
  {
    g_free (mla);
    gtk_widget_hide (progressDialog.dialog);
    return;
  }

  g_free (mla);

  gtk_widget_hide (progressDialog.dialog);

  contactsMemoryInitialized = TRUE;
  statusInfo.ch_ME = statusInfo.ch_SM = 0;
  GUIEventSend (GUI_EVENT_CONTACTS_CHANGED);
  GUIEventSend (GUI_EVENT_SMS_NUMBER_CHANGED);
}


static void ReadSaveCallback (GtkWidget *widget, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  SaveContacts ();
  ReadContactsMain ();
}


static void ReadDontSaveCallback (GtkWidget *widget, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  ReadContactsMain ();
}


static void ReadSaveContacts (void)
{
  static GtkWidget *dialog = NULL;

  if (dialog == NULL)
    dialog = CreateSaveQuestionDialog (ReadSaveCallback, ReadDontSaveCallback);

  gtk_widget_show (dialog);
}


static void ReadContacts (void)
{
  if (contactsMemoryInitialized == TRUE && (statusInfo.ch_ME || statusInfo.ch_SM))
    ReadSaveContacts ();
  else
    ReadContactsMain ();
}


inline void GUI_ReadContacts (void)
{
  ReadContacts ();
}


inline void GUI_SaveContacts (void)
{
  SaveContacts ();
}


inline void GUI_ShowContacts (void)
{
  if (xgnokiiConfig.callerGroups[0] == NULL)
  {
    GUI_Refresh ();
    GUI_InitCallerGroupsInf ();
  }
  gtk_clist_set_column_visibility (GTK_CLIST (clist), 3, (GetModelFeature(FN_CALLERGROUPS)!=0));
  GUI_RefreshContacts ();
  gtk_widget_show (GUI_ContactsWindow);
//  if (!contactsMemoryInitialized)
//    ReadContacts ();
}

static void ExportVCARD (FILE *f)
{
  gchar buf2[10];
  register gint i,j;
  PhonebookEntry *pbEntry;

  for(i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    pbEntry = g_ptr_array_index (contactsMemory, i);

    if (pbEntry->status == E_Deleted || pbEntry->status == E_Empty)
      continue;

    fprintf (f, "BEGIN:VCARD\n");
    fprintf (f, "VERSION:3.0\n");
    fprintf (f, "FN:%s\n", pbEntry->entry.Name);
    fprintf (f, "TEL;PREF:%s\n", pbEntry->entry.Number);

    if (pbEntry->entry.MemoryType == GMT_ME)
      sprintf (buf2, "ME%d", i + 1);
    else
      sprintf (buf2, "SM%d", i - memoryStatus.MaxME + 1);

    fprintf (f, "X_GSM_STORE_AT:%s\n", buf2);
    fprintf (f, "X_GSM_CALLERGROUP:%d\n", pbEntry->entry.Group);

    /* Add ext. pbk info if required */
    if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
      for (j = 0; j < pbEntry->entry.SubEntriesCount; j++)
      {
	if (pbEntry->entry.SubEntries[j].EntryType == GSM_Number)
          fprintf (f, "TEL;UNKNOWN_%d:%s\n", pbEntry->entry.SubEntries[j].NumberType,
                   pbEntry->entry.SubEntries[j].data.Number);
      }

    fprintf (f, "END:VCARD\n\n");
  }
  
  fclose (f);
}


static void ExportNative (FILE *f)
{
  gchar buf[IO_BUF_LEN], buf2[10];
  register gint i,j;
  PhonebookEntry *pbEntry;


  for(i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    pbEntry = g_ptr_array_index (contactsMemory, i);

    if (pbEntry->status != E_Deleted && pbEntry->status != E_Empty)
    {
      if (index (pbEntry->entry.Name, ';') != NULL)
      {
        strcpy (buf, "\"");
        strcat (buf, pbEntry->entry.Name);
        strcat (buf, "\";");
      }
      else
      {
        strcpy (buf, pbEntry->entry.Name);
        strcat (buf, ";");
      }
      
      if (index (pbEntry->entry.Number, ';') != NULL)
      {
        strcat (buf, "\"");
        strcat (buf, pbEntry->entry.Number);
        strcat (buf, "\";");
      }
      else
      {
        strcat (buf, pbEntry->entry.Number);
        strcat (buf, ";");
      }

      if (pbEntry->entry.MemoryType == GMT_ME)
        sprintf (buf2, "B;%d;%d;", i + 1, pbEntry->entry.Group);
      else
        sprintf (buf2, "A;%d;%d;", i - memoryStatus.MaxME + 1, pbEntry->entry.Group);
      strcat (buf, buf2);

      /* Add ext. pbk info if required */
      if (GetModelFeature(FN_PHONEBOOK)==F_PBK71INT)
      {
        for (j = 0; j < pbEntry->entry.SubEntriesCount; j++)
          if (pbEntry->entry.SubEntries[j].EntryType==GSM_Number)
          {
            sprintf(buf2,"%d;",pbEntry->entry.SubEntries[j].NumberType);
            strcat(buf,buf2);

	    if (index (pbEntry->entry.SubEntries[j].data.Number, ';') != NULL)
	    {
              strcat (buf, "\"");
              strcat (buf, pbEntry->entry.SubEntries[j].data.Number);
              strcat (buf, "\";");
            }
	    else
	    {
              strcat (buf, pbEntry->entry.SubEntries[j].data.Number);
              strcat (buf, ";");
            }
	  }
      }
      fprintf (f, "%s\n", buf);
    }
  }
  
  fclose(f);
}


static void ExportContactsMain (gchar *name)
{
  FILE *f;
  gchar buf[IO_BUF_LEN];

  if ((f = fopen (name, "w")) == NULL)
  {
    g_snprintf (buf, IO_BUF_LEN, _("Can't open file %s for writing!"), name);
    gtk_label_set_text (GTK_LABEL (errorDialog.text), buf);
    gtk_widget_show (errorDialog.dialog);
    return;
  }

  if (strstr (name, ".vcard") || strstr (name, ".gcrd"))
    ExportVCARD (f);
  else
    ExportNative (f);
}


static void YesExportDialog (GtkWidget *w, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  ExportContactsMain (exportDialogData.fileName);
}


static void OkExportDialog (GtkWidget *w, GtkFileSelection *fs)
{
  static YesNoDialog dialog = { NULL, NULL};
  FILE *f;
  gchar err[80];

  exportDialogData.fileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  gtk_widget_hide (GTK_WIDGET (fs));

  if ((f = fopen (exportDialogData.fileName, "r")) != NULL)
  {
    fclose (f);
    if (dialog.dialog == NULL)
    {
      CreateYesNoDialog (&dialog, YesExportDialog, CancelDialog, GUI_ContactsWindow);
      gtk_window_set_title (GTK_WINDOW (dialog.dialog), _("Overwrite file?"));
      g_snprintf ( err, 80, _("File %s already exist.\nOverwrite?"), exportDialogData.fileName); 
      gtk_label_set_text (GTK_LABEL(dialog.text), err);
    }
    gtk_widget_show (dialog.dialog);
  }
  else
    ExportContactsMain (exportDialogData.fileName);
}


static void ExportContacts (void)
{
  static GtkWidget *fileDialog = NULL;

  if (contactsMemoryInitialized)
  {
    if (fileDialog == NULL)
    {
      fileDialog = gtk_file_selection_new (_("Export"));
      gtk_signal_connect (GTK_OBJECT (fileDialog), "delete_event",
                          GTK_SIGNAL_FUNC (DeleteEvent), NULL);
      gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->ok_button),
                          "clicked", GTK_SIGNAL_FUNC (OkExportDialog), (gpointer) fileDialog);
      gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->cancel_button),
                          "clicked", GTK_SIGNAL_FUNC (CancelDialog), (gpointer) fileDialog);
    }

    gtk_widget_show (fileDialog);
  }
}


static bool ParseLine (GSM_PhonebookEntry *entry, gint *num, gchar *buf)
{
  register gint i = 0;
  size_t len;
  gchar s_num[5];
  gchar *endptr;

  entry->Empty = FALSE;
  entry->SubEntriesCount=0;

  len = strlen (buf);
  if (buf[i] == '"')
  {
    i++;
    while (i < len && i <= GSM_MAX_PHONEBOOK_NAME_LENGTH && buf[i] != '"')
    {
      entry->Name[i - 1] = buf[i];
      i++;
    }
    entry->Name[i - 1] = '\0';
    if (i >= len)
      return FALSE;
    if (buf[++i] != ';')
      return FALSE;
  }
  else
  {
    while (i < len && i < GSM_MAX_PHONEBOOK_NAME_LENGTH && buf[i] != ';')
    {
      entry->Name[i] = buf[i];
      i++;
    }
    entry->Name[i] = '\0';
    if (buf[i] != ';')
      return FALSE;
    if (i >= len)
      return FALSE;
  }
  buf += (i + 1);
  i = 0;

  len = strlen (buf);
  if (buf[i] == '"')
  {
    i++;
    while (i < len && i <= GSM_MAX_PHONEBOOK_NUMBER_LENGTH && buf[i] != '"')
    {
      entry->Number[i - 1] = buf[i];
      i++;
    }
    entry->Number[i - 1] = '\0';
    if (i >= len)
      return FALSE;
    if (buf[++i] != ';')
      return FALSE;
  }
  else
  {
    while (i < len && i < GSM_MAX_PHONEBOOK_NUMBER_LENGTH && buf[i] != ';')
    {
      entry->Number[i] = buf[i];
      i++;
    }
    entry->Number[i] = '\0';
    if (buf[i] != ';')
      return FALSE;
    if (i >= len)
      return FALSE;
  }
  buf += (i + 1);
  i = 0;

  if (!strncmp(buf, "B;", 2))
  {
    entry->MemoryType = GMT_ME;
    buf += 2;
  }
  else
  if (!strncmp(buf, "A;", 2))
  {
    entry->MemoryType = GMT_SM;
    buf += 2;
  }
  else
  if (!strncmp(buf, "ME;", 3))
  {
    entry->MemoryType = GMT_ME;
    buf += 3;
  }
  else
  if (!strncmp(buf, "SM;", 3))
  {
    entry->MemoryType = GMT_SM;
    buf += 3;
  }
  else
    return FALSE;

  len = strlen (buf);
  while (i < len && i < 4 && buf[i] != ';')
  {
    s_num[i] = buf[i];
    i++;
  }
  s_num[i] = '\0';
  if (buf[i] != ';')
    return FALSE;
  if (i >= len)
    return FALSE;
  buf += (i + 1);
  i = 0;

  *num = strtol (s_num, &endptr, 10);
  if (s_num == endptr)
    return FALSE;
  if (*num == LONG_MIN || *num == LONG_MAX)
    return FALSE;


  if ( *buf == '\n' || *buf == '\r')   /* for compatibility with NCDS */
    entry->Group = 5;
  else
  {
    len = strlen (buf);
    while (i < len && i < 4 && buf[i] != ';')
    {
      s_num[i] = buf[i];
      i++;
    }
    s_num[i] = '\0';
    if (buf[i] != ';')
      return FALSE;
    if (i >= len)
      return FALSE;
    
    entry->Group = strtol (s_num, &endptr, 10);
    if (s_num == endptr)
      return FALSE;
    if (entry->Group == LONG_MIN || entry->Group == LONG_MAX)
      return FALSE;
    if (entry->Group < 0 || entry->Group > 5)
      return FALSE;

    buf += (i + 1);
    i = 0;

    /* Ext Pbk Support */
    
    while (*buf!='\n')
    {
      entry->SubEntries[entry->SubEntriesCount].EntryType=GSM_Number;
      len = strlen (buf);
      while (i < len && i < 4 && buf[i] != ';')
      {
        s_num[i] = buf[i];
        i++;
      }
      s_num[i] = '\0';
      if (buf[i] != ';')
        return FALSE;
      if (i >= len)
        return FALSE;
      entry->SubEntries[entry->SubEntriesCount].NumberType=(GSM_NumberType)strtol (s_num, &endptr, 10);
      if (s_num == endptr)
        return FALSE;
      buf += (i + 1);
      i = 0;
      len = strlen (buf);
      while (i < len && i < GSM_MAX_PHONEBOOK_TEXT_LENGTH && buf[i] != ';')
      {
        entry->SubEntries[entry->SubEntriesCount].data.Number[i] = buf[i];
        i++;
      }
      entry->SubEntries[entry->SubEntriesCount].data.Number[i] = '\0';
      buf += (i + 1);
      i = 0;
      if (i >= len)
        return FALSE;
      entry->SubEntriesCount++;
    }
  }

  return TRUE;
}


static void OkImportDialog (GtkWidget *w, GtkFileSelection *fs)
{
  FILE *f;
  PhoneEvent *e;
  D_MemoryStatus *ms;
  PhonebookEntry *pbEntry;
  GSM_PhonebookEntry gsmEntry;
  gchar buf[IO_BUF_LEN];
  gchar *fileName;
  gint i;

  fileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  gtk_widget_hide (GTK_WIDGET (fs));

  if ((f = fopen (fileName, "r")) == NULL)
  {
    g_snprintf (buf, IO_BUF_LEN, _("Can't open file %s for reading!"), fileName);
    gtk_label_set_text (GTK_LABEL(errorDialog.text), buf);
    gtk_widget_show (errorDialog.dialog);
    return;
  }

  if (contactsMemoryInitialized == TRUE)
  {
    for(i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
    {
      pbEntry = g_ptr_array_index(contactsMemory, i);
      g_free(pbEntry);
    }
    g_ptr_array_free (contactsMemory, TRUE);
    contactsMemory = NULL;
    gtk_clist_clear (GTK_CLIST (clist));
    contactsMemoryInitialized = FALSE;
    memoryStatus.MaxME = memoryStatus.UsedME = memoryStatus.FreeME =
    memoryStatus.MaxSM = memoryStatus.UsedSM = memoryStatus.FreeSM = 0;
    statusInfo.ch_ME = statusInfo.ch_SM = 0;
    RefreshStatusInfo ();
  }

  ms = (D_MemoryStatus *) g_malloc (sizeof (D_MemoryStatus));
  ms->memoryStatus.MemoryType = GMT_ME;
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_GetMemoryStatus;
  e->data = ms;
  GUI_InsertEvent (e);
  pthread_mutex_lock (&memoryMutex);
  pthread_cond_wait (&memoryCond, &memoryMutex);
  pthread_mutex_unlock (&memoryMutex);

  if (ms->status != GE_NONE)
    /* Phone don't support ME (5110) */
    memoryStatus.MaxME = memoryStatus.UsedME = memoryStatus.FreeME = 0;
  else
  {
    memoryStatus.MaxME = ms->memoryStatus.Used + ms->memoryStatus.Free;
    memoryStatus.UsedME = 0;
    memoryStatus.FreeME = memoryStatus.MaxME;
  }

  ms->memoryStatus.MemoryType = GMT_SM;
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_GetMemoryStatus;
  e->data = ms;
  GUI_InsertEvent (e);
  pthread_mutex_lock (&memoryMutex);
  pthread_cond_wait (&memoryCond, &memoryMutex);
  pthread_mutex_unlock (&memoryMutex);

  if (ms->status != GE_NONE)
  {
    gtk_label_set_text (GTK_LABEL (errorDialog.text), _("Can't get SM memory status!\n\n\
Setting max SIM entries set to 100!\n"));
    memoryStatus.MaxSM = memoryStatus.FreeSM = 100;
    memoryStatus.UsedSM = 0;
    gtk_widget_show (errorDialog.dialog);
  }
  else
  {
    memoryStatus.MaxSM = ms->memoryStatus.Used + ms->memoryStatus.Free;
    memoryStatus.UsedSM = 0;
    memoryStatus.FreeSM = memoryStatus.MaxSM;
  }
  g_free (ms);

  statusInfo.ch_ME = statusInfo.ch_SM = 0;

  RefreshStatusInfo ();

  contactsMemory = g_ptr_array_new ();

  for (i = 1; i <= memoryStatus.MaxME; i++)
  {
    if ((pbEntry = (PhonebookEntry *) g_malloc (sizeof (PhonebookEntry))) == NULL)
    {
      fclose (f);
      g_print (_("%s: line %d: Can't allocate memory!\n"), __FILE__, __LINE__);
      g_ptr_array_free (contactsMemory, TRUE);
      return;
    }

    pbEntry->entry.Empty = FALSE;
    pbEntry->entry.Name[0] = '\0';
    pbEntry->entry.Number[0] = '\0';
    pbEntry->entry.MemoryType = GMT_ME;
    pbEntry->entry.Group = 5;
    pbEntry->status = E_Deleted; /* I must set status to E_Deleted, since I   */
                                 /* I want clear empty entries when saving to */
                                 /* to phone                                  */
    pbEntry->entry.SubEntriesCount=0;

    g_ptr_array_add (contactsMemory, (gpointer) pbEntry);
    pbEntry = NULL;
  }

  for (i = 1; i <= memoryStatus.MaxSM; i++)
  {
    if ((pbEntry = (PhonebookEntry *) g_malloc (sizeof (PhonebookEntry))) == NULL)
    {
      fclose (f);
      g_print (_("%s: line %d: Can't allocate memory!\n"), __FILE__, __LINE__);
      g_ptr_array_free (contactsMemory, TRUE);
      return;
    }

    pbEntry->entry.Empty = FALSE;
    pbEntry->entry.Name[0] = '\0';
    pbEntry->entry.Number[0] = '\0';
    pbEntry->entry.MemoryType = GMT_SM;
    pbEntry->entry.Group = 5;
    pbEntry->status = E_Deleted;
    pbEntry->entry.SubEntriesCount=0;

    g_ptr_array_add (contactsMemory, (gpointer) pbEntry);
    pbEntry = NULL;
  }


  while (fgets (buf, IO_BUF_LEN, f))
  {
    if (ParseLine (&gsmEntry, &i, buf))
    {
      if (gsmEntry.MemoryType == GMT_ME && memoryStatus.FreeME > 0 
          && i > 0 && i <= memoryStatus.MaxME)
      {
        pbEntry = g_ptr_array_index (contactsMemory, i - 1);

        if (pbEntry->status == E_Deleted)
        {
          pbEntry->entry = gsmEntry;
          pbEntry->status = E_Changed;
          memoryStatus.UsedME++;
          memoryStatus.FreeME--;
          statusInfo.ch_ME = 1;
        }
      }
      else if (gsmEntry.MemoryType == GMT_SM && memoryStatus.FreeSM > 0
               && i > 0 && i <= memoryStatus.MaxSM)
      {
        pbEntry = g_ptr_array_index (contactsMemory, memoryStatus.MaxME + i - 1);

        if (pbEntry->status == E_Deleted)
        {
          pbEntry->entry = gsmEntry;
          pbEntry->status = E_Changed;
          memoryStatus.UsedSM++;
          memoryStatus.FreeSM--;
          statusInfo.ch_SM = 1;
        }
      }
    }
  }

  contactsMemoryInitialized = TRUE;
  RefreshStatusInfo ();
  GUIEventSend (GUI_EVENT_CONTACTS_CHANGED);
  GUIEventSend (GUI_EVENT_SMS_NUMBER_CHANGED);
}


static void ImportContactsFileDialog ()
{
  static GtkWidget *fileDialog = NULL;

  if (fileDialog == NULL)
  {
    fileDialog = gtk_file_selection_new (_("Import"));
    gtk_signal_connect (GTK_OBJECT (fileDialog), "delete_event",
                        GTK_SIGNAL_FUNC (DeleteEvent), NULL);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (OkImportDialog), (gpointer) fileDialog);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileDialog)->cancel_button),
                        "clicked", GTK_SIGNAL_FUNC (CancelDialog), (gpointer) fileDialog);
  }

  gtk_widget_show (fileDialog);
}


static void ImportSaveCallback (GtkWidget *widget, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  SaveContacts ();
  ImportContactsFileDialog ();
}


static void ImportDontSaveCallback (GtkWidget *widget, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  ImportContactsFileDialog ();
}


void static ImportSaveContacts (void)
{
  static GtkWidget *dialog = NULL;

  if (dialog == NULL)
    dialog = CreateSaveQuestionDialog (ImportSaveCallback, ImportDontSaveCallback);

  gtk_widget_show (dialog);
}


static void ImportContacts (void)
{
  if (contactsMemoryInitialized == TRUE && (statusInfo.ch_ME || statusInfo.ch_SM))
    ImportSaveContacts ();
  else
    ImportContactsFileDialog ();
}


static void QuitSaveCallback (GtkWidget *widget, gpointer data)
{
  PhoneEvent *e;

  gtk_widget_hide (GTK_WIDGET (data));
  SaveContacts ();
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_Exit;
  e->data = NULL;
  GUI_InsertEvent (e);
  pthread_join (monitor_th, NULL); 
  gtk_main_quit ();
}


static void QuitDontSaveCallback (GtkWidget *widget, gpointer data)
{
  PhoneEvent *e;

  gtk_widget_hide (GTK_WIDGET (data));
  e = (PhoneEvent *) g_malloc (sizeof (PhoneEvent));
  e->event = Event_Exit;
  e->data = NULL;
  GUI_InsertEvent (e);
  pthread_join (monitor_th, NULL); 
  gtk_main_quit ();
}


void GUI_QuitSaveContacts (void)
{
  static GtkWidget *dialog = NULL;

  if (dialog == NULL)
    dialog = CreateSaveQuestionDialog (QuitSaveCallback, QuitDontSaveCallback);

  gtk_widget_show (dialog);
}


/* Function take number and return name belonged to number.
   If no name is found, return NULL;
   Do not modify returned name!
*/
gchar *GUI_GetName (gchar *number)
{
  PhonebookEntry *pbEntry;
  register gint i;

  if (contactsMemoryInitialized == FALSE || number == NULL)
    return (gchar *)NULL;

  for(i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    pbEntry = g_ptr_array_index (contactsMemory, i);

    if (pbEntry->status == E_Empty || pbEntry->status == E_Deleted)
      continue;

    if (strcmp (pbEntry->entry.Number, number) == 0)
      return pbEntry->entry.Name;
  }

  for (i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    pbEntry = g_ptr_array_index (contactsMemory, i);

    if (pbEntry->status == E_Empty || pbEntry->status == E_Deleted)
      continue;

    if (strrncmp (pbEntry->entry.Number, number, 9) == 0)
      return pbEntry->entry.Name;
  }

  return NULL;
}


gchar *GUI_GetNameExact (gchar *number)
{
  PhonebookEntry *pbEntry;
  register gint i;

  if (contactsMemoryInitialized == FALSE || number == NULL)
    return (gchar *)NULL;

  for(i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    pbEntry = g_ptr_array_index (contactsMemory, i);

    if (pbEntry->status == E_Empty || pbEntry->status == E_Deleted)
      continue;

    if (strcmp (pbEntry->entry.Number, number) == 0)
      return pbEntry->entry.Name;
  }

  return NULL;
}


gchar *GUI_GetNumber (gchar *name)
{
  PhonebookEntry *pbEntry;
  register gint i;

  if (contactsMemoryInitialized == FALSE || name == NULL || *name == '\0')
    return (gchar *)NULL;

  for(i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    pbEntry = g_ptr_array_index (contactsMemory, i);

    if (pbEntry->status == E_Empty || pbEntry->status == E_Deleted)
      continue;

    if (strcmp (pbEntry->entry.Name, name) == 0)
      return pbEntry->entry.Number;
  }

  return NULL;
}


static void SelectDialogClickEntry (GtkWidget        *clist,
                                    gint              row,
                                    gint              column,
                                    GdkEventButton   *event,
                                    SelectContactData *data )
{
  if(event && event->type == GDK_2BUTTON_PRESS)
    gtk_signal_emit_by_name(GTK_OBJECT (data->okButton), "clicked");
}


SelectContactData *GUI_SelectContactDialog (void)
{
  PhonebookEntry *pbEntry;
  static SelectContactData selectContactData;
  SortColumn *sColumn;
  gchar *titles[4] = { "Name", "Number", "Memory", "Group"};
  gint row_i = 0;
  register gint i;
  gchar string[100];

  if (contactsMemoryInitialized == FALSE)
    return NULL;

  selectContactData.dialog = gtk_dialog_new ();
  gtk_widget_set_usize (GTK_WIDGET (selectContactData.dialog), 436, 200);
  gtk_window_set_title (GTK_WINDOW (selectContactData.dialog), _("Select contacts"));
  gtk_window_set_modal (GTK_WINDOW (selectContactData.dialog), TRUE);

  selectContactData.okButton = gtk_button_new_with_label (_("Ok"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (selectContactData.dialog)->action_area),
                      selectContactData.okButton, TRUE, TRUE, 10);
  GTK_WIDGET_SET_FLAGS (selectContactData.okButton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (selectContactData.okButton);
  gtk_widget_show (selectContactData.okButton);

  selectContactData.cancelButton = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (selectContactData.dialog)->action_area),
                      selectContactData.cancelButton, TRUE, TRUE, 10);
  gtk_widget_show (selectContactData.cancelButton);

  selectContactData.clist = gtk_clist_new_with_titles (4, titles);
  gtk_clist_set_shadow_type (GTK_CLIST (selectContactData.clist), GTK_SHADOW_OUT);
  gtk_clist_set_compare_func (GTK_CLIST (selectContactData.clist), CListCompareFunc);
  gtk_clist_set_sort_column (GTK_CLIST (selectContactData.clist), 0);
  gtk_clist_set_sort_type (GTK_CLIST (selectContactData.clist), GTK_SORT_ASCENDING);
  gtk_clist_set_auto_sort (GTK_CLIST (selectContactData.clist), FALSE);
  gtk_clist_set_selection_mode (GTK_CLIST (selectContactData.clist), GTK_SELECTION_EXTENDED);

  gtk_clist_set_column_width (GTK_CLIST(selectContactData.clist), 0, 150);
  gtk_clist_set_column_width (GTK_CLIST(selectContactData.clist), 1, 115);
  gtk_clist_set_column_width (GTK_CLIST(selectContactData.clist), 3, 70);
  gtk_clist_set_column_justification (GTK_CLIST(selectContactData.clist), 2, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_visibility (GTK_CLIST(selectContactData.clist), 3, (GetModelFeature(FN_CALLERGROUPS)!=0));

  for (i = 0; i < 4; i++)
  {
    if ((sColumn = g_malloc (sizeof (SortColumn))) == NULL)
    {
      g_print (_("Error: %s: line %d: Can't allocate memory!\n"), __FILE__, __LINE__);
      return NULL;
    }
    sColumn->clist = selectContactData.clist;
    sColumn->column = i;
    gtk_signal_connect (GTK_OBJECT (GTK_CLIST (selectContactData.clist)->column[i].button), "clicked",
                        GTK_SIGNAL_FUNC (SetSortColumn), (gpointer) sColumn);
  }

  gtk_signal_connect (GTK_OBJECT (selectContactData.clist), "select_row",
                      GTK_SIGNAL_FUNC (SelectDialogClickEntry),
                      (gpointer) &selectContactData);

  selectContactData.clistScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (selectContactData.clistScrolledWindow),
                     selectContactData.clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (selectContactData.clistScrolledWindow),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (selectContactData.dialog)->vbox),
                      selectContactData.clistScrolledWindow, 
                      TRUE, TRUE, 0);

  gtk_widget_show (selectContactData.clist);
  gtk_widget_show (selectContactData.clistScrolledWindow);

  if (xgnokiiConfig.callerGroups[0] == NULL)
    GUI_InitCallerGroupsInf ();

  gtk_clist_freeze (GTK_CLIST (selectContactData.clist));
  for (i = 0; i < memoryStatus.MaxME + memoryStatus.MaxSM; i++)
  {
    gchar *row[4];

    pbEntry = g_ptr_array_index (contactsMemory, i);
    if (pbEntry->status != E_Empty && pbEntry->status != E_Deleted)
    {
      row[0] = pbEntry->entry.Name;
 
      if (pbEntry->entry.SubEntriesCount>0) {
	snprintf(string,100,"%s *",pbEntry->entry.Number);
	row[1]=string;
      } else row[1] = pbEntry->entry.Number;


      if (pbEntry->entry.MemoryType == GMT_ME)
        row[2] = "P";
      else
        row[2] = "S";
      if (GetModelFeature(FN_CALLERGROUPS)!=0)
        row[3] = xgnokiiConfig.callerGroups[pbEntry->entry.Group];
      else
        row[3] = "";
      gtk_clist_append (GTK_CLIST (selectContactData.clist), row);
      if (pbEntry->entry.MemoryType == GMT_ME)
        gtk_clist_set_pixmap (GTK_CLIST (selectContactData.clist), row_i, 2,
                              memoryPixmaps.phoneMemPix, memoryPixmaps.mask);
      else
        gtk_clist_set_pixmap (GTK_CLIST (selectContactData.clist), row_i, 2,
                              memoryPixmaps.simMemPix, memoryPixmaps.mask);

     gtk_clist_set_row_data (GTK_CLIST (selectContactData.clist), row_i++, (gpointer) pbEntry);
    }
  }

  gtk_clist_sort (GTK_CLIST (selectContactData.clist));
  gtk_clist_thaw (GTK_CLIST (selectContactData.clist));

  gtk_widget_show (selectContactData.dialog);

  return &selectContactData;
}


static GtkItemFactoryEntry menu_items[] = {
  { NULL,		NULL,		NULL, 0, "<Branch>"},
  { NULL,		"<control>R",	ReadContacts, 0, NULL},
  { NULL,		"<control>S",	SaveContacts, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Separator>"},
  { NULL,		"<control>I",	ImportContacts, 0, NULL},
  { NULL,		"<control>E",	ExportContacts, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Separator>"},
  { NULL,		"<control>W",	CloseContacts, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Branch>"},
  { NULL,		"<control>N",	NewEntry, 0, NULL},
  { NULL,		"<control>U",	DuplicateEntry, 0, NULL},
  { NULL,		NULL,		EditEntry, 0, NULL},
  { NULL,		"<control>D",	DeleteEntry, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Separator>"},
  { NULL,		"<control>C",	ChMemType, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Separator>"},
  { NULL,		"<control>F",	FindFirstEntry, 0, NULL},
  { NULL,		"<control>L",	SearchEntry, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Separator>"},
  { NULL,		"<control>A",	SelectAll, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<Branch>"},
  { NULL,		"<control>V",	DialVoice, 0, NULL},
  { NULL,		NULL,		NULL, 0, "<LastBranch>"},
  { NULL,		NULL,		Help1, 0, NULL},
  { NULL,		NULL,		GUI_ShowAbout, 0, NULL},
};


static void InitMainMenu (void)
{
  menu_items[0].path = g_strdup (_("/_File"));
  menu_items[1].path = g_strdup (_("/File/_Read from phone"));
  menu_items[2].path = g_strdup (_("/File/_Save to phone"));
  menu_items[3].path = g_strdup (_("/File/Sep1"));
  menu_items[4].path = g_strdup (_("/File/_Import from file"));
  menu_items[5].path = g_strdup (_("/File/_Export to file"));
  menu_items[6].path = g_strdup (_("/File/Sep2"));
  menu_items[7].path = g_strdup (_("/File/_Close"));
  menu_items[8].path = g_strdup (_("/_Edit"));
  menu_items[9].path = g_strdup (_("/Edit/_New"));
  menu_items[10].path = g_strdup (_("/Edit/D_uplicate"));
  menu_items[11].path = g_strdup (_("/Edit/_Edit"));
  menu_items[12].path = g_strdup (_("/Edit/_Delete"));
  menu_items[13].path = g_strdup (_("/Edit/Sep3"));
  menu_items[14].path = g_strdup (_("/Edit/_Change memory type"));
  menu_items[15].path = g_strdup (_("/Edit/Sep4"));
  menu_items[16].path = g_strdup (_("/Edit/_Find"));
  menu_items[17].path = g_strdup (_("/Edit/Find ne_xt"));
  menu_items[18].path = g_strdup (_("/Edit/Sep5"));
  menu_items[19].path = g_strdup (_("/Edit/Select _all"));
  menu_items[20].path = g_strdup (_("/_Dial"));
  menu_items[21].path = g_strdup (_("/Dial/Dial _voice"));
  menu_items[22].path = g_strdup (_("/_Help"));
  menu_items[23].path = g_strdup (_("/Help/_Help"));
  menu_items[24].path = g_strdup (_("/Help/_About"));
}


void GUI_CreateContactsWindow (void)
{
  int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  SortColumn *sColumn;
  GtkWidget *menubar;
  GtkWidget *main_vbox;
  GtkWidget *toolbar;
  GtkWidget *clistScrolledWindow;
  GtkWidget *status_hbox;
  register gint i;
  gchar *titles[4] = { _("Name"), _("Number"), _("Memory"), _("Group")};

  InitMainMenu ();
  contactsMemoryInitialized = FALSE;
  GUI_ContactsWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (GUI_ContactsWindow), _("Contacts"));
  gtk_widget_set_usize (GTK_WIDGET (GUI_ContactsWindow), 436, 220);
  //gtk_container_set_border_width (GTK_CONTAINER (GUI_ContactsWindow), 10);
  gtk_signal_connect (GTK_OBJECT (GUI_ContactsWindow), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);
  gtk_widget_realize (GUI_ContactsWindow);

  accel_group = gtk_accel_group_new ();
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
                                       accel_group);

  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  gtk_accel_group_attach (accel_group, GTK_OBJECT (GUI_ContactsWindow));

  /* Finally, return the actual menu bar created by the item factory. */ 
  menubar = gtk_item_factory_get_widget (item_factory, "<main>");

  main_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
  gtk_container_add (GTK_CONTAINER (GUI_ContactsWindow), main_vbox);
  gtk_widget_show (main_vbox);

  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, FALSE, 0);
  gtk_widget_show (menubar);

  /* Create the toolbar */

  toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar), GTK_RELIEF_NORMAL);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Read from phone"), NULL,
                           NewPixmap(Read_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) ReadContacts, NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Save to phone"), NULL,
                           NewPixmap(Send_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) SaveContacts, NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Import from file"), NULL,
                           NewPixmap(Open_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) ImportContacts, NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Export to file"), NULL,
                           NewPixmap(Save_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) ExportContacts, NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("New entry"), NULL,
                           NewPixmap(New_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) NewEntry, NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Duplicate entry"), NULL,
                           NewPixmap(Duplicate_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) DuplicateEntry, NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Edit entry"), NULL,
                           NewPixmap(Edit_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) EditEntry, NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Delete entry"), NULL,
                           NewPixmap(Delete_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) DeleteEntry, NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR(toolbar));

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), NULL, _("Dial voice"), NULL,
                           NewPixmap(Dial_xpm, GUI_ContactsWindow->window,
                           &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL]),
                           (GtkSignalFunc) DialVoice, NULL);

//  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);

  gtk_box_pack_start (GTK_BOX (main_vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);


  clist = gtk_clist_new_with_titles (4, titles);
  gtk_clist_set_shadow_type (GTK_CLIST (clist), GTK_SHADOW_OUT);
  gtk_clist_set_compare_func (GTK_CLIST (clist), CListCompareFunc);
  gtk_clist_set_sort_column (GTK_CLIST (clist), 0);
  gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_ASCENDING);
  gtk_clist_set_auto_sort (GTK_CLIST (clist), FALSE);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_EXTENDED);

  gtk_clist_set_column_width (GTK_CLIST (clist), 0, 150);
  gtk_clist_set_column_width (GTK_CLIST (clist), 1, 115);
  gtk_clist_set_column_width (GTK_CLIST (clist), 3, 70);
  gtk_clist_set_column_justification (GTK_CLIST (clist), 2, GTK_JUSTIFY_CENTER);
//  gtk_clist_set_column_visibility (GTK_CLIST (clist), 3, (GetModelFeature(FN_CALLERGROUPS)!=0));

  for (i = 0; i < 4; i++)
  {
    if ((sColumn = g_malloc (sizeof (SortColumn))) == NULL)
    {
      g_print (_("Error: %s: line %d: Can't allocate memory!\n"), __FILE__, __LINE__);
      gtk_main_quit ();
    }
    sColumn->clist = clist;
    sColumn->column = i;
    gtk_signal_connect (GTK_OBJECT (GTK_CLIST (clist)->column[i].button), "clicked",
                        GTK_SIGNAL_FUNC (SetSortColumn), (gpointer) sColumn);
  }

  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
                      GTK_SIGNAL_FUNC (ClickEntry), NULL);

  clistScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (clistScrolledWindow), clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (clistScrolledWindow),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (main_vbox), clistScrolledWindow, 
                      TRUE, TRUE, 0);

  gtk_widget_show (clist);
  gtk_widget_show (clistScrolledWindow);

  status_hbox = gtk_hbox_new (FALSE,20);
  gtk_box_pack_start (GTK_BOX (main_vbox), status_hbox, FALSE, FALSE, 0);
  gtk_widget_show (status_hbox);

  memoryStatus.MaxME = memoryStatus.UsedME = memoryStatus.FreeME =
  memoryStatus.MaxSM = memoryStatus.UsedSM = memoryStatus.FreeSM = 0;
  statusInfo.ch_ME = statusInfo.ch_SM = 0; 

  statusInfo.label = gtk_label_new ("");
  RefreshStatusInfo ();
  gtk_box_pack_start (GTK_BOX (status_hbox), statusInfo.label, FALSE, FALSE, 10);
  gtk_widget_show (statusInfo.label);

  memoryPixmaps.simMemPix = gdk_pixmap_create_from_xpm_d (GUI_ContactsWindow->window,
                                &memoryPixmaps.mask,
                                &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL],
                                sim_xpm);

  memoryPixmaps.phoneMemPix = gdk_pixmap_create_from_xpm_d (GUI_ContactsWindow->window,
                                  &memoryPixmaps.mask,
                                  &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL],
                                  phone_xpm);

  questMark.pixmap = gdk_pixmap_create_from_xpm_d (GUI_ContactsWindow->window,
                         &questMark.mask,
                         &GUI_ContactsWindow->style->bg[GTK_STATE_NORMAL],
                         quest_xpm);

  CreateErrorDialog (&errorDialog, GUI_ContactsWindow);
  GUIEventAdd (GUI_EVENT_CONTACTS_CHANGED, GUI_RefreshContacts);
  GUIEventAdd (GUI_EVENT_CALLERS_GROUPS_CHANGED, GUI_RefreshGroupMenu);
}

