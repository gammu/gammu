#if !defined(AFX_CONTENTEDIT_H__70BA67B3_4EC9_4622_9129_84F3667AE88C__INCLUDED_)
#define AFX_CONTENTEDIT_H__70BA67B3_4EC9_4622_9129_84F3667AE88C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContentEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPhoneNumberEdit window

class CContentEdit : public CEdit
{
// Construction
public:
	CContentEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ContentEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	double GetValue();
	void SetValue (double val);
	virtual ~CContentEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(ContentEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONENUMBEREDIT_H__70BA67B3_4EC9_4622_9129_84F3667AE88C__INCLUDED_)
