//
//	gdiwrap.g
//

#ifndef _GDIWRAP_H
#define _GDIWRAP_H

#include <gdiplus.h>
//#include "GdiplusH.h"
#include "_defanwimageext.h"

using namespace Gdiplus;

#pragma warning(disable : 4244)

///////////////////////////////////////////////////////////////////////////////
//
//	CXPoint : PointF wrapper
//
class ANWIMAGE_EXT CXPoint : public PointF
{
public:
	// constructor
	CXPoint( Point pt )		{	CXPoint( (REAL)pt.X, (REAL)pt.Y );	}
	CXPoint( CPoint pt )	{	CXPoint( (REAL)pt.x, (REAL)pt.y );	}
	CXPoint( int x, int y)	{	CXPoint( (REAL)x, (REAL)y );		}

	operator CPoint()		{	return CPoint( (int)X, (int)Y );	}
	operator Point()		{	return Point( (INT)X, (INT)Y );		}
};

///////////////////////////////////////////////////////////////////////////////
//
//	CXSize : SizeF wrapper
//
class ANWIMAGE_EXT CXSize : public SizeF
{
public:
	// constructor
	CXSize( Size size )				{	CXSize( (REAL)size.Width, (REAL)size.Height );	}
	CXSize( CSize size )			{	CXSize( (REAL)size.cx, (REAL)size.cy);	}
	CXSize( int width, int height )	{	CXSize( (REAL)width, (REAL)height);	}

	operator CSize()				{	return CSize( (int)Width, (int)Height);	}
	operator Size()					{	return Size( (INT)Width, (INT)Height);	}
};

///////////////////////////////////////////////////////////////////////////////
//
//	CXRect : RectF wrapper
//
class ANWIMAGE_EXT CXRect : public RectF
{
public:
	// constructor
	CXRect( Rect rc )						{	xSet( rc );	};
	CXRect( RectF rc )						{	xSet( rc );	};
	CXRect( CRect rc )						{	xSet( rc );	};
	CXRect( CPoint point, CSize size )		{	xSet( point, size );	};
	CXRect( Point point, Size size )		{	xSet( point, size );	};
	CXRect( int nLeft, int nTop, int nWidth, int nHeight )		{	xSet( nLeft, nTop, nWidth, nHeight );	};
//	CXRect( REAL rLeft, REAL rTop, REAL rRight, REAL rBottom )	{	xSet( rLeft, rTop, rRight, rBottom );	};

	int		nWidth()		{ return (int)Width;  };
	int		nHeight()		{ return (int)Height;  };
	operator CRect()		{ return CRect( (int)X, (int)Y, (int)GetRight(), (int)GetBottom() );	};
	operator Rect()			{ return Rect( (int)X, (int)Y, (int)Width, (int)Height );	};

private:

/*	CXRect& xSet( REAL rLeft, REAL rTop, REAL rRight, REAL rBottom )
			{	X = rLeft;	Y = rTop;
				Width = (rRight>rLeft)?(rRight - rLeft):(rLeft - rRight)  );
				Height= (rBottom>rTop)?(rBottom - rTop):(rTop  - rBottom) );
				return *this;
			};
*/
	CXRect& xSet( int nLeft, int nTop, int nWidth, int nHeight )
			{	X = (REAL)nLeft;		Y = (REAL)nTop;
				Width = (REAL)nWidth;	Height= (REAL)nHeight;
				return *this;
			};
	CXRect& xSet( Rect rc )		{	return xSet( rc.X, rc.Y, rc.Width, rc.Height );	};
	CXRect& xSet( RectF rc )	{	return xSet( rc.X, rc.Y, rc.Width, rc.Height );	};
	CXRect& xSet( CXRect& rc )	{	return xSet( rc.X, rc.Y, rc.Width, rc.Height );	};
	CXRect& xSet( CRect rc )	{	return xSet( rc.left, rc.top, rc.Width(), rc.Height() );	};

	CXRect& xSet( Point pt, Size size )		{	return xSet( Rect( pt, size ) );	};		
	CXRect& xSet( CPoint pt, CSize size )	{	return xSet( CRect( pt, size ) );	};
};

///////////////////////////////////////////////////////////////////////////////
//
//	CFontFamily : FontFamily wrapper
//
class ANWIMAGE_EXT CFontFamily : public FontFamily
{	
public:
	// constructor
	CFontFamily( const char* name, const FontCollection* fontCollection );
};

#pragma warning(default : 4244)

#endif //_GDIWRAP_H
