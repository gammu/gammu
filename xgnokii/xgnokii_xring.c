#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <assert.h>

#include "misc.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "gsm-ringtones.h"

#include "xgnokii_xring.h"
#include "xgnokii_common.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii.h"

#include "xpm/Send.xpm"

static GtkWidget *GUI_RingtonesWindow;

GtkWidget * blacks[WHITE_COUNT - 1];
GtkWidget * whites[WHITE_COUNT];

int pressed;

PixmapAndMask pam[KIE_COUNT];

GSM_Ringtone ringtone;

char xwhi[] = { 6, 4, 2, 6, 4, 4, 2 };

/* Bitmaps for keyboard */
static
  char * blackparts[] = {
     ".+@@@@@@@@@@@@+.",
     "+.+@@@@@@@@@@+.+",
     "@+.+@@@@@@@@+.+@",
     "@@+.+@@@@@@+.+@@",
     "@@@+........+@@@",
     "@@@@........@@@@"
  };

static 
  char * whitestarts[][3] = {
    { "        ++++++++++++++++",
      "        +..............+",
      "+++++++++..............+" },
    {
      "        ++++++++        ",
      "        +......+        ",
      "+++++++++......+++++++++" },
    {
      "++++++++++++++++        ",
      "+..............+        ",
      "+..............+++++++++" }};


static
  char * whiteend[] = {
     "+......................+",
     "+@....................@+",
     "+@@..................@@+",
     "++@@@..............@@@++",
     "++++++++++++++++++++++++"
  };

static inline void CloseRingtonesWindow (void) {
  gtk_widget_hide(GUI_RingtonesWindow);
}
  
static GtkItemFactoryEntry ringtonesMenuItems[] = {
  { NULL,	NULL,		NULL,			 0, "<Branch>"},
  { NULL,       "<control>O",   NULL,        		 0, NULL},
  { NULL,       "<control>S",   NULL,   		 0, NULL},
  { NULL,	NULL,		NULL,    	         0, NULL},
  { NULL,       NULL,           NULL,                    0, "<Separator>"},
  { NULL,       "<control>T",   NULL,                    0, NULL},
  { NULL,       NULL,           NULL,                    0, "<Separator>"},
  { NULL,	"<control>C",	CloseRingtonesWindow,    0, NULL},
  { NULL,       NULL,           NULL,                    0, "<Branch>"},
  { NULL,	"<control>S",	NULL,       		 0, NULL},
  { NULL,	"<control>H",	NULL,            	 0, NULL},
};

static void InitRingtonesMenu (void) {
 ringtonesMenuItems[0].path = g_strdup(_("/_File"));
 ringtonesMenuItems[1].path = g_strdup(_("/File/_Open"));
 ringtonesMenuItems[2].path = g_strdup(_("/File/_Save"));
 ringtonesMenuItems[3].path = g_strdup(_("/File/Save _as ..."));
 ringtonesMenuItems[4].path = g_strdup(_("/File/Sep1"));
 ringtonesMenuItems[5].path = g_strdup(_("/File/Se_t ringtone"));
 ringtonesMenuItems[6].path = g_strdup(_("/File/Sep2"));
 ringtonesMenuItems[7].path = g_strdup(_("/File/_Close"));
 ringtonesMenuItems[8].path = g_strdup(_("/_Edit"));
 ringtonesMenuItems[9].path = g_strdup(_("/Edit/_Set ringtone name"));
 ringtonesMenuItems[10].path = g_strdup(_("/Edit/S_how in Composer"));
}

/* Add created pixmap to widget */
static int createpixmap(GtkWidget * widget, PixmapAndMask * pam, char ** a)
{
  GtkStyle * style = gtk_widget_get_default_style();
  pam->pixmap = gdk_pixmap_create_from_xpm_d(widget->window,
					     &pam->mask,
					     &style->bg[GTK_STATE_NORMAL],
					     a);
  return (int)pam->pixmap; /* (to) NULL or not (to) NULL */
}

static void writecolors(char * buf0, char * col0,
			char * buf1, char * col1,
			char * buf2, char * col2)
{
  sprintf(buf0, ".	c #%s", col0);
  sprintf(buf1, "+	c #%s", col1);
  sprintf(buf2, "@	c #%s", col2);
}

