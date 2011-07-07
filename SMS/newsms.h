#if !defined(AFX_NEWSMS_H__44370FB0_98B4_42EC_A506_A49FD3CC5A87__INCLUDED_)
#define AFX_NEWSMS_H__44370FB0_98B4_42EC_A506_A49FD3CC5A87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewSMS.h : header file
//

#include "AutoLayout.h"
#include "PhoneNumberEdit.h"
#include "ContentEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CNewSMS dialog
#define TOOLBAR_BTN_NUM 4
#define SSD_BN_OFFSET	14

class CNewSMS : public CDialog
{
// Construction
public:
	CNewSMS(CWnd* pParent = NULL);   // standard constructor
	HFONT	  m_hFont;
	~CNewSMS();

// Dialog Data
	//{{AFX_DATA(CNewSMS)
	enum { IDD = IDD_DLG_NEWSMS };
	CRescaleButton	m_BTN_Save2Draft;
	CRescaleButton	m_BTN_Send;
	CRescaleButton	m_BTN_Cancel;
	CRescaleButton	m_bt_Contacts;
	CStaticEx		m_ST_Receiver;
	CStaticEx		m_ST_MsgContent;
	CStaticEx		m_ST_Counts;
	CContentEdit		m_EDIT_Contents;
	CPhoneNumberEdit	m_EDIT_ContactsList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewSMS)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRescaleButton	*m_pButton[TOOLBAR_BTN_NUM];

	// Generated message map functions
	//{{AFX_MSG(CNewSMS)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnBtnContacts();
	afx_msg void OnChangeEditContents();
	afx_msg void OnBtnSend();
	afx_msg void OnChangeEditContactslist();
	afx_msg void OnBtnDraft();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMaxtextEditContactslist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	LoadControl(LPCTSTR pSkinPath);
	void	InitValue();
	void	InitString(void);
	
	int		HasHighByte(CString pstrString);
	int		UnicodeString(CString pstrString);
//	void	MultiByteCut(LPCSTR lpszSrc, LPCSTR lpszDes, int nPage);
//	void	SingleByteCut(LPCSTR lpszSrc, LPCSTR lpszDes, int nPage);
	int		InitializeFromString(LPCSTR pstrString);
	void	GetPhoneNumberList(void);

	TCHAR	m_szAppPath[_MAX_PATH];
	HBRUSH	m_hBrush;
	TCHAR	m_strGetMsg[MAX_CONTENT+2];

	bool	m_bMultiByte;
	int		m_nPage;
//	CString m_aryPhoneList[MAX_CONTENT+2];
	CStringList m_aryPhoneList;

	CString	m_strContent;
	CString	m_strPhone;

	int		m_nReply;
	bool	m_bFirstCall;
	bool    m_bIsSend;

public:
	void		SetData(SMS_PARAM msgInfo, int nReply, bool bFirstCall,bool bIsSend);
	//void		SetPhoneNumer(CString *PhoneList);
	//void		GetPhoneNumer(void);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWSMS_H__44370FB0_98B4_42EC_A506_A49FD3CC5A87__INCLUDED_)
