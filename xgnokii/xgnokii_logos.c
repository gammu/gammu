#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "misc.h"
#include "gsm-common.h"
#include "gsm-api.h"
#include "gsm-networks.h"
#include "files/gsm-filetypes.h"
#include "gsm-bitmaps.h"

#include "xgnokii_logos.h"
#include "xgnokii_common.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii.h"

#include "xpm/Operator_logo.xpm"
#include "xpm/Startup_logo.xpm"
#include "xpm/Caller_logo.xpm"

#include "xpm/Black_point.xpm"
#include "xpm/Green_point.xpm"
#include "xpm/Green_pixel.xpm"

#include "xpm/New.xpm"
#include "xpm/Send.xpm"
#include "xpm/Read.xpm"

#include "xpm/Edit_invert.xpm"
#include "xpm/Edit_flip_horizontal.xpm"
#include "xpm/Edit_flip_vertical.xpm"

#include "xpm/Tool_brush.xpm"
#include "xpm/Tool_line.xpm"
#include "xpm/Tool_rectangle.xpm"
#include "xpm/Tool_filled_rectangle.xpm"

extern GSM_Network GSM_Networks[];

static GtkWidget *GUI_LogosWindow;

static ErrorDialog errorDialog = {NULL, NULL};
static InfoDialog infoDialog = {NULL, NULL};

/* stuff for drawingArea */
static GtkWidget *drawingArea = NULL;
static GdkPixmap *drawingPixmap = NULL;
static GdkPixmap *greenPointPixmap, *blackPointPixmap;
static int drawingAreaWidth, drawingAreaHeight; /* in pixels */
static int mouseButtonPushed = 0;

/* stuff for previewArea */
static GtkWidget *previewArea = NULL;
static GdkPixmap *previewPixmap = NULL;
static GdkPixmap *greenPixelPixmap;
static int previewPixmapWidth, previewPixmapHeight;
static int previewAvailable = 1, showPreviewErrorDialog = 1;
static int previewPixmapNumber = 0;

static GSM_Bitmap bitmap, oldBitmap;
static GSM_NetworkInfo networkInfo;

/* widgets for toolbar - some, need global variables */
static GtkWidget *buttonStartup, *buttonOperator, *buttonCaller, *buttonPicture;
static GtkWidget *networkCombo, *callerCombo;

static int activeTool = TOOL_BRUSH;
static int toolStartX, toolStartY, toolLastX, toolLastY;

/* tools for drawing */
static GtkWidget *buttonBrush, *buttonLine, *buttonRectangle;
static GtkWidget *buttonFilledRectangle;

/* Contains fileName for Export dialog. */
typedef struct {
  gchar *fileName;
} ExportDialogData;

static ExportDialogData exportDialogData = {NULL};

GtkWidget *FileSelection;

static int callersGroupsInitialized = 0;

/* returns lowest number from three numbers */
int GetMinFrom3(int a, int b, int c) {
  if (a > b) {
    if (b > c)
      return c;
    else
      return b;
  } else {
    if (a > c)
      return c;
    else
      return a;    
  }
}

/* returns highest number from three numbers */
int GetMaxFrom3(int a, int b, int c) {
  if (a > b) {
    if (c > a)
      return c;
    else
      return a;
  } else {
    if (c > b)
      return c;
    else
      return b;
  }
}

