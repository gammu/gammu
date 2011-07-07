#include "stdafx.h"
#include "PhoneBook.h"

//DLL CONTRUCT
CWnd * __stdcall afxGetMainWnd(void)
{
  return theApp.m_pMainWnd;
}

CWinApp * __stdcall afxGetApp(void)
{
  return &theApp;
}

void DrawShadow( Graphics& gdc, CRect& rect, int shadow, int factor, COLORREF color )
{
	int ncx = rect.Width();
	int ncy = rect.Height();
	if( ncx ==0 || ncy == 0 )
		return;

	int r, g, b;
	int i, j;

	r = GetRValue(color);
	g = GetGValue(color);
	b = GetBValue(color);

	for( i = 1; i <= shadow; i++ )
	{
		SolidBrush brush1(Color(factor * (shadow - i + 1) * shadow, r, g, b));
 
		// right
		gdc.FillRectangle(&brush1, rect.right + i - 1, rect.top + shadow * 2, 1, ncy - shadow * 2);
		// bottom
		gdc.FillRectangle(&brush1, rect.left + shadow * 2, rect.bottom + i - 1, ncx - shadow * 2, 1);
 
		for(j = 1; j <= shadow; j ++)
		{
			SolidBrush brush2(Color(factor * (shadow - i + 1) * j, r, g, b));
 
			// top-right
			gdc.FillRectangle(&brush2, rect.right + i - 1, rect.top + shadow + j - 1, 1, 1);
			// bottom-left
			gdc.FillRectangle(&brush2, rect.left + shadow*2 - i, rect.bottom + shadow - j, 1, 1);
			// bottom-right
			gdc.FillRectangle(&brush2, rect.right + i - 1, rect.bottom + shadow - j, 1, 1);
		}
	}
}

void SystemError( DWORD dwError )
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
				   FORMAT_MESSAGE_FROM_SYSTEM |
				   FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL,
				   dwError,
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),//0, // Default language
				   (LPTSTR) &lpMsgBuf,
				   0,
				   NULL);

	// Process any inserts in lpMsgBuf.
	AfxMessageBox( (LPCTSTR)lpMsgBuf );
	LocalFree( lpMsgBuf );
}

CString FileSize2Str(DWORD dwSize)
{
	long K = 1024;
	long M = 1048576;
	long G = 1073741824;

	CString str;
	
	if( dwSize > G )
	{	str.Format( _T("%.2f GB"), (float)dwSize/(float)G );
	}
	else if( dwSize > M )
	{	str.Format( _T("%.2f MB"), (float)dwSize/(float)M );
	}
	else
		str.Format( _T("%.2f KB"), (float)dwSize/(float)K );

	return str;
}

BOOL CenterRect(CRect *pRCParent,CRect *pRCChild,BOOL bOffset)
{
	if(pRCParent->Width() < pRCChild->Width() || pRCParent->Height() < pRCChild->Height())
		return FALSE;

	if(bOffset)
	{
		CRect rc;
		rc.left = pRCParent->left + (pRCParent->Width() - pRCChild->Width()) / 2;
		rc.right = rc.left + pRCChild->Width();
		rc.top = pRCParent->top + (pRCParent->Height() - pRCChild->Height()) / 2;
		rc.bottom = rc.top + pRCChild->Height();

		*pRCChild = rc;
	}
	else
	{
		CRect rc;
		rc.left = (pRCParent->Width() - pRCChild->Width()) / 2;
		rc.right = rc.left + pRCChild->Width();
		rc.top = (pRCParent->Height() - pRCChild->Height()) / 2;
		rc.bottom = rc.top + pRCChild->Height();

		*pRCChild = rc;
	}

	return TRUE;
}

