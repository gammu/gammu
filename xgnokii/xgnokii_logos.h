#ifndef XGNOKII_LOGOS_H
#define XGNOKII_LOGOS_H

/* drawable point size - depends on .xpm file */
#define POINTSIZE 5

/* maximal size for drawing area */
#define MAXWIDTH 82
#define MAXHEIGHT 48

/* where to draw preview logos in previewPixmap */
#define PREVIEWSTARTX 28 
#define PREVIEWSTARTY 160

#define PREVIEWWIDTH 138
#define PREVIEWHEIGHT 289

/* relative movement caller & operator logo from startuplogo */
#define PREVIEWJUMPX 6 
#define PREVIEWJUMPY 6 

#define TOOL_BRUSH 0
#define TOOL_LINE 1
#define TOOL_RECTANGLE 2 
#define TOOL_FILLED_RECTANGLE 3
#define TOOL_CIRCLE 4
#define TOOL_FILLED_CIRCLE 5
#define TOOL_ELIPSE 6 
#define TOOL_FILLED_ELIPSE 7 
#define TOOL_TEXT 8 

extern void GUI_ShowLogosWindow (void);
extern void GUI_CreateLogosWindow (void);

/* this is called from optionsApplyCallback when some changes
 * caller groups names */
extern void GUI_RefreshLogosGroupsCombo (void);

#endif