/* load preview pixmap from file */
GdkPixmap *GetPreviewPixmap (GtkWidget *widget) {
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  gchar *file;

  if (previewPixmapNumber == 0)
  {
    if (!strcmp (xgnokiiConfig.model, "6110") ||
        !strcmp (xgnokiiConfig.model, "6120"))
    {
      file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_6110.xpm");
      previewPixmapNumber = 1;
    }
    else if (!strcmp (xgnokiiConfig.model, "6130") ||
             !strcmp (xgnokiiConfig.model, "6150") ||
             !strcmp (xgnokiiConfig.model, "616x") ||
             !strcmp (xgnokiiConfig.model, "6185") ||
             !strcmp (xgnokiiConfig.model, "6190"))
    {
      file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_6150.xpm");
      previewPixmapNumber = 2;
    }
    else if (!strcmp (xgnokiiConfig.model, "3210"))
    {
      file = g_strdup_printf("%s%s", xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_3210.xpm");
      previewPixmapNumber = 3;
    }
    else if (!strcmp (xgnokiiConfig.model, "3310") ||
             !strcmp (xgnokiiConfig.model, "3330"))
    {
      file = g_strdup_printf("%s%s", xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_3310.xpm");
      previewPixmapNumber = 4;
    }
    else if (!strcmp (xgnokiiConfig.model, "5110") ||
             !strcmp (xgnokiiConfig.model, "5130"))
    {
      file = g_strdup_printf("%s%s", xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_5110.xpm");
      previewPixmapNumber = 5;
    }
    else if (!strcmp (xgnokiiConfig.model, "6250"))
    {
      file = g_strdup_printf("%s%s", xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_6250.xpm");
      previewPixmapNumber = 6;
    }
    else if (!strcmp (xgnokiiConfig.model, "7110"))
    {
      file = g_strdup_printf("%s%s", xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_7110.xpm");
      previewPixmapNumber = 7;
    }
    else
    {
      file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                             "/xpm/Preview_6210.xpm");
      previewPixmapNumber = 8;
    }
  }
  else
    switch (previewPixmapNumber)
    {
      case 1: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_6110.xpm"); 
              break;
      case 2: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_6150.xpm"); 
              break;
      case 3: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_3210.xpm"); 
              break;
      case 4: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_3310.xpm"); 
              break;
      case 5: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_5110.xpm"); 
              break;
      case 6: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_6250.xpm"); 
              break;
      case 7: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                     "/xpm/Preview_7110.xpm"); 
              break;
      default: file = g_strdup_printf("%s%s",xgnokiiConfig.xgnokiidir,
                                      "/xpm/Preview_6210.xpm");
              break;
    }

  pixmap = gdk_pixmap_create_from_xpm(widget->window,&mask,
                                      &widget->style->bg[GTK_STATE_NORMAL],
                                      file);
  g_free(file);

  return pixmap;
}

/* ********************************************************
 * ** SET/CLEAR POINTS ************************************
 * ********************************************************
 */
void SetPreviewPoint(GtkWidget *widget, int x, int y, int update) {
  if (!previewAvailable) return;
  
  /* there is difference between positiong of startupLogo and others */
  if (bitmap.type != GSM_StartupLogo && bitmap.type != GSM_7110StartupLogo &&
      bitmap.type != GSM_6210StartupLogo) {
    x += PREVIEWJUMPX;
    y += PREVIEWJUMPY;
  }

  /* draw point to pixmap */
  if (previewPixmap)
    gdk_draw_point(previewPixmap,widget->style->black_gc,
                   x+PREVIEWSTARTX,y+PREVIEWSTARTY);

  if (update) {
    GdkRectangle updateRect;

    /* update point on screen */
    updateRect.width = 1;
    updateRect.height = 1;
    updateRect.x = PREVIEWSTARTX+x;
    updateRect.y = PREVIEWSTARTY+y;

    gtk_widget_draw(previewArea,&updateRect);
  }  
}

void ClearPreviewPoint(GtkWidget *widget, int x, int y, int update) {
  if (!previewAvailable) return;
  
  /* there is difference between positiong of startupLogo and others */
  if (bitmap.type != GSM_StartupLogo && bitmap.type != GSM_7110StartupLogo &&
      bitmap.type != GSM_6210StartupLogo) {
    x += PREVIEWJUMPX;
    y += PREVIEWJUMPY;
  }

  /* clean point from pixmap - any idea how to draw green point without pixmap? */
  if (previewPixmap)
    gdk_draw_pixmap(previewPixmap,
                    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                    greenPixelPixmap,0,0,x+PREVIEWSTARTX,y+PREVIEWSTARTY,1,1);
  if (update) {
    GdkRectangle updateRect;

    /* clean from screen too */
    updateRect.width = 1;
    updateRect.height = 1;
    updateRect.x = PREVIEWSTARTX+x;
    updateRect.y = PREVIEWSTARTY+y;

    gtk_widget_draw(previewArea,&updateRect);
  }
}

int IsPoint(int x, int y) {
  return GSM_IsPointBitmap(&bitmap,x,y);
}

void SetPoint(GtkWidget *widget, int x, int y, int update) {

  GSM_SetPointBitmap(&bitmap,x,y);

  /* draw point to pixmap */
  gdk_draw_pixmap(drawingPixmap,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
	  blackPointPixmap,0,0,x*(POINTSIZE+1),y*(POINTSIZE+1),-1,-1);

  if (update) {
    GdkRectangle updateRect;

    /* calculate update rectangle */
    updateRect.width = POINTSIZE+2;
    updateRect.height = POINTSIZE+2;
    updateRect.x = x * (POINTSIZE+1);
    updateRect.y = y * (POINTSIZE+1);

    /* update on screen */
    gtk_widget_draw(drawingArea,&updateRect);
  }

  /* draw preview point too */
  if (previewAvailable) SetPreviewPoint(widget,x,y,update); 
}

void ClearPoint(GtkWidget *widget, int x, int y, int update) {

  GSM_ClearPointBitmap(&bitmap,x,y);

  /* clear point from pixmap */
  gdk_draw_pixmap(drawingPixmap,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
          greenPointPixmap,0,0,x*(POINTSIZE+1),y*(POINTSIZE+1),-1,-1);

  if (update) {
    GdkRectangle updateRect;

    /* calculate update rectangle */
    updateRect.width = POINTSIZE+2;
    updateRect.height = POINTSIZE+2;
    updateRect.x = x * (POINTSIZE+1);
    updateRect.y = y * (POINTSIZE+1);
          
    /* update on screen */
    gtk_widget_draw(drawingArea,&updateRect);
  }

  /* clear point from previewArea too */
  if (previewAvailable) ClearPreviewPoint(widget,x,y,update); 
}

/* ****************************************************
 * *** UPDATES - PREVIEW & DRAWING AREAS **************
 * ****************************************************
 */

/* this redraw all logo points - preview & drawing area */
void UpdatePointsRectangle(GtkWidget *widget, int x1, int y1, int x2, int y2) {
  GdkRectangle updateRect;
  int x, y, dx = 0, dy = 0;

  if (bitmap.type != GSM_StartupLogo && bitmap.type != GSM_7110StartupLogo &&
      bitmap.type != GSM_6210StartupLogo) {
    dx = PREVIEWJUMPX;
    dy = PREVIEWJUMPY;
  }

  if (x1 > x2) {
    x = x1;
    x1 = x2;
    x2 = x;
  }

  if (y1 > y2) {
    y = y1;
    y1 = y2;
    y2 = y;
  }
  
  for (y = y1;y <= y2;y++)
    for (x = x1;x <= x2;x++) {
      if (IsPoint(x,y)) {
        /* set on drawing area */
        gdk_draw_pixmap(drawingPixmap,
                        drawingArea->style->fg_gc[GTK_WIDGET_STATE(drawingArea)],
                        blackPointPixmap,0,0,x*(POINTSIZE+1),y*(POINTSIZE+1),-1,-1);

        /* set on preview */
        if (previewAvailable && previewPixmap) 
          gdk_draw_point(previewPixmap,previewArea->style->black_gc,
                         x+PREVIEWSTARTX+dx,y+PREVIEWSTARTY+dy);
      } else {
        /* clear from drawing */
        gdk_draw_pixmap(drawingPixmap,
                        drawingArea->style->fg_gc[GTK_WIDGET_STATE(drawingArea)],
                        greenPointPixmap,0,0,x*(POINTSIZE+1),y*(POINTSIZE+1),-1,-1);

        /* clear from preview */
        if (previewAvailable && previewPixmap) 
          gdk_draw_pixmap(previewPixmap,
                          previewArea->style->fg_gc[GTK_WIDGET_STATE(previewArea)],
                          greenPixelPixmap,0,0,x+PREVIEWSTARTX+dx,y+PREVIEWSTARTY+dy,
                          1,1);
      }
    }
     
  if (previewAvailable) {  
    updateRect.x = PREVIEWSTARTX+dx+x1;
    updateRect.y = PREVIEWSTARTY+dy+y1;
    updateRect.width = x2-x1+1;
    updateRect.height = y2-y1+1;
    gtk_widget_draw(previewArea,&updateRect);
  }  

  updateRect.x = x1*(POINTSIZE+1);
  updateRect.y = y1*(POINTSIZE+1);
  updateRect.width = (x2-x1+1)*(POINTSIZE+1)+1;
  updateRect.height = (y2-y1+1)*(POINTSIZE+1)+1;
  gtk_widget_draw(drawingArea,&updateRect); 
}

void UpdatePoints(GtkWidget *widget) {
  UpdatePointsRectangle(widget,0,0,bitmap.width-1,bitmap.height-1);
}

/* this redraw all logo points in previewArea, NO DRAWING AREA */
void UpdatePreviewPoints (void) {
  GdkRectangle updateRect;
  int x, y, dx = 0, dy = 0;

  if (!previewPixmap || !previewAvailable) return;  

  if (bitmap.type != GSM_StartupLogo && bitmap.type != GSM_7110StartupLogo &&
      bitmap.type != GSM_6210StartupLogo) {
    dx = PREVIEWJUMPX;
    dy = PREVIEWJUMPY;
  }

  for (y = 0;y < bitmap.height;y++)
    for (x = 0;x < bitmap.width;x++) {
      if (IsPoint(x,y)) {
        gdk_draw_point(previewPixmap,previewArea->style->black_gc,
                       x+PREVIEWSTARTX+dx,y+PREVIEWSTARTY+dy);
      } else {
        gdk_draw_pixmap(previewPixmap,
                        previewArea->style->fg_gc[GTK_WIDGET_STATE(previewArea)],
                        greenPixelPixmap,0,0,
                        x+PREVIEWSTARTX+dx,y+PREVIEWSTARTY+dy,1,1);
      }
    }
              
  updateRect.x = dx; 
  updateRect.y = dy;
  updateRect.width = bitmap.width;
  updateRect.height = bitmap.height;
  gtk_widget_draw(previewArea,&updateRect);
}

/* ******************************************************
 * **** DRAWING TOOLS ***********************************
 * ******************************************************
 */

/* TOOL - BRUSH */
void ToolBrush(GtkWidget *widget, int column, int row, int button) {
  /* only this tool directly update bitmap & screen */
  if (button > 1) 
    ClearPoint(widget,column,row,1);
  else
    SetPoint(widget,column,row,1);
}

/* TOOL - LINE */
/* going to rewrite to Bresenham algorithm */ 
static void ToolLineUpdate(GtkWidget *widget, int column, int row) {
  double delta, y, x;
  int i, x1, y1, x2, y2;
  
  if (abs(toolStartX-toolLastX) >= abs(toolStartY-toolLastY)) {
    if (toolStartX > toolLastX) {
      x1 = toolLastX;
      x2 = toolStartX;
      y1 = toolLastY;
      y2 = toolStartY;
    } else {
      x1 = toolStartX;
      x2 = toolLastX;
      y1 = toolStartY;
      y2 = toolLastY;
    }
 
    delta = ((double)(y2-y1))/((double)(x2-x1));
    y = (double)y1;
    for (i = x1;i <= x2;i++) {
      if (!GSM_IsPointBitmap(&oldBitmap,i,(int)y)) ClearPoint(widget,i,(int)y,0);
      y += delta;
    }
  } else {
    if (toolStartY > toolLastY) {
      x1 = toolLastX;
      x2 = toolStartX;
      y1 = toolLastY;
      y2 = toolStartY;      
    } else {
      x1 = toolStartX;
      x2 = toolLastX;
      y1 = toolStartY;
      y2 = toolLastY;
    }
    delta = ((double)(x2-x1))/((double)(y2-y1));

    x = (double)x1;
    for (i = y1;i <= y2;i++) {
      if (!GSM_IsPointBitmap(&oldBitmap,(int)x,i)) ClearPoint(widget,(int)x,i,0);
      x += delta;
    }
  } 

  if (abs(toolStartX-column) >= abs(toolStartY-row)) {
    if (toolStartX > column) {
      x1 = column;
      x2 = toolStartX;
      y1 = row;
      y2 = toolStartY;
    } else {
      x1 = toolStartX;
      x2 = column;
      y1 = toolStartY;
      y2 = row;
    }

    delta = ((double)(y2-y1))/((double)(x2-x1));
    y = (double)y1;
    for (i = x1;i <= x2;i++) {
      SetPoint(widget,i,(int)y,0);
      y += delta;
    }
  } else {
    if (toolStartY > row) {
      x1 = column;
      x2 = toolStartX;
      y1 = row;
      y2 = toolStartY;
    } else {
      x1 = toolStartX;
      x2 = column;
      y1 = toolStartY;
      y2 = row;
    }

    delta = ((double)(x2-x1))/((double)(y2-y1));

    x = (double)x1;
    for (i = y1;i <= y2;i++) {
      SetPoint(widget,(int)x,i,0);
      x += delta;
    }
  }
}

/* TOOL - FILLED RECT */
static void ToolFilledRectangleUpdate(GtkWidget *widget, int column, int row) {
  int i, j, x1, y1, x2, y2;

  if (toolStartX > toolLastX) {
    x1 = toolLastX;
    x2 = toolStartX;
  } else {
    x1 = toolStartX;
    x2 = toolLastX;
  }

  if (toolStartY > toolLastY) {
    y1 = toolLastY;
    y2 = toolStartY;
  } else {
    y1 = toolStartY;
    y2 = toolLastY;
  }

  for (j = y1;j <= y2;j++)
    for (i = x1;i <= x2;i++)
      if (!GSM_IsPointBitmap(&oldBitmap,i,j)) ClearPoint(widget,i,j,0);

  if (toolStartX > column) {
    x1 = column;
    x2 = toolStartX;
  } else {
    x1 = toolStartX;
    x2 = column;
  }

  if (toolStartY > row) {
    y1 = row;
    y2 = toolStartY;
  } else {
    y1 = toolStartY;
    y2 = row;
  }

  for (j = y1;j <= y2;j++)
    for (i = x1;i <= x2;i++)
      SetPoint(widget,i,j,0);
}

/* TOOL - RECTANGLE */
static void ToolRectangleUpdate(GtkWidget *widget, int column, int row) {
  int i, j, x1, y1, x2, y2;

  /* clear old rectangle */
  if (toolStartX > toolLastX) {
    x1 = toolLastX;
    x2 = toolStartX;
  } else {
    x1 = toolStartX;
    x2 = toolLastX;
  }

  if (toolStartY > toolLastY) {
    y1 = toolLastY;
    y2 = toolStartY;
  } else {
    y1 = toolStartY;
    y2 = toolLastY;
  }

  for (i = x1;i <= x2;i++) {
    if (!GSM_IsPointBitmap(&oldBitmap,i,y1)) ClearPoint(widget,i,y1,0);
    if (!GSM_IsPointBitmap(&oldBitmap,i,y2)) ClearPoint(widget,i,y2,0);                
  }

  for (j = y1;j <= y2;j++) {
    if (!GSM_IsPointBitmap(&oldBitmap,x1,j)) ClearPoint(widget,x1,j,0);
    if (!GSM_IsPointBitmap(&oldBitmap,x2,j)) ClearPoint(widget,x2,j,0);
  }

  /* draw new rectangle */
  if (toolStartX > column) {
    x1 = column;
    x2 = toolStartX;
  } else {
    x1 = toolStartX;
    x2 = column;
  }

  if (toolStartY > row) {
    y1 = row;
    y2 = toolStartY;
  } else {
    y1 = toolStartY;
    y2 = row;
  }

  for (i = x1;i <= x2;i++) {
    if (!IsPoint(i,y1)) SetPoint(widget,i,y1,0);
    if (!IsPoint(i,y2)) SetPoint(widget,i,y2,0);
  }

  for (j = y1;j <= y2;j++) {
    if (!IsPoint(x1,j)) SetPoint(widget,x1,j,0);
    if (!IsPoint(x2,j)) SetPoint(widget,x2,j,0);
  }
}

static void UpdateToolScreen(GtkWidget *widget, int x1, int y1, int x2, int y2) {
  GdkRectangle updateRect;

  /* update preview area */
  updateRect.x = PREVIEWSTARTX+x1;
  updateRect.y = PREVIEWSTARTY+y1;
  if (bitmap.type != GSM_StartupLogo && bitmap.type != GSM_7110StartupLogo &&
      bitmap.type != GSM_6210StartupLogo) {
    updateRect.x += PREVIEWJUMPX;
    updateRect.y += PREVIEWJUMPY;
  }
  updateRect.width = x2-x1+1;
  updateRect.height = y2-y1+1;
  gtk_widget_draw(previewArea,&updateRect);

  /* update drawing area */
  updateRect.x = x1 * (POINTSIZE+1);
  updateRect.y = y1 * (POINTSIZE+1);
  updateRect.width = (x2-x1+1)*(POINTSIZE+2);
  updateRect.height = (y2-y1+1)*(POINTSIZE+2);
  gtk_widget_draw(drawingArea,&updateRect);
}

/* *************************************
 * ** PREVIEW AREA EVENTS **************
 * *************************************
 */

static gint PreviewAreaButtonPressEvent(GtkWidget *widget, GdkEventButton *event) {
  previewPixmapNumber = (previewPixmapNumber % 8) + 1;

  gtk_drawing_area_size(GTK_DRAWING_AREA(previewArea),
                        previewPixmapWidth,previewPixmapHeight);

  return TRUE;
}

static gint PreviewAreaConfigureEvent(GtkWidget *widget, GdkEventConfigure *event) {
  if (previewPixmap) gdk_pixmap_unref(previewPixmap);
  previewPixmap = GetPreviewPixmap(widget);

  UpdatePreviewPoints();
        
  return TRUE;
}

static gint PreviewAreaExposeEvent(GtkWidget *widget, GdkEventExpose *event) {
  /* got previewPixmap? */
  if (previewPixmap)
    /* yes - simply redraw some rectangle */      
    gdk_draw_pixmap(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                    previewPixmap,event->area.x,event->area.y,event->area.x,
                    event->area.y,event->area.width,event->area.height);

  return FALSE;
}

/* ********************************
 * ** DRAWING AREA EVENTS *********
 * ********************************
 */

static gint DrawingAreaButtonPressEvent(GtkWidget *widget, GdkEventButton *event) {
  /* got drawingPixmap? */
  if (drawingPixmap == NULL) return TRUE;

  if (!mouseButtonPushed) {
    if ((event->button == 1 && activeTool != TOOL_BRUSH) || 
        (activeTool == TOOL_BRUSH)) {
      /* position from we starting drawing */
      toolStartX = event->x / (POINTSIZE+1);
      if (toolStartX < 0) toolStartX = 0;
      if (toolStartX > bitmap.width-1) toolStartX = bitmap.width-1;
    
      toolStartY = event->y / (POINTSIZE+1);
      if (toolStartY < 0) toolStartY = 0;
      if (toolStartY > bitmap.height-1) toolStartY = bitmap.height-1;

      toolLastX = toolStartX;
      toolLastY = toolStartY;

      /* store old bitmap for drawing, resp. for moving, resizing primitive */
      memcpy(&oldBitmap,&bitmap,sizeof(oldBitmap));
    }

    if (event->button == 1) mouseButtonPushed = 1;
  
    switch (activeTool) { 
      case TOOL_BRUSH:
        ToolBrush(widget,toolStartX,toolStartY,event->button);
        break;
      case TOOL_LINE:
      case TOOL_RECTANGLE:
        if (event->button == 1) ToolBrush(widget,toolStartX,toolStartY,event->button);
        break;
    }
  }

  /* user is drawing some tool other than TOOL_BRUSH and pushed mouse button
   * another than first => cancel tool and redraw to oldBitmap (bitmap when
   * user start drawing)
   */
  if (mouseButtonPushed && activeTool != TOOL_BRUSH && event->button != 1) {
    int lowestX, lowestY, highestX, highestY;
    int i, j;

    lowestX  = GetMinFrom3(toolStartX,toolLastX,toolLastX);
    lowestY  = GetMinFrom3(toolStartY,toolLastY,toolLastY);
    highestX = GetMaxFrom3(toolStartX,toolLastX,toolLastX);
    highestY = GetMaxFrom3(toolStartY,toolLastY,toolLastY);

    for (j = lowestY;j <= highestY;j++)
      for (i = lowestX;i <= highestX;i++)
        if (GSM_IsPointBitmap(&oldBitmap,i,j))
          SetPoint(widget,i,j,0);
        else
          ClearPoint(widget,i,j,0);
    UpdateToolScreen(widget,lowestX,lowestY,highestX,highestY);

    mouseButtonPushed = 0;
  }

  return TRUE;
}

static gint DrawingAreaButtonReleaseEvent(GtkWidget *widget, GdkEventButton *event) {
  if (event->button == 1)
    mouseButtonPushed = 0;

  return TRUE;
}

static gint DrawingAreaMotionNotifyEvent(GtkWidget *widget, GdkEventMotion *event) {
  int x,y;
  GdkModifierType state;
  
  if (!mouseButtonPushed && activeTool != TOOL_BRUSH) return TRUE;
  
  if (event->is_hint)
    gdk_window_get_pointer(event->window,&x,&y,&state);
  else {
    x = event->x;
    y = event->y;
    state = event->state;
  }

  x = x / (POINTSIZE+1);
  y = y / (POINTSIZE+1);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > bitmap.width-1) x = bitmap.width-1;
  if (y > bitmap.height-1) y = bitmap.height-1;

  if (y == toolLastY && x == toolLastX) return TRUE;
  
  switch (activeTool) {
    case TOOL_BRUSH:
      if (state & GDK_BUTTON1_MASK && drawingPixmap != NULL) ToolBrush(widget,x,y,1);
      if (state & GDK_BUTTON2_MASK && drawingPixmap != NULL) ToolBrush(widget,x,y,2);
      if (state & GDK_BUTTON3_MASK && drawingPixmap != NULL) ToolBrush(widget,x,y,3);
      break;
    case TOOL_RECTANGLE:
      if (drawingPixmap != NULL) ToolRectangleUpdate(widget,x,y);
      break;
    case TOOL_FILLED_RECTANGLE:
      if (drawingPixmap != NULL) ToolFilledRectangleUpdate(widget,x,y); 
      break; 
    case TOOL_LINE:
      if (drawingPixmap != NULL) ToolLineUpdate(widget,x,y); 
      break; 
  }   

  /* what is this?
   * it's simple, above tools updates only bitmap in memory and this
   * function update from bitmap to screen, it's made as non-blinking
   * drawing functions with this, simply draw everything we need and
   * after that, redraw to screen rectangle in which we made changes 
   * it's not redrawing pixel by pixel (blinking)
   */
  if (activeTool != TOOL_BRUSH) {
    int lowestX, lowestY, highestX, highestY;

    lowestX  = GetMinFrom3(toolStartX,toolLastX,x);
    lowestY  = GetMinFrom3(toolStartY,toolLastY,y);
    highestX = GetMaxFrom3(toolStartX,toolLastX,x);
    highestY = GetMaxFrom3(toolStartY,toolLastY,y);
  
    UpdateToolScreen(widget,lowestX,lowestY,highestX,highestY);  
  }

  toolLastX = x;
  toolLastY = y; 
  return TRUE;  
}

/* configureEvent? -> event when someone resize windows, ... */
static gint DrawingAreaConfigureEvent(GtkWidget *widget, GdkEventConfigure *event) {
  int x, y;
  /* got drawingPixmap? */
  if (drawingPixmap) gdk_pixmap_unref(drawingPixmap); /* got, erase it */

  /* make a new pixmap */
  drawingPixmap = gdk_pixmap_new(widget->window,drawingAreaWidth,
   	          drawingAreaHeight,-1);

  /* draw grid into pixmap */
  for (y = 0;y < bitmap.height;y++)
    for (x = 0;x < bitmap.width;x++)
      if (IsPoint(x,y))
        gdk_draw_pixmap(drawingPixmap,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                        blackPointPixmap,0,0,x*(POINTSIZE+1),y*(POINTSIZE+1),-1,-1);
      else
        gdk_draw_pixmap(drawingPixmap,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                        greenPointPixmap,0,0,x*(POINTSIZE+1),y*(POINTSIZE+1),-1,-1);
  
  return TRUE;
}

static gint DrawingAreaExposeEvent(GtkWidget *widget, GdkEventExpose *event) {
  /* got drawingPixmap? */
  if (drawingPixmap)
    /* got - draw it */
    gdk_draw_pixmap(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
  		    drawingPixmap,
		    event->area.x,event->area.y,event->area.x,event->area.y,
		    event->area.width,event->area.height);
  return FALSE; 
}

/* *****************************************
 * ** TOOLBAR & MENU EVENTS ****************
 * *****************************************
 */

static void GetNetworkInfoEvent(GtkWidget *widget) {
  GSM_Error error;
  PhoneEvent *e = (PhoneEvent *)g_malloc(sizeof(PhoneEvent));
  D_NetworkInfo *data = (D_NetworkInfo *)g_malloc(sizeof(D_NetworkInfo));

  /* prepare data for event */
  data->info = &networkInfo;
  e->event = Event_GetNetworkInfo;
  e->data = data;

  /* launch event and wait for result */
  GUI_InsertEvent(e);
  pthread_mutex_lock(&getNetworkInfoMutex);
  pthread_cond_wait(&getNetworkInfoCond,&getNetworkInfoMutex);
  pthread_mutex_unlock(&getNetworkInfoMutex);
  error = data->status;
  g_free(data);

  /* watch for errors */
  if (error != GE_NONE) {
    gchar *buf = g_strdup_printf(_("Error getting network info\n(error=%d)"),error);
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);
  }

  /* set new operator name to combo */
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(networkCombo)->entry),
               GSM_GetNetworkName(networkInfo.NetworkCode));
}

