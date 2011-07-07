// TabButtonEx2.h: interface for the CTabButtonEx2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABBUTTONEX2_H__96E7D46F_7978_4352_8020_F63922F0E59B__INCLUDED_)
#define AFX_TABBUTTONEX2_H__96E7D46F_7978_4352_8020_F63922F0E59B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTabButtonEx2 : public CTabButtonEx  
{
public:
	CTabButtonEx2();
	virtual ~CTabButtonEx2();
	void SetSelectedGrayTextColor(COLORREF crSelectedGray){ m_crSGText = crSelectedGray;}
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabButtonEx)
	protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

protected:
	COLORREF	m_crSGText;
	//{{AFX_MSG(CTabButtonEx2)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_TABBUTTONEX2_H__96E7D46F_7978_4352_8020_F63922F0E59B__INCLUDED_)
