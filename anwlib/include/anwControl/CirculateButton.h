#if !defined(AFX_CIRCULATEBUTTON_H__DA961638_195F_4EF7_A536_3E1650CC1B7B__INCLUDED_)
#define AFX_CIRCULATEBUTTON_H__DA961638_195F_4EF7_A536_3E1650CC1B7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlphaButton.h : header file
//
#include "AlphaButton.h"
#include <vector>
/////////////////////////////////////////////////////////////////////////////
// CCirculateButton window

class CONTROL_EXT CCirculateButton : public CAlphaButton
{
	DECLARE_DYNAMIC(CCirculateButton);
// Construction
public:
	CCirculateButton();
	virtual ~CCirculateButton();

// Attributes
public:
	void SetRepeatNum( int nNormal, int nDown, int nShine, int nDisable);
	void RebuildImgArray();
	virtual BOOL LoadBitmap( LPCTSTR pszFileName );
	static void FillColorRect(CImage* pImg, Color& color, CRect rect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCirculateButton)
	//}}AFX_VIRTUAL

	// Timer function and variable
public:
	void SetCirculateTime( UINT nTime );
	static void SetAllCirculateTime( UINT nTime );
	static void __stdcall TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
protected:
	static std::vector<CCirculateButton*>	s_vObjectList;
	static int s_nNormalCount;
	static int s_nDownCount;
	static int s_nShineCount;
	static int s_nDisableCount;
	static UINT s_uElapseTime;
	static BOOL	s_bTimerStart;
	static UINT_PTR s_uTimerID;

protected:
	int m_nNormalCount;
	int m_nDownCount;
	int m_nShineCount;
	int m_nDisableCount;

	int m_nLastNormalCount;
	int m_nLastDownCount;
	int m_nLastShineCount;
	int m_nLastDisableCount;

	UINT m_uElapseTime;


protected:
	void xCreateFirstArray();
	void xMakeImageArray();

	CImage*	m_pCirculateImg;
	BOOL	m_bSetBitmap;
	BOOL	m_bSetRepeat;

	int m_nNormal;
	int m_nDown;
	int m_nShine;
	int m_nDisable;

	//{{AFX_MSG(CCirculateButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CIRCULATEBUTTON_H__DA961638_195F_4EF7_A536_3E1650CC1B7B__INCLUDED_)