static void GetLogoEvent(GtkWidget *widget) {
  int i;
  GSM_Error error;
  PhoneEvent *e = (PhoneEvent *) g_malloc(sizeof(PhoneEvent));
  D_Bitmap *data = (D_Bitmap *)g_malloc(sizeof(D_Bitmap));
  char *operator = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(networkCombo)->entry));

  /* prepare data for event */
  strncpy(bitmap.netcode,GSM_GetNetworkCode(operator),7);
  data->bitmap = &bitmap;
  e->event = Event_GetBitmap;
  e->data = data;
  if (GetModelFeature(FN_CALLERGROUPS)!=0) {
    for (i = 0;i < 6;i++) 
      if (strcmp(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(callerCombo)->entry)),
                 xgnokiiConfig.callerGroups[i]) == 0) bitmap.number = i; 
  }
  
  /* launch event and wait for result */
  GUI_InsertEvent(e);
  pthread_mutex_lock(&getBitmapMutex);
  pthread_cond_wait(&getBitmapCond,&getBitmapMutex);
  pthread_mutex_unlock(&getBitmapMutex);
  error = data->status;
  g_free(data);

  /* watch for errors */
  if (error != GE_NONE) {
    gchar *buf = g_strdup_printf(_("Error getting bitmap\n(error=%d)"),error);
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);
  } else {
    /* no error, draw logo from phone */
    UpdatePoints(drawingArea);
  }
}

