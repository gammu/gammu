#if !defined(AFX_OPENREPEATQUESTDLG_H__5CA140F6_1326_40F6_865F_2D3D2D348D78__INCLUDED_)
#define AFX_OPENREPEATQUESTDLG_H__5CA140F6_1326_40F6_865F_2D3D2D348D78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpenrepeatQuestDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpenrepeatQuestDlg dialog

class COpenrepeatQuestDlg : public CDialog
{
// Construction
public:
	void xUpdateControl();
	void SetSubject(CString str);
	COpenrepeatQuestDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL m_bOpenCurrent;
	CString m_strSubject;
	~COpenrepeatQuestDlg();
	HBRUSH    m_hBrush;
// Dialog Data
	//{{AFX_DATA(COpenrepeatQuestDlg)
	enum { IDD = IDD_OPENREPEATQUESTDLG };
	CStaticEx	m_stMsg;
	CRescaleButton	m_btOK;
	CRescaleButton	m_btCancel;
	CRadioEx        m_rbOpenCurent;
	CRadioEx        m_rbOpenSerial;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenrepeatQuestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COpenrepeatQuestDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnOpencurrentradio();
	afx_msg void OnOpenserialradio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void xUpdateText();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENREPEATQUESTDLG_H__5CA140F6_1326_40F6_865F_2D3D2D348D78__INCLUDED_)