void GetSliderData(SLDCTRLEX2_BUF &sld_data ,LPTSTR sSection ,LPTSTR sProfilePath)
{
	al_GetSettingRect(sSection,_T("rect_slider"),sProfilePath,sld_data.rect);
	al_GetSettingRect(sSection,_T("thumb_range"),sProfilePath,sld_data.thumb_range);
	al_GetSettingBorder(sSection,_T("bar_border"),sProfilePath,sld_data.bar_border);
	al_GetSettingSize(sSection,_T("range"),sProfilePath,sld_data.range);
	al_GetSettingInt(sSection,_T("pos_thumb"),sProfilePath,sld_data.pos);
	al_GetSettingPoint(sSection,_T("left_pos"),sProfilePath,sld_data.left_pos);
	al_GetSettingPoint(sSection,_T("right_pos"),sProfilePath,sld_data.right_pos);

	CString sSkin,sSkinPath;
	al_GetSettingString(sSection,_T("image_bar"),sProfilePath,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sSkinPath = CString(theApp.m_szSkin) + sSkin;
//	SAFE_DELPTR(sld_data.pImgBar);
	sld_data.pImgBar = new CImageArray(sSkinPath);

	al_GetSettingString(sSection,_T("image_thumb"),sProfilePath,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sSkinPath = CString(theApp.m_szSkin) + sSkin;
//	SAFE_DELPTR(sld_data.pImgThumb);
	sld_data.pImgThumb = new CImageArray(sSkinPath);
	sld_data.pImgThumb->SetCount(4);

	al_GetSettingString(sSection,_T("image_right"),sProfilePath,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sSkinPath = CString(theApp.m_szSkin) + sSkin;
//	SAFE_DELPTR(sld_data.pImgRB);
	sld_data.pImgRB = new CImageArray(sSkinPath);

	al_GetSettingString(sSection,_T("image_left"),sProfilePath,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sSkinPath = CString(theApp.m_szSkin) + sSkin;
//	SAFE_DELPTR(sld_data.pImgLB);
	sld_data.pImgLB = new CImageArray(sSkinPath);
}

int GetLargeStringLength(CString &str1 ,CString &str2,CDC *pDC)
{
	int len = 0;
	CSize size = pDC->GetTextExtent(str1);
	len = size.cx;
	size = pDC->GetTextExtent(str2);
	return  ((len < size.cx) ? size.cx : len);
}

bool GetProfileFont(TCHAR *profile, TCHAR *sec,int &nFontSize , TCHAR* szFontName)
{
	bool bRet = true;
	if(!profile){
		if(!al_GetSettingInt(_T("Font"),_T("fontsize"),theApp.m_szFont,nFontSize))
			bRet = false;
		if(!al_GetSettingString(_T("Font"),_T("facename"),theApp.m_szFont,szFontName))
			bRet &= false;
	}
	else{
		if(!al_GetSettingInt(sec,_T("font_size"),profile,nFontSize))
			if(!al_GetSettingInt(_T("button_font"),_T("font_size"),profile,nFontSize))
				if(!al_GetSettingInt(_T("Font"),_T("fontsize"),theApp.m_szFont,nFontSize))
					bRet = false;
		if(!al_GetSettingString(sec,_T("font_name"),profile,szFontName))
			if(!al_GetSettingString(_T("button_font"),_T("font_name"),profile,szFontName))
				if(!al_GetSettingString(_T("Font"),_T("facename"),theApp.m_szFont,szFontName))
					bRet &= false;
	}
	return bRet;
}

bool GetProfileFont(TCHAR *profile, TCHAR *sec,TCHAR *sec2 ,int &nFontSize , TCHAR* szFontName)
{
	bool bRet = true;
	if(!al_GetSettingInt(sec,_T("font_size"),profile,nFontSize))
		if(!al_GetSettingInt(sec2,_T("font_size"),profile,nFontSize))
			if(!al_GetSettingInt(_T("Font"),_T("fontsize"),theApp.m_szFont,nFontSize))
				bRet = false;
	if(!al_GetSettingString(sec,_T("font_name"),profile,szFontName))
		if(!al_GetSettingString(sec2,_T("font_name"),profile,szFontName))
			if(!al_GetSettingString(_T("Font"),_T("facename"),theApp.m_szFont,szFontName))
				bRet &= false;
	return bRet;
}

HFONT GetFontEx(TCHAR *szFontName,int &nFontSize)
{
	return al_CreateFont2(szFontName,theApp.m_iCodepage,nFontSize);
}

void GetButtonFromSetting(CRescaleButton *pButton , TCHAR *sec ,TCHAR* szTextID , TCHAR* szToolTip,TCHAR *profile )
{
	BOOL bRet = FALSE;
	CString sSkin,sSkinPath;
	int nFontSize;
	TCHAR szFontName[MAX_PATH];
	
	//Get control font size and name.
	GetProfileFont(profile,sec,_T("button_font"),nFontSize,szFontName);

	//get font color
	TCHAR szFontColor[4][32] = {_T("font_color_normal") , _T("font_color_down")
		,_T("font_color_hover"),_T("font_color_gray")};
	COLORREF fcolor[4];
	for(int i = 0 ; i < 4 ; i ++)
	{
		al_GetSettingColor(_T("button_font"),szFontColor[i],profile,fcolor[i]);
	}
	FONTCOLOR color = {fcolor[0],fcolor[1],fcolor[2],fcolor[3]};

	//load button image
	bRet = al_GetSettingString(sec , _T("image_left") , profile ,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	if(!sSkin.IsEmpty() && bRet)
	{
		sSkinPath = CString(theApp.m_szSkin) + sSkin;
		CImageArray *pImg = new CImageArray;
		pImg->LoadFile(sSkinPath);
		pButton->LoadLeftBg(pImg);
	}
	sSkin.Empty();
	bRet = al_GetSettingString(sec , _T("image_center") , profile ,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	if(!sSkin.IsEmpty() && bRet)
	{
		sSkinPath = CString(theApp.m_szSkin) + sSkin;
		CImageArray *pImg = new CImageArray;
		pImg->LoadFile(sSkinPath);
		pButton->LoadCenterBg(pImg);
	}
	sSkin.Empty();
	bRet =	al_GetSettingString(sec , _T("image_right") , profile ,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	if(!sSkin.IsEmpty() && bRet)
	{
		sSkinPath = CString(theApp.m_szSkin) + sSkin;
		CImageArray *pImg = new CImageArray;
		pImg->LoadFile(sSkinPath);
		pButton->LoadRightBg(pImg);
	}
	sSkin.Empty();
	
	//set tooltip
	if(szToolTip){
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),szToolTip,theApp.m_szRes,szTxt);
		pButton->SetToolTipText(szTxt);
	}

	//set button text
	if(szTextID)
	{
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),szTextID,theApp.m_szRes,szTxt);
		
		pButton->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
		pButton->SetWindowText(szTxt);
		pButton->SetTextColor(color.crNormal, &color.crDown, &color.crGray);
		pButton->SetHTextColor(color.crHover);
		pButton->SetTextAlignment(BA_CENTER);
		pButton->ShowText(TRUE);
	}

	CRect rc,rect;
	//set button positoion and size
	al_GetSettingRect(sec , _T("rect") , profile ,rect);
	pButton->MoveWindow(&rect);

	//get the rescale button 3 parts and 1 flag 
	if(al_GetSettingRect(sec , _T("part_width") , profile , rc)){
		pButton->SetButtonPartWidth(rc.left,rc.top,rc.Width(),rc.Height());
		//Set button text flag 
		if(rc.Height()){
			int width = pButton->GetButtonWidth();
			rc = CRect(rect.left,rect.top,rect.left + width,rect.bottom);
			pButton->MoveWindow(&rc);
		}
	}
}

void GetHeaderControlFromSetting(CSkinHeaderCtrl *pCtrl , TCHAR *sec , TCHAR *profile )
{
	BOOL bRet = FALSE;
	CString sSkin,sSkinPath;
	int nFontSize;
	TCHAR szFontName[MAX_PATH];
	
	//Get control font size and name.
	GetProfileFont(profile,sec,_T("header_font"),nFontSize,szFontName);

	//get font color
	TCHAR szFontColor[4][32] = {_T("font_color_normal") , _T("font_color_down")
		,_T("font_color_hover"),_T("font_color_gray")};
	COLORREF fcolor[4];
	for(int i = 0 ; i < 4 ; i ++)
	{
		al_GetSettingColor( _T("header_font"),szFontColor[i],profile,fcolor[i]);
	}
	FONTCOLOR color = {fcolor[0],fcolor[1],fcolor[2],fcolor[3]};

	//load button image
	CImageArray *pImg[3];
	bRet = al_GetSettingString(sec , _T("image_left") , profile ,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	if(!sSkin.IsEmpty() && bRet)
	{
		sSkinPath = CString(theApp.m_szSkin) + sSkin;
		pImg[0] = new CImageArray;
		pImg[0]->LoadFile(sSkinPath);
	}
	sSkin.Empty();
	bRet = al_GetSettingString(sec , _T("image_center") , profile ,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	if(!sSkin.IsEmpty() && bRet)
	{
		sSkinPath = CString(theApp.m_szSkin) + sSkin;
		pImg[1] = new CImageArray;
		pImg[1]->LoadFile(sSkinPath);
	}
	sSkin.Empty();
	bRet =	al_GetSettingString(sec , _T("image_right") , profile ,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	if(!sSkin.IsEmpty() && bRet)
	{
		sSkinPath = CString(theApp.m_szSkin) + sSkin;
		pImg[2] = new CImageArray;
		pImg[2]->LoadFile(sSkinPath);
	}
	sSkin.Empty();
	
	pCtrl->LoadBitmap(pImg[0],pImg[1],pImg[2]);

	//set button text
	pCtrl->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
	pCtrl->SetTextColor(color.crNormal, &color.crDown, &color.crGray);
	pCtrl->SetHTextColor(color.crHover);
	pCtrl->SetTextAlignment(BA_LEFT);
	pCtrl->ShowText(TRUE);
}

void GetCheckFromSetting(CCheckEx2 *pCheck,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile)
{
	BOOL bRet = FALSE;
	CString sSkin,sSkinPath;
	int nFontSize;
	TCHAR szFontName[MAX_PATH];
	//Get control font size and name.
	GetProfileFont(profile,sec,_T("button_font"),nFontSize,szFontName);

	//Get control font color
	TCHAR szFontColor[4][32] = {_T("font_color_normal") , _T("font_color_down")
		,_T("font_color_hover"),_T("font_color_gray")};
	COLORREF fcolor[4];
	for(int i = 0 ; i < 4 ; i ++)
	{
		al_GetSettingColor(_T("button_font"),szFontColor[i],profile,fcolor[i]);
	}
	FONTCOLOR color = {fcolor[0],fcolor[1],fcolor[2],fcolor[3]};

	//set this control to be owner draw style
	LONG style = GetWindowLong(pCheck->GetSafeHwnd(),GWL_STYLE);
	style |= BS_OWNERDRAW;
	SetWindowLong(pCheck->GetSafeHwnd(),GWL_STYLE,style);

	//Losd control image
	CRect rc;
	al_GetSettingString(sec,_T("image_check"),profile,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sSkinPath = CString(theApp.m_szSkin) + sSkin;
	CImageArray *pImg = new CImageArray;
	pImg->LoadFile(sSkinPath);
	pCheck->LoadBitmap(pImg);
	//Set default check
	pCheck->SetCheck(1);
	pCheck->ShowText(FALSE);
	pCheck->SetTextColor( color.crNormal, &color.crDown, &color.crGray );
	//set background brush
	COLORREF crBrush;
	al_GetSettingColor(_T("panel"),_T("color"),profile,crBrush);
	pCheck->SetBkColor(crBrush);
	//set text font
	pCheck->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY);
	//set control position and size
	al_GetSettingRect(sec,_T("rect_check"),profile,rc);
	pCheck->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	//set control text
	if(szTxtID)
	{
		TCHAR szText[MAX_PATH];
		al_GetSettingString(_T("public"),szTxtID,theApp.m_szRes,szText);
		pCheck->ShowText(TRUE);
		pCheck->SetWindowText(szText);
	}
}

void GetPicFromSetting(CImage &Img,CRect &rc ,TCHAR *sec,TCHAR *profile)
{
	al_GetSettingRect(sec,_T("rect_pic"),profile,rc);
	CString sSkin,sPath;
	al_GetSettingString(sec,_T("image_pic"),profile,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sPath = CString(theApp.m_szSkin) + sSkin;
	Img.LoadFile(sPath);
}

void GetSliderFromSetting(CSliderCtrlEx2 *pSldCtrl,TCHAR* sec,TCHAR *profile)
{
	SLDCTRLEX2_BUF sld_data;
	CString sSkin,sSkinPath,sProfilePath;
	
	GetSliderData(sld_data,sec,profile);
	COLORREF crBrush;
	al_GetSettingColor(_T("panel"),_T("color"),profile,crBrush);	
	pSldCtrl->SetBgColor(crBrush);
	pSldCtrl->LoadSetting(sld_data);
	pSldCtrl->SetWindowPos(NULL,sld_data.rect.left,sld_data.rect.top,
		sld_data.rect.Width(),sld_data.rect.Height(),SWP_NOZORDER);
}
/*
void GetVSliderFromSetting(CVSliderCtrlEx2 *pSldCtrl,char* sec,char *profile)
{
	SLDCTRLEX2_BUF sld_data;
	CString sSkin,sSkinPath,sProfilePath;
	
	GetSliderData(sld_data,sec,profile);
	sld_data.pImgThumb->SetCount(1);

	COLORREF crBrush;
	al_GetSettingColor(_T("panel"),_T("color"),profile,crBrush);	
	pSldCtrl->SetBgColor(crBrush);
	pSldCtrl->LoadSetting(sld_data);
	pSldCtrl->SetWindowPos(NULL,sld_data.rect.left,sld_data.rect.top,
		sld_data.rect.Width(),sld_data.rect.Height(),SWP_NOZORDER);
}
*/
void GetRadioFromSetting(CRadioEx *pRadio,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile)
{
	BOOL bRet = FALSE;
	CString sSkin,sSkinPath;
	int nFontSize;
	TCHAR szFontName[MAX_PATH];
	//Get control font size and name.
	GetProfileFont(profile,sec,_T("button_font"),nFontSize,szFontName);
	//get font color
	TCHAR szFontColor[4][32] = {_T("font_color_normal") , _T("font_color_down")
		,_T("font_color_hover"),_T("font_color_gray")};
	COLORREF fcolor[4];
	for(int i = 0 ; i < 4 ; i ++)
	{
		al_GetSettingColor(_T("button_font"),szFontColor[i],profile,fcolor[i]);
	}
	FONTCOLOR color = {fcolor[0],fcolor[1],fcolor[2],fcolor[3]};

	//set control style
	LONG style = GetWindowLong(pRadio->GetSafeHwnd(),GWL_STYLE);
	style |= BS_OWNERDRAW;
	SetWindowLong(pRadio->GetSafeHwnd(),GWL_STYLE,style);
	//Load contreol image
	al_GetSettingString(sec,_T("image_radio"),profile,sSkin.GetBuffer(_MAX_PATH));
	sSkin.ReleaseBuffer();
	sSkinPath = CString(theApp.m_szSkin) + sSkin;
	CImageArray *pImg = new CImageArray;
	pImg->LoadFile(sSkinPath);
	pRadio->LoadBitmap(pImg);
	
	pRadio->SetCheck(1);
	pRadio->ShowText(FALSE);
	pRadio->SetTextColor( color.crNormal, &color.crDown, &color.crGray );
	//set background color
	COLORREF crBrush;
	al_GetSettingColor(_T("panel"),_T("color"),profile,crBrush);
	pRadio->SetBkColor(crBrush);
	//set text font 
	pRadio->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY);
	//set control pos and size
	CRect rc;
	al_GetSettingRect(sec,_T("rect_radio"),profile,rc);
	pRadio->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	//set control text 
	if(szTxtID)
	{
		TCHAR szText[MAX_PATH];
		al_GetSettingString(_T("public"),szTxtID,theApp.m_szRes,szText);
		pRadio->ShowText(TRUE);
		pRadio->SetWindowText(szText);
	}
}

void GetStaticFromSetting(CStaticEx *pStatic,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile)
{
	CString sSkin,sSkinPath;
	int nFontSize;
	TCHAR szFontName[MAX_PATH];
	//Get control font size and name.
	GetProfileFont(profile,sec,_T("button_font"),nFontSize,szFontName);
	//set font color
	TCHAR szFontColor[2][32] = {_T("font_color_normal") ,_T("font_color_gray")};
	COLORREF fcolor[2];
	for(int i = 0 ; i < 2 ; i ++)
	{
		al_GetSettingColor(sec,szFontColor[i],profile,fcolor[i]);
	}
	//set font
	pStatic->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
	pStatic->SetTextColor(&fcolor[0], &fcolor[1]);
	//set control text
	if(szTxtID)
	{
		TCHAR szText[MAX_PATH];
		al_GetSettingString(_T("public"),szTxtID,theApp.m_szRes,szText);
		pStatic->SetWindowText(szText);
	}
	//set background color
	COLORREF brush;
	al_GetSettingColor(_T("panel"),_T("color"),profile,brush);
	pStatic->SetBrushColor(brush);
	//set control pois and size
	CRect rc;
	al_GetSettingRect(sec,_T("rect"),profile,rc);
	pStatic->MoveWindow(rc);
}
void GetStaticFromSettingWithText(CStaticEx *pStatic,TCHAR *sec,TCHAR *szTxt,TCHAR *profile)
{
	CRect rect;
	if(!al_GetSettingRect(sec,_T("rect"),profile,rect))
		pStatic->ShowWindow(SW_HIDE);
	else 
	{
		CString sSkin,sSkinPath;
		int nFontSize;
		TCHAR szFontName[MAX_PATH];
		//Get control font size and name.
		GetProfileFont(profile,sec,_T("button_font"),nFontSize,szFontName);
		//set font color
		TCHAR szFontColor[2][32] = {_T("font_color_normal") ,_T("font_color_gray")};
		COLORREF fcolor[2];
		for(int i = 0 ; i < 2 ; i ++)
		{
			al_GetSettingColor(sec,szFontColor[i],profile,fcolor[i]);
		}
		//set font
		pStatic->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
		pStatic->SetTextColor(&fcolor[0], &fcolor[1]);
		//set control text
		if(szTxt)
			pStatic->SetWindowText(szTxt);
		//set background color
		COLORREF brush;
		al_GetSettingColor(_T("panel"),_T("color"),profile,brush);
		pStatic->SetBrushColor(brush);
		//set control pois and size
		CRect rc;
		al_GetSettingRect(sec,_T("rect"),profile,rc);
		pStatic->MoveWindow(rc);
	}
}
void GetStaticFromSetting2(CStaticEx *pStatic,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile)
{
	CRect rect;
	if(!al_GetSettingRect(sec,_T("rect"),profile,rect))
		pStatic->ShowWindow(SW_HIDE);
	else 
	{
		GetStaticFromSetting(pStatic,sec,szTxtID,profile);
	}
}
void GetCheckFromSetting2(CCheckEx2 *pCheck,TCHAR *sec,TCHAR *szTxtID,TCHAR *profile)
{
	CRect rect;
	if(!al_GetSettingRect(sec,_T("rect_check"),profile,rect))
		pCheck->ShowWindow(SW_HIDE);
	else 
	{
		GetCheckFromSetting(pCheck,sec,szTxtID,profile);
	}
}
void DrawStringInImage(CImage *pImg, CString string ,CPoint pt,REAL FontSize,
				INT FontStyle,Color FontColor,int nDrawType,CSize EffectOffset,TCHAR*  pszFont )
{
	ASSERT(pImg);
	ASSERT(pszFont);

	//Get title bar font and draw text
	WCHAR wsName[_MAX_PATH]=L"",wstr[_MAX_PATH]=L"";

#ifdef _UNICODE
	wsprintf(wsName,_T("%s"),pszFont);
#else
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pszFont, -1,
				wsName, sizeof(wsName)/sizeof(wsName[0]));
#endif
	Font myFont( wsName, FontSize, FontStyle,UnitPixel  );
	SolidBrush blackBrush(Color(255, 0, 0, 0));
	SolidBrush myBrush(FontColor);
#ifdef _UNICODE
	wsprintf(wstr,_T("%s"),string);
#else
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, string, -1,
					wstr, sizeof(wstr)/sizeof(wstr[0]));
#endif
	Graphics graph(pImg);
	RectF rcstring;
	graph.MeasureString( wstr,wcslen(wstr), &myFont,PointF(0.0,0.0), &rcstring);	//compute string length with pixel
	if(nDrawType == DT_LEFT) 
	{
		if(EffectOffset == CSize(0,0))
		{
			PointF origin(0.0,0.0);
			graph.DrawString(wstr,wcslen(wstr),&myFont,origin,&myBrush);
		}
		else
		{
			PointF origin(REAL(0.0+EffectOffset.cx/2),REAL(pt.y+EffectOffset.cy/2));
			PointF origin2(origin.X-EffectOffset.cx,origin.Y-EffectOffset.cy);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin, &blackBrush);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin2,&myBrush);
		}		
	}
	else if(nDrawType == DT_CENTER)
	{
		if(EffectOffset == CSize(0,0))
		{
			PointF origin(REAL((pImg->Width()-rcstring.Width)/2),REAL(pt.y));
			graph.DrawString(wstr,wcslen(wstr),&myFont,origin,&myBrush);
		}
		else
		{
			PointF origin(REAL((pImg->Width() -  rcstring.Width)/2 + EffectOffset.cx/2),REAL(pt.y + EffectOffset.cy/2));
			PointF origin2(origin.X-EffectOffset.cx,origin.Y - EffectOffset.cy);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin, &blackBrush);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin2,&myBrush);
		}
	}
	else if(nDrawType == DT_RIGHT)
	{
		if(EffectOffset == CSize(0,0))
		{
			PointF origin(REAL(pImg->Width()-rcstring.Width-8),REAL(pt.y));
			graph.DrawString(wstr,wcslen(wstr),&myFont,origin,&myBrush);
		}
		else
		{
			PointF origin(REAL(pImg->Width() -  rcstring.Width),REAL(pt.y + EffectOffset.cy/2));
			PointF origin2(origin.X-EffectOffset.cx,origin.Y - EffectOffset.cy);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin, &blackBrush);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin2,&myBrush);
		}
	}
	else
	{
		if(EffectOffset == CSize(0,0))
		{
			PointF origin(REAL(pt.x),REAL(pt.y));
			graph.DrawString(wstr,wcslen(wstr),&myFont,origin,&myBrush);
		}
		else
		{
			PointF origin(REAL(pt.x+EffectOffset.cx/2),REAL(pt.y+EffectOffset.cy/2));
			PointF origin2(origin.X-EffectOffset.cx,origin.Y-EffectOffset.cy);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin, &blackBrush);
			graph.DrawString( wstr, wcslen(wstr), &myFont, origin2,&myBrush);
		}
	}
}