static void SetLogoEvent(GtkWidget *widget) {
  GSM_Error error;
  PhoneEvent *e = (PhoneEvent *)g_malloc(sizeof(PhoneEvent));
  D_Bitmap *data = (D_Bitmap *)g_malloc(sizeof(D_Bitmap));
  char *operator = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(networkCombo)->entry));
  int i;

  /* prepare data */
  strncpy(bitmap.netcode,GSM_GetNetworkCode(operator),7);
 
  if (bitmap.type == GSM_CallerLogo) {
    /* above condition must be there, because if you launch logos before
     * callerGroups are available, you will see segfault - callerGroups not initialized 
     */
    if (GetModelFeature(FN_CALLERGROUPS)!=0) {
      for (i = 0;i < 6;i++) 
        if (strcmp(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(callerCombo)->entry)),
                   xgnokiiConfig.callerGroups[i]) == 0) bitmap.number = i; 
    }  
  }
  if (bitmap.type == GSM_StartupLogo) {
    bitmap.number=0;
  }
  
  data->bitmap = &bitmap;
  e->event = Event_SetBitmap;
  e->data = data;

  /* launch event and wait for result */
  GUI_InsertEvent(e);
  pthread_mutex_lock(&setBitmapMutex);
  pthread_cond_wait(&setBitmapCond,&setBitmapMutex);
  pthread_mutex_unlock(&setBitmapMutex);
  error = data->status;
  g_free(data);

  /* watch for errors */
  if (error != GE_NONE) {
    gchar *buf = g_strdup_printf(_("Error setting bitmap\n(error=%d)"),error);
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);
  }
}

