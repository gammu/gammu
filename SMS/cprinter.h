#include "stdafx.h"

#define	IGNORE_PARAM	0xffff
#define	USE_DEFAULTS	IGNORE_PARAM
#define	LABEL_RIGHT		0
#define	LABEL_LEFT		1
#define TEXT_NORMAL	    0x0000
#define TEXT_BOLD	    0x0001
#define TEXT_ITALIC	    0x0002
#define TEXT_UNDERLINED     0x0004
#define TEXT_STRIKEOUT	    0x0008
#define TEXT_RECT	    0x0010
#define TEXT_LEFT	    0x0020
#define TEXT_RIGHT	    0x0040
#define TEXT_CENTER	    0x0080
#define TEXT_EXPANDTABS     0x0100
#define TEXT_SINGLELINE     0x0200
#define TEXT_NOCLIP	    0x0400  
#define	TEXT_VCENTER	0x0800

#define FILL_NONE	    0x0000
#define FILL_GRAY	    0x0001
#define FILL_LTGRAY	    0x0002
#define FILL_DKGRAY	    0x0004
#define FILL_BLACK	    0x0008

#define PEN_SOLID	    0x0000
#define PEN_DOT		    0x0001
#define PEN_DASH	    0x0002
#define PEN_THIN	    0x0004
#define PEN_THICK	    0x0008
#define PEN_DASHDOT	    0x0010
#define PEN_DASHDOTDOT	    0x0020

#ifndef	COLOR_RED

	#define	COLOR_RED		RGB(255,0,0)
	#define	COLOR_GREEN		RGB(0,255,0)
	#define	COLOR_BLUE		RGB(0,0,255)
	#define	COLOR_MAGENTA	RGB(255,0,255)
	#define	COLOR_YELLOW	RGB(255,255,0)
	#define	COLOR_WHITE		RGB(255,255,255)
	#define	COLOR_BLACK		RGB(0,0,0)
	#define	COLOR_CYAN		RGB(0,255,255)
	#define	COLOR_GRAY		RGB(192,192,192)
	#define COLOR_SILVER	COLOR_YELLOW|COLOR_GRAY

	#define	COLOR_LTGREEN	RGB(0,192,128)

#endif

typedef	struct tagPrinter{
							CDC		*pDC;
							RECT	rc;
							int		PointSize;
							CString	Text;
							UINT	uTextFlags;
							UINT	uFillFlags;
							UINT	uPenFlags;
							CString	FontName;
							//	static	data unchanging for life of page
							UINT	n_maxWidth;    // max width of formatting rect
							UINT	n_maxLength;   //	ditto len
							UINT	n_minNextLine; //	next posiible legible line
							int		m_NextCharPos; // pos in line of next char						
							int		m_MinDisplacement; // default size of yes/no boxes
							RECT	LastPrintArea; // printing rect last used
							int		MarginOffset;
							TEXTMETRIC*	tm;
					}PRTTYPE;		

class CPrinter{
public:
	CPrinter();
	virtual ~CPrinter();

	virtual int GetPrintInfo(PRTTYPE *ps,double LineSpacing);
	virtual int PrintText(PRTTYPE *ps,double LineSpacing);
	virtual void DrawLine(PRTTYPE *ps,int LineSize);
	virtual void DrawRect(PRTTYPE *ps,int LineSize);
	virtual	void FillRect(PRTTYPE *ps);

	int	RotationAngle;
};
