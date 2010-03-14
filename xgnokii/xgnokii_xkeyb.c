/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <string.h>
#include <gtk/gtk.h>
#include "misc.h"
#include "xgnokii_common.h"
#include "xgnokii.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii_xkeyb.h"

typedef struct {
  int top_left_x, top_left_y;
  int bottom_right_x, bottom_right_y;
  int code;
} ButtonT;


static GtkWidget *GUI_XkeybWindow;
static GtkWidget *pixArea;
static GtkWidget *phonePixmap = NULL;
static ErrorDialog errorDialog = {NULL, NULL};
static ButtonT *button = NULL;

static ButtonT button_6110[30] = {
  { 103,  91, 114, 107, PHONEKEY_POWER          }, /* Power */
  {  28, 240,  54, 263, PHONEKEY_MENU           }, /* Menu */
  {  84, 240, 110, 263, PHONEKEY_NAMES          }, /* Names */
  {  58, 245,  82, 258, PHONEKEY_UP             }, /* Up */
  {  55, 263,  85, 276, PHONEKEY_DOWN           }, /* Down */
  {  22, 271,  50, 289, PHONEKEY_GREEN          }, /* Green */
  {  91, 271, 115, 289, PHONEKEY_RED            }, /* Red */
  {  18, 294,  44, 310, PHONEKEY_1              }, /* 1 */
  {  56, 294,  85, 310, PHONEKEY_2              }, /* 2 */
  {  98, 294, 121, 310, PHONEKEY_3              }, /* 3 */
  {  18, 317,  44, 333, PHONEKEY_4              }, /* 4 */
  {  56, 317,  85, 333, PHONEKEY_5              }, /* 5 */
  {  98, 317, 121, 333, PHONEKEY_6              }, /* 6 */
  {  18, 342,  44, 356, PHONEKEY_7              }, /* 7 */
  {  56, 342,  85, 356, PHONEKEY_8              }, /* 8 */
  {  98, 342, 121, 356, PHONEKEY_9              }, /* 9 */
  {  18, 365,  44, 380, PHONEKEY_ASTERISK       }, /* * */
  {  56, 365,  85, 380, PHONEKEY_0              }, /* 0 */
  {  98, 365, 121, 380, PHONEKEY_HASH           }, /* # */
  {   1, 138,  10, 150, PHONEKEY_INCREASEVOLUME }, /* Volume + */
  {   1, 165,  10, 176, PHONEKEY_DECREASEVOLUME }, /* Volume - */
  {   0,   0,   0,   0, 0x00 }
};

static ButtonT button_6150[30] = {
  {  99,  78, 114,  93, PHONEKEY_POWER          }, /* Power */
  {  20, 223,  49, 245, PHONEKEY_MENU           }, /* Menu */
  {  90, 223, 120, 245, PHONEKEY_NAMES          }, /* Names */
  {  59, 230,  83, 247, PHONEKEY_UP             }, /* Up */
  {  56, 249,  84, 265, PHONEKEY_DOWN           }, /* Down */
  {  14, 254,  51, 273, PHONEKEY_GREEN          }, /* Green */
  {  90, 255, 126, 273, PHONEKEY_RED            }, /* Red */
  {  18, 281,  53, 299, PHONEKEY_1              }, /* 1 */
  {  55, 280,  86, 299, PHONEKEY_2              }, /* 2 */
  {  90, 281, 122, 299, PHONEKEY_3              }, /* 3 */
  {  18, 303,  53, 323, PHONEKEY_4              }, /* 4 */
  {  55, 303,  87, 323, PHONEKEY_5              }, /* 5 */
  {  90, 303, 122, 323, PHONEKEY_6              }, /* 6 */
  {  18, 327,  53, 346, PHONEKEY_7              }, /* 7 */
  {  53, 327,  87, 346, PHONEKEY_8              }, /* 8 */
  {  90, 327, 122, 346, PHONEKEY_9              }, /* 9 */
  {  18, 349,  53, 370, PHONEKEY_ASTERISK       }, /* * */
  {  56, 349,  87, 370, PHONEKEY_0              }, /* 0 */
  {  98, 349, 122, 370, PHONEKEY_HASH           }, /* # */
  {   2, 131,  10, 147, PHONEKEY_INCREASEVOLUME }, /* Volume + */
  {   2, 155,  10, 173, PHONEKEY_DECREASEVOLUME }, /* Volume - */
  {   0,   0,   0,   0, 0x00 }
};