static void ClearLogoEvent(GtkWidget *widget) {
  
  bitmap.size=GSM_GetBitmapSize(&bitmap);
  
  GSM_ClearBitmap(&bitmap);

  UpdatePoints(widget);
}

static void InvertLogoEvent(GtkWidget *widget) {
  int column, row;

  for (column = 0;column < bitmap.width;column++)
    for (row = 0;row < bitmap.height;row++)
      if (IsPoint(column,row))
	GSM_ClearPointBitmap(&bitmap,column,row);
      else
	GSM_SetPointBitmap(&bitmap,column,row);

  UpdatePoints(widget);
}

static void UpLogoEvent(GtkWidget *widget) {
  int column, row;

  GSM_Bitmap tbitmap;
  
  tbitmap=bitmap;
  
  for (row = 0;row < bitmap.height-1;row++)
    for (column = 0;column < bitmap.width;column++)
      if (IsPoint(column,row+1))
	GSM_SetPointBitmap(&bitmap,column,row);
      else
	GSM_ClearPointBitmap(&bitmap,column,row);

  for (column = 0;column < bitmap.width;column++)
    if (GSM_IsPointBitmap(&tbitmap,column,0))
      GSM_SetPointBitmap(&bitmap,column,row);
    else
      GSM_ClearPointBitmap(&bitmap,column,row);    

  UpdatePoints(widget); 
}

static void DownLogoEvent(GtkWidget *widget) {
  int column, row;

  GSM_Bitmap tbitmap;
  
  tbitmap=bitmap;

  for (row = bitmap.height-1;row > 0;row--)
    for (column = 0;column < bitmap.width;column++)
      if (IsPoint(column,row-1))
        GSM_SetPointBitmap(&bitmap,column,row);
      else
        GSM_ClearPointBitmap(&bitmap,column,row);

  for (column = 0;column < bitmap.width;column++)
    if (GSM_IsPointBitmap(&tbitmap,column,bitmap.height-1))
      GSM_SetPointBitmap(&bitmap,column,0);
    else
      GSM_ClearPointBitmap(&bitmap,column,0);

  UpdatePoints(widget);
}

static void LeftLogoEvent(GtkWidget *widget) {
  int column, row;

  GSM_Bitmap tbitmap;
  
  tbitmap=bitmap;

  for (column = 0; column < bitmap.width-1;column++)
    for (row = 0;row < bitmap.height;row++)
      if (IsPoint(column+1,row))
        GSM_SetPointBitmap(&bitmap,column,row);
      else
	GSM_ClearPointBitmap(&bitmap,column,row);

  for (row = 0;row < bitmap.height;row++)
    if (GSM_IsPointBitmap(&tbitmap,0,row))
      GSM_SetPointBitmap(&bitmap,bitmap.width-1,row);
    else
      GSM_ClearPointBitmap(&bitmap,bitmap.width-1,row);

  UpdatePoints(widget);
}

static void RightLogoEvent(GtkWidget *widget) {
  int column, row;

  GSM_Bitmap tbitmap;
  
  tbitmap=bitmap;

  for (column = bitmap.width-1;column > 0;column--)
    for (row = 0;row < bitmap.height;row++)
      if (IsPoint(column-1,row))
        GSM_SetPointBitmap(&bitmap,column,row);
      else
        GSM_ClearPointBitmap(&bitmap,column,row);

  for (row = 0;row < bitmap.height;row++)
    if (GSM_IsPointBitmap(&tbitmap,bitmap.width-1,row))
      GSM_SetPointBitmap(&bitmap,0,row);
    else
      GSM_ClearPointBitmap(&bitmap,0,row);

  UpdatePoints(widget);
}

static void FlipVerticalLogoEvent(GtkWidget *widget) {
  int row, column, temp;

  for (row = 0;row < (bitmap.height/2);row++)
    for (column = 0;column < bitmap.width;column++) {
      temp = IsPoint(column,row);
      if (IsPoint(column,bitmap.height-1-row))
	GSM_SetPointBitmap(&bitmap,column,row);
      else
	GSM_ClearPointBitmap(&bitmap,column,row);

      if (temp)
	GSM_SetPointBitmap(&bitmap,column,bitmap.height-1-row);
      else
	GSM_ClearPointBitmap(&bitmap,column,bitmap.height-1-row);
    }

  UpdatePoints(widget);
}

static void FlipHorizontalLogoEvent(GtkWidget *widget) {
  int row, column, temp;

  for (row = 0;row < bitmap.height;row++)
    for (column = 0;column < (bitmap.width/2);column++) {
      temp = IsPoint(column,row);

      if (IsPoint(bitmap.width-1-column,row))
	GSM_SetPointBitmap(&bitmap,column,row);
      else
        GSM_ClearPointBitmap(&bitmap,column,row);

      if (temp)
	GSM_SetPointBitmap(&bitmap,bitmap.width-1-column,row);
      else
	GSM_ClearPointBitmap(&bitmap,bitmap.width-1-column,row);
    }

  UpdatePoints(widget);
} 

/* this is launched when tool was changed */
static gint ToolTypeEvent(GtkWidget *widget) {
  if (GTK_TOGGLE_BUTTON(buttonBrush)->active) activeTool = TOOL_BRUSH; else
  if (GTK_TOGGLE_BUTTON(buttonLine)->active) activeTool = TOOL_LINE; else
  if (GTK_TOGGLE_BUTTON(buttonRectangle)->active) activeTool = TOOL_RECTANGLE; else
  if (GTK_TOGGLE_BUTTON(buttonFilledRectangle)->active)
    activeTool = TOOL_FILLED_RECTANGLE;

  return 0;
}

/* this is launched when logo type was change by buttons on toolbar */
static gint LogoTypeEvent(GtkWidget *widget) {
  int clear = 0;

  /* is startupLogo? */
  if (GTK_TOGGLE_BUTTON(buttonStartup)->active &&
      bitmap.type != GSM_StartupLogo && bitmap.type != GSM_6210StartupLogo &&
      bitmap.type != GSM_7110StartupLogo) {
    /* look for old bitmap type, clean if another */
    clear = 1;
    
    if (!strcmp(xgnokiiConfig.model,"7110")) {
      GSM_ResizeBitmap(&bitmap,GSM_7110StartupLogo);
    } else if (!strcmp(xgnokiiConfig.model,"6210") || !strcmp(xgnokiiConfig.model,"6250")) {
      GSM_ResizeBitmap(&bitmap,GSM_6210StartupLogo);
    } else {
      GSM_ResizeBitmap(&bitmap,GSM_StartupLogo);
    }
  }
  
  /* new type is PictureImage */
  if (GTK_TOGGLE_BUTTON(buttonPicture)->active && bitmap.type != GSM_PictureImage) {
    /* look for old bitmap type, clean if another */
    clear = 1;
    GSM_ResizeBitmap(&bitmap,GSM_PictureImage);
  }
  
  /* new type is callerLogo */
  if (GTK_TOGGLE_BUTTON(buttonCaller)->active && bitmap.type != GSM_CallerLogo) {
    /* previous was startup or Picture ? clear and draw batteries, signal, ... */      
    if (bitmap.type != GSM_OperatorLogo) clear = 1;
    GSM_ResizeBitmap(&bitmap,GSM_CallerLogo);
  }

  /* is new type operatorLogo? */
  if (GTK_TOGGLE_BUTTON(buttonOperator)->active && bitmap.type != GSM_OperatorLogo &&
      bitmap.type != GSM_7110OperatorLogo) {

    /* previous startup or Picture ? clear and draw batteries, signal, ... */      
    if (bitmap.type != GSM_CallerLogo) clear = 1;

    if (!strcmp(xgnokiiConfig.model,"7110")) {
      GSM_ResizeBitmap(&bitmap,GSM_7110OperatorLogo);
    } else if (!strcmp(xgnokiiConfig.model,"6210") || !strcmp(xgnokiiConfig.model,"6250")) {
      GSM_ResizeBitmap(&bitmap,GSM_7110OperatorLogo);
    } else {
      GSM_ResizeBitmap(&bitmap,GSM_OperatorLogo);
    }
  }

  /* must clear? */
  if (clear) {
    if (previewAvailable) {
      /* configure event reload pixmap from disk and redraws */
      gtk_drawing_area_size(GTK_DRAWING_AREA(previewArea),
                            previewPixmapWidth,previewPixmapHeight);
    }   

    /* change new drawingArea size */
    drawingAreaWidth = bitmap.width * (POINTSIZE+1)+1;
    drawingAreaHeight = bitmap.height * (POINTSIZE+1)+1;

    gtk_drawing_area_size(GTK_DRAWING_AREA(drawingArea),
                          drawingAreaWidth,drawingAreaHeight);
  }

  return 0;
}