void DrawStringInImage(CImage *pImg, CString string ,CRect &rcStr,REAL FontSize,
				INT FontStyle,Color FontColor,int nDrawType,CSize EffectOffset,TCHAR*  pszFont )
{
	ASSERT(pImg);
	ASSERT(pszFont);

	//Get title bar font and draw text
	WCHAR wsName[_MAX_PATH]=L"",wstr[_MAX_PATH]=L"";

#ifdef _UNICODE
	wsprintf(wsName,_T("%s"),pszFont);
#else
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pszFont, -1,
				wsName, sizeof(wsName)/sizeof(wsName[0]));
#endif
	Font myFont( wsName, FontSize, FontStyle, UnitPixel);
	SolidBrush blackBrush(Color(255, 0, 0, 0));
	SolidBrush myBrush(FontColor);
#ifdef _UNICODE
	wsprintf(wstr,_T("%s"),string);
#else
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, string, -1,
					wstr, sizeof(wstr)/sizeof(wstr[0]));
#endif
	Graphics graph(pImg);
	RectF rcstring;
	graph.MeasureString( wstr,wcslen(wstr), &myFont,PointF(0.0,0.0), &rcstring);	//compute string length with pixel
	
	StringFormat format;
	
	if(nDrawType == DT_LEFT) 
	{
		format.SetAlignment(StringAlignmentNear);
	}
	else if(nDrawType == DT_CENTER)
	{
		format.SetAlignment(StringAlignmentCenter);
	}
	else if(nDrawType == DT_RIGHT)
	{
		format.SetAlignment(StringAlignmentFar);
	}

	if(EffectOffset == CSize(0,0))
	{
		RectF rc(static_cast<float>(rcStr.left),static_cast<float>(rcStr.top),
			static_cast<float>(rcStr.Width()),static_cast<float>(rcStr.Height()));
		graph.DrawString(wstr,wcslen(wstr),&myFont,rc,&format,&myBrush);
	}
	else
	{
		RectF rc(static_cast<float>(rcStr.left+EffectOffset.cx/2),static_cast<float>(rcStr.top+EffectOffset.cy/2),
			static_cast<float>(rcStr.Width()),static_cast<float>(rcStr.Height()));
		graph.DrawString(wstr,wcslen(wstr),&myFont,rc,&format,&blackBrush);
		
		RectF rcTmp(static_cast<float>(rcStr.left-EffectOffset.cx),static_cast<float>(rcStr.top-EffectOffset.cy),
			static_cast<float>(rcStr.Width()),static_cast<float>(rcStr.Height()));
		graph.DrawString(wstr,wcslen(wstr),&myFont,rcTmp,&format,&myBrush);
	}
}

