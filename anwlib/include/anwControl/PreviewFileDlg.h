/******************************************************************************
Copyright (c) 2000-2002 Advanced & Wise Technology Corp.  All Rights Reserved.
File Name:		PreviewFileDlg.h
Author:			Chen Min_Zon
Description:	provide a open or save dialog with preview,
Platform:		Win32
Compiler:		MS Visual C++ 6.0 in sp5 with Platform SDK Nov 2001.
Data:			2002/11/07 v0.1			min_zon start develop
******************************************************************************/
#if !defined(AFX_PREVIEWFILEDLG_H__14DEDBB7_0054_4A0A_8740_4E81B6717040__INCLUDED_)
#define AFX_PREVIEWFILEDLG_H__14DEDBB7_0054_4A0A_8740_4E81B6717040__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define IDD_MYOPENDLG		6000

#define kDefaultBufSize		(MAX_PATH*1024)//256k

// Uncomment following line if you are using this class outside the DLL
/*
#define _FILEDIALOGST_NODLL_
#ifndef _FILEDIALOGST_NODLL_
#ifndef	_CMLHTDLL_NOLIB_
	#ifdef _DEBUG
		#ifdef _UNICODE
			#pragma comment(lib, "CmlHTud.lib")
		#else
			#pragma comment(lib, "CmlHTd.lib")
		#endif
		#else
		#ifdef _UNICODE
			#pragma comment(lib, "CmlHTu.lib")
		#else
			#pragma comment(lib, "CmlHT.lib")
		#endif
	#endif
#endif

	#ifdef	_CMLHTDLL_BUILDDLL_
		#define	FILEDIALOGST_EXPORT	__declspec(dllexport)
	#else
		#define	FILEDIALOGST_EXPORT	__declspec(dllimport)
	#endif

#else
		#define	FILEDIALOGST_EXPORT
#endif
*/
//#define	FILEDIALOGST_EXPORT AFX_EXT_CLASS
#include "_defControlext.h"

class CONTROL_EXT CPreviewFileDlg  
{
public:
	
	void SetPreviewSize(int nWidth, int nHeight);
	inline const OPENFILENAME *GetOPENFILENAME(void);
	inline BOOL SetOPENFILENAME(OPENFILENAME *pofn);
	unsigned int GetSelectFileNum(void);
	int DoModal(void);

	CPreviewFileDlg( BOOL bOpenFileDialog,
					 LPCTSTR lpszDefExt = NULL,
					 LPCTSTR lpszFileName = NULL,
					 DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					 LPCTSTR lpszFilter = NULL,
					 CWnd *pParentWnd = NULL,
					 HINSTANCE hInstance = NULL
					);

	CPreviewFileDlg( BOOL bOpenFileDialog,
					 LPCTSTR lpszDefExt = NULL,
					 LPCTSTR lpszFileName = NULL,
					 LPCTSTR lpstrInitialDir = NULL,
					 DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					 LPCTSTR lpszFilter = NULL,
					 CWnd *pParentWnd = NULL,
					 HINSTANCE hInstance = NULL
					);

	CPreviewFileDlg();
	virtual ~CPreviewFileDlg();

	//max
	inline void SetFilterIndex(int nIndex);
	inline void GetFileTitle( TCHAR* szText ) const;
	inline void GetFileExt( TCHAR* szText ) const;
	void GetFileDir( TCHAR* szText ) const;
	inline void GetFileDrive( TCHAR* szText ) const;
	void GetNextPathName( POSITION& pos, TCHAR* szText ) const;

	inline void GetPathName( TCHAR* szText ) const;
	inline void GetFileName( TCHAR* szText ) const;
	inline POSITION GetStartPosition() const;

private:
	struct OPENFILENAMEEX : public OPENFILENAME 
	{ 
		void*	pvReserved;
		DWORD	dwReserved;
		DWORD	FlagsEx;
	};
	int		Show98PreviewDlg(void);
	BOOL	IsWin98Above(void);
	void	InitialMemberVar(void);
	void	ExtractSturct(void);
	void    OnInitailDone(HWND hdlg, LPOFNOTIFY pOfnNotify);
	void    OnSelectChange(HWND hdlg, LPOFNOTIFY pOfnNotify);
	BOOL	IsValidFile(TCHAR *szPath);
	void    ShowThumbnail( HWND hdlg, TCHAR *szPath,HWND m_WndPreview);

	static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	int		HandleNotify(HWND hdlg, WPARAM wParam, LPARAM lParam);

//	static DWORD WINAPI PollingThreadProc(LPVOID param);
	void GetListCtrlRect( HWND hdlg, LPRECT lpRect );

	void xShow(HDC hDC);
	HWND GetListCtrl( HWND hdlg );
	RECT GetPreviewRect( HWND hdlg );
	int Size( HWND hdlg );

public:
	TCHAR			m_pStrBuf[kDefaultBufSize];

private:
	OPENFILENAME	m_ofn;
	CWnd			*m_pParentWnd;
	unsigned int	m_SelectNum;
	BOOL			m_bIsOpenFileDialog;
	HINSTANCE		m_hAppInstance;

	TCHAR			m_szFileTitle[MAX_PATH];
	
	// 98 preview dlg
	int				m_98PreviewWidth;
	int				m_98PreviewHeight;
	HWND			m_WndPreview;

	HWND			m_dlg;
	CImage*			m_pThumb;

};

inline void CPreviewFileDlg::SetFilterIndex(int nIndex) 
{	m_ofn.nFilterIndex = nIndex;
}

inline const OPENFILENAME *CPreviewFileDlg::GetOPENFILENAME(void)
{
	return &m_ofn;
}

inline BOOL CPreviewFileDlg::SetOPENFILENAME(OPENFILENAME *pofn)
{
	if(pofn == NULL) return FALSE;

	::CopyMemory(&m_ofn, pofn, sizeof(OPENFILENAME));
	return TRUE;
}

inline void CPreviewFileDlg::GetFileTitle( TCHAR* szText ) const
{	_tsplitpath(m_ofn.lpstrFile, NULL, NULL, szText, NULL);
}

inline void CPreviewFileDlg::GetFileExt( TCHAR* szText ) const
{	_tsplitpath(m_ofn.lpstrFile, NULL, NULL, NULL, szText);
}

inline void CPreviewFileDlg::GetFileDrive( TCHAR* szText ) const
{	_tsplitpath(m_ofn.lpstrFile, szText, NULL, NULL, NULL);
} 

inline void CPreviewFileDlg::GetPathName( TCHAR* szText ) const
{	_tcscpy( szText, m_ofn.lpstrFile );
}

inline void CPreviewFileDlg::GetFileName( TCHAR* szText ) const
{	_tcscpy( szText, m_ofn.lpstrFileTitle );	
}

inline POSITION CPreviewFileDlg::GetStartPosition() const
{	return (POSITION)m_ofn.lpstrFile;	
}

#endif // !defined(AFX_PREVIEWFILEDLG_H__14DEDBB7_0054_4A0A_8740_4E81B6717040__INCLUDED_)
