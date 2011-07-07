// CSVHandle.h: interface for the CCSVHandle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CSVHANDLE_H__51D8E1D6_4D89_477E_92F1_645B6C161506__INCLUDED_)
#define AFX_CSVHANDLE_H__51D8E1D6_4D89_477E_92F1_645B6C161506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Data Structure\CardPanel.h"

class CCSVHandle  
{
public:
	//write to file
	bool WriteHandle(deque<Tstring> &deqHandle);
	bool WriteContact(DATALIST &deqContact);
	bool WriterDescription();
	//get from file
	bool CheckFormat();
	bool GetHandle(deque<Tstring> &deqHandle);
	bool GetContact(DATALIST &deqContact);
	
	void SetFilePath(LPCTSTR lpszFile);
	CCSVHandle(LPCTSTR lpszFile);
	CCSVHandle();
	virtual ~CCSVHandle();
	
protected:
	Tstring m_sFile;	//store the file path
};

#endif // !defined(AFX_CSVHANDLE_H__51D8E1D6_4D89_477E_92F1_645B6C161506__INCLUDED_)
