/*
This SortClass was written by Iuri Apollonio (Excellent job)
Modified for Sorting Date and Incorporated 
	in this project by Barretto VN
*/

#include "stdafx.h"
#include "SortClass.h"

CString dataType;
/////////////////////////////////////////////////////////////////////////////
// CSortClass
CSortClass::CSortClass(CListCtrl * _pWnd, const int _iCol, const CString _DataType) //bool _bIsNumeric )
{	
	iCol = _iCol;	
	pWnd = _pWnd;	
//	bIsNumeric = _bIsNumeric;		
	dataType = _DataType;
	ASSERT(pWnd);
	int max = pWnd->GetItemCount();	
	DWORD dw;	
	CString txt;	
	
//	else
	if(dataType == "C")
	{
		for (int t = 0; t < max; t++)	
		{			
			dw = pWnd->GetItemData(t);
			txt = pWnd->GetItemText(t, iCol);
			pWnd->SetItemData(t, (DWORD) new CSortItem(dw, txt));
		}
		return;
	}

	if(dataType == "D")
	{
		for (int t = 0; t < max; t++)	
		{			
			dw = pWnd->GetItemData(t);
			txt = pWnd->GetItemText(t, iCol);
			pWnd->SetItemData(t, (DWORD) new CSortItemDate(dw, txt));
		}
		return;
	}

	if (dataType == "N" ) //(bIsNumeric)	
	{
		for (int t = 0; t < max; t++)		
		{			
			dw = pWnd->GetItemData(t);
			txt = pWnd->GetItemText(t, iCol);
			pWnd->SetItemData(t, (DWORD) new CSortItemInt(dw, txt));		
		}	
		return;
	}	

}

CSortClass::~CSortClass()
{	
	ASSERT(pWnd);
	int max = pWnd->GetItemCount();

	if(dataType == "C")
	{
		CSortItem * pItem;
		for (int t = 0; t < max; t++)
		{
			pItem = (CSortItem *) pWnd->GetItemData(t);
			ASSERT(pItem);
			pWnd->SetItemData(t, pItem->dw);
			delete pItem;
		}
		return;
	}

	if(dataType == "D")
	{
		CSortItemDate * pItem;
		for (int t = 0; t < max; t++)
		{
			pItem = (CSortItemDate *) pWnd->GetItemData(t);
			ASSERT(pItem);
			pWnd->SetItemData(t, pItem->dw);
			delete pItem;
		}
		return;
	}

	if(dataType == "N")
	{		
		CSortItemInt * pItem;		
		for (int t = 0; t < max; t++)	
		{
			pItem = (CSortItemInt *) pWnd->GetItemData(t);			ASSERT(pItem);
			pWnd->SetItemData(t, pItem->dw);	
			delete pItem;	
		}
		return;
	}	
//	else
}

void CSortClass::Sort(const bool bAsc)
{	
//	if (bIsNumeric)	
	if(dataType == "D")
	{		
		if (bAsc)
			pWnd->SortItems(CompareAscI, 0L);
		else	
			pWnd->SortItems(CompareDesI, 0L);

		return;
	}
//	else
	if(dataType == "C")
	{	
		if (bAsc)	
			pWnd->SortItems(CompareAsc, 0L);	
		else
			pWnd->SortItems(CompareDes, 0L);

		return;
	}
	if(dataType = "N")
	{	
		if (bAsc)	
			pWnd->SortItems(CompareAscI, 0L);	
		else
			pWnd->SortItems(CompareDesI, 0L);

		return;
	}

}

int CALLBACK CSortClass::CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;	
	ASSERT(i1 && i2);
	return i1->txt.CompareNoCase(i2->txt);
}

int CALLBACK CSortClass::CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;	
	ASSERT(i1 && i2);
	return i2->txt.CompareNoCase(i1->txt);
}

int CALLBACK CSortClass::CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ASSERT(i1 && i2);
	if (i1->iInt == i2->iInt) 
		return 0;	
	return i1->iInt > i2->iInt ? 1 : -1;
}

int CALLBACK CSortClass::CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ASSERT(i1 && i2);
	if (i1->iInt == i2->iInt)
		return 0;	
	return i1->iInt < i2->iInt ? 1 : -1;
}

CSortClass::CSortItem::CSortItem(const DWORD _dw, const CString & _txt)
{
	dw = _dw;
	txt = _txt;
}

CSortClass::CSortItem::~CSortItem()
{

}

CSortClass::CSortItemInt::CSortItemInt(const DWORD _dw, const CString & _txt)
{
	iInt = _ttoi(_txt);	
	dw = _dw;
}

CSortClass::CSortItemDate::CSortItemDate(const DWORD _dw, const CString & _txt)
{

	if(_txt != "")
	{
		CString mth = _txt.Left(3);
		int iMth = 1;

		if(mth == "Jan")
				iMth = 1;
		if(mth == "Feb")
				iMth = 2;
		if(mth == "Mar")
				iMth = 3;
		if(mth == "Apr")
				iMth = 4;
		if(mth == "May")
				iMth = 5;
		if(mth == "Jun")
				iMth = 6;
		if(mth == "Jul")
				iMth = 7;
		if(mth == "Aug")
				iMth = 8;
		if(mth == "Sep")
				iMth = 9;
		if(mth == "Oct")
				iMth = 10;
		if(mth == "Nov")
				iMth = 11;
		if(mth == "Dec")
				iMth = 12;
		
		int yr = _ttoi(_txt.Mid(7,4));
		if(yr > 1970)
		{
			int dt = _ttoi(_txt.Mid(4,2));
			CTime t( yr , iMth , dt , 0 , 0 , 0);  	
			dDate = t;
			dw = _dw;
		}

	}
}


