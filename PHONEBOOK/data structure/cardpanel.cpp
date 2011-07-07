// CardViewPanel.cpp: implementation of the CCardPanel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\PhoneBook.h"
#include "CardPanel.h"

#include "..\Panel\PrevView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CImage* CCardPanel::m_pImgCardNormal = NULL;
CImage* CCardPanel::m_pImgCardSelect = NULL;
int CCardPanel::m_siCount = 0;

CCardPanel::CCardPanel()
{
//	m_pImgBuffer = NULL;
	m_bSel = false;
	m_siCount ++;
}

CCardPanel::~CCardPanel()
{
}

void CCardPanel::init() 
{
	//Get the profile path
	Tstring sProfile(theApp.m_szSkin);
	sProfile += Tstring(_T("organize\\card.ini"));

	//get background image
	TCHAR szSkin[MAX_PATH];
	if(!m_pImgCardNormal){
		if(al_GetSettingString(_T("panel"),_T("image_nor"),const_cast<TCHAR *>(sProfile.c_str()),szSkin)){
			//Load the image
			Tstring sSkin(theApp.m_szSkin);
			sSkin += Tstring(szSkin);
			m_pImgCardNormal = new CImage;
			if(m_pImgCardNormal){
				if(m_pImgCardNormal->LoadFile(sSkin.c_str()) != Ok){
					SAFE_DELPTR(m_pImgCardNormal);
				}
			}	
		}
	}

	if(!m_pImgCardSelect){
		if(al_GetSettingString(_T("panel"),_T("image_sel"),const_cast<TCHAR *>(sProfile.c_str()),szSkin)){
			//Load the image
			Tstring sSkin(theApp.m_szSkin);
			sSkin += Tstring(szSkin);
			m_pImgCardSelect = new CImage;
			if(m_pImgCardSelect){
				if(m_pImgCardSelect->LoadFile(sSkin.c_str()) != Ok){
					SAFE_DELPTR(m_pImgCardSelect);
				}
			}	
		}
	}

//	xDrawBuffer();
}