void MeasureStringRectangle(CImage *pImg,LPCTSTR lpStr,REAL FontSize,INT FontStyle,LPCTSTR lpFont,CRect &rcBound)
{
	ASSERT(pImg);
	ASSERT(lpFont);

	//Get title bar font and draw text
	WCHAR wsName[_MAX_PATH]=L"",wstr[_MAX_PATH]=L"";
#ifdef _UNICODE
	wsprintf(wsName,_T("%S"),lpFont);
#else

	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpFont, -1,
				wsName, sizeof(wsName)/sizeof(wsName[0]));
#endif
	Font myFont( wsName, FontSize, FontStyle, UnitPixel);
#ifdef _UNICODE
	wsprintf(wstr,_T("%S"),lpStr);
#else
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpStr, -1,
					wstr, sizeof(wstr)/sizeof(wstr[0]));
#endif
	Graphics graph(pImg);

//	RectF rcLayout(0,0,static_cast<float>(pImg->Width()),static_cast<float>(pImg->Height())),rcBounding;
//	graph.MeasureString( wstr,wcslen(wstr), &myFont,rcLayout,&rcBounding);
	PointF pt(0.0,0.0);
	RectF rcBounding;
	graph.MeasureString( wstr,wcslen(wstr), &myFont,pt,&rcBounding);
	rcBound.SetRect(static_cast<int>(rcBounding.X + .5),static_cast<int>(rcBounding.Y),
		static_cast<int>(rcBounding.Width),static_cast<int>(rcBounding.Height));
}

