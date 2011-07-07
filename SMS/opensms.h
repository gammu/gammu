#if !defined(AFX_OPENSMS_H__DAEB6D06_5B0D_47A4_AEF3_C03C629E9758__INCLUDED_)
#define AFX_OPENSMS_H__DAEB6D06_5B0D_47A4_AEF3_C03C629E9758__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpenSMS.h : header file
//

#define ITEM_BTN_NUM 4

/////////////////////////////////////////////////////////////////////////////
// COpenSMS dialog

class COpenSMS : public CDialog
{
// Construction
public:
	COpenSMS(CWnd* pParent = NULL);   // standard constructor
	HFONT	  m_hFont;
	~COpenSMS();

// Dialog Data
	//{{AFX_DATA(COpenSMS)
	enum { IDD = IDD_DLG_OPENMSG };
	CRescaleButton	m_BTN_Next;
	CRescaleButton	m_BTN_Reply;
	CRescaleButton	m_BTN_Prev;
	CRescaleButton	m_BTN_Cancel;
	CEdit			m_EDIT_Content;
	CEdit			m_EDIT_Sender;
	CEdit			m_EDIT_Time;
	CStaticEx		m_ST_MsgContent;
	CStaticEx		m_ST_Sender;
	CStaticEx		m_ST_Time;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenSMS)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRescaleButton	*m_pButton[ITEM_BTN_NUM];

	// Generated message map functions
	//{{AFX_MSG(COpenSMS)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnBtnPrev();
	afx_msg void OnBtnNext();
	afx_msg void OnBtnReply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	LoadControl(LPCTSTR pSkinPath);
	void	InitValue();
	void	InitString(void);

	


	TCHAR	m_szAppPath[_MAX_PATH];
	HBRUSH	m_hBrush;

	SMS_PARAM	*m_MsgInfo;
	int		m_nCurrentItem;
	int		m_MaxItems;
	bool    m_bSender;


public:
	void    SetData(SMS_PARAM *msgInfo, int nCurrentItem, int nMaxItems,bool bSender = true);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENSMS_H__DAEB6D06_5B0D_47A4_AEF3_C03C629E9758__INCLUDED_)
