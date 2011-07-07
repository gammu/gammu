
#include <math.h>
#include <gdiplus.h>
//#include "GdiplusH.h"
using namespace Gdiplus;

#define mid(a,b,c)	( (a)>= (b) && (a)>=(c) ? (a):((b)>=(a) && (b)<=(c)?(b):(c)) )
#define SQR(a)		( (a)*(a) )
#define SQR2(a,b)   ( (double)(SQR(a)+SQR(b)) )
#define ROUND(a)	( (a)>0 ? (int)((a)+0.5) : (int)((a)-0.5) )
#define ROUNDX(a)	( (a)>0 ? (int)(((long)(a)+5000L)/10000L) : (int)(((long)(a)-5000L)/10000L) )
#define DTOR    (double) 0.0174532                // pi/180

//int modAngle( int Angle, int base = 360 );
//float modAngleF( float Angle, float fbase = 360.0 );

inline int ai_modAngle( int Angle, int base = 360 )
{	
	if( Angle >= 0 && Angle < 360 )
		return Angle;
	else
		return ( Angle >= 0 )? (Angle%base) : base + (Angle%-base);
}

inline float ai_modAngleF( float Angle, float fbase  = 360.0)
{	
	if( Angle >= 0.0 && Angle < 360.0 )
		return Angle;
	else
		return (float)(( Angle >= 0.0 )? fmod( Angle, fbase ) : fbase + fmod( Angle, -fbase ));
}

void ai_RotatePt( LPPOINT lpPt, int nCount, POINT ptCenter, float Angle );
void ai_RotatePtF( LPPOINT lpPt, int nCount, PointF ptCenter, float Angle );
void ai_GetVPt2Rect( CPoint* pPt, CRect& rect );
void ai_GetRect2VPt( CRect& rect, CPoint* pPt );
void ai_RotateRect( CRect& rect, float Angle, LPPOINT lpPtCenter = NULL );
void ai_RotateRectF( CRect& rect, float Angle, LPPOINT lpPtCenter = NULL );

float ai_GetRotateAngleF( CPoint point, PointF ptCenter );
inline float ai_GetRotateAngle( CPoint point, CPoint ptCenter )
{	return ai_GetRotateAngleF( point, PointF( (REAL)ptCenter.x, (REAL)ptCenter.y) );
};