bool nocase_compare(WCHAR c1,WCHAR c2)
{
	return toupper(c1) == toupper(c2);
}

//to calc real rectangle of a CStaticEx. if change window text in static, please call again
void CalcStaticRealRect(CStaticEx* pTxt, CRect& rect)
{
	CPaintDC dc(pTxt); // device context for painting
	
	// TODO: Add your message handler code here
	pTxt->GetClientRect(&rect);
	
	DWORD dwStyle = GetWindowLong(pTxt->m_hWnd, GWL_STYLE);
	UINT uFormat = pTxt->m_type;// | DT_WORDBREAK;
	
	if((dwStyle & SS_LEFTNOWORDWRAP) == SS_LEFTNOWORDWRAP)
		uFormat &= (~DT_WORDBREAK);
	
	if((dwStyle & SS_LEFT) == SS_LEFT)
		uFormat |= DT_LEFT;
	if((dwStyle & SS_CENTER) == SS_CENTER)
		uFormat |= DT_CENTER;
	if((dwStyle & SS_RIGHT) == SS_RIGHT || (dwStyle & SS_RIGHT) == SS_RIGHTJUST)
		uFormat |= DT_RIGHT;
	if((dwStyle & SS_NOPREFIX) == SS_NOPREFIX)
		uFormat |= SS_NOPREFIX;
	
	TCHAR szText[_MAX_PATH];
	pTxt->GetWindowText( szText, sizeof(szText) );
	
	if( szText[0] != '\0' )
	{
		HFONT hOldFont = NULL;
		if(pTxt->m_hFont) 
			hOldFont = (HFONT)::SelectObject( dc.GetSafeHdc(), pTxt->m_hFont);
		
		int mode = dc.SetBkMode(TRANSPARENT);
		
		if( (dwStyle & WS_DISABLED) == WS_DISABLED )
			dc.SetTextColor(pTxt->m_crGray);
		else
			dc.SetTextColor(pTxt->m_crText);
		
		uFormat |= DT_CALCRECT;
		::DrawText( dc.GetSafeHdc(), szText, -1, &rect, uFormat );
		dc.SetBkMode(mode);
		
		if(hOldFont)
			::SelectObject(dc.GetSafeHdc(), hOldFont);
	}
	
	
	
}

void SortList(DATALIST &lsData)
{
	DATALIST lsME,lsSIM;
	for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++){
		if(strcmp((*iter).GetStorageType(),MEMORY_NAME) == 0)
			lsME.push_back((*iter));
		else if(strcmp((*iter).GetStorageType(),SIM_NAME) == 0)
			lsSIM.push_back((*iter));
	}
	lsData.clear();
	lsData = lsME;
	lsData.splice(lsData.end(),lsSIM);
}
void SplitString(CString strSrc ,int  Split,CString& str1,CString& str2)
{
	int nLen = strSrc.GetLength();
	for(int i = 0; i<nLen ;i++)
	{
		if(strSrc.GetAt(i) ==Split)
		{
			if(i==0)
			{
				str1.Empty();
				str2.Format(_T("%s"),strSrc.GetBuffer(MAX_PATH)+1);
				strSrc.ReleaseBuffer();
				break;
			}
			else if(strSrc.GetAt(i-1)!='\\')
			{
				str1.Format(_T("%s"),strSrc);
				str2.Format(_T("%s"),strSrc.GetBuffer(MAX_PATH)+i+1);
				strSrc.ReleaseBuffer();
				str1.SetAt(nLen-str2.GetLength()-1 , '\0');
				break;
			}
		}
	}
	if(i==nLen)
	{
		str2.Empty();
		str1.Format(_T("%s"),strSrc);
	}
}
void MotoSpriteAddress( LPCTSTR szAddress, LPTSTR szStreet2, LPTSTR szStreet, LPTSTR szCity , LPTSTR szState, LPTSTR szZIP, LPTSTR szCountry)
{
	CString strSource=szAddress;
	CString str;
	
	//skip first ;
//	strSource = strSource.Right(strSource.GetLength()-1);
	CString str1,str2;
//Get Street2
	SplitString(strSource,';',str1,str2);
	if(szStreet2)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szStreet2,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szStreet2, str1);
	}
	strSource = str2;
//Get Street
	SplitString(strSource,';',str1,str2);
	if(szStreet)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szStreet,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szStreet, str1);
	}
	strSource = str2;
	//Get City
	SplitString(strSource,';',str1,str2);
	if(szCity)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szCity,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szCity, str1);
	}
	strSource = str2;
	//get state
	SplitString(strSource,';',str1,str2);
	if(szState)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szState,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szState, str1);
	}
	strSource = str2;
	//get ZIP
	SplitString(strSource,';',str1,str2);
	if(szZIP)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szZIP,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szZIP, str1);
	}
	//get Country
	if(szCountry)
	{
		str2.Replace(_T("\\;"),_T(";"));
		memset(szCountry,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szCountry, str2);
	}
}

void SpriteAddress( int nCompany, LPCTSTR szAddress, LPTSTR szStreet, LPTSTR szCity , LPTSTR szState, LPTSTR szZIP, LPTSTR szCountry)
{
	// ;Street;City;State;ZIP;Country
	CString strSource=szAddress;
	CString str;
	
	//skip first ;
	strSource = strSource.Right(strSource.GetLength()-1);
	CString str1,str2;
//Get Street
	SplitString(strSource,';',str1,str2);
	if(szStreet)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szStreet,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szStreet, str1);
	}
	strSource = str2;
	//Get City
	SplitString(strSource,';',str1,str2);
	if(szCity)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szCity,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szCity, str1);
	}
	strSource = str2;
	//get state
	SplitString(strSource,';',str1,str2);
	if(szState)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szState,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szState, str1);
	}
	strSource = str2;
	//get ZIP
	SplitString(strSource,';',str1,str2);
	if(szZIP)
	{
		str1.Replace(_T("\\;"),_T(";"));
		memset(szZIP,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szZIP, str1);
	}
	//get Country
	if(szCountry)
	{
		str2.Replace(_T("\\;"),_T(";"));
		memset(szCountry,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szCountry, str2);
	}

