// PhBoundary.h: interface for the CPhBoundary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHBOUNDARY_H__AE0A3D53_DD6D_4788_9966_826B1DA05F7A__INCLUDED_)
#define AFX_PHBOUNDARY_H__AE0A3D53_DD6D_4788_9966_826B1DA05F7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPhBoundary  
{
public:
	//Construction
	CPhBoundary(const PHONE_BOUNDARY &pb);
	CPhBoundary();
	CPhBoundary(const CPhBoundary &pb);
	//Destruction
	virtual ~CPhBoundary();
	//operator
	const CPhBoundary& operator =(const CPhBoundary &pb);
	const CPhBoundary& operator =(const PHONE_BOUNDARY &pb);
	//get data
	int GetType() {return m_iType;}
	int GetBoundary() {return m_iBoundary;}
	//set data
	void SetType(int iType) {m_iType = iType;}
	void SetBoundary(int iBoundary) {m_iBoundary = iBoundary;}
protected:
	int m_iType;
	int m_iBoundary;
};

#endif // !defined(AFX_PHBOUNDARY_H__AE0A3D53_DD6D_4788_9966_826B1DA05F7A__INCLUDED_)