static ButtonT button_5110[30] = {
  { 100,  85, 114,  99, PHONEKEY_POWER    }, /* Power */
  {  50, 240,  85, 265, PHONEKEY_MENU     }, /* Menu */
  {  20, 240,  45, 260, PHONEKEY_NAMES    }, /* Names */
  { 100, 240, 117, 258, PHONEKEY_UP       }, /* Up */
  {  93, 267, 112, 287, PHONEKEY_DOWN     }, /* Down */
  {  14, 294,  44, 312, PHONEKEY_1        }, /* 1 */
  {  54, 294,  83, 312, PHONEKEY_2        }, /* 2 */
  {  94, 294, 122, 312, PHONEKEY_3        }, /* 3 */
  {  14, 320,  44, 338, PHONEKEY_4        }, /* 4 */
  {  54, 320,  83, 338, PHONEKEY_5        }, /* 5 */
  {  94, 320, 122, 338, PHONEKEY_6        }, /* 6 */
  {  14, 345,  44, 363, PHONEKEY_7        }, /* 7 */
  {  54, 345,  83, 363, PHONEKEY_8        }, /* 8 */
  {  94, 345, 122, 363, PHONEKEY_9        }, /* 9 */
  {  18, 374,  49, 389, PHONEKEY_ASTERISK }, /* * */
  {  53, 371,  82, 387, PHONEKEY_0        }, /* 0 */
  {  96, 374, 119, 389, PHONEKEY_HASH     }, /* # */
  {   0,   0,   0,   0, 0x00 }
};

static inline void Help1 (GtkWidget *w, gpointer data)
{
  gchar *indx = g_strdup_printf ("/%s/gnokii/xgnokii/xkeyb/index.htm", xgnokiiConfig.locale);
  Help (w, indx);
  g_free (indx);
}


static GtkWidget *GetPixmap (void)
{
  GtkWidget *wpixmap;
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  gchar *file;

  if (!strcmp (phoneMonitor.phone.model, "6110") ||
      !strcmp (phoneMonitor.phone.model, "6120"))
  {
    button = button_6110;
    file = g_strdup_printf ("%s%s", xgnokiiConfig.xgnokiidir, "/xpm/6110.xpm");
  }
  else if (!strcmp (phoneMonitor.phone.model, "6130") ||
           !strcmp (phoneMonitor.phone.model, "6150") ||
           !strcmp (phoneMonitor.phone.model, "616x") ||
           !strcmp (phoneMonitor.phone.model, "6185") ||
           !strcmp (phoneMonitor.phone.model, "6190"))
  {
    button = button_6150;
    file = g_strdup_printf ("%s%s", xgnokiiConfig.xgnokiidir, "/xpm/6150.xpm");
  }
  else if (!strcmp (phoneMonitor.phone.model, "5110") ||
           !strcmp (phoneMonitor.phone.model, "5130") ||
           !strcmp (phoneMonitor.phone.model, "5160") ||
           !strcmp (phoneMonitor.phone.model, "5190"))
  {
    button = button_5110;
    file = g_strdup_printf ("%s%s", xgnokiiConfig.xgnokiidir, "/xpm/5110.xpm");
  }
  else
    return NULL;

  pixmap = gdk_pixmap_create_from_xpm (pixArea->window, &mask,
                                       &pixArea->style->bg[GTK_STATE_NORMAL],
                                       file);
  g_free (file);

  if (pixmap == NULL)
    return NULL;
    
  wpixmap = gtk_pixmap_new (pixmap, mask);
  
  return wpixmap;
}


static inline void CloseXkeyb (GtkWidget *w, gpointer data)
{
  gtk_widget_hide (GUI_XkeybWindow);
}


