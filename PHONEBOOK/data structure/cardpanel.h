// CardPanel.h: interface for the CCardPanel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARDPANEL_H__813DD8D3_AFFF_4202_88FE_D4EBF30809EB__INCLUDED_)
#define AFX_CARDPANEL_H__813DD8D3_AFFF_4202_88FE_D4EBF30809EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ListData.h"

#define THUMB_WIDTH		208
#define THUMB_HEIGHT	126
#define THUMB_SHADOW	3

class CCardPanel : public CListData
{
public:
	void SetSelect(bool bSel);
//	CImage * GetImgBuffer();
	void init();
	bool GetSel(){return m_bSel;}
	virtual void Release();

public:
	bool GetDrawImage(CImage **pImgBuffer);
	static void ClearShareImage();
	bool GetSIMForamt(CCardPanel &data);
//	void ResetImgBuffer();
	CCardPanel();
	virtual ~CCardPanel();

	//operator
	const CCardPanel& operator=(const CCardPanel& cp)
	{
		//operator by ListData 
		CListData::operator =(static_cast<CListData>(cp));
		//operator by CCardPanel`
		m_bSel = cp.m_bSel;

		return *this;
	}

	const CCardPanel& operator=(const GSM_MemoryEntry &Entry)
	{
		CListData::operator =(Entry);
		return *this;
	}

	const bool operator<(const CCardPanel& cp)
	{
		return m_iMemoryType < cp.m_iMemoryType;
	}
protected:
	bool xDrawBuffer(CImage **pImgBuffer);

	static CImage *m_pImgCardNormal;
	static CImage *m_pImgCardSelect;
	static int m_siCount;
//	CImage *m_pImgBuffer;
	bool m_bSel;
};

#endif // !defined(AFX_CARDVIEWPANEL_H__813DD8D3_AFFF_4202_88FE_D4EBF30809EB__INCLUDED_)