static inline void CloseLogosWindow (void) {
  gtk_widget_hide(GUI_LogosWindow);
}

void ExportLogoFileMain(gchar *name)
{
  GSM_Bitmap tbitmap;
  GSM_Error error;
  
  tbitmap=bitmap;

  strncpy(tbitmap.netcode,GSM_GetNetworkCode(networkInfo.NetworkCode),7);

  error=GSM_SaveBitmapFile(name,&tbitmap);
  if (error!=GE_NONE) {
    gchar *buf = g_strdup_printf(_("Error saving file\n(error=%d)"),error);
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);
  }
}

static void YesLogoFileExportDialog (GtkWidget *w, gpointer data)
{
  gtk_widget_hide (GTK_WIDGET (data));
  ExportLogoFileMain(exportDialogData.fileName);
}

static void ExportFileSelected (GtkWidget *w, GtkFileSelection *fs)
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
      CreateYesNoDialog (&dialog, YesLogoFileExportDialog, CancelDialog, GUI_LogosWindow);
      gtk_window_set_title (GTK_WINDOW (dialog.dialog), _("Overwrite file?"));
      g_snprintf ( err, 80, _("File %s already exist.\nOverwrite?"), exportDialogData.fileName); 
      gtk_label_set_text (GTK_LABEL(dialog.text), err);
    }
    gtk_widget_show (dialog.dialog);
  }
  else
    ExportLogoFileMain(exportDialogData.fileName);
}

void ImportFileSelected(GtkWidget *w, GtkFileSelection *fs)
{
  GSM_Bitmap tbitmap;
  GSM_Error error=0;
  
  gchar *fileName;
  FILE *f;

  fileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  gtk_widget_hide (GTK_WIDGET (fs));

  if ((f = fopen (fileName, "r")) == NULL) {
    gchar *buf = g_strdup_printf(_("Can't open file %s for reading !"),fileName);
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);
    return;
  }

  error=GSM_ReadBitmapFile(fileName,&tbitmap);
  if (error!=GE_NONE) {
    gchar *buf = g_strdup_printf(_("Error reading file\n(error=%d)"),error);
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);
    return;
  }

  exportDialogData.fileName=fileName;
  
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonStartup),false);
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonOperator),false);
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonCaller),false);
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonPicture),false);  

  if (tbitmap.type==GSM_OperatorLogo || tbitmap.type==GSM_7110OperatorLogo) gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonOperator),true);
  if (tbitmap.type==GSM_StartupLogo || tbitmap.type==GSM_7110StartupLogo || tbitmap.type==GSM_6210StartupLogo) gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonStartup),true);
  if (tbitmap.type==GSM_CallerLogo) gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonCaller),true);
  if (tbitmap.type==GSM_PictureImage) gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(buttonPicture),true);  
  
  memcpy(&bitmap,&tbitmap,sizeof(GSM_Bitmap));

  if (!strcmp(xgnokiiConfig.model,"7110")) { //7110
    if (bitmap.type==GSM_StartupLogo || bitmap.type==GSM_6210StartupLogo)
      GSM_ResizeBitmap(&bitmap,GSM_7110StartupLogo);

    if (bitmap.type==GSM_OperatorLogo)
      GSM_ResizeBitmap(&bitmap,GSM_7110OperatorLogo);

  } else if (!strcmp(xgnokiiConfig.model,"6210") || !strcmp(xgnokiiConfig.model,"6250")) { //6210,6250
    if (bitmap.type==GSM_StartupLogo || bitmap.type==GSM_7110StartupLogo)
      GSM_ResizeBitmap(&bitmap,GSM_6210StartupLogo);

    if (bitmap.type==GSM_OperatorLogo)
      GSM_ResizeBitmap(&bitmap,GSM_7110OperatorLogo);

  } else {
    if (bitmap.type==GSM_6210StartupLogo || bitmap.type==GSM_7110StartupLogo)
      GSM_ResizeBitmap(&bitmap,GSM_StartupLogo);
    if (bitmap.type==GSM_7110OperatorLogo)
      GSM_ResizeBitmap(&bitmap,GSM_OperatorLogo);
  }  
  
  UpdatePoints(drawingArea);
}

void SaveLogoAs(GtkWidget *widget)
{
  FileSelection=gtk_file_selection_new ("Save logo as ...");

  gtk_signal_connect (
	GTK_OBJECT (GTK_FILE_SELECTION (FileSelection)->ok_button),
	"clicked", (GtkSignalFunc) ExportFileSelected, FileSelection);
    
  gtk_signal_connect_object (
	GTK_OBJECT(GTK_FILE_SELECTION(FileSelection)->cancel_button),
	"clicked", (GtkSignalFunc) gtk_widget_destroy,
        GTK_OBJECT (FileSelection));
    
  gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(FileSelection));

  gtk_widget_show(FileSelection);
}

void SaveLogo(GtkWidget *widget)
{
  if (exportDialogData.fileName==NULL) {
    SaveLogoAs(widget);
  } else {
    ExportLogoFileMain(exportDialogData.fileName);
  }
}

void OpenLogo(GtkWidget *widget)
{
  FileSelection=gtk_file_selection_new ("Open logo...");

  gtk_signal_connect (
	GTK_OBJECT (GTK_FILE_SELECTION (FileSelection)->ok_button),
	"clicked", (GtkSignalFunc) ImportFileSelected, FileSelection);
    
  gtk_signal_connect_object (
	GTK_OBJECT(GTK_FILE_SELECTION(FileSelection)->cancel_button),
	"clicked", (GtkSignalFunc) gtk_widget_destroy,
        GTK_OBJECT (FileSelection));
    
  gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(FileSelection));

  gtk_widget_show(FileSelection);
}

static GtkItemFactoryEntry logosMenuItems[] = {
  { NULL,	NULL,		NULL,			 0, "<Branch>"},
  { NULL,       "<control>O",   OpenLogo,		 0, NULL},
  { NULL,       "<control>S",   SaveLogo,		 0, NULL},
  { NULL,	NULL,		SaveLogoAs,	         0, NULL},
  { NULL,       NULL,           NULL,                    0, "<Separator>"},
  { NULL,       "<control>G",   GetNetworkInfoEvent,     0, NULL},
  { NULL,       NULL,           GetLogoEvent,            0, NULL},
  { NULL,       "<control>T",   SetLogoEvent,            0, NULL},
  { NULL,       NULL,           NULL,                    0, "<Separator>"},
  { NULL,       "<control>W",   CloseLogosWindow,        0, NULL},
  { NULL,	NULL,		NULL,			 0, "<Branch>"},
  { NULL,	"<control>C",	ClearLogoEvent,		 0, NULL},
  { NULL,	"<control>I",	InvertLogoEvent,	 0, NULL},
  { NULL,	NULL,		NULL,			 0, "<Separator>"},
  { NULL, 	"<control>U",	UpLogoEvent,		 0, NULL},
  { NULL,	"<control>D",	DownLogoEvent,		 0, NULL},
  { NULL,	"<control>L",	LeftLogoEvent,		 0, NULL},
  { NULL,	"<control>R",	RightLogoEvent,		 0, NULL},
  { NULL,	NULL,		NULL,			 0, "<Separator>"},
  { NULL,	"<control>H",	FlipHorizontalLogoEvent, 0, NULL},
  { NULL,	"<control>V",	FlipVerticalLogoEvent,	 0, NULL},
};

