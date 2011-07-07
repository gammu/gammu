// RadioEx2.h: interface for the CRadioEx2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RADIOEX2_H__6E4B0C79_F344_4066_B379_CB99B98E3AA8__INCLUDED_)
#define AFX_RADIOEX2_H__6E4B0C79_F344_4066_B379_CB99B98E3AA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "radioex.h"

class CONTROL_EXT CRadioEx2 : public CRadioEx  
{
public:
	CRadioEx2();
	virtual ~CRadioEx2();
	
	void SetTextBg(CImageArray *pImg){m_pImgTextBg = pImg;}
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckEx)
	protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL
protected:
	CImageArray *m_pImgTextBg;
};

#endif // !defined(AFX_RADIOEX2_H__6E4B0C79_F344_4066_B379_CB99B98E3AA8__INCLUDED_)
