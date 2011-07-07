#include "stdafx.h"
#include "SMSUtility.h"
//#include "EnhanceDlg\thumbviewdlg.h"

/*
CString g_sFontProfilePath;
*/

//DLL CONTRUCT
CWnd * __stdcall afxGetMainWnd(void)
{
  return theApp.m_pMainWnd;
}
 
CWinApp * __stdcall afxGetApp(void)
{
  return &theApp;
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
		al_GetSettingColor(_T("header_font"),szFontColor[i],profile,fcolor[i]);
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
/*
bool GetProfileFont(TCHAR *profile, TCHAR *sec,int &nFontSize , TCHAR* szFontName)
{
	bool bRet = true;
	if(!al_GetSettingInt(sec,_T("font_size"),profile,nFontSize))
		if(!al_GetSettingInt(_T("button_font"),_T("font_size"),profile,nFontSize))
			if(!al_GetSettingInt("Font","fontsize",theApp.m_szFont,nFontSize))
				bRet = false;
	if(!al_GetSettingString(sec,_T("font_name"),profile,szFontName))
		if(!al_GetSettingString(_T("button_font"),_T("font_name"),profile,szFontName))
			if(!al_GetSettingString("Font","facename",theApp.m_szFont,szFontName))
				bRet &= false;
	return bRet;
}*/

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
/*	string sFontProfile(theApp.m_szSkin);
	sFontProfile += string("font.ini");
	int iChar = 0;
	if(al_GetSettingInt("Font","characterSet",const_cast<TCHAR *>(sFontProfile.c_str()),iChar)){
		return al_CreateFont2(szFontName,iChar,nFontSize);
	}
	else 
		return NULL;*/
	return al_CreateFont2(szFontName,theApp.m_iCodepage,nFontSize);
}

void GetButtonFromSetting(CRescaleButton *pButton , TCHAR *sec ,UINT nTextID , UINT nToolTip,TCHAR *profile )
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
		Status s = pImg->LoadFile(sSkinPath);
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
	if(nToolTip)
		pButton->SetToolTipText(nToolTip);

	//set button text
	if(nTextID)
	{
		CString str;
		str.LoadString(nTextID);
		
		pButton->SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
		pButton->SetWindowText((LPCTSTR)str);
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

void GetCheckFromSetting(CCheckEx2 *pCheck,TCHAR *sec,UINT nTextID,TCHAR *profile)
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
	if(nTextID)
	{
		CString str;
		str.LoadString(nTextID);
		pCheck->ShowText(TRUE);
		pCheck->SetWindowText(str);
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
void GetVSliderFromSetting(CVSliderCtrlEx2 *pSldCtrl,TCHAR* sec,TCHAR *profile)
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
void GetRadioFromSetting(CRadioEx *pRadio,TCHAR *sec,UINT nTextID,TCHAR *profile)
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
	if(nTextID)
	{
		CString str;
		str.LoadString(nTextID);
		pRadio->ShowText(TRUE);
		pRadio->SetWindowText(str);
	}
}

void GetStaticFromSetting(CStaticEx *pStatic,TCHAR *sec,UINT nTxtID,TCHAR *profile)
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
	if(nTxtID)
	{
		CString str;
		str.LoadString(nTxtID);
		pStatic->SetWindowText(str);
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

void DrawStringInImage(CImage *pImg, CString string ,CPoint pt,REAL FontSize,
				INT FontStyle,Color FontColor,int nDrawType,CSize EffectOffset,TCHAR*  pszFont )
{
	ASSERT(pImg);
	//Get title bar font and draw text
	CString sName,sProfilePath,sFontProfilePath,sModulePath;
	WCHAR wsName[_MAX_PATH]=L"",wstr[_MAX_PATH]=L"";

	al_GetSettingString(_T("Font"),_T("facename"),pszFont,sName.GetBuffer(_MAX_PATH));
	sName.ReleaseBuffer();
#ifdef _UNICODE
	wsprintf(wsName,_T("%s"),sName);
#else
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, sName, -1,
				wsName, sizeof(wsName)/sizeof(wsName[0]));
#endif
	Font myFont( wsName, FontSize, FontStyle );
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
			PointF origin(REAL(pImg->Width()-rcstring.Width),REAL(pt.y));
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


