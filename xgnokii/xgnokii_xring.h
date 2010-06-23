/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef XGNOKII_XRING_H
#define XGNOKII_XRING_H

#include <assert.h>

enum {
  KIE_BLACK,	KIE_BLACKSEL,
  KIE_WHITEL,	KIE_WHITELSEL,
  KIE_WHITEM,	KIE_WHITEMSEL,
  KIE_WHITER,	KIE_WHITERSEL,
  KIE_COUNT
};

#define WHITE_COUNT 21 /* how many keys on the keyboard */

#define BLACK_PRESSED 64
#define WHITE_PRESSED 128

typedef struct
{
  GdkPixmap * pixmap;
  GdkBitmap * mask;
} PixmapAndMask;

extern void GUI_ShowRingtonesWindow ();
extern void GUI_CreateRingtonesWindow ();

#endif
