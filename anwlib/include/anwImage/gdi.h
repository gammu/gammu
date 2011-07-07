//
// gdi.h
//

#ifndef __ANWIMAGE_GDI_H_
#define __ANWIMAGE_GDI_H_

#include <gdiplus.h>
//#include "GdiplusH.h"
#include "_defanwimageext.h"
using namespace Gdiplus;

void ai_DrawShadow( Graphics& gdc, CRect& rect, int shadow, int factor=5, COLORREF color=RGB(64,64,64) );

class ANWIMAGE_EXT CDrawObj
{
public:
	CDrawObj();
	~CDrawObj();

	void SetParam( int wFrame, CSize szEllip, CSize szOff=CSize(0,0), 
						COLORREF crHighlight=0xFFFFFF, COLORREF crShadow=0x808080 );
	void DrawLine( Graphics *pGraph, CRect& rect, COLORREF crHighlight, COLORREF crShadow,
				  int wFrame=1, CSize szOff=CSize(0,0) );

	void DrawGroupBox( Graphics *pGraph, CRect& rect, int wFrame,
					   CSize szEllip, COLORREF crHighlight, COLORREF crShadow,
					   CSize szOff = CSize(0,0) );
	inline void DrawLine( Graphics *pGraph, CRect& rect );
	inline void DrawGroupBox( Graphics *pGraph, CRect& rect );
	void DrawRoundRect(Graphics *pGraph, Pen& pen, CRect& rect, int nWidthEllipse, int nHeightEllipse );
	void DrawShadow( Graphics *pGraph, CRect& rect, int shadow, int factor=5, COLORREF color=RGB(64,64,64) );

	int m_wFrame;
	COLORREF m_crFrame;
	COLORREF m_crShadow;
	CSize m_ellip;
	CSize m_off;
};

inline void CDrawObj::DrawGroupBox( Graphics *pGraph, CRect& rect )
{	DrawGroupBox( pGraph, rect, m_wFrame, m_ellip, m_crFrame, m_crShadow, m_off );
}

inline void CDrawObj::DrawLine( Graphics* pGraph, CRect& rect )
{	DrawLine(pGraph, rect, m_crFrame, m_crShadow, m_wFrame, m_off);
}

#endif //__ANWIMAGE_GDI_H_