/* Creates white pixmap in the keyboard */
int KeyboardCreateWhitePixmap(GtkWidget * window, PixmapAndMask * pam,
                              char * start[], char * color0, char * color1, char * color2)
{
  int i = 0, j = 0;
  char * a[160 + 5];
  char col0[20];
  char col1[20];
  char col2[20];

  a[i++] = "24 160 4 1";
  a[i++] = " 	c None";
  writecolors(col0, color0, col1, color1, col2, color2);
  a[i++] = col0;
  a[i++] = col1;
  a[i++] = col2;
  
  a[i++] = start[0];
  for (j = 0; j < 99; j++)
    a[i++] = start[1];
  a[i++] = start[2];

  for (j = 0; j < 54; j++)
    a[i++] = whiteend[0];

  a[i++] = whiteend[1];
  a[i++] = whiteend[1];
  a[i++] = whiteend[2];
  a[i++] = whiteend[3];
  a[i++] = whiteend[4];

  assert(i == 165);

  return createpixmap(window, pam, a);
  
}

/* Creates black pixmap in the keyboard */
int KeyboardCreateBlackPixmap(GtkWidget * window, PixmapAndMask * pam,
		              char * color0, char * color1, char * color2)
{
  int i = 0, j = 0;
  char * a[160 + 4];
  char col0[20];
  char col1[20];
  char col2[20];

  a[i++] = "16 100 3 1";
  writecolors(col0, color0, col1, color1, col2, color2);
  a[i++] = col0;
  a[i++] = col1;
  a[i++] = col2;

  for (j = 0; j < 5; j++)    a[i++] = blackparts[j];
  for (j = 0; j < 90; j++)   a[i++] = blackparts[5];
  for (j = 4; j >= 0; j--)   a[i++] = blackparts[j];

  assert(i == 104);

  return createpixmap(window, pam, a);
}

static void KeyboardSetPixmap(int flag)
{
  int i = pressed & ~(BLACK_PRESSED|WHITE_PRESSED);
  int j;

  if (pressed & BLACK_PRESSED)
  {
    j = flag? KIE_BLACKSEL: KIE_BLACK;
    gtk_pixmap_set(GTK_PIXMAP(blacks[i]),pam[j].pixmap, pam[j].mask);
  }
  else
  {
    j = xwhi[i % 7] + (flag? 1: 0);
    gtk_pixmap_set(GTK_PIXMAP(whites[i]),pam[j].pixmap, pam[j].mask);
  }
}

static void KeyboardSendTune()
{
  /* frequencies generated with:
     perl -e 'print int(4400 * (2 **($_/12)) + .5)/10, "\n" for(3..14)'
  */
  struct {
  float wf;
  float bf;
  } notefreq[7] = {
    { 523.3, 554.4 },  //C
    { 587.3, 622.3 },  //D
    { 659.3, 0 },      //E
    { 698.5, 740   },  //F
    { 784  , 830.6 },  //G
    { 880  , 932.3 },  //A
    { 987.8, 0 }       //H
  };

  int i=pressed;
  
  int j, r;
  float freq;

  if (i) {
    j = (i & BLACK_PRESSED)? 1: 0;

    i &= ~(BLACK_PRESSED|WHITE_PRESSED);

    freq = j? notefreq[i % 7].bf: notefreq[i % 7].wf;

    switch (i / 7)
    {
      case  0: r = (int)(freq / 2.0); break;
      case  2: r = (int)(freq * 2.0); break;
      default: r = (int)(freq); break;
    }

    GSM->PlayTone(r,100);

  } else {

    GSM->PlayTone(0,0);

  }
}

/* One drawn key was released */
static gint KeyboardButtonRelease(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  KeyboardSetPixmap(FALSE);
  
  pressed = 0;

  KeyboardSendTune();

  return TRUE;
}

/* One drawn key was pressed */
static gint KeyboardButtonPress(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  GdkEventButton * e = (GdkEventButton *)event;
  int i;

  if (!pressed)
  {
    guint x = e->x;
    guint y = e->y;
    
    if (y < 100) /* possible black pressed... */
      for ( i = 0; i < WHITE_COUNT - 1; i++)
	if (blacks[i] && x - 16 - 24 * i < 16)
	{
	  pressed = i | BLACK_PRESSED;
	  KeyboardSetPixmap(TRUE);
	  KeyboardSendTune();
	  return TRUE;
	}
    for ( i = 0; i < WHITE_COUNT; i++) /* whites ? */
      if (x - 24 * i < 24)
      {
	pressed = i | WHITE_PRESSED;
	KeyboardSetPixmap(TRUE);
	KeyboardSendTune();
	return TRUE;
      }
  }
  return TRUE;
}

