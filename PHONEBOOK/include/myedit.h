#if !defined(AFX_MYEDIT_H__F1AC5312_28F3_4398_9992_CF36CE515A66__INCLUDED_)
#define AFX_MYEDIT_H__F1AC5312_28F3_4398_9992_CF36CE515A66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// myedit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyEdit window

class CMyEdit : public CEdit
{
// Construction
public:
	CMyEdit();

// Attributes
public:

// Operations
public:
	void SetNumFlag(bool bFlag){m_bNumFlag = bFlag;}
	void SetAddressFlag(bool bFlag){m_bAddressFlag = bFlag;}
	bool GetNumFlag(){return m_bNumFlag;}
	bool m_bAddressFlag;	//check ';' 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyEdit();

	// Generated message map functions
protected:
	bool m_bNumFlag;
	//{{AFX_MSG(CMyEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYEDIT_H__F1AC5312_28F3_4398_9992_CF36CE515A66__INCLUDED_)