static void InitLogosMenu (void) {
 logosMenuItems[0].path = g_strdup(_("/_File"));
 logosMenuItems[1].path = g_strdup(_("/File/_Open"));
 logosMenuItems[2].path = g_strdup(_("/File/_Save"));
 logosMenuItems[3].path = g_strdup(_("/File/Save _as ..."));
 logosMenuItems[4].path = g_strdup(_("/File/Sep1"));
 logosMenuItems[5].path = g_strdup(_("/File/_Get operator"));
 logosMenuItems[6].path = g_strdup(_("/File/Get _logo"));
 logosMenuItems[7].path = g_strdup(_("/File/Se_t logo"));
 logosMenuItems[8].path = g_strdup(_("/File/Sep2"));
 logosMenuItems[9].path = g_strdup(_("/File/_Close"));
 logosMenuItems[10].path = g_strdup(_("/_Edit"));
 logosMenuItems[11].path = g_strdup(_("/Edit/_Clear"));
 logosMenuItems[12].path = g_strdup(_("/Edit/_Invert"));
 logosMenuItems[13].path = g_strdup(_("/Edit/Sep3"));
 logosMenuItems[14].path = g_strdup(_("/Edit/_Up logo"));
 logosMenuItems[15].path = g_strdup(_("/Edit/_Down logo"));
 logosMenuItems[16].path = g_strdup(_("/Edit/_Left logo"));
 logosMenuItems[17].path = g_strdup(_("/Edit/_Right logo"));
 logosMenuItems[18].path = g_strdup(_("/Edit/Sep4"));
 logosMenuItems[19].path = g_strdup(_("/Edit/Flip _horizontal"));
 logosMenuItems[20].path = g_strdup(_("/Edit/Flip _vertical"));
}