void GUI_ShowXkeyb (void)
{
  if (phonePixmap == NULL)
  {
    phonePixmap = GetPixmap ();
    if (phonePixmap != NULL)
    {
      gtk_fixed_put (GTK_FIXED (pixArea), phonePixmap, 0 , 0);
      gtk_widget_show (phonePixmap);
    }
    else
    {
      gtk_label_set_text (GTK_LABEL (errorDialog.text),
                          _("Cannot load background pixmap!"));
      gtk_widget_show (errorDialog.dialog);
    }
  }
  gtk_widget_show (GUI_XkeybWindow);
}


static gint ButtonEvent (GtkWidget *widget, GdkEventButton *event)
{
  unsigned char req[] = {0x00,0x00};
  register gint i = 0;

  if (button == NULL)
    return TRUE;

  if (event->button != 1)
    return TRUE;

  if (event->type == GDK_BUTTON_PRESS) 
    req[0] = PRESSPHONEKEY;
  else if (event->type == GDK_BUTTON_RELEASE)
    req[0] = RELEASEPHONEKEY;
  else
    return TRUE;

//  g_print ("%f %f\n", event->x, event->y);

  while (button[i].top_left_x != 0) {
    if (button[i].top_left_x <= event->x &&
	event->x <= button[i].bottom_right_x &&
	button[i].top_left_y <= event->y &&
	event->y <= button[i].bottom_right_y)
    {
      PhoneEvent *e = g_malloc (sizeof (PhoneEvent));
      
      req[1]=button[i].code;
      e->event = Event_SendKeyStroke;
      e->data = g_memdup (req, sizeof (req));
      GUI_InsertEvent (e);
    }

    i++;
  }

  return TRUE;
}


static GtkItemFactoryEntry menu_items[] = {
  {NULL,	NULL,		NULL, 0, "<Branch>"},
  {NULL,	"<control>W",	CloseXkeyb, 0, NULL},
  {NULL,	NULL,		NULL, 0, "<LastBranch>"},
  {NULL,	NULL,		Help1, 0, NULL},
  {NULL,	NULL,		GUI_ShowAbout, 0, NULL},
};

static void InitMainMenu (void)
{
  menu_items[0].path = g_strdup (_("/_File"));
  menu_items[1].path = g_strdup (_("/File/_Close"));
  menu_items[2].path = g_strdup (_("/_Help"));
  menu_items[3].path = g_strdup (_("/Help/_Help"));
  menu_items[4].path = g_strdup (_("/Help/_About"));
}


void GUI_CreateXkeybWindow (void)
{
  int nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  GtkWidget *menubar;
  GtkWidget *main_vbox;

  InitMainMenu ();
  GUI_XkeybWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (GUI_XkeybWindow), _("XGnokii Keyboard"));
  //gtk_widget_set_usize (GTK_WIDGET (GUI_XkeybWindow), 436, 220);
  gtk_signal_connect (GTK_OBJECT (GUI_XkeybWindow), "delete_event",
                      GTK_SIGNAL_FUNC (DeleteEvent), NULL);
  gtk_widget_realize (GUI_XkeybWindow);

  accel_group = gtk_accel_group_new ();
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
                                       accel_group);

  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  gtk_accel_group_attach (accel_group, GTK_OBJECT (GUI_XkeybWindow));

  /* Finally, return the actual menu bar created by the item factory. */ 
  menubar = gtk_item_factory_get_widget (item_factory, "<main>");

  main_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
  gtk_container_add (GTK_CONTAINER (GUI_XkeybWindow), main_vbox);
  gtk_widget_show (main_vbox);

  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, FALSE, 0);
  gtk_widget_show (menubar);

  pixArea = gtk_fixed_new ();
  gtk_signal_connect (GTK_OBJECT (pixArea), "button_press_event",
		      (GtkSignalFunc) ButtonEvent, NULL);
  gtk_signal_connect (GTK_OBJECT (pixArea), "button_release_event",
		      (GtkSignalFunc) ButtonEvent, NULL);
  gtk_widget_set_events (pixArea, GDK_EXPOSURE_MASK
			 | GDK_LEAVE_NOTIFY_MASK
			 | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 | GDK_POINTER_MOTION_MASK
			 | GDK_POINTER_MOTION_HINT_MASK);

  gtk_box_pack_start (GTK_BOX (main_vbox), pixArea, FALSE, FALSE, 3);
  gtk_widget_show (pixArea);

  CreateErrorDialog (&errorDialog, GUI_XkeybWindow);
}
