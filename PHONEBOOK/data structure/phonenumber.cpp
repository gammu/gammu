// PhoneNumber.cpp: implementation of the CPhoneNumber class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\phonebook.h"
#include "PhoneNumber.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPhoneNumber::CPhoneNumber()
{
	m_iType = 0;
}

CPhoneNumber::~CPhoneNumber()
{

}

void CPhoneNumber::SetType(int iType)
{
	m_iType = iType;
	switch(m_iType){
	case PBK_Number_General:
		break;
	case PBK_Number_Mobile:
		break;
	case PBK_Number_Work:
		break;
	}
}
