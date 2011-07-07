// basedlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "basedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseDlg dialog


CBaseDlg::CBaseDlg(UINT nIDTemplate,CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	m_iMode = OTHER;
	m_pData = NULL;
	m_iIsME = -1;
	//{{AFX_DATA_INIT(CBaseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBaseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBaseDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBaseDlg, CDialog)
	//{{AFX_MSG_MAP(CBaseDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CBaseDlg::SetMode(int iMode)
{
	m_iMode = iMode;
}

void CBaseDlg::SetData(CCardPanel *pData)
{
	m_pData = pData;
}

void CBaseDlg::SetIsME(int iFlag)
{
	m_iIsME = iFlag;
}

/////////////////////////////////////////////////////////////////////////////
// CBaseDlg message handlers

CBaseDlg::xSetRadioBtn()
{
	if(m_iIsME == -1){
		if(strcmp(m_pData->GetStorageType(),SIM_NAME) == 0){
			OnRdSim();
		}
		else{
			OnRdMobile();
		}
	}
	else if(m_iIsME == FALSE){
		OnRdSim();
	}
	else if(m_iIsME == TRUE){
		OnRdMobile();
	}

}

void CBaseDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
}
