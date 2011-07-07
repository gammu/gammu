// ContentEdit.cpp : implementation file
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "ContentEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhoneNumberEdit

CContentEdit::CContentEdit()
{
}

CContentEdit::~CContentEdit()
{
}


BEGIN_MESSAGE_MAP(CContentEdit, CEdit)
	//{{AFX_MSG_MAP(CContentEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneNumberEdit message handlers

void CContentEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar ==0x0D)//081031libaoliu Enter 
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	if (nChar == 8)
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	else
	{
		POINT caret;
		::GetCaretPos (&caret);
		caret.x = LOWORD (CharFromPos (caret));

		CString text;
		GetWindowText (text);

		if (nChar != 0x0d) 
			CEdit::OnChar(nChar, nRepCnt, nFlags);
	
	}
	

	
	//CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CContentEdit::SetValue(double val)
{
	CString tmp;
	tmp.Format (_T("%G"),val);
	SetWindowText (tmp);
}

double CContentEdit::GetValue()
{
	CString tmp;
	GetWindowText (tmp);
	return _tcstod (tmp,NULL);
}

