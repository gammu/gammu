#if !defined(AFX_CONTACTSDLG_H__BD91C5A0_A20B_4151_A556_762C07A17A0E__INCLUDED_)
#define AFX_CONTACTSDLG_H__BD91C5A0_A20B_4151_A556_762C07A17A0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContactsDlg.h : header file
//

#include "ReportCtrl.h"
#include "AviDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CContactsDlg dialog
#define	PhoneBook_BTN_NUM 4
#define PhoneBook_BN_OFFSET	14

class CContactsDlg : public CDialog
{
// Construction
public:
	CContactsDlg(CWnd* pParent = NULL);   // standard constructor
	~CContactsDlg();
	HFONT	  m_hFont;

	static		CContactsDlg* s_ActiveDlg;
	static DWORD WINAPI TransferProc(LPVOID lpParam);
	//CAviDlg *Avidlg;

// Dialog Data
	//{{AFX_DATA(CContactsDlg)
	enum { IDD = IDD_DLG_CONTACTS };
	CRescaleButton	m_BTN_OK;
	CRescaleButton	m_BTN_Cancel;
	CRescaleButton	m_BTN_Delete;
	CRescaleButton	m_BTN_Add;
	//CListCtrl		m_LIST_SeletePhone;
	//CListCtrl		m_LIST_Contacts;
	CReportCtrl		m_LIST_Contacts;
	CReportCtrl		m_LIST_SeletePhone;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRescaleButton	*m_pButton[PhoneBook_BTN_NUM];

	// Generated message map functions
	//{{AFX_MSG(CContactsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnClickListContacts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickListSelphone(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	void		LoadControl(LPCTSTR pSkinPath);
	void		InitValue(void);
	void		InitString(void);

	void		ShowPhoneData(void);
	void		InitSelectPhoneList(void);
	void		InitAllListStates(void);

	bool		InitialLibrary(void);
    void        FreeLibrary(HMODULE m_hDriver);
	int			OpenMobileConnect(void);
	int			ConnectStatusCallBack(int State);
	int			GetMobileSetting(void);
	int			GetPBInfo(void);
	int			GetPBData(char *strMemType, int nMemtype, int nPos, bool bStart);
	int			GetPBOneData(char *strMemType, int nMemtype, int nPos, bool bStart);
	BOOL		InitImageList();
	int			ParserPBData(GSM_MemoryEntry entryList, ContactList *pContactInfo);


	HMODULE		m_hMobileDll;
	TCHAR		m_szAppPath[_MAX_PATH];
	HBRUSH		m_hBrush;

	int			m_nSelectPhoneCount;
//	CString		*m_PhoneList;
	CStringList *m_PhoneList;

	bool		m_bStart;
	int			m_nMemFlag;

//	int			m_nSIMUsedNum, m_nMEUsedNum;
	int			m_TotalItems;

	TCHAR		m_strPort[MAX_PATH], m_strPhone[MAX_PATH], m_strConnectMode[MAX_PATH];
	TCHAR		m_strMobileName[MAX_PATH], m_strCompany[MAX_PATH], m_strIMEI[MAX_PATH];
	TCHAR		m_strMobileDLL[MAX_PATH];

	CImageList	m_cImageListSmall, m_cImageListNormal;//, m_cImageListState;

public:
//	ContactList	*m_pContactInfo;
	CPtrList    m_ContactInfoList;
	int			m_nSIMUsedNum, m_nMEUsedNum;
	int			m_nPos;
	int			m_nNextPost;
//	void		SetData(ContactList	*pContactInfo);
	void		SetPhoneNumer(CStringList *PhoneList);
	void		GetPhoneNumer(void);
	static	int	GetAllPBData(int nNeedCount);
	static	int	xDownloadPB(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTSDLG_H__BD91C5A0_A20B_4151_A556_762C07A17A0E__INCLUDED_)