/*	//Get Street
	str = strSource.Left(strSource.Find(';'));
	strSource = strSource.Right( strSource.GetLength() - strSource.Find(';') - 1 );
	if(szStreet){
		memset(szStreet,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szStreet, str);
	}

	//Get City
	str = strSource.Left(strSource.Find(';'));
	strSource = strSource.Right( strSource.GetLength() - strSource.Find(';') - 1 );
	if(szCity){
		memset(szCity,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szCity, str);
	}

	//get state
	str = strSource.Left(strSource.Find(';'));
	strSource = strSource.Right( strSource.GetLength() - strSource.Find(';') - 1 );
	if(szState){
		memset(szState,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szState, str);
	}

	//Get ZIP
	str = strSource.Left(strSource.Find(';'));
	strSource = strSource.Right( strSource.GetLength() - strSource.Find(';') - 1 );
	if(szZIP){
		memset(szZIP,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szZIP, str);
	}

	//Get Country
	if(szCountry){
		memset(szCountry,0,sizeof(TCHAR) * MAX_PATH);
		_tcscpy(szCountry, strSource);
	}*/

}
void MotoMargeAddress( LPTSTR szAddress,LPCTSTR szStreet2, LPCTSTR szStreet1, LPCTSTR szCity, LPCTSTR szState , LPCTSTR szZIP, LPCTSTR szCountry)
{
	CString str;
//	wsprintf( szAddress, ";%s;%s;%s;%s;%s", szStreet, szCity, szState, szZIP, szCountry );
	CString strStreet2 = szStreet2;
	CString strStreet = szStreet1;
	CString strCity = szCity;
	CString strState = szState;
	CString strZIP = szZIP;
	CString stCountry = szCountry;

	strStreet.Replace(_T(";"),_T("\\;"));
	strCity.Replace(_T(";"),_T("\\;"));
	strState.Replace(_T(";"),_T("\\;"));
	strZIP.Replace(_T(";"),_T("\\;"));
	stCountry.Replace(_T(";"),_T("\\;"));

	wsprintf( szAddress, _T("%s;%s;%s;%s;%s;%s"),szStreet2, strStreet, strCity, strState, strZIP, stCountry );

}

