// AutoLayoutDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "PhotoDownloadTool.h"
#include "AutoLayout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////
// static function
BOOL LAutoLayout::al_GetSettingResizeRect( LPCSTR sec, LPCSTR key, LPCSTR profile, RECT* rect )
{
	TCHAR buf[_MAX_PATH];
	::GetPrivateProfileString( sec, key, _T(""), buf, _MAX_PATH, profile );
	if( buf[0] )	{	
//		int l,t,r,b;
		sscanf( buf, "%d,%d,%d,%d", &rect->left, &rect->top, &rect->right, &rect->bottom );
//		rect->left = l;
//		rect->top = t;
//		rect->right = r;
//		rect->bottom = b;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// LAutoLayout dialog

LAutoLayout::LAutoLayout()
{
	m_pResizeTable = NULL;
	m_nTableSize = 0;
	m_BorderSize.cx=m_BorderSize.cy=0;

	//Make default skin path
	::GetModuleFileName( NULL, m_sSkinPath.GetBuffer(MAX_PATH), MAX_PATH );
	m_sSkinPath.ReleaseBuffer();
	m_sSkinPath = m_sSkinPath.Left( m_sSkinPath.ReverseFind('\\') );

	::GetShortPathName(m_sSkinPath, m_sModulePath.GetBuffer(MAX_PATH), MAX_PATH );
	m_sModulePath.ReleaseBuffer();

	m_BorderSize.cx = 0;
	m_BorderSize.cy = 0;
	m_MinSize.cx = 0;
	m_MinSize.cy = 0;
	m_InitSize.cx = 0;
	m_InitSize.cy = 0;
	
	m_sSkinPath = m_sModulePath;
	m_sSkinPath += CString("\\Skin\\default\\");

	m_nBgMode = BM_None;
	m_pBgColor = NULL;
//	m_pBgBrush = NULL;
	m_pImgBg1 = NULL;
	m_pImgBg2 = NULL;
	m_pImgBg3 = NULL;
	m_pImgBg4 = NULL;
	m_pImgBg5 = NULL;
	m_pImgBg6 = NULL;
	m_pImgBg7 = NULL;
	m_pImgBg8 = NULL;
	m_pImgBg9 = NULL;
}

LAutoLayout::~LAutoLayout()
{
	if(m_pResizeTable)	delete []m_pResizeTable;
	xFreeImage();
}

/////////////////////////////////////////////////////////////////////////////
// LAutoLayout message handlers
BOOL
LAutoLayout::SetBitmapMode( LPCSTR szProfile, int nMode )
{
#define	LOADIMAGE( SZKEY, IMG ) \
			bRet = al_GetSettingString( "Panel", (SZKEY), szSkinProfile, buf ); \
			if(!bRet) break; \
			_tcscpy( buf2, m_sSkinPath ); \
			_tcscat( buf2, buf ); \
			(IMG) = new CImage(buf2); \
			if((IMG)->GetLastStatus()!=Ok) break;

	BOOL	bRet=FALSE;
	if(m_nBgMode==nMode)
		return FALSE;
	
	TCHAR szSkinProfile[MAX_PATH];
	_tcscpy( szSkinProfile, m_sSkinPath );
	_tcscat( szSkinProfile, szProfile );

	xFreeImage();
	switch(nMode)	{
	case BM_None:
		bRet = TRUE;
		break;
	case BM_FillColor:	{
			COLORREF dwColor;
			bRet = al_GetSettingColor( "Panel", "Bg_Color", szSkinProfile, dwColor );
			if(bRet) m_pBgColor = new Color;
			m_pBgColor->SetFromCOLORREF(dwColor);
//			if(bRet) m_pBgBrush = new SolidBrush(gdiColor);
		}
		break;
	case BM_SingleTiled:
	case BM_SingleStretched:	{
			TCHAR buf[_MAX_FNAME];
			TCHAR buf2[_MAX_FNAME];
			LOADIMAGE( "Bg_Img", m_pImgBg1 )
		}
		break;
	case BM_HorizontalTiled:
	case BM_VerticalTiled:	
		break;
	case BM_NinePalette:	{
			TCHAR buf[_MAX_FNAME];
			TCHAR buf2[_MAX_FNAME];
			LOADIMAGE( "Bg_imgLT", m_pImgBg1 )
			LOADIMAGE( "Bg_imgCT", m_pImgBg2 )
			LOADIMAGE( "Bg_imgRT", m_pImgBg3 )
			LOADIMAGE( "Bg_imgLC", m_pImgBg4 )
			LOADIMAGE( "Bg_imgCC", m_pImgBg5 )
			LOADIMAGE( "Bg_imgRC", m_pImgBg6 )
			LOADIMAGE( "Bg_imgLB", m_pImgBg7 )
			LOADIMAGE( "Bg_imgCB", m_pImgBg8 )
			LOADIMAGE( "Bg_imgRB", m_pImgBg9 )
		}
		break;
	default:
		break;
	};
	

	if(bRet)	{
		m_nBgMode=nMode;
	}
	else	{
		bRet = BM_None;
		xFreeImage();
	}
	return bRet;
}

void
LAutoLayout::xFreeImage()
{
	if(m_pBgColor) delete m_pBgColor;
	m_pBgColor = NULL;
	if(m_pImgBg1) delete m_pImgBg1;
	m_pImgBg1 = NULL;
	if(m_pImgBg2) delete m_pImgBg2;
	m_pImgBg2 = NULL;
	if(m_pImgBg3) delete m_pImgBg3;
	m_pImgBg3 = NULL;
	if(m_pImgBg4) delete m_pImgBg4;
	m_pImgBg4 = NULL;
	if(m_pImgBg5) delete m_pImgBg5;
	m_pImgBg5 = NULL;
	if(m_pImgBg6) delete m_pImgBg6;
	m_pImgBg6 = NULL;
	if(m_pImgBg7) delete m_pImgBg7;
	m_pImgBg7 = NULL;
	if(m_pImgBg8) delete m_pImgBg8;
	m_pImgBg8 = NULL;
	if(m_pImgBg9) delete m_pImgBg9;
	m_pImgBg9 = NULL;
}

BOOL
LAutoLayout::xLoadProfileSetting( LPCSTR szProfile, MYPROFILEMAP_ENTRY* pTable, BOOL bIncImage, CWnd* pWnd )
{
	int i = 0;
	CRect	rect;

	CString strSkinProfile;
	strSkinProfile = m_sSkinPath;
	strSkinProfile += CString(szProfile);
	//Get Table Size, Table end in NULL.
	while( pTable[i++].nID != 0 )	{
	}
	
	if(i<2)	//must has Panel Info
		return FALSE;
	//Resize Table memory.
	if(m_pResizeTable)	delete []m_pResizeTable;
	m_pResizeTable = new MYRESIZEMAP_ENTRY[i-2];

	//Table Size is not include [Panle] sec
	m_nTableSize = i-2;

	if( pTable[0].nID )	{	
		al_GetSettingResizeRect(pTable[0].pSecName, "min_client", strSkinProfile, &rect);
		m_MinSize.cx = rect.right; m_MinSize.cy = rect.bottom;

		al_GetSettingResizeRect(pTable[0].pSecName, "init_rect", strSkinProfile, &rect);
		m_InitSize.cx = rect.right; m_InitSize.cy = rect.bottom;
	}

	i = 1;	//Start in 1, 0 is Panel info, from 1 to end is control info
	TCHAR	buf[MAX_PATH];
	COLORREF	colorTemp;
	while( i <= m_nTableSize )	{
		al_GetSettingResizeRect(pTable[i].pSecName, "resize_rect", strSkinProfile, rect);
		m_pResizeTable[i-1].nID =  pTable[i].nID;
		m_pResizeTable[i-1].rcRect =  rect;

		//CButtonEx LoadBitmap
		if( al_GetSettingString(pTable[i].pSecName, "button_image", (LPTSTR)(LPCTSTR)strSkinProfile, buf ) )	{
			ASSERT(pTable[i].pControl);
			CString	str = m_sSkinPath;
			str += CString(buf);
			if( pTable[i].pControl )	{
				((CButtonEx*)(pTable[i].pControl))->ModifyStyle(0,BS_OWNERDRAW);
				((CButtonEx*)(pTable[i].pControl))->LoadBitmap(str);
			}
			//Load Font setting
			FontInfo	font;
			al_GetSettingString(pTable[i].pSecName, "check_font", (LPTSTR)(LPCTSTR)strSkinProfile, buf );
			xReadFontSection( strSkinProfile, buf, font );
			if( pTable[i].pControl )	{
				((CButtonEx*)(pTable[i].pControl))->SetTextFont(CString(font.szFontName), font.nFontSize, font.nFontStyle);
				((CButtonEx*)(pTable[i].pControl))->SetTextColor(font.nNormalColor, &font.nDownColor, &font.nGrayColor);
				((CButtonEx*)(pTable[i].pControl))->SetHTextColor(font.nHightColor);
			}
		}

		//CRescaleButton LoadBitmap
		if( al_GetSettingRect(pTable[i].pSecName, "part_width", (LPTSTR)(LPCTSTR)strSkinProfile, rect ) )	{
			ASSERT(pTable[i].pControl);
			CString	str;
			al_GetSettingString(pTable[i].pSecName, "button_imageL", (LPTSTR)(LPCTSTR)strSkinProfile, buf);
			if( pTable[i].pControl )	{
				str = m_sSkinPath;
				str += CString(buf);
				((CRescaleButton*)(pTable[i].pControl))->ModifyStyle(0,BS_OWNERDRAW);
				
				CImageArray* pBgImg = new CImageArray(str);
				//Load Ico File
				if( al_GetSettingString(pTable[i].pSecName, "button_ico", (LPTSTR)(LPCTSTR)strSkinProfile, buf) )	{
					str = m_sSkinPath;
					str += CString(buf);
					HICON hIcon = ExtractIcon( ::afxGetApp()->m_hInstance,str, 0 );
					CImage theImg(hIcon);
					DestroyIcon(hIcon);
					int	 nPosX = pBgImg->Width() / 4;
					int	 nPosY = pBgImg->Height();
					//Center the Ico
					CRect	rcIconRect;
					rcIconRect.left = (nPosX-32)/2;
					rcIconRect.top = (nPosY-32)/2;
					rcIconRect.right = (nPosX-32)/2 + 32;
					rcIconRect.bottom = (nPosY-32)/2 + 32;

					theImg.DrawImage( *pBgImg, rcIconRect.left, rcIconRect.top, rcIconRect.Width(), rcIconRect.Height(), 0, 0, theImg.Width(), theImg.Height() );
					theImg.DrawImage( *pBgImg, rcIconRect.left + nPosX, rcIconRect.top, rcIconRect.Width(), rcIconRect.Height(), 0, 0, theImg.Width(), theImg.Height() );
					theImg.DrawImage( *pBgImg, rcIconRect.left + nPosX*2, rcIconRect.top, rcIconRect.Width(), rcIconRect.Height(), 0, 0, theImg.Width(), theImg.Height() );
					theImg.DrawImage( *pBgImg, rcIconRect.left + nPosX*3, rcIconRect.top, rcIconRect.Width(), rcIconRect.Height(), 0, 0, theImg.Width(), theImg.Height() );
				}
				((CRescaleButton*)(pTable[i].pControl))->LoadLeftBg(pBgImg);
			}
			al_GetSettingString(pTable[i].pSecName, "button_imageC", (LPTSTR)(LPCTSTR)strSkinProfile, buf);
			if( pTable[i].pControl )	{
				str = m_sSkinPath;
				str += CString(buf);
				((CRescaleButton*)(pTable[i].pControl))->LoadCenterBg(str);
			}
			al_GetSettingString(pTable[i].pSecName, "button_imageR", (LPTSTR)(LPCTSTR)strSkinProfile, buf);
			if( pTable[i].pControl )	{
				str = m_sSkinPath;
				str += CString(buf);
				((CRescaleButton*)(pTable[i].pControl))->LoadRightBg(str);
			}
			if( pTable[i].pControl )	{
				((CRescaleButton*)(pTable[i].pControl))->
					SetButtonPartWidth( rect.left, rect.top, rect.Width(), rect.Height() );
			}
			//Load Font setting
			FontInfo	font;
			al_GetSettingString(pTable[i].pSecName, "check_font", (LPTSTR)(LPCTSTR)strSkinProfile, buf );
			xReadFontSection( strSkinProfile, buf, font );
			if( pTable[i].pControl )	{
				((CRescaleButton*)(pTable[i].pControl))->SetTextFont(CString(font.szFontName), font.nFontSize, font.nFontStyle);
				((CRescaleButton*)(pTable[i].pControl))->SetTextColor(font.nNormalColor, &font.nDownColor, &font.nGrayColor);
				((CRescaleButton*)(pTable[i].pControl))->SetHTextColor(font.nHightColor);
			}
		}

		//CCheckEx LoadBitmap
		if( al_GetSettingString(pTable[i].pSecName, "check_image", (LPTSTR)(LPCTSTR)strSkinProfile, buf ) )	{
			ASSERT(pTable[i].pControl);
			CString	str = m_sSkinPath;
			str += CString(buf);
			if( pTable[i].pControl )	{
				((CCheckEx*)(pTable[i].pControl))->ModifyStyle(0,BS_OWNERDRAW);
				((CCheckEx*)(pTable[i].pControl))->LoadBitmap(str);
			}
			//Load Font setting
			FontInfo	font;
			al_GetSettingString(pTable[i].pSecName, "check_font", (LPTSTR)(LPCTSTR)strSkinProfile, buf );
			xReadFontSection( strSkinProfile, buf, font );
			if( pTable[i].pControl )	{
				((CCheckEx*)(pTable[i].pControl))->SetTextFont(CString(font.szFontName), font.nFontSize, font.nFontStyle);
				((CCheckEx*)(pTable[i].pControl))->SetTextColor(font.nNormalColor, &font.nDownColor, &font.nGrayColor);
				((CCheckEx*)(pTable[i].pControl))->SetHTextColor(font.nHightColor);
			}
		}

		//CRadioEx LoadBitmap
		if( al_GetSettingString(pTable[i].pSecName, "radio_image", (LPTSTR)(LPCTSTR)strSkinProfile, buf ) )	{
			CString	str = m_sSkinPath;
			str += CString(buf);
			ASSERT(pTable[i].pControl);
			if( pTable[i].pControl )	{
				((CRadioEx*)(pTable[i].pControl))->ModifyStyle(0,BS_OWNERDRAW);
				((CRadioEx*)(pTable[i].pControl))->LoadBitmap(str);
			}
			//Load Font setting
			FontInfo	font;
			al_GetSettingString(pTable[i].pSecName, "radio_font", (LPTSTR)(LPCTSTR)strSkinProfile, buf );
			xReadFontSection( strSkinProfile, buf, font );
			if( pTable[i].pControl )	{
				((CRadioEx*)(pTable[i].pControl))->SetTextFont(CString(font.szFontName), font.nFontSize, font.nFontStyle);
				((CRadioEx*)(pTable[i].pControl))->SetTextColor(font.nNormalColor, &font.nDownColor, &font.nGrayColor);
				((CRadioEx*)(pTable[i].pControl))->SetHTextColor(font.nHightColor);
			}
		}
		//CStaticEx Load Font Setting
		if( al_GetSettingString(pTable[i].pSecName, "static_font", (LPTSTR)(LPCTSTR)strSkinProfile, buf ) )	{
			FontInfo	font;
			xReadFontSection( strSkinProfile, buf, font );
			ASSERT(pTable[i].pControl);
			if( pTable[i].pControl )	{
				((CStaticEx*)(pTable[i].pControl))->SetTextFont(CString(font.szFontName), font.nFontSize, font.nFontStyle);
				((CStaticEx*)(pTable[i].pControl))->SetTextColor(&font.nNormalColor, &font.nGrayColor);
				if(font.nBackColor!=NBKCOLOR)
					((CStaticEx*)(pTable[i].pControl))->SetBrushColor(font.nBackColor);
			}
		}
		//CProgressCtrlEx Initial
		if( al_GetSettingColor(pTable[i].pSecName, "prog_bar_color", (LPTSTR)(LPCTSTR)strSkinProfile, colorTemp ) )	{
			ASSERT(pTable[i].pControl);
//			if( pTable[i].pControl )	{
//				CBrush bhBar;
//				bhBar.CreateSolidBrush(colorTemp);
//				((CProgressCtrlEx*)(pTable[i].pControl))->SetBarBrush(&bhBar);
//			}
			al_GetSettingColor(pTable[i].pSecName, "prog_bg_color", (LPTSTR)(LPCTSTR)strSkinProfile, colorTemp);
			if( pTable[i].pControl )	{
				((CProgressCtrlEx*)(pTable[i].pControl))->SetBkColor(colorTemp);
			}
			if(al_GetSettingColor(pTable[i].pSecName, "prog_text_color1", (LPTSTR)(LPCTSTR)strSkinProfile, colorTemp)){
				COLORREF colorTemp2;			
				al_GetSettingColor(pTable[i].pSecName, "prog_text_color2", (LPTSTR)(LPCTSTR)strSkinProfile, colorTemp2);
				if( pTable[i].pControl )	{
					((CProgressCtrlEx*)(pTable[i].pControl))->SetTextColor(colorTemp,colorTemp2);
				}
			}
		}

		i++;
	}
	return TRUE;
}


BOOL
LAutoLayout::xDynamicRescale( CWnd* pWnd )
{
	if( m_pResizeTable==NULL )
		return FALSE;
	if( pWnd->m_hWnd==NULL )
		return FALSE;
		
	CRect rectClient, rectWindow;
	pWnd->GetWindowRect(&rectWindow);
	pWnd->GetClientRect(&rectClient);
	m_BorderSize.cx = rectWindow.Width()-rectClient.Width();
	m_BorderSize.cy = rectWindow.Height()-rectClient.Height();

	CRect rcControl, rect;
	CWnd*	pControl;
	for( int i=0; i<m_nTableSize; i++ )	{
		rect = m_pResizeTable[i].rcRect;
		rcControl.left = (rect.left>0) ? rectClient.left + rect.left: rectClient.right + rect.left;
		rcControl.right = (rect.right>0) ? rectClient.left + rect.right: rectClient.right + rect.right;
		rcControl.top = (rect.top>0) ? rectClient.top + rect.top: rectClient.bottom + rect.top;
		rcControl.bottom = (rect.bottom>0) ? rectClient.left + rect.bottom: rectClient.bottom + rect.bottom;
		pControl = pWnd->GetDlgItem(m_pResizeTable[i].nID);
		if(pControl)
			pControl->MoveWindow(&rcControl);
	}

	return TRUE;
}

BOOL
LAutoLayout::xStaticRescale( CWnd* pWnd )
{
	if( m_pResizeTable==NULL )
		return FALSE;
	if( pWnd->m_hWnd==NULL )
		return FALSE;
		
	CRect rect;
	CWnd* pControl;
	for( int i=0; i<m_nTableSize; i++ )	{
		rect = m_pResizeTable[i].rcRect;
		pControl = pWnd->GetDlgItem(m_pResizeTable[i].nID);
		if(pControl)
			pControl->MoveWindow(&rect);
	}
	return TRUE;
}

BOOL
LAutoLayout::xFillBackground( CSize winSize, CImage* pGraphics )
{
#define	DRAWIMAGE( PIMG, STARTX, STARTY )	\
	(PIMG)->Draw( *pGraphics, (STARTX), (STARTY), ((PIMG)->Width()), ((PIMG)->Height()) );
	
	BOOL bRet = FALSE;
	
	switch(m_nBgMode)	{
	case BM_None:
		bRet = TRUE;
		break;
	case BM_FillColor:	{
			ASSERT(m_pBgColor);
			pGraphics->FillColor(*m_pBgColor);
		}
		break;
	case BM_SingleTiled:	{
			ASSERT(m_pImgBg1);
			int	nXCount = (winSize.cx + m_pImgBg1->Width() - 1)/m_pImgBg1->Width();
			int	nYCount = (winSize.cy + m_pImgBg1->Height() - 1)/m_pImgBg1->Height();
			for( int y=0; y<nYCount; y++)
				for( int x=0; x<nXCount; x++)
					DRAWIMAGE( m_pImgBg1, x*m_pImgBg1->Width(), y*m_pImgBg1->Height() );
		}
		break;
	case BM_SingleStretched:	{
			ASSERT(m_pImgBg1);
			m_pImgBg1->Draw( *pGraphics, 0, 0, winSize.cx, winSize.cy );
		}
		break;
	case BM_HorizontalTiled:
		break;
	case BM_VerticalTiled:
		break;
	case BM_NinePalette:	{
			int	nXCount = winSize.cx - m_pImgBg1->Width() - m_pImgBg3->Width();
			nXCount += (m_pImgBg2->Width() - 1);
			nXCount /= m_pImgBg2->Width();
			int	nYCount = winSize.cy - m_pImgBg1->Height() - m_pImgBg7->Height();
			nYCount += (m_pImgBg4->Height() - 1);
			nYCount /= m_pImgBg4->Height();

			int x, y;
			//畫第一列, Draw first column
			DRAWIMAGE( m_pImgBg1, 0, 0 );
			for( x=0; x<nXCount; x++)
				DRAWIMAGE( m_pImgBg2, m_pImgBg1->Width() + x*m_pImgBg2->Width(), 0 );
			DRAWIMAGE( m_pImgBg3, winSize.cx - m_pImgBg3->Width() , 0 );

			//畫其他列, Draw others column
			for( y=0; y<nYCount; y++)	{
				DRAWIMAGE( m_pImgBg4, 0, m_pImgBg1->Height() + y*m_pImgBg4->Height() );
				for( x=0; x<nXCount; x++)
					DRAWIMAGE( m_pImgBg5, m_pImgBg4->Width() + x*m_pImgBg5->Width(), m_pImgBg1->Height() + y*m_pImgBg5->Height() );
				DRAWIMAGE( m_pImgBg6, winSize.cx-m_pImgBg6->Width(), m_pImgBg1->Height() + y*m_pImgBg6->Height() );
			}

			//畫最後一列, Draw last column
			DRAWIMAGE( m_pImgBg7, 0, winSize.cy - m_pImgBg7->Height() );
			for( x=0; x<nXCount; x++)
				DRAWIMAGE( m_pImgBg8, m_pImgBg7->Width() + x*m_pImgBg8->Width(), winSize.cy - m_pImgBg7->Height() );
			DRAWIMAGE( m_pImgBg9, winSize.cx - m_pImgBg9->Width() , winSize.cy - m_pImgBg7->Height() );
		}
		break;
	default:
		break;
	};

	return bRet;
}

BOOL
LAutoLayout::xLoadControlText( MYTEXTMAP_ENTRY* pTable, CWnd* pWnd )
{
	int i = 0;
	while( pTable[++i].nID != 0 )	{
	}
	if(i<1)	return FALSE;	//must has Panel Info
		
	CString	str;
	//Table index 0 is Window Title
	if( pTable[0].nID )	{
		str.LoadString(pTable[0].nTextID);
		pWnd->SetWindowText(str);
	}

	//Start in 1, 0 is Window text, from 1 to end is control text
//	CRescaleButton *pControl;
	while( --i > 0 )	{
		//Set Control Text
		if(pTable[i].nTextID)	{
			str.LoadString(pTable[i].nTextID);
			pWnd->SetDlgItemText( pTable[i].nID, str );
//			(pWnd->GetDlgItem(pTable[i].nID))->SetWindowText(str);
		}
		//Set Control Tips
		if(pTable[i].nTipsID)	{
			CButtonEx* pControl = (CButtonEx*)pWnd->GetDlgItem(pTable[i].nID);
			pControl->SetToolTipText(pTable[i].nTipsID, TRUE);
		}


	}

	return TRUE;
}

void
LAutoLayout::xReadFontSection( LPCTSTR szProfile, LPCTSTR szSecName, FontInfo& outFontInfo )
{
	CString	szGProfile;
	szGProfile = m_sModulePath;
	szGProfile += CString("\\Skin\\Font.ini");

	if( !al_GetSettingString(szSecName, _T("font_name"), szProfile, outFontInfo.szFontName) )
		if( !al_GetSettingString("FontGlobal", _T("font_name"), szProfile, outFontInfo.szFontName) )
			if( !al_GetSettingString("Font", "facename", (LPTSTR)(LPCTSTR)szGProfile, outFontInfo.szFontName) )
				_tcscpy(outFontInfo.szFontName, "System");

	if( !al_GetSettingInt(szSecName, _T("font_size"), szProfile, outFontInfo.nFontSize) )
		if( !al_GetSettingInt("FontGlobal", _T("font_size"), szProfile, outFontInfo.nFontSize) )
			if( !al_GetSettingInt("Font", "fontsize", (LPTSTR)(LPCTSTR)szGProfile, outFontInfo.nFontSize) )
				outFontInfo.nFontSize = 10;

	if( !al_GetSettingFontStyle(szSecName, "font_style", szProfile, outFontInfo.nFontStyle) )
		if( !al_GetSettingFontStyle("FontGlobal", "font_style", szProfile, outFontInfo.nFontStyle) )
			outFontInfo.nFontStyle = 0;

	if( !al_GetSettingColor(szSecName, "font_color_normal", szProfile, outFontInfo.nNormalColor) )
		if( !al_GetSettingColor("FontGlobal", "font_color_normal", szProfile, outFontInfo.nNormalColor) )
			outFontInfo.nNormalColor = RGB(0, 0, 0);

	if( !al_GetSettingColor(szSecName, "font_color_down", szProfile, outFontInfo.nDownColor) )
		if( !al_GetSettingColor("FontGlobal", "font_color_down", szProfile, outFontInfo.nDownColor) )
			outFontInfo.nDownColor = RGB(0, 0, 0);
				
	if( !al_GetSettingColor(szSecName, "font_color_gray", szProfile, outFontInfo.nGrayColor) )
		if( !al_GetSettingColor("FontGlobal", "font_color_gray", szProfile, outFontInfo.nGrayColor) )
			outFontInfo.nGrayColor = RGB(0, 0, 0);

	if( !al_GetSettingColor(szSecName, "font_color_high", szProfile, outFontInfo.nHightColor) )
		if( !al_GetSettingColor("FontGlobal", "font_color_high", szProfile, outFontInfo.nHightColor) )
			outFontInfo.nHightColor = RGB(0, 0, 0);

	if( !al_GetSettingColor(szSecName, "font_color_bk", szProfile, outFontInfo.nBackColor) )
		if( !al_GetSettingColor("FontGlobal", "font_color_bk", szProfile, outFontInfo.nBackColor) )
			outFontInfo.nBackColor = NBKCOLOR;
}