void GUI_CreateRingtonesWindow (void)
{
  int nMenuItems = sizeof (ringtonesMenuItems) / sizeof (ringtonesMenuItems[0]);
  GtkAccelGroup *accelGroup;
  GtkItemFactory *itemFactory;
  GtkWidget *menuBar;
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox2;
  GtkWidget *melody;
  GtkWidget *buttons, *buttons2;
  GtkWidget *separator;
  GtkWidget *toolBar;
  GtkWidget *label, *note1, *note2, *note3, *note4, *note5;
  GtkWidget *label2, *button, *button2, *button3, *button4, *button5,
            *button6, *button7, *button8, *button9, *button10, *button11, *button12;
  
  GtkWidget *keyboard;  

  int i;
  
  InitRingtonesMenu();

  /* realize top level window for logos */
  GUI_RingtonesWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(GUI_RingtonesWindow),1,1,1);
  gtk_window_set_title(GTK_WINDOW(GUI_RingtonesWindow),_("Ringtones"));
  gtk_signal_connect(GTK_OBJECT(GUI_RingtonesWindow),"delete_event",
                     GTK_SIGNAL_FUNC(DeleteEvent),NULL);
  gtk_widget_set_usize(GUI_RingtonesWindow,630,320);
  gtk_widget_realize(GUI_RingtonesWindow);

  accelGroup = gtk_accel_group_new();
  gtk_accel_group_attach(accelGroup,GTK_OBJECT(GUI_RingtonesWindow));

  /* create main vbox */
  vbox = gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(GUI_RingtonesWindow),vbox);
  gtk_widget_show(vbox);

  itemFactory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,"<main>",accelGroup);
  gtk_item_factory_create_items(itemFactory,nMenuItems,ringtonesMenuItems,NULL);
  menuBar = gtk_item_factory_get_widget(itemFactory,"<main>");

  gtk_box_pack_start(GTK_BOX(vbox),menuBar,FALSE,FALSE,0);
  gtk_widget_show(menuBar);

  /* toolbar */
  toolBar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_button_relief(GTK_TOOLBAR(toolBar),GTK_RELIEF_NORMAL);
  gtk_toolbar_set_style(GTK_TOOLBAR(toolBar),GTK_TOOLBAR_ICONS);

  gtk_toolbar_append_item(GTK_TOOLBAR(toolBar),NULL,"Set ringtone",NULL,
                 NewPixmap(Send_xpm,GUI_RingtonesWindow->window,
                 &GUI_RingtonesWindow->style->bg[GTK_STATE_NORMAL]),
                 (GtkSignalFunc)NULL,toolBar);

  gtk_box_pack_start(GTK_BOX(vbox),toolBar,FALSE,FALSE,0);
  gtk_widget_show(toolBar);  

  /* vertical separator */
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(separator),FALSE,FALSE,0);

  /* Melody drawing area */  
  melody = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(melody), 20,100);
    
  gtk_box_pack_start(GTK_BOX(vbox),melody,FALSE,FALSE,0);
  gtk_widget_show(melody);
  
  /* create horizontal box for keyboard and buttons area */
  hbox = gtk_hbox_new(FALSE,5);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  /* Box for buttons */
  hbox2 = gtk_hbox_new(FALSE,5);
  gtk_box_pack_start(GTK_BOX(hbox),hbox2,FALSE,FALSE,0);
  gtk_widget_set_usize( hbox2, 120, 160 );
  gtk_widget_show(hbox2);

  /* Buttons with notes length */
  buttons = gtk_vbox_new(FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox2),buttons,FALSE,FALSE,0);

  label = gtk_label_new (_("Length"));
  gtk_box_pack_start (GTK_BOX(buttons), label, FALSE, FALSE, 3);

  note1 = gtk_radio_button_new_with_label (NULL, _("1"));
  gtk_box_pack_end (GTK_BOX (buttons), note1, TRUE, FALSE, 2);
  gtk_widget_show (note1);

  note2 = gtk_radio_button_new_with_label (NULL, _("1/2"));
  gtk_box_pack_end (GTK_BOX (buttons), note2, TRUE, FALSE, 2);
  gtk_widget_show (note2);

  note3 = gtk_radio_button_new_with_label (NULL, _("1/4"));
  gtk_box_pack_end (GTK_BOX (buttons), note3, TRUE, FALSE, 2);
  gtk_widget_show (note3);  
  gtk_widget_show(buttons);

  note4 = gtk_radio_button_new_with_label (NULL, _("1/16"));
  gtk_box_pack_end (GTK_BOX (buttons), note4, TRUE, FALSE, 2);
  gtk_widget_show (note4);

  note5 = gtk_radio_button_new_with_label (NULL, _("1/32"));
  gtk_box_pack_end (GTK_BOX (buttons), note5, TRUE, FALSE, 2);
  gtk_widget_show (note5);
  
  gtk_widget_show (buttons);

  /* Buttons with notes length */
  buttons2 = gtk_vbox_new(FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox2),buttons2,FALSE,FALSE,0);    

  label2 = gtk_label_new (_("Note"));
  gtk_box_pack_start (GTK_BOX(buttons2), label2, FALSE, FALSE, 3);

  button = gtk_button_new_with_label (_("C"));
  gtk_box_pack_start (GTK_BOX (buttons2), button, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button3 = gtk_button_new_with_label (_("D"));
  gtk_box_pack_start (GTK_BOX (buttons2), button3, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button3);
  gtk_widget_show (button);

  button5 = gtk_button_new_with_label (_("E"));
  gtk_box_pack_start (GTK_BOX (buttons2), button5, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button5);
  
  button6 = gtk_button_new_with_label (_("F"));
  gtk_box_pack_start (GTK_BOX (buttons2), button6, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button6);

  button8 = gtk_button_new_with_label (_("G"));
  gtk_box_pack_start (GTK_BOX (buttons2), button8, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button8);
  
  button10 = gtk_button_new_with_label (_("A"));
  gtk_box_pack_start (GTK_BOX (buttons2), button10, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button10);
  
  button12 = gtk_button_new_with_label (_("H"));
  gtk_box_pack_start (GTK_BOX (buttons2), button12, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button12);
  
  gtk_widget_show (label2);
      
  button2 = gtk_button_new_with_label (_("Cis"));
  gtk_box_pack_start (GTK_BOX (buttons2), button2, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button2);  
  
  button4 = gtk_button_new_with_label (_("Dis"));
  gtk_box_pack_start (GTK_BOX (buttons2), button4, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button4);

  button7 = gtk_button_new_with_label (_("Fis"));
  gtk_box_pack_start (GTK_BOX (buttons2), button7, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button7);  

  button9 = gtk_button_new_with_label (_("Gis"));
  gtk_box_pack_start (GTK_BOX (buttons2), button9, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button9);

  button11 = gtk_button_new_with_label (_("Ais"));
  gtk_box_pack_start (GTK_BOX (buttons2), button11, TRUE, FALSE, 0);
//  gtk_signal_connect (GTK_OBJECT (button), "clicked",
//                      GTK_SIGNAL_FUNC (HideCallDialog), (gpointer) inCallDialog.dialog);
//  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (button);
  gtk_widget_show (button11);
  
  gtk_widget_show (buttons2);
  
  /* Keyboard */
  keyboard = gtk_fixed_new();
  gtk_widget_set_usize( keyboard, 504, 160 );
  gtk_box_pack_start(GTK_BOX(hbox),keyboard,FALSE,FALSE,0);

  gtk_signal_connect(GTK_OBJECT (keyboard), "button_press_event",
		      GTK_SIGNAL_FUNC (KeyboardButtonPress), &hbox);
  gtk_signal_connect(GTK_OBJECT (keyboard), "button_release_event",
		      GTK_SIGNAL_FUNC (KeyboardButtonRelease), &hbox);

  gtk_widget_add_events(keyboard, GDK_FOCUS_CHANGE_MASK|
			GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
  
  KeyboardCreateBlackPixmap(hbox, &pam[KIE_BLACK], "333333", "666666", "999999");
  KeyboardCreateBlackPixmap(hbox, &pam[KIE_BLACKSEL], "000000", "333333", "666666");

  for (i = 0; i < 6; i += 2)
  {
    KeyboardCreateWhitePixmap(hbox, &pam[KIE_WHITEL + i],
		              whitestarts[i / 2], "FFFFFF", "999999", "CCCCCC");
    KeyboardCreateWhitePixmap(hbox, &pam[KIE_WHITELSEL + i],
                              whitestarts[i / 2], "CCCCCC", "666666", "999999");
  }
  
  for (i = 0; i < WHITE_COUNT - 1; i++)
  {
    PixmapAndMask * b = &pam[KIE_BLACK];
    if (xwhi[i % 7] == 2) continue;
    blacks[i] = gtk_pixmap_new( b->pixmap, b->mask );
    gtk_fixed_put( GTK_FIXED(keyboard), blacks[i], 16 + 24 * i, 0 );
  }

  for (i = 0; i < WHITE_COUNT; i++)
  {
    int j = xwhi[i % 7];
    whites[i] = gtk_pixmap_new( pam[j].pixmap, pam[j].mask );
    gtk_fixed_put( GTK_FIXED(keyboard), whites[i], 24 * i, 0 );
  }

  gtk_widget_show(keyboard);
}

void GUI_ShowRingtonesWindow (void)
{
  gtk_widget_show_all(GUI_RingtonesWindow);
}