void GUI_CreateLogosWindow (void) {
  int nMenuItems = sizeof (logosMenuItems) / sizeof (logosMenuItems[0]);
  GtkAccelGroup *accelGroup;
  GtkItemFactory *itemFactory;
  GtkWidget *menuBar;
  GtkWidget *toolBar, *vertToolBar;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *drawingBox;
  GtkWidget *separator;
  GdkBitmap *mask;

  GList *glistNetwork = NULL;

  int i = 0;
 
  previewPixmapWidth = PREVIEWWIDTH;
  previewPixmapHeight = PREVIEWHEIGHT;
  
  InitLogosMenu();

  /* realize top level window for logos */
  GUI_LogosWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(GUI_LogosWindow),1,1,1);
  gtk_window_set_title(GTK_WINDOW(GUI_LogosWindow),_("Logos"));
  gtk_signal_connect(GTK_OBJECT(GUI_LogosWindow),"delete_event",
                     GTK_SIGNAL_FUNC(DeleteEvent),NULL);
  gtk_widget_realize(GUI_LogosWindow);

  CreateErrorDialog(&errorDialog,GUI_LogosWindow);
  CreateInfoDialog(&infoDialog,GUI_LogosWindow);

  accelGroup = gtk_accel_group_new();
  gtk_accel_group_attach(accelGroup,GTK_OBJECT(GUI_LogosWindow));
  
  /* create main vbox */
  vbox = gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(GUI_LogosWindow),vbox);
  gtk_widget_show(vbox);
 
  itemFactory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,"<main>",accelGroup);
  gtk_item_factory_create_items(itemFactory,nMenuItems,logosMenuItems,NULL);
  menuBar = gtk_item_factory_get_widget(itemFactory,"<main>");

  gtk_box_pack_start(GTK_BOX(vbox),menuBar,FALSE,FALSE,0);
  gtk_widget_show(menuBar);

  /* toolbar */
  toolBar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_button_relief(GTK_TOOLBAR(toolBar),GTK_RELIEF_NORMAL);
  gtk_toolbar_set_style(GTK_TOOLBAR(toolBar),GTK_TOOLBAR_ICONS);

  gtk_toolbar_append_item(GTK_TOOLBAR(toolBar),NULL,"Clear logo",NULL,
		  NewPixmap(New_xpm,GUI_LogosWindow->window,
		  &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
		  (GtkSignalFunc)ClearLogoEvent,toolBar);

  gtk_toolbar_append_space(GTK_TOOLBAR(toolBar));

  gtk_toolbar_append_item(GTK_TOOLBAR(toolBar),NULL,"Get logo",NULL,
                 NewPixmap(Read_xpm,GUI_LogosWindow->window,
                 &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                 (GtkSignalFunc)GetLogoEvent,toolBar);

  gtk_toolbar_append_item(GTK_TOOLBAR(toolBar),NULL,"Set logo",NULL,
                 NewPixmap(Send_xpm,GUI_LogosWindow->window,
                 &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                 (GtkSignalFunc)SetLogoEvent,toolBar);

  gtk_toolbar_append_space(GTK_TOOLBAR(toolBar));

  buttonStartup = gtk_toolbar_append_element(GTK_TOOLBAR(toolBar),
		  GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,NULL,"Startup logo",
		  "",NewPixmap(Startup_logo_xpm,GUI_LogosWindow->window,
                  &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
		  GTK_SIGNAL_FUNC(LogoTypeEvent),NULL);

  buttonOperator = gtk_toolbar_append_element(GTK_TOOLBAR(toolBar),
                  GTK_TOOLBAR_CHILD_RADIOBUTTON,buttonStartup,NULL,"Operator logo",
		  "",NewPixmap(Operator_logo_xpm,GUI_LogosWindow->window,
		  &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
		  GTK_SIGNAL_FUNC(LogoTypeEvent),NULL);

  buttonCaller = gtk_toolbar_append_element(GTK_TOOLBAR(toolBar),
                 GTK_TOOLBAR_CHILD_RADIOBUTTON,
		 buttonOperator,
		 NULL,"Caller logo",
		 "",NewPixmap(Caller_logo_xpm,GUI_LogosWindow->window,
	         &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
		 GTK_SIGNAL_FUNC(LogoTypeEvent),NULL);

  buttonPicture = gtk_toolbar_append_element(GTK_TOOLBAR(toolBar),
                  GTK_TOOLBAR_CHILD_RADIOBUTTON,
		  buttonOperator,
		  NULL,"Picture image",
		  "",NewPixmap(Caller_logo_xpm,GUI_LogosWindow->window,
	          &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
		  GTK_SIGNAL_FUNC(LogoTypeEvent),NULL);

  gtk_toolbar_append_space(GTK_TOOLBAR(toolBar));
  
  networkCombo = gtk_combo_new();
  gtk_combo_set_use_arrows_always(GTK_COMBO(networkCombo),1);
  while (strcmp(GSM_Networks[i].Name,"unknown"))
    glistNetwork = g_list_insert_sorted(glistNetwork,GSM_Networks[i++].Name,
                   (GCompareFunc)strcmp);
  gtk_combo_set_popdown_strings(GTK_COMBO(networkCombo),glistNetwork);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(networkCombo)->entry),FALSE);
  gtk_toolbar_append_widget(GTK_TOOLBAR(toolBar),networkCombo,"","");
  gtk_widget_show(networkCombo);
  g_list_free(glistNetwork);

  callerCombo = gtk_combo_new();
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(callerCombo)->entry),FALSE);
  gtk_toolbar_append_widget(GTK_TOOLBAR(toolBar),callerCombo,"","");
  gtk_widget_show(callerCombo);
  
  gtk_box_pack_start(GTK_BOX(vbox),toolBar,FALSE,FALSE,0);
  gtk_widget_show(toolBar); 

  /* vertical separator */
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(separator),FALSE,FALSE,0);
  
  /* create horizontal box for preview and drawing areas */
  hbox = gtk_hbox_new(FALSE,5);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);
 
  /* set GSM_Bitmap width,height needed for creating drawinArea
   * we are starting, default is startupLogo
   */
  bitmap.type = GSM_7110StartupLogo;
  bitmap.height = 65;
  bitmap.width = 96;
  bitmap.size = bitmap.height * bitmap.width / 8;
  if (!strcmp(xgnokiiConfig.model,"7110")) { //7110
    GSM_ResizeBitmap(&bitmap,GSM_7110StartupLogo);
  } else if (!strcmp(xgnokiiConfig.model,"6210") || !strcmp(xgnokiiConfig.model,"6250")) { //6210,6250
    GSM_ResizeBitmap(&bitmap,GSM_6210StartupLogo);
  } else {
    GSM_ResizeBitmap(&bitmap,GSM_StartupLogo);
  }
  drawingAreaWidth = bitmap.width * (POINTSIZE+1)+1;
  drawingAreaHeight = bitmap.height * (POINTSIZE+1)+1;
 
  /* previewArea */
  previewPixmap = GetPreviewPixmap(GUI_LogosWindow);

  if (previewPixmap != NULL) {
    previewArea = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(previewArea),
                          previewPixmapWidth,previewPixmapHeight);
    
    greenPixelPixmap = gdk_pixmap_create_from_xpm_d(GUI_LogosWindow->window,
                       &mask,&GUI_LogosWindow->style->bg[GTK_STATE_NORMAL],
                       Green_pixel_xpm);

    gtk_signal_connect(GTK_OBJECT(previewArea),"expose_event",
  		       (GtkSignalFunc)PreviewAreaExposeEvent,NULL);
    gtk_signal_connect(GTK_OBJECT(previewArea),"configure_event",
                       (GtkSignalFunc)PreviewAreaConfigureEvent,NULL);
    gtk_signal_connect(GTK_OBJECT(previewArea),"button_press_event",
                       (GtkSignalFunc)PreviewAreaButtonPressEvent,NULL);

    gtk_widget_set_events(previewArea,GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK );

    gtk_box_pack_start(GTK_BOX(hbox),previewArea,FALSE,FALSE,0);
    gtk_widget_show(previewArea);
    
    /* clear battery, signal, menu & names from preview phone */
    UpdatePreviewPoints (); 

  } else previewAvailable = 0;

  /* drawingArea */
  greenPointPixmap = gdk_pixmap_create_from_xpm_d(GUI_LogosWindow->window,
		  &mask,&GUI_LogosWindow->style->bg[GTK_STATE_NORMAL],Green_point_xpm);
  blackPointPixmap = gdk_pixmap_create_from_xpm_d(GUI_LogosWindow->window,
		  &mask,&GUI_LogosWindow->style->bg[GTK_STATE_NORMAL],Black_point_xpm);
  
  drawingBox = gtk_vbox_new(FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox),drawingBox,FALSE,FALSE,0);
  gtk_widget_show(drawingBox);

  drawingArea = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(drawingArea),
		  drawingAreaWidth,drawingAreaHeight);

  gtk_signal_connect(GTK_OBJECT(drawingArea),"configure_event",
		  (GtkSignalFunc)DrawingAreaConfigureEvent,NULL);
  gtk_signal_connect(GTK_OBJECT(drawingArea),"expose_event",
		  (GtkSignalFunc)DrawingAreaExposeEvent,NULL);
  gtk_signal_connect(GTK_OBJECT(drawingArea),"button_press_event",
		  (GtkSignalFunc)DrawingAreaButtonPressEvent,NULL);
  gtk_signal_connect(GTK_OBJECT(drawingArea),"button_release_event",
                  (GtkSignalFunc)DrawingAreaButtonReleaseEvent,NULL);
  gtk_signal_connect(GTK_OBJECT(drawingArea),"motion_notify_event",
		  (GtkSignalFunc)DrawingAreaMotionNotifyEvent,NULL);
  
  gtk_widget_set_events(drawingArea,GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK |
		  GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | 
                  GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

  gtk_box_pack_start(GTK_BOX(drawingBox),drawingArea,FALSE,FALSE,0);
  gtk_widget_show(drawingArea);

  /* vertical tool bar */
  vertToolBar = gtk_toolbar_new(GTK_ORIENTATION_VERTICAL,GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_button_relief(GTK_TOOLBAR(vertToolBar),GTK_RELIEF_NORMAL);
  gtk_toolbar_set_style(GTK_TOOLBAR(vertToolBar),GTK_TOOLBAR_ICONS);

  buttonBrush = gtk_toolbar_append_element(GTK_TOOLBAR(vertToolBar),
                GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,NULL,"Brush tool",
                "",NewPixmap(Tool_brush_xpm,GUI_LogosWindow->window,
                &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                GTK_SIGNAL_FUNC(ToolTypeEvent),NULL);

  buttonLine = gtk_toolbar_append_element(GTK_TOOLBAR(vertToolBar),
               GTK_TOOLBAR_CHILD_RADIOBUTTON,buttonBrush,NULL,"Line tool",
               "",NewPixmap(Tool_line_xpm,GUI_LogosWindow->window,
               &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
               GTK_SIGNAL_FUNC(ToolTypeEvent),NULL);

  buttonRectangle = gtk_toolbar_append_element(GTK_TOOLBAR(vertToolBar),
                    GTK_TOOLBAR_CHILD_RADIOBUTTON,buttonLine,NULL,"Rectangle tool",
                    "",NewPixmap(Tool_rectangle_xpm,GUI_LogosWindow->window,
                    &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                    GTK_SIGNAL_FUNC(ToolTypeEvent),NULL);

  buttonFilledRectangle = gtk_toolbar_append_element(GTK_TOOLBAR(vertToolBar),
                          GTK_TOOLBAR_CHILD_RADIOBUTTON,buttonRectangle,NULL,
                          "Filled rectangle tool",
                          "",NewPixmap(Tool_filled_rectangle_xpm,
                          GUI_LogosWindow->window,
                          &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                          GTK_SIGNAL_FUNC(ToolTypeEvent),NULL);

  gtk_toolbar_append_space(GTK_TOOLBAR(vertToolBar));
 
  gtk_toolbar_append_item(GTK_TOOLBAR(vertToolBar),NULL,"Invert logo",NULL,
                          NewPixmap(Edit_invert_xpm,GUI_LogosWindow->window,
                          &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                          (GtkSignalFunc)InvertLogoEvent,vertToolBar);

  gtk_toolbar_append_item(GTK_TOOLBAR(vertToolBar),NULL,"Horizontal flip",NULL,
                          NewPixmap(Edit_flip_horizontal_xpm,GUI_LogosWindow->window,
                          &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                          (GtkSignalFunc)FlipHorizontalLogoEvent,vertToolBar);

  gtk_toolbar_append_item(GTK_TOOLBAR(vertToolBar),NULL,"Vertical flip",NULL,
                          NewPixmap(Edit_flip_vertical_xpm,GUI_LogosWindow->window,
                          &GUI_LogosWindow->style->bg[GTK_STATE_NORMAL]),
                          (GtkSignalFunc)FlipVerticalLogoEvent,vertToolBar);

  
  gtk_box_pack_start(GTK_BOX(hbox),vertToolBar,FALSE,FALSE,0);
  gtk_widget_show(vertToolBar);

  GUIEventAdd(GUI_EVENT_CALLERS_GROUPS_CHANGED,&GUI_RefreshLogosGroupsCombo);
}

void GUI_RefreshLogosGroupsCombo (void) {
  GList *callerList = NULL;
  int i;
  
  for (i = 0;i < 6;i++)
    callerList = g_list_insert(callerList,xgnokiiConfig.callerGroups[i],i);
  
  gtk_combo_set_popdown_strings(GTK_COMBO(callerCombo),callerList);
  g_list_free(callerList);

  if (!callersGroupsInitialized) callersGroupsInitialized = 1;
}

void GUI_ShowLogosWindow (void) {

  gtk_widget_show(buttonCaller);
  gtk_widget_show(callerCombo); 

  if (xgnokiiConfig.callerGroups[0] == NULL) {
    GUI_Refresh(); 
    GUI_InitCallerGroupsInf ();
  }

  if (!callersGroupsInitialized) GUI_RefreshLogosGroupsCombo ();           
  
  gtk_widget_show(GUI_LogosWindow);

  if (!previewAvailable && showPreviewErrorDialog) {
    gchar *buf = g_strdup(_("Load preview pixmap error, feature disabled."));
    gtk_label_set_text(GTK_LABEL(errorDialog.text),buf);
    gtk_widget_show(errorDialog.dialog);
    g_free(buf);

    showPreviewErrorDialog = 0;
  }
}
