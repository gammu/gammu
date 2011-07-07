#if !defined(AFX_BASEDLG_H__59EDBAF4_8A35_4472_90CA_A53065EF3EA2__INCLUDED_)
#define AFX_BASEDLG_H__59EDBAF4_8A35_4472_90CA_A53065EF3EA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// basedlg.h : header file
//
#include "..\Data Structure\CardPanel.h"
#include "..\Data Structure\SupportEntry.h"
typedef struct tagContactInfo{
	GSM_EntryType entryType;
	TCHAR	   	szData[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
}ContactInfo;

/////////////////////////////////////////////////////////////////////////////
// CBaseDlg dialog

class CBaseDlg : public CDialog
{
// Construction
public:
	CBaseDlg(UINT nIDTemplate,CWnd* pParent = NULL);   // standard constructor

	//set the default storage pos
	void SetIsME(int iFlag);
	//set data to eidt
	void SetData(CCardPanel *pData);
	//set mode for add,edit,or view
	void SetMode(int iMode);
	//set support column
	void SetSupportColumn(CSupportEntry &se){m_se = se;}

// Dialog Data
	//{{AFX_DATA(CBaseDlg)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual  xSetRadioBtn();
	COLORREF	m_crBg;
	int m_iMode;	// set dlg mode
	int m_iIsME;	//set default storage type
	CCardPanel	*m_pData;
	//a member variable to store 
	CSupportEntry m_se;
	//detain data
	DEQPHONEDATA m_deqDetainData;

	virtual void OnRdSim() {}
	virtual void OnRdMobile() {}
	// Generated message map functions
	//{{AFX_MSG(CBaseDlg)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEDLG_H__59EDBAF4_8A35_4472_90CA_A53065EF3EA2__INCLUDED_)
