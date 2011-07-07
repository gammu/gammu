#if !defined(AFX_STATICEX2_H__46E97388_0ACC_4EC9_85AF_FDAD42CCB88C__INCLUDED_)
#define AFX_STATICEX2_H__46E97388_0ACC_4EC9_85AF_FDAD42CCB88C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticEx2.h : header file
//
#include "staticex.h"
/////////////////////////////////////////////////////////////////////////////
// CStaticEx2 window

class CONTROL_EXT CStaticEx2 : public CStaticEx
{
	DECLARE_DYNAMIC(CStaticEx2);

// Construction
public:
	CStaticEx2();

// Attributes
public:

	UINT m_nCount;	//Image Count
	UINT m_nIndex;

// Operations
public:
	void SetIndex(UINT nIndex)
			{	m_nIndex = min(m_nCount,max(nIndex,0));
				Invalidate();
			}
	BOOL LoadBitmap(LPCTSTR pszFileName, UINT nCount=1);
	BOOL LoadBitmap(CImageArray* pImage, BOOL bSafeDel);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticEx2)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticEx2();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticEx2)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICEX2_H__46E97388_0ACC_4EC9_85AF_FDAD42CCB88C__INCLUDED_)