void MargeAddress( int nCompany, LPTSTR szAddress, LPCTSTR szStreet, LPCTSTR szCity, LPCTSTR szState , LPCTSTR szZIP, LPCTSTR szCountry)
{
	// ;Street;City;;ZIP;Country
	CString str;
//	wsprintf( szAddress, ";%s;%s;%s;%s;%s", szStreet, szCity, szState, szZIP, szCountry );
	CString strStreet = szStreet;
	CString strCity = szCity;
	CString strState = szState;
	CString strZIP = szZIP;
	CString stCountry = szCountry;

	strStreet.Replace(_T(";"),_T("\\;"));
	strCity.Replace(_T(";"),_T("\\;"));
	strState.Replace(_T(";"),_T("\\;"));
	strZIP.Replace(_T(";"),_T("\\;"));
	stCountry.Replace(_T(";"),_T("\\;"));

	wsprintf( szAddress, _T(";%s;%s;%s;%s;%s"), strStreet, strCity, strState, strZIP, stCountry );

//	str.Format( ";%s;%s;;%s;%s", szStreet, szCity, szZIP, szCountry );

}
void AnaString(LPTSTR szString,CStringList *pstrlist )
{
	CString strall(szString);
	int  i, j;
	TCHAR szStr[128];
	strall +=';';
	pstrlist->RemoveAll();

	for( i = 0, j = 0; i < strall.GetLength(); i++ )
	{
	    if (strall[i] == ';')
	    {
			szStr[j] = '\0';
		//	if(strlen(szStr)>0)
			pstrlist->AddTail(szStr);
			j = 0;
	    }
	    else
			szStr[j++] = strall[i];
	}

}
void FreeContactSubEntryList(Contact_Data_Struct* pData)
{
	POSITION pos = pData->Datalist.GetHeadPosition();
	while(pos)
	{
		ContactEntry* pEntry = (ContactEntry*)pData->Datalist.GetNext(pos);
		if(pEntry)
		{
			delete pEntry;
			pEntry = NULL;
		}
	}
	pData->Datalist.RemoveAll();
}
void MemoryEntry2ContactDataStruct(GSM_MemoryEntry* entry , Contact_Data_Struct* pContactData)
{
//	pContactData->Index = entry->Location;
	sprintf(pContactData->szIndex,entry->szIndex);
	for(int i = 0; i<entry->EntriesNum ; i++)
	{
		ContactEntry* pdataentry = new ContactEntry;
//		ZeroMemory(pdataentry,sizeof(ContactEntry));
		pdataentry->MobileEntryType =(GSM_EntryType)0;
		pdataentry->MSOTEntryType =(MSOT_EntryType)0;
		pdataentry->nMobileEntrytypeIndex =pdataentry->Number= 0;
		ZeroMemory(&pdataentry->Date,sizeof(GSM_DateTime));
		pdataentry->strText=_T("");
		memcpy(&pdataentry->Date,&entry->Entries[i].Date ,sizeof(GSM_DateTime));
		if(entry->Entries[i].EntryType != PBK_Date && entry->Entries[i].EntryType != PBK_Category && entry->Entries[i].EntryType != PBK_Private &&
			entry->Entries[i].EntryType != PBK_Caller_Group && entry->Entries[i].EntryType != PBK_RingtoneID && entry->Entries[i].EntryType != PBK_PictureID)
		//	swprintf((WCHAR*)pdataentry->Text,(WCHAR*)entry->Entries[i].Text);
			pdataentry->strText.Format((TCHAR*)entry->Entries[i].Text);
		pdataentry->MobileEntryType = entry->Entries[i].EntryType;
		pdataentry->Number = entry->Entries[i].Number;
//		pdataentry->nUpdateFalg =SYNCDATA_UPDATE_NONE;
		pdataentry->MSOTEntryType =OT_PBK_NONEDEFINED;
		pContactData->Datalist.AddTail(pdataentry);
	}

}
void GetAddressText(CString& strAddress)
{
	CString strTemp;
	CString strTemp1;
	int nLen;

	strTemp1.Format(_T("%s"),strAddress);
	int nMaxlen = (GSM_PHONEBOOK_TEXT_LENGTH*2);
	int nth = strTemp1.ReverseFind(';');
	if(nth<0)
	{
		nLen =  nMaxlen/sizeof(TCHAR);
		strAddress.SetAt(nLen,'\0');
		return;
	}
	
	strTemp = strTemp1.Left(nth);
	nLen = (strTemp.GetLength())*sizeof(TCHAR);
	nMaxlen -=sizeof(TCHAR);
	if(nLen < nMaxlen)
	{
		nLen =  (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
		strAddress.SetAt(nLen,'\0');
	}
	else
	{
		nth = strTemp.ReverseFind(';');
		if(nth<0)
		{
			nLen =  (nMaxlen)/sizeof(TCHAR);
			strTemp.SetAt(nLen,'\0');

			strAddress.Format(_T("%s;"),strTemp);
			return;
		}
		strTemp1 = strTemp.Left(nth);
		nLen = (strTemp1.GetLength())*sizeof(TCHAR);
		nMaxlen -=sizeof(TCHAR);
		if(nLen < nMaxlen)
		{
			nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
			strTemp.SetAt(nLen,'\0');
			strAddress.Format(_T("%s;"),strTemp);
		}
		else
		{
			nth = strTemp1.ReverseFind(';');
			if(nth<0)
			{
				nLen =  (nMaxlen)/sizeof(TCHAR);
				strTemp1.SetAt(nLen,'\0');

				strAddress.Format(_T("%s;;"),strTemp1);
				return;
			}
			strTemp = strTemp1.Left(nth);
			nLen = (strTemp.GetLength())*sizeof(TCHAR);
			nMaxlen -=sizeof(TCHAR);
			if(nLen < nMaxlen)
			{
				nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
				strTemp1.SetAt(nLen,'\0');
				strAddress.Format(_T("%s;;"),strTemp1);
			}
			else
			{
				nth = strTemp.ReverseFind(';');
				if(nth<0)
				{
					nLen =  (nMaxlen)/sizeof(TCHAR);
					strTemp.SetAt(nLen,'\0');

					strAddress.Format(_T("%s;;;"),strTemp);
					return;
				}
				strTemp1 = strTemp.Left(nth);
				nLen = (strTemp1.GetLength())*sizeof(TCHAR);
				nMaxlen -=sizeof(TCHAR);
				if(nLen < nMaxlen)
				{
					nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
					strTemp.SetAt(nLen,'\0');
					strAddress.Format(_T("%s;;;"),strTemp);
				}
				else
				{
					nth = strTemp1.ReverseFind(';');
					if(nth<0)
					{
						nLen =  (nMaxlen)/sizeof(TCHAR);
						strTemp1.SetAt(nLen,'\0');

						strAddress.Format(_T("%s;;;;"),strTemp1);
						return;
					}
					strTemp = strTemp1.Left(nth);
					nLen = (strTemp.GetLength())*sizeof(TCHAR);
					nMaxlen -=sizeof(TCHAR);
					if(nLen < nMaxlen)
					{
						nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
						strTemp1.SetAt(nLen,'\0');
						strAddress.Format(_T("%s;;;;"),strTemp1);
					}
					else
					{
						nth = strTemp.ReverseFind(';');
						if(nth<0)
						{
							nLen =  (nMaxlen)/sizeof(TCHAR);
							strTemp.SetAt(nLen,'\0');

							strAddress.Format(_T("%s;;;;;"),strTemp);
							return;
						}
						strTemp1 = strTemp.Left(nth);
						nLen = (strTemp1.GetLength())*sizeof(TCHAR);
						nMaxlen -=sizeof(TCHAR);
						if(nLen < nMaxlen)
						{
							nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
							strTemp.SetAt(nLen,'\0');
							strAddress.Format(_T("%s;;;;;"),strTemp);
						}
						else
						{
							nth = strTemp1.ReverseFind(';');
							if(nth<0)
							{
								nLen =  (nMaxlen)/sizeof(TCHAR);
								strTemp1.SetAt(nLen,'\0');

								strAddress.Format(_T("%s;;;;;;"),strTemp1);
								return;
							}
							strTemp = strTemp1.Left(nth);
							nLen = (strTemp.GetLength())*sizeof(TCHAR);
							nMaxlen -=sizeof(TCHAR);
							if(nLen < nMaxlen)
							{
								nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
								strTemp1.SetAt(nLen,'\0');
								strAddress.Format(_T("%s;;;;;;"),strTemp1);
							}
							else
							{
								nLen = (nMaxlen+sizeof(TCHAR))/sizeof(TCHAR);
								strTemp.SetAt(nLen,'\0');
								strAddress.Format(_T("%s;;;;;;"),strTemp);
							}
						}
					}
				}
			}
		}

	}

}
void ContactDataStruct2MemoryEntry(Contact_Data_Struct* pContactData,GSM_MemoryEntry* entry)
{
	POSITION pos = pContactData->Datalist.GetHeadPosition();
	int i = 0;
	while(pos)
	{
		
		ContactEntry* pdataentry =(ContactEntry* ) pContactData->Datalist.GetNext(pos);
		memcpy(&entry->Entries[i].Date ,&pdataentry->Date,sizeof(GSM_DateTime));
	//	swprintf((WCHAR*)entry->Entries[i].Text,(WCHAR*)pdataentry->Text);
		int nLen = (pdataentry->strText.GetLength())*sizeof(TCHAR);
		if(nLen > ((GSM_PHONEBOOK_TEXT_LENGTH)*2))
		{
			if( pdataentry->MobileEntryType == PBK_Text_Postal || pdataentry->MobileEntryType == PBK_Text_Postal_Home || pdataentry->MobileEntryType == PBK_Text_Postal_Work )
				GetAddressText(pdataentry->strText);
			else
			{
					nLen =  ((GSM_PHONEBOOK_TEXT_LENGTH)*2)/sizeof(TCHAR);
					pdataentry->strText.SetAt(nLen,'\0');
			}
		}
		wsprintf((TCHAR*)entry->Entries[i].Text,pdataentry->strText);

		entry->Entries[i].EntryType = pdataentry->MobileEntryType;
		entry->Entries[i].Number = pdataentry->Number;
		i++;
	}
	entry->EntriesNum = i;
	
}
/*
WCHAR* GetWharPhoneNumber(TCHAR *szPhoneNumber)
{
#ifdef _UNICODE
	return szPhoneNumber;
#else
	USES_CONVERSION;
	return A2W(szPhoneNumber);
#endif

}*/
void MultiByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage)
{
	WCHAR* pWsrc = NULL;
#ifdef _UNICODE
	pWsrc = lpszSrc.GetBuffer(MAX_PATH);
#else
	USES_CONVERSION;
	TCHAR* pSrc = lpszSrc.GetBuffer(MAX_PATH);
	pWsrc =  A2W(pSrc);
#endif
	int nLen = wcslen(pWsrc);
	if(nLen >= MULTIBYTE*nPage)
	{
#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(MULTIBYTE*(nPage-1)), MULTIBYTE*sizeof(TCHAR));
		lpszDes[MULTIBYTE] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[MULTIBYTE+1];

		memcpy(wDet, pWsrc+(MULTIBYTE*(nPage-1)), MULTIBYTE*sizeof(WCHAR));
		wDet[MULTIBYTE] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}
	else if(nLen > MULTIBYTE*(nPage-1))
	{
		int ncopylen = nLen -( MULTIBYTE*(nPage-1));

#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(MULTIBYTE*(nPage-1)), ncopylen*sizeof(TCHAR));
		lpszDes[ncopylen] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[MULTIBYTE+1];

		memcpy(wDet, pWsrc+(MULTIBYTE*(nPage-1)), ncopylen*sizeof(WCHAR));
		wDet[ncopylen] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}

	lpszSrc.ReleaseBuffer();

}

void SingleByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage)
{
	WCHAR* pWsrc = NULL;
#ifdef _UNICODE
	pWsrc = lpszSrc.GetBuffer(MAX_PATH);
#else
	USES_CONVERSION;
	TCHAR* pSrc = lpszSrc.GetBuffer(MAX_PATH);
	pWsrc =  A2W(pSrc);
#endif
	int nLen = wcslen(pWsrc);
	if(nLen >= SINGLEBYTE*nPage)
	{
#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(SINGLEBYTE*(nPage-1)), SINGLEBYTE*sizeof(TCHAR));
		lpszDes[SINGLEBYTE] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[SINGLEBYTE+1];

		memcpy(wDet, pWsrc+(SINGLEBYTE*(nPage-1)), SINGLEBYTE*sizeof(WCHAR));
		wDet[SINGLEBYTE] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}
	else if(nLen > SINGLEBYTE*(nPage-1))
	{
		int ncopylen = nLen -( SINGLEBYTE*(nPage-1));
#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(SINGLEBYTE*(nPage-1)), ncopylen*sizeof(TCHAR));
		lpszDes[ncopylen] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[SINGLEBYTE+1];

		memcpy(wDet, pWsrc+(SINGLEBYTE*(nPage-1)), ncopylen*sizeof(WCHAR));
		wDet[ncopylen] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}

	lpszSrc.ReleaseBuffer();

}
bool EntryTypeString2EntryType(CString szEntry,GSM_EntryType *pentryType)
{
	bool bFind = false;
	TCHAR szTxtEntry[94][32] ={
	_T("PBK_Number_General"),_T("PBK_Number_Mobile"),_T("PBK_Number_Work"),_T("PBK_Number_Fax"),_T("PBK_Number_Home"),
	_T("PBK_Number_Pager"),_T("PBK_Number_Other"),_T("PBK_Text_Note"),_T("PBK_Text_Postal"),_T("PBK_Text_Email"),
	_T("PBK_Text_Email2"),_T("PBK_Text_URL"),_T("PBK_Date"),_T("PBK_Caller_Group"),_T("PBK_Text_Name"),
	_T("PBK_Text_LastName"),_T("PBK_Text_FirstName"),_T("PBK_Text_Company"),_T("PBK_Text_JobTitle"),_T("PBK_Category"),
	_T("PBK_Private"),_T("PBK_Text_StreetAddress"),_T("PBK_Text_City"),_T("PBK_Text_State"),_T("PBK_Text_Zip"),
	_T("PBK_Text_Country"),_T("PBK_Text_Custom1"),_T("PBK_Text_Custom2"),_T("PBK_Text_Custom3"),_T("PBK_Text_Custom4"),
	_T("PBK_RingtoneID"),_T("PBK_RingtoneFileSystemID"),_T("PBK_PictureID"),_T("PBK_SMSListID"),_T("PBK_Text_UserID"),
	_T("PBK_Text_Picture"),_T("PBK_Text_Ring"),_T("PBK_Number_Sex"),_T("PBK_Number_Light"),_T("PBK_Push_Talk"),
	_T("PBK_Caller_Group_Text"),_T("PBK_Text_StreetAddress2"),_T("PBK_Text_Nickname"),_T("PBK_Number_Mobile_Home"),_T("PBK_Number_Mobile_Work"),
	_T("PBK_Number_Fax_Home"),_T("PBK_Number_Fax_Work"),_T("PBK_Text_Email_Home"),_T("PBK_Text_Email_Work"),_T("PBK_Text_URL_Home"),
	_T("PBK_Text_URL_Work"),_T("PBK_Text_Postal_Home"),_T("PBK_Text_Postal_Work"),_T("PBK_Number_Pager_Home"),_T("PBK_Number_Pager_Work"),
	_T("PBK_Number_VideoCall"),_T("PBK_Number_VideoCall_Home"),_T("PBK_Number_VideoCall_Work"),_T("PBK_Text_MiddleName"),_T("PBK_Text_Suffix"),
	_T("PBK_Text_Title"),
	_T("PBK_Text_Email_Mobile"),_T("PBK_Text_Email_Unknown"),_T("PBK_Number_Assistant") ,_T("PBK_Number_Business") ,_T("PBK_Number_Callback"),//66
	_T("PBK_Number_Car"),_T("PBK_Number_ISDN"),	_T("PBK_Number_Primary"),	_T("PBK_Number_Radio"),_T("PBK_Number_Telix"),_T("PBK_Number_TTYTDD"),_T("PBK_Text_Department"),//73
	_T("PBK_Text_Office"),_T("PBK_Text_Profession"),_T("PBK_Text_Manager_Name"),_T("PBK_Text_Assistant_Name"),_T("PBK_Text_Spouse_Name"),//78
	_T("PBK_Date_Anniversary"),_T("PBK_Text_Directory_Server"),_T("PBK_Text_Email_alias"),_T("PBK_Text_Internet_Address"),//82
	_T("PBK_Text_Children"),_T("PBK_Text_StreetAddress_Work"),_T("PBK_Text_City_Work"),_T("PBK_Text_State_Work"),_T("PBK_Text_Zip_Work"),//87
	_T("PBK_Text_Country_Work"),_T("PBK_Text_StreetAddress_Home"),_T("PBK_Text_City_Home"),_T("PBK_Text_State_Home"),_T("PBK_Text_Zip_Home"),_T("PBK_Text_Country_Home"),_T("PBK_Text_IMID")};

		
	GSM_EntryType nType[94] ={
	PBK_Number_General,PBK_Number_Mobile,PBK_Number_Work,PBK_Number_Fax,PBK_Number_Home,
	PBK_Number_Pager,PBK_Number_Other,PBK_Text_Note,PBK_Text_Postal,PBK_Text_Email,
	PBK_Text_Email2,PBK_Text_URL,PBK_Date,PBK_Caller_Group,PBK_Text_Name,
	PBK_Text_LastName,PBK_Text_FirstName,PBK_Text_Company,PBK_Text_JobTitle,PBK_Category,
	PBK_Private,PBK_Text_StreetAddress,PBK_Text_City,PBK_Text_State,PBK_Text_Zip,
	PBK_Text_Country,PBK_Text_Custom1,PBK_Text_Custom2,PBK_Text_Custom3,PBK_Text_Custom4,
	PBK_RingtoneID,PBK_RingtoneFileSystemID,PBK_PictureID,PBK_SMSListID,PBK_Text_UserID,
	PBK_Text_Picture,PBK_Text_Ring,PBK_Number_Sex,PBK_Number_Light,PBK_Push_Talk,
	PBK_Caller_Group_Text,PBK_Text_StreetAddress2,PBK_Text_Nickname,PBK_Number_Mobile_Home,PBK_Number_Mobile_Work,
	PBK_Number_Fax_Home,PBK_Number_Fax_Work,PBK_Text_Email_Home,PBK_Text_Email_Work,PBK_Text_URL_Home,
	PBK_Text_URL_Work,PBK_Text_Postal_Home,PBK_Text_Postal_Work,PBK_Number_Pager_Home,PBK_Number_Pager_Work,
	PBK_Number_VideoCall,PBK_Number_VideoCall_Home,PBK_Number_VideoCall_Work,PBK_Text_MiddleName,PBK_Text_Suffix,
	PBK_Text_Title,
	PBK_Text_Email_Mobile,PBK_Text_Email_Unknown,PBK_Number_Assistant ,PBK_Number_Business ,PBK_Number_Callback,//66
	PBK_Number_Car,PBK_Number_ISDN,	PBK_Number_Primary,	PBK_Number_Radio,PBK_Number_Telix,PBK_Number_TTYTDD,PBK_Text_Department,//73
	PBK_Text_Office,PBK_Text_Profession,PBK_Text_Manager_Name,PBK_Text_Assistant_Name,PBK_Text_Spouse_Name,//78
	PBK_Date_Anniversary,PBK_Text_Directory_Server,PBK_Text_Email_alias,PBK_Text_Internet_Address,//82
	PBK_Text_Children,PBK_Text_StreetAddress_Work,PBK_Text_City_Work,PBK_Text_State_Work,PBK_Text_Zip_Work,//87
	PBK_Text_Country_Work,PBK_Text_StreetAddress_Home,PBK_Text_City_Home,PBK_Text_State_Home,PBK_Text_Zip_Home,	PBK_Text_Country_Home,PBK_Text_IMID};

	for(int i =0 ; i<94 ;i++)
	{
		if(szEntry.CompareNoCase(szTxtEntry[i]) == 0)
		{
			*pentryType = nType[i];
			bFind = true;
			break;
		}
	}
	return bFind;


}