bool CCardPanel::xDrawBuffer(CImage **pImgBuffer)
{
	if(m_pImgCardSelect && m_pImgCardNormal)
	{
		SAFE_DELPTR(*pImgBuffer);
		*pImgBuffer = new CImage(THUMB_WIDTH + THUMB_SHADOW,THUMB_HEIGHT + THUMB_SHADOW,PT_24BITSRGB,RGB(255,255,255));
		//Draw shadow
		Graphics grap(*pImgBuffer);
		int nX = (THUMB_WIDTH - m_pImgCardSelect->Width())/2;
		int nY = (THUMB_HEIGHT- m_pImgCardSelect->Height())/2;
		ai_DrawShadow( grap, CRect(nX, nY, nX+m_pImgCardSelect->Width(), nY+m_pImgCardSelect->Height()), THUMB_SHADOW, 15, RGB(0, 0, 0) );
 
		//Check select flag
		if(m_bSel){
			if(m_pImgCardSelect){
				m_pImgCardSelect->Draw(**pImgBuffer,0,0,m_pImgCardSelect->Width(),m_pImgCardSelect->Height());
			}
		}
		else{
			if(m_pImgCardNormal){
				m_pImgCardNormal->Draw(**pImgBuffer,0,0,m_pImgCardNormal->Width(),m_pImgCardNormal->Height());
			}
		}

		//Get the profile path
		Tstring sProfile(theApp.m_szSkin);
		sProfile += Tstring(_T("organize\\card.ini"));

		//Draw list data to image buffer
		TCHAR szFont[MAX_PATH];
		int iSize;

		//Get title font name and size
		GetProfileFont(const_cast<TCHAR *>(sProfile.c_str()),_T("font_title"),iSize,szFont);
		
		//Get string pos
		CPoint pt(0,0);
		al_GetSettingPoint(_T("name"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		
		//Get font color
		COLORREF cr;
		al_GetSettingColor(_T("font_title"),_T("font_color"),const_cast<TCHAR *>(sProfile.c_str()),cr);
		Color color;
		color.SetFromCOLORREF(cr);
		
		//Get all data prepare to show
		//get name
		TCHAR szName[MAX_PATH];
		memset(szName,0,sizeof(TCHAR) * MAX_PATH);
		GetName(szName);
		Tstring sName(szName);
		//get group id
		int iGroup = GetGroup();
		//get other data
		Tstring sText[6];
		int i = 0 ;
		for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++ )
		{
			int iType = (*iter).GetType();
			if( iType == PBK_Number_General || iType == PBK_Number_Mobile || iType == PBK_Number_Work
				|| iType == PBK_Number_Fax || iType == PBK_Number_Home || iType == PBK_Text_Note
				|| iType == PBK_Text_Postal || iType == PBK_Text_Email || iType == PBK_Text_URL 
				|| iType == PBK_Number_Other|| iType == PBK_Number_Pager  || iType == PBK_Text_Nickname 
				|| iType ==PBK_Number_Mobile_Home || iType ==PBK_Number_Mobile_Work
				|| iType ==PBK_Number_Fax_Home || iType ==PBK_Number_Fax_Work
				|| iType ==PBK_Text_Email_Home || iType ==PBK_Text_Email_Work
				|| iType ==PBK_Text_URL_Home || iType ==PBK_Text_URL_Work
				|| iType ==PBK_Text_Postal_Home || iType ==PBK_Text_Postal_Work
				|| iType ==PBK_Number_Pager_Home || iType ==PBK_Number_Pager_Work
				|| iType ==PBK_Number_VideoCall || iType ==PBK_Number_VideoCall_Home|| iType ==PBK_Number_VideoCall_Work
				|| iType ==PBK_Text_Email_Mobile || iType ==PBK_Text_Email_Unknown|| iType ==PBK_Text_Company)
			{
				if(iType ==PBK_Text_Postal_Home || iType ==PBK_Text_Postal_Work ||iType == PBK_Text_Postal)
				{ 
					CString strPostal;
					strPostal.Format(_T("%s"),(*iter).GetText());
					if(strPostal.CompareNoCase(_T(";;;;;")) == 0)
						continue;
				}
				const TCHAR *pTxt = (*iter).GetDisplayName();
				if(pTxt[_tcslen(pTxt) - 1] != ':')
					sText[i ++] = (*iter).GetDisplayName() + Tstring(_T(" : ")) + (*iter).GetText();
				else 
					sText[i ++] = Tstring((*iter).GetDisplayName()) + Tstring(_T(" "))+ Tstring((*iter).GetText());
				if( i >= 6)
					break;
			}
		}		

		
		//draw group
		TCHAR szTxt[MAX_PATH];
		//get group name
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,iGroup,reinterpret_cast<LPARAM>(szTxt));
		CString sTmp(szTxt);
		DrawStringInImage(*pImgBuffer,sTmp,pt,REAL(iSize),0,color,DT_RIGHT,CSize(0,0),szFont);

		//Draw name 
		CRect rcBound;
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sName.c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		CRect rcStr;
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,pt.x + ( 2 * THUMB_WIDTH ) / 3, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sName.c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);

		//Get font name and size
		if(m_bSel){
			GetProfileFont(const_cast<TCHAR *>(sProfile.c_str()),_T("font_sel"),iSize,szFont);
			al_GetSettingColor(_T("font_sel"),_T("font_color"),const_cast<TCHAR *>(sProfile.c_str()),cr);
			color.SetFromCOLORREF(cr);
		}
		else{
			GetProfileFont(const_cast<TCHAR *>(sProfile.c_str()),_T("font_normal"),iSize,szFont);
			al_GetSettingColor(_T("font_normal"),_T("font_color"),const_cast<TCHAR *>(sProfile.c_str()),cr);
			color.SetFromCOLORREF(cr);
		}

		//draw text1
		al_GetSettingPoint(_T("company"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sText[0].c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,THUMB_WIDTH - 5, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sText[0].c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);
//		DrawStringInImage(*pImgBuffer,CString(sText[0].c_str()),pt,REAL(iSize),0,color,-1,CSize(0,0),szFont);
		
		//draw text2
		al_GetSettingPoint(_T("handle"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sText[1].c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,THUMB_WIDTH - 5, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sText[1].c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);
//		DrawStringInImage(*pImgBuffer,CString(sText[1].c_str()),pt,REAL(iSize),0,color,-1,CSize(0,0),szFont);
		
		//draw text3
		al_GetSettingPoint(_T("companytel"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sText[2].c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,THUMB_WIDTH - 5, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sText[2].c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);
//		DrawStringInImage(*pImgBuffer,CString(sText[2].c_str()),pt,REAL(iSize),0,color,-1,CSize(0,0),szFont);
		
		//draw text4
		al_GetSettingPoint(_T("hometel"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sText[3].c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,THUMB_WIDTH - 5, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sText[3].c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);
//		DrawStringInImage(*pImgBuffer,CString(sText[3].c_str()),pt,REAL(iSize),0,color,-1,CSize(0,0),szFont);
		
		//draw text5
		al_GetSettingPoint(_T("mobiletel"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sText[4].c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,THUMB_WIDTH - 5, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sText[4].c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);
//		DrawStringInImage(*pImgBuffer,CString(sText[4].c_str()),pt,REAL(iSize),0,color,-1,CSize(0,0),szFont);
		al_GetSettingPoint(_T("companyphs"),_T("pos"),const_cast<TCHAR *>(sProfile.c_str()),pt);
		//measure string rectangle
		MeasureStringRectangle(*pImgBuffer,sText[5].c_str(),static_cast<REAL>(iSize),0,szFont,rcBound);
		//set string rectangle
		rcStr.SetRect(pt.x,pt.y,THUMB_WIDTH - 5, pt.y + rcBound.Height());
		DrawStringInImage(*pImgBuffer,CString(sText[5].c_str()),rcStr,REAL(iSize),0,color,-1,CSize(0,0),szFont);


		return true;
	}
	return false;
}

void CCardPanel::Release() 
{
	m_siCount --;
	if(m_siCount < 0)
		m_siCount = 0;

	//if there are no other data object ,delete the static images.
	if(m_siCount <= 0){
		SAFE_DELPTR(m_pImgCardNormal);
		SAFE_DELPTR(m_pImgCardSelect);
	}

//	SAFE_DELPTR(m_pImgBuffer);
}
/*
CImage * CCardPanel::GetImgBuffer()
{
	return m_pImgBuffer;
}
*/
void CCardPanel::SetSelect(bool bSel)
{
	if(m_bSel != bSel){
		m_bSel = bSel;
//		xDrawBuffer();
	}
}
/*
void CCardPanel::ResetImgBuffer()
{
	SAFE_DELPTR(m_pImgBuffer);
}
*/
bool CCardPanel::GetSIMForamt(CCardPanel &data)
{
	data = *this;
	data.m_deqData.clear();

	bool bFlag = false;
	int nGeneral , nMobile, nWork, nHome,nFax,nOther,nPager,nLastName,nFirstName,nName;
	nGeneral= nMobile= nWork= nHome=nFax=nOther=nPager = -1;
	int 	nMobileH,nMobileW,nFaxH,nFaxW,nPagerH,nPagerW,nVideo,nVideoH,nVideoW;
	nMobileH=nMobileW=nFaxH=nFaxW=nPagerH=nPagerW=nVideo=nVideoH=nVideoW=-1;
	int nAssistant,nBusiness,nCallback,nCar,nISDN,nPrimary,nRadio,nTelix,nTTYTDD;
	nAssistant=nBusiness=nCallback=nCar=nISDN=nPrimary=nRadio=nTelix=nTTYTDD = -1;

	nLastName=nFirstName=nName = -1;
	int nIndex = 0;
	for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++){
		if(/*(*iter).GetType() == PBK_Text_Name||*/	(*iter).GetType() == PBK_Caller_Group ) 
		{
			if((*iter).GetType() == PBK_Caller_Group && theApp.m_iMobileCompany == CO_MOTO)	
				(*iter).SetNumber(1);
			else if((*iter).GetType() == PBK_Caller_Group && theApp.m_iMobileCompany != CO_NOKIA)	
				(*iter).SetNumber(0);

			data.m_deqData.push_back((*iter));
		}
/*		else if( !bFlag  && ((*iter).GetType() == PBK_Number_General || (*iter).GetType() == PBK_Number_Mobile ||
			(*iter).GetType() == PBK_Number_Work || (*iter).GetType() == PBK_Number_Fax || (*iter).GetType() == PBK_Number_Other ||
			(*iter).GetType() == PBK_Number_Home)){
			if(theApp.m_iMobileCompany == CO_ASUS)
				(*iter).SetType(PBK_Number_Mobile);
			else
				(*iter).SetType(PBK_Number_General);

			data.m_deqData.push_back((*iter));
			bFlag = true;
		}*/
		int nType = (*iter).GetType();
		switch(nType)
		{
			case PBK_Number_General:
				nGeneral = nIndex;
				break;
			case PBK_Number_Mobile:
				nMobile = nIndex;
				break;
			case PBK_Number_Work:
				nWork = nIndex;
				break;
			case PBK_Number_Home:
				nHome = nIndex;
				break;
			case PBK_Number_Fax:
				nFax = nIndex;
				break;
			case PBK_Number_Other:
				nOther = nIndex;
				break;
			case PBK_Number_Pager:
				nPager = nIndex;
				break;
			case PBK_Text_LastName:
				nLastName = nIndex;
				break;
			case PBK_Text_FirstName:
				nFirstName = nIndex;
				break;
			case PBK_Text_Name:
				nName = nIndex;
				break;
			case PBK_Number_Mobile_Home:
				nMobileH = nIndex;
				break;	
			case PBK_Number_Mobile_Work:
				nMobileW = nIndex;
				break;	
			case PBK_Number_Fax_Home:
				nFaxH = nIndex;
				break;	
			case PBK_Number_Fax_Work:
				nFaxW = nIndex;
				break;	
			case PBK_Number_Pager_Home:
				nPagerH = nIndex;
				break;	
			case PBK_Number_Pager_Work:
				nPagerW = nIndex;
				break;	
			case PBK_Number_VideoCall:
				nVideo = nIndex;
				break;	
			case PBK_Number_VideoCall_Home:
				nVideoH = nIndex;
				break;	
			case PBK_Number_VideoCall_Work:
				nVideoW = nIndex;
				break;	
			//new add
			case PBK_Number_Assistant:
				nAssistant = nIndex;
				break;
			case PBK_Number_Business:
				nBusiness = nIndex;
				break;
			case PBK_Number_Callback:
				nCallback = nIndex;
				break;
			case PBK_Number_Car:
				nCar = nIndex;
				break;
			case PBK_Number_ISDN:
				nISDN = nIndex;
				break;
			case PBK_Number_Primary:
				nPrimary = nIndex;
				break;
			case PBK_Number_Radio:
				nRadio = nIndex;
				break;
			case PBK_Number_Telix:
				nTelix = nIndex;
				break;
			case PBK_Number_TTYTDD:
				nTTYTDD = nIndex;
				break;	
		}
		nIndex ++;
	}
	if(nName!=-1)
	{
		data.m_deqData.push_back(*(m_deqData.begin() + nName));
	}
	else if(nFirstName!=-1 || nLastName!= -1)
	{
		TCHAR szName[MAX_PATH];
		szName[0] ='\0';
		if(nFirstName!=-1)
			wsprintf(szName,_T("%s"),(*(m_deqData.begin() + nFirstName)).GetText());
		if(nLastName!=-1)
		{
			if(_tcslen(szName) > 0)
				wsprintf(szName,_T("%s %s"),szName,(*(m_deqData.begin() + nLastName)).GetText());
			else
				wsprintf(szName,_T("%s"),(*(m_deqData.begin() + nLastName)).GetText());
		}
		CPhoneData Phonedata;
		Phonedata.SetType(PBK_Text_Name);
		Phonedata.SetText(szName);
		data.m_deqData.push_back(Phonedata);

	}
	DEQPHONEDATA::iterator iter2;
	if(nGeneral != -1) iter2 =  m_deqData.begin() + nGeneral;
	else if(nMobile != -1) iter2 =  m_deqData.begin() + nMobile;
	else if(nWork != -1) iter2 =  m_deqData.begin() + nWork;
	else if(nHome != -1) iter2 =  m_deqData.begin() + nHome;
	else if(nFax != -1) iter2 =  m_deqData.begin() + nFax;
	else if(nOther != -1) iter2 =  m_deqData.begin() + nOther;
	else if(nPager != -1) iter2 =  m_deqData.begin() + nPager;
	else if(nVideo != -1) iter2 =  m_deqData.begin() + nVideo;
	else if(nMobileH != -1) iter2 =  m_deqData.begin() + nMobileH;
	else if(nFaxH != -1) iter2 =  m_deqData.begin() + nFaxH;
	else if(nPagerH != -1) iter2 =  m_deqData.begin() + nPagerH;
	else if(nVideoH != -1) iter2 =  m_deqData.begin() + nVideoH;
	else if(nMobileW != -1) iter2 =  m_deqData.begin() + nMobileW;
	else if(nFaxW != -1) iter2 =  m_deqData.begin() + nFaxW;
	else if(nPagerW != -1) iter2 =  m_deqData.begin() + nPagerW;
	else if(nVideoW != -1) iter2 =  m_deqData.begin() + nVideoW;
	
	else if(nBusiness != -1) iter2 =  m_deqData.begin() + nBusiness;
	else if(nPrimary != -1) iter2 =  m_deqData.begin() + nPrimary;
	else if(nAssistant != -1) iter2 =  m_deqData.begin() + nAssistant;
	else if(nCallback != -1) iter2 =  m_deqData.begin() + nCallback;
	else if(nCar != -1) iter2 =  m_deqData.begin() + nCar;
	else if(nISDN != -1) iter2 =  m_deqData.begin() + nISDN;
	else if(nRadio != -1) iter2 =  m_deqData.begin() + nRadio;
	else if(nTelix != -1) iter2 =  m_deqData.begin() + nTelix;
	else if(nTTYTDD != -1) iter2 =  m_deqData.begin() + nTTYTDD;


	if(nGeneral != -1 ||nMobile != -1 ||nWork != -1 ||nHome != -1 ||nFax != -1 ||nOther != -1 ||nPager != -1
		||nVideo != -1 ||nMobileH != -1||nFaxH != -1||nPagerH != -1||nVideoH != -1||nMobileW != -1||
		nFaxW != -1||nPagerW != -1||nVideoW != -1 ||
		nBusiness != -1||nPrimary != -1||nAssistant != -1||nCallback != -1||nCar != -1||nISDN != -1||
		nRadio != -1 || nTelix != -1 ||nTTYTDD != -1)
	{
		if(theApp.m_iMobileCompany == CO_ASUS&& (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))
			(*iter2).SetType(PBK_Number_Mobile);
		else
			(*iter2).SetType(PBK_Number_General);

		data.m_deqData.push_back((*iter2));
		bFlag = true;
	}
	return bFlag;
}

void CCardPanel::ClearShareImage()
{
	SAFE_DELPTR(m_pImgCardNormal);
	SAFE_DELPTR(m_pImgCardSelect);
}

bool CCardPanel::GetDrawImage(CImage **pImgBuffer)
{
	return xDrawBuffer(pImgBuffer);
}
