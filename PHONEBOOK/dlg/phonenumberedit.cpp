// PhoneNumberEdit.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "PhoneNumberEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhoneNumberEdit

CPhoneNumberEdit::CPhoneNumberEdit()
{
}

CPhoneNumberEdit::~CPhoneNumberEdit()
{
}


BEGIN_MESSAGE_MAP(CPhoneNumberEdit, CEdit)
	//{{AFX_MSG_MAP(CPhoneNumberEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneNumberEdit message handlers

void CPhoneNumberEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == 8)
		CEdit::OnChar(nChar, nRepCnt, nFlags);

	POINT caret;
	::GetCaretPos (&caret);
	caret.x = LOWORD (CharFromPos (caret));

	CString text;
	GetWindowText (text);

	if (isdigit(nChar) || nChar == ';') 
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	
	//CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CPhoneNumberEdit::SetValue(double val)
{
	CString tmp;
	tmp.Format (_T("%G"),val);
	SetWindowText (tmp);
}

double CPhoneNumberEdit::GetValue()
{
	CString tmp;
	GetWindowText (tmp);
	return _tcstod (tmp,NULL);
}

