#if !defined(AFX_DIALOGBAREX_H__5A195653_A6A4_4E7D_8586_980730BDAE02__INCLUDED_)
#define AFX_DIALOGBAREX_H__5A195653_A6A4_4E7D_8586_980730BDAE02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogBarEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogBarEx dialog

class CDialogBarEx : public CDialogBar
{
// Construction
public:
	CDialogBarEx();   // standard constructor
	void SetImgBk(LPCTSTR lpszPath, bool bUseBK = false);

// Dialog Data
	//{{AFX_DATA(CDialogBarEx)
	enum { IDD = IDD_DLGBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogBarEx)
public:	
	virtual BOOL Create(CWnd *pWnd, UINT nIDtem, UINT nStyle, UINT nID, CBitmap *pBkBmp, COLORREF bkColor);
protected:	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogBarEx)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//afx_msg LRESULT OnShowText(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

private:
	CBitmap *m_pBkBmp;
	COLORREF m_bkColor;
	CImage *m_pImgBk;		//±³¾°Í¼Æ¬
	bool m_bUseImgBk;		//Ê¹ÓÃ±³¾°Í¼Æ¬µÄ±êÖ¾
	CString m_Title;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGBAREX_H__5A195653_A6A4_4E7D_8586_980730BDAE02__INCLUDED_)
