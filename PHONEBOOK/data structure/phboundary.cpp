// PhBoundary.cpp: implementation of the CPhBoundary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\phonebook.h"
#include "PhBoundary.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPhBoundary::CPhBoundary()
{
}

CPhBoundary::CPhBoundary(const CPhBoundary &pb):m_iType(pb.m_iType),m_iBoundary(pb.m_iBoundary)
{
}

CPhBoundary::CPhBoundary(const PHONE_BOUNDARY &pb):m_iType(pb.Phone_Type),m_iBoundary(pb.Boundary)
{
}

CPhBoundary::~CPhBoundary()
{

}

const CPhBoundary& CPhBoundary::operator =(const CPhBoundary &pb)
{
	m_iType = pb.m_iType;
	m_iBoundary = pb.m_iBoundary;
	return *this;
}

const CPhBoundary& CPhBoundary::operator =(const PHONE_BOUNDARY &pb)
{
	m_iType = pb.Phone_Type;
	m_iBoundary = pb.Boundary;
	return *this;
}
