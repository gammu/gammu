// ProgressCtrlEx2.h: interface for the CProgressCtrlEx2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROGRESSCTRLEX2_H__5AB1F283_8212_4A8B_A5F7_1866A3C5848D__INCLUDED_)
#define AFX_PROGRESSCTRLEX2_H__5AB1F283_8212_4A8B_A5F7_1866A3C5848D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "progressctrlex.h"

#define PBS_SHOW_PERCENT         0x0100
#define PBS_SHOW_POSITION        0x0200
#define PBS_SHOW_TEXTONLY        0x0300
#define PBS_TEXTMASK             0x0300

class CONTROL_EXT CProgressCtrlEx2 : public CProgressCtrlEx  
{
public:
	CProgressCtrlEx2();
	virtual ~CProgressCtrlEx2();

	// show number
	inline void SetShowNumber(BOOL fShowNumber = TRUE);
	inline BOOL GetShowNumber();
protected:
	virtual void DrawText(const CDrawInfo& info, const CRect &rcMax, const CRect &rcGrad);
};

inline void CProgressCtrlEx2::SetShowNumber(BOOL fShowNumber)
{	
	SetTextFormat(fShowNumber ? _T("%d / %d") : NULL, PBS_SHOW_POSITION); 
}

inline BOOL CProgressCtrlEx2::GetShowNumber() 
{	return GetStyle()&PBS_SHOW_POSITION; 
}

#endif // !defined(AFX_PROGRESSCTRLEX2_H__5AB1F283_8212_4A8B_A5F7_1866A3C5848D__INCLUDED_)
