#include "stdafx.h"
#include "CPage.h"

////////////////////////////////////////////////////////////////////////////
//	Desc: Constructor
//	params: RECT ( the area allowed to print to ) CDC* Display context nMapMode the mapping mode
//	Returns: nada
///////////////////////////////////////////////////////////////////////////
CPage::CPage(RECT rectDraw, CDC* pDC, int nMapMode)
{
	if(nMapMode !=MM_TEXT && nMapMode != MM_ANISOTROPIC)
		nMapMode=MM_TEXT;
	pDC->SetMapMode(nMapMode );	
	m_PrtDesc.tm=&tm;
	StateInd=-1;
	m_PrintMode=GetPrinterMode(pDC);
	if(nMapMode==MM_ANISOTROPIC)
	{	
    	pDC->SetWindowOrg (0, 0 );
		// if you change these numbers you must also change them in maxWidth 
		// and maxLength for ConvertToMappedUnits()  conversion to be correct
    	pDC->SetWindowExt (1000, 1000 );
    	pDC->SetViewportOrg (rectDraw.left,rectDraw.top );
    	pDC->SetViewportExt (rectDraw.right, rectDraw.bottom );
		//	SEE ABOVE
		m_PrtDesc.n_maxWidth=1000;
		m_PrtDesc.n_maxLength=1000;
		m_PrtDesc.rc.left=0;
		m_PrtDesc.rc.top=0;
		//	SEE ABOVE
		m_PrtDesc.rc.right=1000;
		m_PrtDesc.rc.bottom=1000;
		
	}   
	else
	{
		m_PrtDesc.n_maxWidth=rectDraw.right;
		m_PrtDesc.n_maxLength=rectDraw.bottom;
		m_PrtDesc.rc.left=rectDraw.left;
		m_PrtDesc.rc.top=rectDraw.top;
		m_PrtDesc.rc.right=rectDraw.right;
		m_PrtDesc.rc.bottom=rectDraw.bottom;
	}   
	//	all the textmetrics we need 
	m_PixPerInchX=pDC->GetDeviceCaps(LOGPIXELSX);
	m_PixPerInchY=pDC->GetDeviceCaps(LOGPIXELSY);
	//	determine how many inches wide and deep the rectangle is
	m_WidthInches=rectDraw.right/(double)m_PixPerInchX;
	m_LengthInches=rectDraw.bottom/(double)m_PixPerInchY;


    m_PrtDesc.pDC=pDC;                    
    //	default font stuff
    m_PrtDesc.FontName="Tahoma";
    m_PrtDesc.PointSize=10;
	m_PrtDesc.m_NextCharPos=0;
	//	default print flags
	m_PrtDesc.uFillFlags=FILL_NONE;  	
	m_PrtDesc.uTextFlags=TEXT_NOCLIP|TEXT_LEFT;	
	m_PrtDesc.uPenFlags=PEN_SOLID;
	//	do not change this value
	m_PrtDesc.m_MinDisplacement=10;
	//	modify to increase line spacing but should be no smaller than this
	m_Spacing=1.0;
	pUserFunc=NULL;

	//	Test Stuff
	m_PrtDesc.MarginOffset=0;
	if(m_PrintMode== DMORIENT_LANDSCAPE)
	{
		if(m_WidthInches > 10.9)  // we got a fax driver here
		{
			double Diff=m_WidthInches-10.5;
			Diff=ConvertToMappedUnits(Diff,HORZRES);
				m_PrtDesc.MarginOffset=(int) (Diff/2);
		}
	}
	else
	{
		if(m_WidthInches > 8.0)  // we got a fax driver here
		{
			double Diff=m_WidthInches-8.0;
			Diff=ConvertToMappedUnits(Diff,HORZRES);
				m_PrtDesc.MarginOffset=(int) (Diff/2);
		}
	}
	////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Sets the address of the user supplied function to be
//			called for the virtual info printing functions
//	params:	Pointer to a functio of type PF_REMOTE
//	Returns:	
///////////////////////////////////////////////////////////////////////////
void CPage::SetUserFunction(PF_REMOTE ptr)
{
	pUserFunc=ptr;
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Returns the current print mode 
//	params:	The display context* for the output
//	Returns:	either DMORIENT_LANDSACPE or DMORIENT_PORTRIAT
///////////////////////////////////////////////////////////////////////////

int CPage::GetPrinterMode(CDC* pDC)
{
	int Mode;
	if(!pDC->IsPrinting())
		return 0;
	PRINTDLG* pPrintDlg = new PRINTDLG;
	afxGetApp()->GetPrinterDeviceDefaults(pPrintDlg);
	DEVMODE* lpDevMode = (DEVMODE*)::GlobalLock(pPrintDlg->hDevMode);		
	Mode= lpDevMode->dmOrientation;
	::GlobalUnlock(pPrintDlg->hDevMode);	
	delete pPrintDlg;
	return Mode;
}


////////////////////////////////////////////////////////////////////////////
//	Desc: Destructor deletes the regions from the list if any
//	params:	None
//	Returns: None
///////////////////////////////////////////////////////////////////////////
CPage::~CPage()
{

	CPrintRegion* pRegion;
	for(int y=0;y < m_RegionList.GetSize();++y)
	{                 
		pRegion=(CPrintRegion*)m_RegionList[y];
		delete pRegion;
	}
	m_RegionList.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Saves the current printer variables to a psuedo stack
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::SaveState()
{
	m_SaveState[++StateInd]=m_PrtDesc;
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Restores printer variables saved by above
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::RestoreState()
{
	if(StateInd==-1)
		return;
	m_PrtDesc=m_SaveState[StateInd--];
}



////////////////////////////////////////////////////////////////////////////
//	PARAMETER ACCESS ROUTINES
////////////////////////////////////////////////////////////////////////////
//	Desc: Changes the width of the default allowable printing area
//	params: The new right side coordinates in mapping units:If 0 there is no change
//	if -1 margin is set to maximum allowable size
//	Returns: The previous coordinates
///////////////////////////////////////////////////////////////////////////
int CPage::SetRightMargin(int w)
{
	int temp=m_PrtDesc.rc.right;
	if(w > 0)
		m_PrtDesc.rc.right=w;
	if(w==-1)
		m_PrtDesc.rc.right=m_PrtDesc.n_maxWidth;
	return temp;
}
////////////////////////////////////////////////////////////////////////////
//	Desc: Sets new coordinates for allowable printing rectangle depth
//	params: The new coordinate in mapping units:If 0 there is no change
//	if -1 margin is set to maximum allowable size
//	Returns: The old coordinate
///////////////////////////////////////////////////////////////////////////
int CPage::SetBottomMargin(int w)
{
	int temp=m_PrtDesc.rc.bottom;
	if(w > 0)
		m_PrtDesc.rc.bottom=w;
	if(w== -1)
		m_PrtDesc.rc.right=m_PrtDesc.n_maxLength;
	return temp;
}

////////////////////////////////////////////////////////////////////////////
//	Desc: See above
//	params: Same as above except units are in inches 
//	Returns: See above
///////////////////////////////////////////////////////////////////////////
double CPage::SetRightMargin(double w)
{
	int temp=m_PrtDesc.rc.right;
	if(w > 0)
		m_PrtDesc.rc.right=ConvertToMappedUnits(w,HORZRES);
	if(w==-1)
		m_PrtDesc.rc.right=m_PrtDesc.n_maxWidth;
	return ConvertToInches(temp,HORZRES);
}
////////////////////////////////////////////////////////////////////////////
//	Desc: See Above
//	params: Same as above except units are in inches: if 0 no change
//	Returns: See Above
///////////////////////////////////////////////////////////////////////////
double CPage::SetBottomMargin(double w)
{
	int temp=m_PrtDesc.rc.bottom;
	if(w > 0)
		m_PrtDesc.rc.bottom=ConvertToMappedUnits(w,VERTRES);
	if(w==-1)
		m_PrtDesc.rc.right=m_PrtDesc.n_maxLength;
	return ConvertToInches(temp,VERTRES);
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Changes the space returned for the next logical line
//	params: The constant applied to the fontsize to produce the spacing The formula is
//	ConvertToMappedUnits(PointSize/72.0,VERTRES)*(m_Spacing-1))
//	Returns: The old spacing factor
///////////////////////////////////////////////////////////////////////////
double	CPage::SetLineSpacing(double Spacing)
{
	double temp=m_Spacing;
	if(Spacing > 0)
		m_Spacing=Spacing;
	return temp;
}
////////////////////////////////////////////////////////////////////////////
//	Desc: Changes the font face used for printing
//	params: The new face name IE Courier 
//	Returns: he old font face
///////////////////////////////////////////////////////////////////////////
LPCTSTR	CPage::SetFont(LPCSTR FontName)
{
	static	TCHAR	buff[40];
	_tcscpy(buff,m_PrtDesc.FontName);
	if(FontName != NULL)
		m_PrtDesc.FontName=FontName;
	return buff;
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Sets The text Color if the device supports colored text
//	params: The color
//	Returns:The old color
///////////////////////////////////////////////////////////////////////////
COLORREF CPage::SetColor(COLORREF Color)                                 
{
		return m_PrtDesc.pDC->SetTextColor(Color);
}

////////////////////////////////////////////////////////////////////////////
//	Desc:		changes the default font size
//	params:		the size
//	Returns:	the old size
///////////////////////////////////////////////////////////////////////////
int CPage::SetFontSize(int sz)
{
	int temp=m_PrtDesc.PointSize;
	m_PrtDesc.PointSize=sz;      
	return temp;
}

////////////////////////////////////////////////////////////////////////////
//	Desc:		fetch the display context used by this object
//	params:		
//	Returns:	a pointer to the display context
///////////////////////////////////////////////////////////////////////////
CDC* CPage::GetDisplayContext()
{
	return m_PrtDesc.pDC;
}

////////////////////////////////////////////////////////////////////////////
//	Desc: returns the next logical print column. Used in printing continious text that
//        may have different text attributes
//	params:Convert flag to convert to inches.AddOffset flag add a extra space 
//	Returns: the logical column offset.double is used so it can handle all mapping units.
//			 if Convert is true the result is in inches else in device units.
///////////////////////////////////////////////////////////////////////////
double	CPage::GetNextLogicalColumn(BOOL Convert,BOOL AddOffset)
{
	if(Convert==FALSE)
	{
		if(AddOffset)
			return m_nNextPos+tm.tmMaxCharWidth;
		else
			return m_nNextPos;
	}
	else
	{
		if(AddOffset)
			return ConvertToInches(m_nNextPos+tm.tmMaxCharWidth,HORZRES);
		else
			return ConvertToInches(m_nNextPos,HORZRES);
	}
}

////////////////////////////////////////////////////////////////////////////
//	UNIT CONVERSION ROUTINES
////////////////////////////////////////////////////////////////////////////
//	Desc:  		Converts inches to physical units based on the mapping mode
//	params:     dwInch ( the number of inches bWidth either VERTRES or HORTZRES
//	Returns:    the physical device displacment representing the number of inches
//
//	conversion is done as follows:mapmode=MM_TEXT;
//	The number of inches are multiplied by the constant for the # of pixels per inch
//	in the diminsion requested.The value(s) for the # of pixels are set at creation
//	of the object and are retrieved from the GetDevCaps function
//	mapmode=MM_ANISOTROPIC:
//	The size in inches for the requested diminsion are devided by 1000 to get the
//	size in inches for each unit. This value is devided into the requested size
//	to return the physical offset, for example
//	width of display is 10 in
//	10/1000=.01. So we want to go three inches to the right 3.0/.01=300 logical
//	units
//	hight of display is 12 in
//	12/1000=.012. To move 5 inches down 5.0/.012=417 logical units
//	NOTES:
//	These conversion routines attempt to normalize positioning between the two allowed
//	mapping modes but are not exact due to cumalitive rounding errors.While in MM_TEXT
//	mode the units will be much smaller than in MM_ANISOTROPIC mode and therefore the
//	class can position the text with a much finer position. When in ANISOTROPIC mode
//	the smallest unit can be as much as 2.8 pixels per unit resulting in small but
//	noticable differences in positioning
///////////////////////////////////////////////////////////////////////////
int	CPage::ConvertToMappedUnits(double dwInch,int bWidth)
{   
	double tempx,tempy;                              
	if(dwInch <=0)
		return 0;
	if(m_PrtDesc.pDC->GetMapMode()==MM_TEXT)
	{
		if(bWidth==HORZRES)
			return (int)(dwInch*m_PixPerInchX);
		else	
	 		return (int)(dwInch*m_PixPerInchY);
	} 		
	tempx=m_WidthInches/m_PrtDesc.n_maxWidth;
	tempy=m_LengthInches/m_PrtDesc.n_maxLength;
	if(bWidth==HORZRES)
		return (int)(dwInch/tempx);
	else	
 		return (int)(dwInch/tempy);
}

////////////////////////////////////////////////////////////////////////////
//	Desc:		Converts physical device units to inches
//	params:		value ( the # of physical units to convert bWidth HORZRES or VERTRES
//	Returns:    The number of inches defined by the displacment
///////////////////////////////////////////////////////////////////////////
double	CPage::ConvertToInches(int value,int bWidth)
{   
	double tempx,tempy;                              
	if(value <=0)
		return 0;

	if(m_PrtDesc.pDC->GetMapMode()==MM_TEXT)
	{
		if(bWidth==HORZRES)
			return ((double)value/(double)m_PixPerInchX);
		else	
	 		return ((double)value/(double)m_PixPerInchY);
	} 		
	tempx=m_WidthInches/(double)m_PrtDesc.n_maxWidth;
	tempy=m_LengthInches/(double)m_PrtDesc.n_maxLength;
	if(bWidth==HORZRES)
		return (double)(value*tempx);
	else	
 		return (double)(value*tempy);
}

////////////////////////////////////////////////////////////////////////////
//	CONVERSION ROUTINES pixels to inches
////////////////////////////////////////////////////////////////////////////
//	Desc:	Converts a position in inches to logical units 
//	params:  Row ( same as rect.top) Col ( same as rect.left)
//	Returns: none but parameters are changed indirectly
///////////////////////////////////////////////////////////////////////////
void CPage::ConvertPosition(double& Row,double& Col)
{   
	Row=ConvertToMappedUnits(Row,VERTRES);
	Col=ConvertToMappedUnits(Col,HORZRES);
}			

////////////////////////////////////////////////////////////////////////////
//	Desc:	same as above but does the whole rectangle at once
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::ConvertArea(double& top,double& left,double& bottom,double& right)
{
	top=ConvertToMappedUnits(top,VERTRES);
	left=ConvertToMappedUnits(left,HORZRES);
	bottom=ConvertToMappedUnits(bottom,VERTRES);
	right=ConvertToMappedUnits(right,HORZRES);
}	

////////////////////////////////////////////////////////////////////////////
//	TEXT PRINT ROUTINES
////////////////////////////////////////////////////////////////////////////
//	Desc:	Very low level call. All Print Routines end up here. Do not call direct
//	params: LPCSTR the text to print StartPos starting column Text flasg and Pointsize
//	Returns: The next logical vertical print position
///////////////////////////////////////////////////////////////////////////

int CPage::Print(LPCTSTR Text,int StartPos,UINT flags,int PointSize)
{       
	if(StartPos > SetRightMargin(0))
		return m_PrtDesc.rc.top;
	if(m_PrtDesc.rc.top > SetBottomMargin(0))
		return m_PrtDesc.rc.top;
	//m_PrtDesc.rc.left+=MarginOffset;
	m_PrtDesc.Text=Text;
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       
	if(StartPos==-1)
		m_PrtDesc.rc.left=m_PrtDesc.m_NextCharPos;
	else
	    m_PrtDesc.rc.left=StartPos;
	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	return m_PrtDesc.LastPrintArea.bottom;
}



////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints using printf variable length print arguments
//	params: Row,Col-Location Coordinates arg list uses current default flags
//	Returns:The next logical print line using the current font size and line spacing
///////////////////////////////////////////////////////////////////////////
int CPage::Print(int row,int col,const TCHAR* fmt,...)
{
	va_list t;
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	m_PrtDesc.rc.top=row;
	int res=Print(Buffer,col,m_PrtDesc.uTextFlags,m_PrtDesc.PointSize);
	va_end(t);


	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return res+nLineSpacing;

}
////////////////////////////////////////////////////////////////////////////
//	Desc: same as above only position info is in inches, not device units
//	params:
//	Returns:same as above only in inches, not device units
///////////////////////////////////////////////////////////////////////////
double CPage::Print(double row,double col,const TCHAR* fmt,...)
{
	va_list t;
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row;
	int res=Print(Buffer,(int)col,m_PrtDesc.uTextFlags,m_PrtDesc.PointSize);
	va_end(t);


	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return ConvertToInches(res+nLineSpacing,VERTRES);

}
double CPage::Print(double row,double col,CString strText)
{
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row;
	int res=Print(strText,(int)col,m_PrtDesc.uTextFlags,m_PrtDesc.PointSize);

	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return ConvertToInches(res+nLineSpacing,VERTRES);

}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints using data supplied as return value from user function
//	params: Row,Col-Location Coordinates user ID to be supplied to user function
//	Returns:The next logical print line using the current font size and line spacing
///////////////////////////////////////////////////////////////////////////
int CPage::Print(int row,int col,int ID)
{
	if(pUserFunc==NULL)
		return 0;
	m_PrtDesc.rc.top=row;
	int res=Print(pUserFunc(ID),col,m_PrtDesc.uTextFlags,m_PrtDesc.PointSize);
	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return res+nLineSpacing;

}
////////////////////////////////////////////////////////////////////////////
//	Desc: same as above only position info is in inches, not device units
//	params:
//	Returns:same as above only in inches, not device units
///////////////////////////////////////////////////////////////////////////
double CPage::Print(double row,double col,int ID)
{
	if(pUserFunc==NULL)
		return 0;
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row;
	int res=Print(pUserFunc(ID),(int)col,m_PrtDesc.uTextFlags,m_PrtDesc.PointSize);
	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return ConvertToInches(res+nLineSpacing,VERTRES);

}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints by calling user defined function for data
//	params: Row,Col-Location Coordinates TextFlags Point size user id
//	Returns:The next logical print line using the current font size and line spacing
///////////////////////////////////////////////////////////////////////////
int CPage::Print(int row,int col,UINT TextFlags,int PointSize,int ID)
{
	if(pUserFunc==NULL)
		return 0;
	m_PrtDesc.rc.top=row;
	int res=Print(pUserFunc(ID),col,TextFlags,PointSize);
	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return res+nLineSpacing;

}
////////////////////////////////////////////////////////////////////////////
//	Desc: same as above only position info is in inches, not device units
//	params:
//	Returns:same as above only in inches, not device units
///////////////////////////////////////////////////////////////////////////
double CPage::Print(double row,double col,UINT TextFlags,int PointSize,int ID)
{
	if(pUserFunc==NULL)
		return 0;
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row;
	int res=Print(pUserFunc(ID),(int)col,TextFlags,PointSize);
	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return ConvertToInches(res+nLineSpacing,VERTRES);

}


////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints using printf variable length print arguments
//	params: Row,Col-Location Coordinates TextFlags Point size variable arg list
//	Returns:The next logical print line using the current font size and line spacing
///////////////////////////////////////////////////////////////////////////
int CPage::Print(int row,int col,UINT TextFlags,int PointSize,const TCHAR* fmt,...)
{
	va_list t;
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	m_PrtDesc.rc.top=row;
	int res=Print(Buffer,col,TextFlags,PointSize);
	va_end(t);


	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return res+nLineSpacing;

}
////////////////////////////////////////////////////////////////////////////
//	Desc: same as above only position info is in inches, not device units
//	params:
//	Returns:same as above only in inches, not device units
///////////////////////////////////////////////////////////////////////////
double CPage::Print(double row,double col,UINT TextFlags,int PointSize,const TCHAR* fmt,...)
{
	va_list t;
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row;
	int res=Print(Buffer,(int)col,TextFlags,PointSize);
	va_end(t);


	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return ConvertToInches(res+nLineSpacing,VERTRES);

}

////////////////////////////////////////////////////////////////////////////
//	Desc: Same as above except col is a pointer and is updated to the next locical print column
//	params:
//	Returns: The next logical print row
///////////////////////////////////////////////////////////////////////////
double CPage::Print(double row,double *col,UINT TextFlags,int PointSize,const TCHAR* fmt,...)
{
	va_list t;
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	ConvertPosition(row,*col);
	m_PrtDesc.rc.top=(int)row;
	int res=Print(Buffer,(int)(*col),TextFlags,PointSize);
	va_end(t);

	*col=GetNextLogicalColumn();

	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return ConvertToInches(res+nLineSpacing,VERTRES);
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Same as above except col is updates to the next logical column
//	params: See above
//	Returns: See above
///////////////////////////////////////////////////////////////////////////
int CPage::Print(int row,int *col,UINT TextFlags,int PointSize,const TCHAR* fmt,...)
{
	va_list t;
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	m_PrtDesc.rc.top=row;
	int res=Print(Buffer,*col,TextFlags,PointSize);
	va_end(t);


	*col=(int)GetNextLogicalColumn(FALSE);
	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	return res+nLineSpacing;

}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints using printf variable length print arguments
//	params:
//	Returns:The next logical print line using the current font size and line spacing
///////////////////////////////////////////////////////////////////////////
int CPage::Print(CPrintRegion* pRegion,int row,int col,UINT TextFlags,int PointSize,const TCHAR* fmt,...)
{
	UINT	OldMode,OldColor;
	va_list t;
	SaveState();
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	m_PrtDesc.rc.top=row+pRegion->FirstY;

	m_PrtDesc.rc.bottom=pRegion->bottom;
	m_PrtDesc.rc.right=pRegion->right;
	m_PrtDesc.rc.left=pRegion->FirstX;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	int res=Print(Buffer,col+pRegion->FirstX,TextFlags,PointSize);
	va_end(t);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	RestoreState();
	return  (res-pRegion->FirstY)+nLineSpacing;

}

////////////////////////////////////////////////////////////////////////////
//	Desc: same as above only position info is in inches, not device units
//	params:
//	Returns:same as above only in inches, not device units
///////////////////////////////////////////////////////////////////////////
double CPage::Print(CPrintRegion* pRegion,double row,double col,UINT TextFlags,int PointSize,const TCHAR* fmt,...)
{
	UINT	OldColor,OldMode;
	va_list t;
	SaveState();
	va_start(t,fmt);
	_vstprintf(Buffer,fmt,t);
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row+pRegion->FirstY;

	m_PrtDesc.rc.bottom=pRegion->bottom;
	m_PrtDesc.rc.right=pRegion->right;
	m_PrtDesc.rc.left=pRegion->FirstX;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	int res=Print(Buffer,(int)col+pRegion->FirstX,TextFlags,PointSize);
	va_end(t);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	RestoreState();
	return ConvertToInches( (int)( (res-pRegion->FirstY)+nLineSpacing),VERTRES);

}
////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints using user defined function for data
//	params:
//	Returns:The next logical print line using the current font size and line spacing
///////////////////////////////////////////////////////////////////////////
int CPage::Print(CPrintRegion* pRegion,int row,int col,UINT TextFlags,int PointSize,int ID)
{
	UINT	OldColor,OldMode;
	if(pUserFunc==NULL)
		return 0;
	SaveState();
	m_PrtDesc.rc.top=row+pRegion->FirstY;

	m_PrtDesc.rc.bottom=pRegion->bottom;
	m_PrtDesc.rc.right=pRegion->right;
	m_PrtDesc.rc.left=pRegion->FirstX;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	int res=Print(pUserFunc(ID),col+pRegion->FirstX,TextFlags,PointSize);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	RestoreState();
	return  (res-pRegion->FirstY)+nLineSpacing;

}

////////////////////////////////////////////////////////////////////////////
//	Desc: same as above only position info is in inches, not device units
//	params:
//	Returns:same as above only in inches, not device units
///////////////////////////////////////////////////////////////////////////
double CPage::Print(CPrintRegion* pRegion,double row,double col,UINT TextFlags,int PointSize,int ID)
{
	UINT	OldColor,OldMode;
	if(pUserFunc==NULL)
		return 0;
	SaveState();
	ConvertPosition(row,col);
	m_PrtDesc.rc.top=(int)row+pRegion->FirstY;

	m_PrtDesc.rc.bottom=pRegion->bottom;
	m_PrtDesc.rc.right=pRegion->right;
	m_PrtDesc.rc.left=pRegion->FirstX;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	int res=Print(pUserFunc(ID),(int)col+pRegion->FirstX,TextFlags,PointSize);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	int nLineSpacing = m_Spacing > 1 ? (int)(ConvertToMappedUnits(m_PrtDesc.PointSize/72.0,VERTRES)*(m_Spacing-1)):0;
	RestoreState();
	return ConvertToInches( (int)( (res-pRegion->FirstY)+nLineSpacing),VERTRES);

}
////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params: coordinates of the bounding rectangle flags pointsize and text to print
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,LPCSTR Text)
{
	if(Left > SetRightMargin(0))
		return;
	if(Top > SetBottomMargin(0))
		return;
	SaveState();
	m_PrtDesc.Text=Text;
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=Left;
    m_PrtDesc.rc.top=Top;
	m_PrtDesc.rc.right=Right;
    m_PrtDesc.rc.bottom=Bottom;
	    
	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,LPCSTR Text)
{
	if(Left > SetRightMargin(0.0))
		return;
	if(Top > SetBottomMargin(0.0))
		return;
	SaveState();
	m_PrtDesc.Text=Text;
	ConvertArea(Top,Left,Bottom,Right);
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=(int)Left;
    m_PrtDesc.rc.top=(int)Top;
	m_PrtDesc.rc.right=(int)Right;
    m_PrtDesc.rc.bottom=(int)Bottom;
	    
	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();
}


////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params: coordinates of the bounding rectangle flags pointsize and user supplied ID
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,int ID)
{
	if(pUserFunc==NULL)
		return;
	if(Left > SetRightMargin(0))
		return;
	if(Top > SetBottomMargin(0))
		return;
	SaveState();
	m_PrtDesc.Text=pUserFunc(ID);
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=Left;
    m_PrtDesc.rc.top=Top;
	m_PrtDesc.rc.right=Right;
    m_PrtDesc.rc.bottom=Bottom;
	    
	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params:  Calls user defined function for data
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,int ID)
{
	if(pUserFunc==NULL)
		return;
	if(Left > SetRightMargin(0.0))
		return;
	if(Top > SetBottomMargin(0.0))
		return;
	SaveState();
	m_PrtDesc.Text=pUserFunc(ID);
	ConvertArea(Top,Left,Bottom,Right);
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=(int)Left;
    m_PrtDesc.rc.top=(int)Top;
	m_PrtDesc.rc.right=(int)Right;
    m_PrtDesc.rc.bottom=(int)Bottom;
	    
	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(CPrintRegion* pRegion,int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,LPCSTR Text)
{
	UINT	OldColor,OldMode;
	SaveState();
	m_PrtDesc.Text=Text;
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=Left+pRegion->FirstX;
    m_PrtDesc.rc.top=Top+pRegion->FirstY;
	m_PrtDesc.rc.right=Right+pRegion->FirstX;
    m_PrtDesc.rc.bottom=Bottom+pRegion->FirstY;

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;
	
	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(CPrintRegion* pRegion,double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,LPCSTR Text)
{
	UINT	OldColor,OldMode;
	SaveState();
	m_PrtDesc.Text=Text;
	ConvertArea(Top,Left,Bottom,Right);
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=(int)Left+pRegion->FirstX;
    m_PrtDesc.rc.top=(int)Top+pRegion->FirstY;
	m_PrtDesc.rc.right=(int)Right+pRegion->FirstX;
    m_PrtDesc.rc.bottom=(int)Bottom+pRegion->FirstY;

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;
	
	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params:  Calls user defined function for data
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(CPrintRegion* pRegion,int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,int ID)
{
	UINT	OldColor,OldMode;

	if(pUserFunc==NULL)
		return;
	SaveState();
	m_PrtDesc.Text=pUserFunc(ID);
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=Left+pRegion->FirstX;
    m_PrtDesc.rc.top=Top+pRegion->FirstY;
	m_PrtDesc.rc.right=Right+pRegion->FirstX;
    m_PrtDesc.rc.bottom=Bottom+pRegion->FirstY;

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;
	
	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints text clipped to a bounding rectangle. See the header for default
//		    parameters. Allows for newspaper like columns
//	params:  Calls user defined function for data
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintColumn(CPrintRegion* pRegion,double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,int ID)
{
	UINT	OldColor,OldMode;
	if(pUserFunc==NULL)
		return ;
	SaveState();
	m_PrtDesc.Text=pUserFunc(ID);
	ConvertArea(Top,Left,Bottom,Right);
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=(int)Left+pRegion->FirstX;
    m_PrtDesc.rc.top=(int)Top+pRegion->FirstY;
	m_PrtDesc.rc.right=(int)Right+pRegion->FirstX;
    m_PrtDesc.rc.bottom=(int)Bottom+pRegion->FirstY;

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;
	
	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();
}





////////////////////////////////////////////////////////////////////////////
//	DRAWING ROUTINES ( SHAPES )
////////////////////////////////////////////////////////////////////////////
//	Desc: Draws a line from point top,left to bottom,right
//	params: position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void	CPage::Line(int top,int left,int bottom,int right,int LineSize,UINT flag)
{     
	SaveState();
	if(flag != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=flag;

	m_PrtDesc.rc.top=top;
	m_PrtDesc.rc.bottom=bottom;
	m_PrtDesc.rc.left=left;
	m_PrtDesc.rc.right=right;

	ThePrinter.DrawLine(&m_PrtDesc,LineSize);
	RestoreState();
}	

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a line using parameters passed
//	params: position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void	CPage::Line(double top,double left,double bottom,double right,int LineSize,UINT flag)
{           
	SaveState();
	if(flag != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=flag;
	ConvertArea(top,left,bottom,right);

	m_PrtDesc.rc.top=(int)top;
	m_PrtDesc.rc.bottom=(int)bottom;
	m_PrtDesc.rc.left=(int)left;
	m_PrtDesc.rc.right=(int)right;

	ThePrinter.DrawLine(&m_PrtDesc,LineSize);
	RestoreState();
}	

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a rectangle using parameters passed
//	params: position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::Box(int top,int left,int bottom,int right,int LineSize,UINT Fillflags,UINT PenFlags)
{
	SaveState();
	if(Fillflags != IGNORE_PARAM)
		m_PrtDesc.uFillFlags=Fillflags;			
	if(PenFlags != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=PenFlags;			

	m_PrtDesc.rc.top=top;		
	m_PrtDesc.rc.bottom=bottom;		
	m_PrtDesc.rc.left=left;		
	m_PrtDesc.rc.right=right;		

	ThePrinter.DrawRect(&m_PrtDesc,LineSize);
	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a rectangle using parameters passed
//	params: position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::Box(double top,double left,double bottom,double right,int LineSize,UINT Fillflags,UINT PenFlags)
{
	SaveState();
	if(Fillflags != IGNORE_PARAM)
		m_PrtDesc.uFillFlags=Fillflags;			
	if(PenFlags != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=PenFlags;			
	ConvertArea(top,left,bottom,right);

	m_PrtDesc.rc.top=(int)top;		
	m_PrtDesc.rc.bottom=(int)bottom;		
	m_PrtDesc.rc.left=(int)left;		
	m_PrtDesc.rc.right=(int)right;		

	ThePrinter.DrawRect(&m_PrtDesc,LineSize);
	RestoreState();
}
////////////////////////////////////////////////////////////////////////////
//	Desc: Draws a line from point top,left to bottom,right
//	params: CPrintRegion*, position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void	CPage::Line(CPrintRegion* pRegion,int top,int left,int bottom,int right,int LineSize,UINT flag)
{   
	UINT OldColor,OldMode;
	SaveState();
	if(flag != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=flag;

	m_PrtDesc.rc.top=(int)top+pRegion->FirstY;		
	m_PrtDesc.rc.bottom=(int)bottom+pRegion->FirstY;		
	m_PrtDesc.rc.left=(int)left+pRegion->FirstX;		
	m_PrtDesc.rc.right=(int)right+pRegion->FirstX;		

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.DrawLine(&m_PrtDesc,LineSize);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}	

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a line using parameters passed
//	params:CPrintRegion*, position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void	CPage::Line(CPrintRegion* pRegion,double top,double left,double bottom,double right,int LineSize,UINT flag)
{   
	UINT	OldColor,OldMode;
	SaveState();
	if(flag != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=flag;
	ConvertArea(top,left,bottom,right);

	m_PrtDesc.rc.top=(int)top+pRegion->FirstY;		
	m_PrtDesc.rc.bottom=(int)bottom+pRegion->FirstY;		
	m_PrtDesc.rc.left=(int)left+pRegion->FirstX;		
	m_PrtDesc.rc.right=(int)right+pRegion->FirstX;		

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.DrawLine(&m_PrtDesc,LineSize);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}	

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a rectangle using parameters passed
//	params:CPrintRegion*, position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::Box(CPrintRegion* pRegion,int top,int left,int bottom,int right,int LineSize,UINT Fillflags,UINT PenFlags)
{
	UINT	OldColor,OldMode;
	SaveState();
	if(Fillflags != IGNORE_PARAM)
		m_PrtDesc.uFillFlags=Fillflags;			
	if(PenFlags != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=PenFlags;			

	m_PrtDesc.rc.top=(int)top+pRegion->FirstY;		
	m_PrtDesc.rc.bottom=(int)bottom+pRegion->FirstY;		
	m_PrtDesc.rc.left=(int)left+pRegion->FirstX;		
	m_PrtDesc.rc.right=(int)right+pRegion->FirstX;		

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.DrawRect(&m_PrtDesc,LineSize);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a rectangle using parameters passed
//	params:CPrintRegion*, position, line size 1--x,flag is pen flags (see header)
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::Box(CPrintRegion* pRegion,double top,double left,double bottom,double right,int LineSize,UINT Fillflags,UINT PenFlags)
{
	UINT	OldColor,OldMode;
	SaveState();
	if(Fillflags != IGNORE_PARAM)
		m_PrtDesc.uFillFlags=Fillflags;			
	if(PenFlags != IGNORE_PARAM)
		m_PrtDesc.uPenFlags=PenFlags;			
	ConvertArea(top,left,bottom,right);

	m_PrtDesc.rc.top=(int)top+pRegion->FirstY;		
	m_PrtDesc.rc.bottom=(int)bottom+pRegion->FirstY;		
	m_PrtDesc.rc.left=(int)left+pRegion->FirstX;		
	m_PrtDesc.rc.right=(int)right+pRegion->FirstX;		

	if(m_PrtDesc.rc.bottom > pRegion->bottom)
		m_PrtDesc.rc.bottom=pRegion->bottom;
	if(m_PrtDesc.rc.left > pRegion->right)
		m_PrtDesc.rc.left=pRegion->right;
	if(m_PrtDesc.rc.top > pRegion->bottom)
		m_PrtDesc.rc.top=pRegion->bottom;
	if(m_PrtDesc.rc.right > pRegion->right)
		m_PrtDesc.rc.right=pRegion->right;

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	ThePrinter.DrawRect(&m_PrtDesc,LineSize);

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a checkbox using parameters passed
//	params: 
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::CheckBox(LPCTSTR Caption,BOOL Data,double top,double left,int FontSize,int Direction,int LineSize,UINT FillFlag,UINT PrintFlags)
{
	ConvertPosition(top,left);
	CheckBox(Caption,Data,(int) top,(int)left,FontSize,Direction,LineSize,FillFlag,PrintFlags);
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a checkbox using parameters passed
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::CheckBox(LPCTSTR Caption,BOOL Data,int top,int left,int FontSize,int Direction,int LineSize,UINT FillFlag,UINT PrintFlags)
{
	SaveState();
	if(Direction==LABEL_LEFT)
	{
		Print(top,left,PrintFlags,FontSize,Caption);
		left=(int)GetNextLogicalColumn(FALSE,TRUE);
		Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
		if(Data)
		{
			if(FillFlag!= FILL_NONE)
				Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize,FillFlag);
			else
			{
				Line(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
				Line(top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left,top,left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);

			}
		}
	}
	else
	{
		Print(top,left+(2*ConvertToMappedUnits(FontSize/72.0,HORZRES)),PrintFlags,FontSize,Caption);
		Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
		if(Data)
		{
			if(FillFlag!= FILL_NONE)
				Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize,FillFlag);
			else
			{
				Line(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
				Line(top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left,top,left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);

			}
		}
	}
	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a checkbox using parameters passed
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::CheckBox(CPrintRegion* pRegion,LPCTSTR Caption,BOOL Data,double top,double left,int FontSize,int Direction,int LineSize,UINT FillFlag,UINT PrintFlags)
{
	ConvertPosition(top,left);
	CheckBox(pRegion,Caption,Data,(int) top,(int)left,FontSize,Direction,LineSize,FillFlag,PrintFlags);
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw a checkbox using parameters passed
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::CheckBox(CPrintRegion* pRegion,LPCTSTR Caption,BOOL Data,int top,int left,int FontSize,int Direction,int LineSize,UINT FillFlag,UINT PrintFlags)
{
	UINT	OldColor,OldMode;
	SaveState();

	if(pRegion->FillColor > FILL_NONE)
		OldMode=m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		OldColor= SetColor(COLOR_WHITE); 		

	m_PrtDesc.rc.bottom=pRegion->bottom;
	m_PrtDesc.rc.right=pRegion->right;
	m_PrtDesc.rc.left=pRegion->FirstX;

	top+=pRegion->FirstY;
	left+=pRegion->FirstX;
	if(Direction==LABEL_LEFT)
	{
		Print(top,left,PrintFlags,FontSize,Caption);
		left=(int)GetNextLogicalColumn(FALSE,TRUE);
		Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
		if(Data)
		{
			if(FillFlag!= FILL_NONE)
				Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize,FillFlag);
			else
			{
				Line(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
				Line(top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left,top,left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);

			}
		}
	}
	else
	{
		Print(top,left+(2*ConvertToMappedUnits(FontSize/72.0,HORZRES)),PrintFlags,FontSize,Caption);
		Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
		if(Data)
		{
			if(FillFlag!= FILL_NONE)
				Box(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize,FillFlag);
			else
			{
				Line(top,left,top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);
				Line(top+ConvertToMappedUnits(FontSize/72.0,VERTRES),left,top,left+ConvertToMappedUnits(FontSize/72.0,HORZRES),LineSize);

			}
		}	
	}

	if(pRegion->FillColor > FILL_NONE)
		m_PrtDesc.pDC->SetBkMode(OldMode); 		
	
	if(pRegion->FillColor > FILL_LTGRAY)
		SetColor(OldColor); 		

	RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Creates a moveable print region
//	params:	location of region in 4 points
//	Returns: Pointer to the new region or a NULL
///////////////////////////////////////////////////////////////////////////

CPrintRegion* CPage::CreateRegion(double ptop,double pleft,double pbottom, double pright,UINT Fill)
{
	ConvertArea(ptop,pleft,pbottom,pright);
	return CreateRegion((int) ptop,(int) pleft,(int) pbottom, (int) pright,Fill);
}

////////////////////////////////////////////////////////////////////////////
//	Desc:  Create a CprintRegion structure and maintain a list of created structures
//  to free allocated memory when destructor runs
//	params: bounding rectangle for the region
//	Returns: pointer to the created structure
///////////////////////////////////////////////////////////////////////////
CPrintRegion* CPage::CreateRegion(int ptop,int pleft,int pbottom, int pright,UINT Fill)
{
	CPrintRegion*	pRegion= new CPrintRegion;
	if(pRegion==NULL)
		return pRegion;
	pRegion->FillColor=Fill;
	if( pRegion->Create(this,ptop,pleft,pright, pbottom,Fill)==FALSE)
	{
		delete pRegion;
		return (CPrintRegion*)0;
	}
	
	m_RegionList.Add(pRegion);
	return pRegion;
}


////////////////////////////////////////////////////////////////////////////
//	Desc:	 Constructor for a table descriptor class
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
TABLEHEADER::TABLEHEADER()
{
	SetSkip=0;
	AutoSize=TRUE;
	UseInches=FALSE;
	PointSize=10;
	LineSize=1;
	NumColumns=2;
	NumRows=2;
	Border=TRUE;
	VLines=TRUE;
	HLines=TRUE;
	NoHeader=HeaderOnly=FALSE;
	HeaderLines=1;
	NumPrintLines=1;
	FillFlag=FILL_NONE;
	StartRow=EndCol=StartCol=0;
	pClsTable=NULL;
}

TABLEHEADER::~TABLEHEADER()
{
	if(pClsTable != NULL)
		delete pClsTable;
};





////////////////////////////////////////////////////////////////////////////
//	Desc:	Create and attach a table object to the print object. If the unit
//	of measurment is in inches all conversions are done at this point.Once the
//	table object is created it is displayed by the call to PrintTable.DO NOT CALL
//	PRINTTABLE DIRECTLY ALWAYS USE THIS FUNCTION
//	params:	The table descriptor for the table
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::Table(TABLEHEADER* TheTable)
{
	PRTTYPE	temp;
	if(TheTable->NoHeader==TRUE)
		TheTable->HeaderLines=0;
	if(TheTable->UseInches)
	{
		TheTable->StartRow=ConvertToMappedUnits(TheTable->StartRow,VERTRES);  
		TheTable->StartCol=ConvertToMappedUnits(TheTable->StartCol,HORZRES);
		TheTable->EndCol=ConvertToMappedUnits(TheTable->EndCol,HORZRES);
		TheTable->EndRow=ConvertToMappedUnits(TheTable->EndRow,VERTRES);	
		for(int y=0;y < TheTable->NumColumns;++y)
			TheTable->ColDesc[y].Width=ConvertToMappedUnits(TheTable->ColDesc[y].Width,HORZRES);
		
	}
	//	check for horizontal margins
	if(TheTable->EndCol > SetRightMargin(0))
		return;
	//	create a table printing object and attach it to the table descriptor
	CPrintTable* pTable=new CPrintTable(TheTable,this);		
	//	pTable will be freed when the destructor for Thetable runs
	//	if you free it early there will be a system crash
	TheTable->pClsTable=pTable;
	temp=m_PrtDesc;
	//	print rhe table
	pTable->PrintTable();
	if(TheTable->UseInches)
		TheTable->EndRow=ConvertToInches((int)TheTable->EndRow,VERTRES);	
	m_PrtDesc=temp;
	
}

////////////////////////////////////////////////////////////////////////////
//	Desc:   See CPage::Print () for details
//	params: The table row and cloumn to print into, pointsixe textflags and variable arg list
//	Returns:
///////////////////////////////////////////////////////////////////////////

void CPage::Print(TABLEHEADER* TheTable,int row,int col,int PointSize,UINT TextFlags,TCHAR* fmt,...)
{
	CPrintTable* pTable=TheTable->pClsTable;
	if(pTable==NULL)
		return;
	TCHAR* buff=new TCHAR[1000];
	va_list t;
	SaveState();    
	va_start(t,fmt);
	_vstprintf(buff,fmt,t);
	if(TheTable->HeaderOnly==FALSE)
		pTable->InsertItem(buff,row,col,PointSize,TextFlags);
	else
		pTable->InsertVirtualItem(buff,row,col,PointSize,TextFlags);	
    va_end(t);
	delete[] buff;
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();	
}


////////////////////////////////////////////////////////////////////////////
//	Desc:	Print a disk based bitmapped imiage to the display
//	params:	The location in 4 points and full path to the imiage
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintBitMap(int top,int left,int bottom,int right,LPCSTR name)
{
	extern BOOL WINAPI PrintTheBitMap(PRTTYPE *ps);
	SaveState();    
	m_PrtDesc.rc.top=top;		
	m_PrtDesc.rc.bottom=bottom;		
	m_PrtDesc.rc.left=left;		
	m_PrtDesc.rc.right=right;		
	m_PrtDesc.Text=name;
	PrintTheBitMap(&m_PrtDesc);
	RestoreState();	
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	See above
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintBitMap(double top,double left,double bottom,double right,LPCSTR name)
{
	extern BOOL WINAPI PrintTheBitMap(PRTTYPE *ps);
	SaveState();
	ConvertArea(top,left,bottom,right);
	m_PrtDesc.rc.top=(int)top;		
	m_PrtDesc.rc.bottom=(int)bottom;		
	m_PrtDesc.rc.left=(int)left;		
	m_PrtDesc.rc.right=(int)right;		
	m_PrtDesc.Text=name;
	PrintTheBitMap(&m_PrtDesc);
	RestoreState();	
}
////////////////////////////////////////////////////////////////////////////
//	Helper class CPrintTable here
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	Desc: places a value in a cell on the table defined as row,col. Do not call direct
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrintTable::InsertItem(LPCTSTR Text,int row,int col,int PointSize,UINT TextFlags)
{                       
	int	y,SCol,ECol,Row,OldColor,OldMode;
	int	 sz=GetVerticalSpacing();
	if(row >= m_pTable->NumRows)        
		return;                                                                   
	if(PointSize < 1)
		PointSize=m_pTable->PointSize;		              
	// 	vertical displacement is determined by the font used by the column headers. If
	//	the font used to insert a item is larger it will look funny. So we adjust that
	//	here by assuring the font for the item is no larger than the column header
	if(PointSize > m_pTable->PointSize)
	{
		if(m_pTable->HLines==TRUE && m_pTable->HeaderOnly==FALSE)		
			PointSize=m_pTable->PointSize;		
	}			                                                
	//	determine the bounds of the print rect
	//	row	
	Row=m_pTable->NoHeader==FALSE ? (int) (m_pTable->StartRow+(m_pTable->HeaderLines*(GetVerticalSpacing(FALSE)))+( (sz)*row)):
	                                (int) (m_pTable->StartRow+( (sz)*row));			                                
	//	col
	SCol=(int) m_pTable->StartCol;
	for(y=0;y < col;++y)
		SCol+=(int) m_pTable->ColDesc[y].Width;
		
	ECol=(int)(SCol+m_pTable->ColDesc[col].Width);
	m_ps->rc.left=SCol+2;
	m_ps->rc.right=ECol-2;
	m_ps->rc.top=Row+2;
	m_ps->rc.bottom=(Row+sz)-2;
	m_ps->Text=Text;

	m_ps->uFillFlags=m_pTable->ColDesc[col].FillFlag;
	if(m_ps->uFillFlags > FILL_LTGRAY)
	{
		OldColor= p_Print->SetColor(COLOR_WHITE); 		
	}
	if(m_ps->uFillFlags > FILL_NONE)
	{
		OldMode=p_Print->m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	}

	if(m_pTable->NumPrintLines==1)
		m_ps->uTextFlags=TextFlags|TEXT_VCENTER|TEXT_SINGLELINE;
	else
		m_ps->uTextFlags=TextFlags;
	m_ps->PointSize=PointSize;	
	p_Print->ThePrinter.PrintText(m_ps,2);			
	p_Print->m_nNextPos=m_ps->m_NextCharPos;

	if(m_ps->uFillFlags > FILL_NONE)
	{
		OldMode=p_Print->m_PrtDesc.pDC->SetBkMode(OldMode); 		
	}
	if(m_ps->uFillFlags > FILL_LTGRAY)
	{
		OldColor= p_Print->SetColor(OldColor); 		
	}
}

////////////////////////////////////////////////////////////////////////////
//	Desc: places a value in a cell on the table defined as row,col
//	used only with tables that are header only type. This routine does no
//	checking for valid rows only for columns
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
	
void CPrintTable::InsertVirtualItem(LPCTSTR Text,int row,int col,int PointSize,UINT TextFlags)
{
	int	y,SCol,ECol,Row,OldColor,OldMode;
	//	not Noheader NoHeader means we have a grid with no column descriptors while
	//	HeaderOnly means we have a header with no table body. With header only we
	//	will still have the column descriptors to use to set margins for each field
	//	so we can add fields till we run out of room on the page
	int	 sz=GetVerticalSpacing();
	if(m_pTable->HeaderOnly==FALSE)
		return;
	if(PointSize < 1)
		PointSize=m_pTable->PointSize;		
	//	determine the bounds of the print rect
	//	row	. 
	//  The start pos + (the # Header lines * the size of line)+(row# * size)
	Row=(int) (m_pTable->StartRow+(m_pTable->HeaderLines*(GetVerticalSpacing(FALSE)))+( (sz)*row));		
	//	col
	SCol=(int) m_pTable->StartCol;
	for(y=0;y < col;++y)
		SCol+=(int) m_pTable->ColDesc[y].Width;
		
	ECol=(int) (SCol+m_pTable->ColDesc[col].Width);
	m_ps->rc.left=SCol+1;
	m_ps->rc.right=ECol-1;
	m_ps->rc.top=Row+1;
	m_ps->rc.bottom=Row+(sz)-1;
	m_ps->Text=Text;
	m_ps->uTextFlags=TextFlags;
	m_ps->PointSize=PointSize;	
	m_ps->uFillFlags=m_pTable->ColDesc[col].FillFlag;
	if(m_ps->uFillFlags > FILL_LTGRAY)
	{
		OldColor= p_Print->SetColor(COLOR_WHITE); 		
	}
	if(m_ps->uFillFlags > FILL_NONE)
	{
		OldMode=p_Print->m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
	}
	p_Print->ThePrinter.PrintText(m_ps,2);
	p_Print->m_nNextPos=m_ps->m_NextCharPos;
	if(m_ps->uFillFlags > FILL_NONE)
	{
		OldMode=p_Print->m_PrtDesc.pDC->SetBkMode(OldMode); 		
	}
	if(m_ps->uFillFlags > FILL_LTGRAY)
	{
		OldColor= p_Print->SetColor(OldColor); 		
	}
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draw horizontal lines for esch item on table
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrintTable::PrintHLines()
{
	int	y,SCol,ECol,Row,SCount=0;
	//	get out if needed
	int	 sz=GetVerticalSpacing();
	if(m_pTable->HLines==FALSE||m_pTable->HeaderOnly==TRUE)
	return;
	//	calc starting column and ending column
	SCol=(int) m_pTable->StartCol;
	ECol=(int) m_pTable->EndCol;                                                     
	//	if we have a header skip over it else start at start pos
	Row=m_pTable->NoHeader==FALSE ? (int) m_pTable->StartRow+ (GetVerticalSpacing(FALSE)*m_pTable->HeaderLines):
										(int) m_pTable->StartRow+sz;

	//	draw a line for each row from one side to the other									
	for(y=0;y < m_pTable->NumRows;++y)
	{
		if(m_pTable->SetSkip)
		{
			++SCount;
			if(m_pTable->SetSkip==SCount)
			{
				p_Print->Line(Row,SCol,Row,ECol,m_pTable->LineSize,PEN_SOLID);
				SCount=0;
			}
		}
		else
			p_Print->Line(Row,SCol,Row,ECol,m_pTable->LineSize,PEN_SOLID);
		Row+=sz;	
    }
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Draws vertical line seperators for column entries
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrintTable::PrintVLines()
{   
	int	y,start;
	int	 sz=GetVerticalSpacing();
	//	get out if needed
	if(m_pTable->VLines==FALSE||m_pTable->HeaderOnly==TRUE)
	return;
	//	col to start drawing at
	start=(int) m_pTable->StartCol;
	int BRow,ERow;               
	//	calc the beginning and ending rows to draw lines the correct length
	if(m_pTable->NoHeader==FALSE)
	{
		//	skip header area and calc area under header for x items
		BRow=(int) m_pTable->StartRow+(m_pTable->HeaderLines*(GetVerticalSpacing(FALSE)));
		ERow=(int) m_pTable->StartRow+(m_pTable->HeaderLines*(GetVerticalSpacing(FALSE))+(sz * m_pTable->NumRows));
	}	
	else     
	{   
		//	no header so calc only for x items
		BRow=(int) m_pTable->StartRow;
		ERow=(int) m_pTable->StartRow+( sz*m_pTable->NumRows);
	}	                                                                 
	//	loop through drawing lines on the right margin except the last column
	//	the last column will have a line drawn by the border
	for(y=0;y < m_pTable->NumColumns-1;++y)		
	{                                
		start+=(int) m_pTable->ColDesc[y].Width;
		m_ps->rc.top=BRow;
		m_ps->rc.bottom=ERow;
		m_ps->rc.left=m_ps->rc.right=start;
		p_Print->ThePrinter.DrawLine(m_ps,m_pTable->LineSize);
	}


}
////////////////////////////////////////////////////////////////////////////
//	Desc:	draw the border around the data area of a table. This is not the header
//	but the area that will contain the table data
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrintTable::PrintBorder()
{                                                           
	int	 sz=GetVerticalSpacing();
	//	check flags and exit if needed
	int ERow;                         
	//	calc the ending row of the table                              
	if(m_pTable->NoHeader==TRUE)
		ERow=(int) m_pTable->StartRow+( sz*m_pTable->NumRows);
	else
		ERow=(int) m_pTable->StartRow+( (m_pTable->HeaderLines*GetVerticalSpacing(FALSE))+(sz * m_pTable->NumRows));

	m_pTable->EndRow=ERow;		

	if(m_pTable->Border==FALSE || m_pTable->HeaderOnly==TRUE)
		return;                                  
	
	
	//	draw a box around the table diminisions	
	p_Print->Box(
	(int)m_pTable->StartRow,
	(int)m_pTable->StartCol,
	(int)ERow,
	(int)m_pTable->EndCol,
	m_pTable->LineSize,
	FILL_NONE,
	PEN_SOLID);
	
}

////////////////////////////////////////////////////////////////////////////
//	Desc: Print the column headers for the table. The member variable HeaderLines
//	determines how many text rows are to be in the header (Default=1)
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////

void CPrintTable::PrintHeader()
{   
	int start,y;
	CString s;              
	int	OldMode;
	COLORREF OldColor;                          
	//	check flag and exit if needed
	if(m_pTable->NoHeader==TRUE)
		return;     
	p_Print->SetFontSize(m_pTable->PointSize);		
	int	 sz=GetVerticalSpacing(FALSE);

	//	draw the box the correct size forgetting about the columns right now
	p_Print->Box((int)m_pTable->StartRow,
	              (int)m_pTable->StartCol,
				   (int)(m_pTable->StartRow+(m_pTable->HeaderLines*( sz))),
	              (int)m_pTable->EndCol,
	              m_pTable->LineSize,
	              FILL_NONE,
	              PEN_SOLID);
	//	adjust the width of the last column so it finishes out the header
	//	if the total of the widths is different from the total width of the
	//	table
	start=(int) m_pTable->StartCol;
	for(y=0;y < m_pTable->NumColumns;++y)
		start+=(int) m_pTable->ColDesc[y].Width;
	if(start < m_pTable->EndCol)
		m_pTable->ColDesc[m_pTable->NumColumns-1].Width+=m_pTable->EndCol-start;
	if(start > m_pTable->EndCol)
		m_pTable->ColDesc[m_pTable->NumColumns-1].Width-=start-m_pTable->EndCol;
		
	//	now do the columns drawing a box around each one and centering the text
	//	and applying and shading if needed also set the font size here
	
	start=(int) m_pTable->StartCol;
	for(y=0;y < m_pTable->NumColumns;++y)
	{
		//	the column text
		s=m_pTable->ColDesc[y].Text;
		//	change this to change default behavior for untiltled columns
		if(s.IsEmpty())
			s.Format(_T("Col %d"),y+1);
		//	create a print rectangle with the diminsions of the table item	
		m_ps->rc.left=start;
		m_ps->rc.right=(int) (m_ps->rc.left+m_pTable->ColDesc[y].Width);
		m_ps->rc.top=(int) m_pTable->StartRow;
	    m_ps->rc.bottom=(int) m_pTable->StartRow+(sz*m_pTable->HeaderLines);
		m_ps->Text=s;
		m_ps->uTextFlags=TEXT_CENTER|TEXT_BOLD|TEXT_RECT|TEXT_VCENTER;
		//	if the shading is dark set the color to white so it will show up
		if(m_ps->uFillFlags > FILL_NONE)
			OldMode=p_Print->m_PrtDesc.pDC->SetBkMode(TRANSPARENT); 		
		
		if(m_pTable->FillFlag > FILL_LTGRAY)
			OldColor= p_Print->SetColor(COLOR_WHITE); 		
		m_ps->uFillFlags=m_pTable->FillFlag;
		//	print the text
		p_Print->ThePrinter.PrintText(m_ps,2);
		p_Print->m_nNextPos=m_ps->m_NextCharPos;
		//	restore the old color scheme
		if(m_pTable->FillFlag > FILL_LTGRAY)
			p_Print->SetColor(OldColor);
		if(m_ps->uFillFlags > FILL_NONE)
			p_Print->m_PrtDesc.pDC->SetBkMode(OldMode); 		
		//	move to start of next column	
		start=m_ps->rc.right;
	}	          
}

////////////////////////////////////////////////////////////////////////////
//	Desc: places a fill in a cell on the table defined as row,col
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
	
void CPrintTable::FillColumn(int row,int col)
{
	if(m_pTable->SetSkip)
		return;
	p_Print->SaveState();
	int	y,SCol,ECol,Row;
	int	 sz=GetVerticalSpacing();
	//	determine the bounds of the print rect
	//	row	. 
	//  The start pos + (the # Header lines * the size of line)+(row# * size)
	Row=(int) (m_pTable->StartRow+(m_pTable->HeaderLines*(GetVerticalSpacing(FALSE)))+( (sz)*row));		
	//	col
	SCol=(int) m_pTable->StartCol;
	for(y=0;y < col;++y)
		SCol+=(int) m_pTable->ColDesc[y].Width;
		
	ECol=(int) (SCol+m_pTable->ColDesc[col].Width);
	m_ps->rc.left=SCol;
	m_ps->rc.right=ECol;
	m_ps->rc.top=Row;
	m_ps->rc.bottom=Row+(sz);
	p_Print->m_PrtDesc.uFillFlags=m_pTable->ColDesc[col].FillFlag;
	p_Print->ThePrinter.DrawRect(&p_Print->m_PrtDesc,1);
	p_Print->RestoreState();
}
	
////////////////////////////////////////////////////////////////////////////
//	Desc:	Returns the # of vertical units to seperate rows in the table. The
//	distance is dependant on the selected point size and map mode. The scaling factor
//	of 1.5 was determined by trial and error to look best. If space is ata premium
//	this can be reduced but should be no smaller than 1.25 or the results are UGLY
//	params:
//	Returns: the vertical size for a logical row of print to occupy
///////////////////////////////////////////////////////////////////////////
int CPrintTable::GetVerticalSpacing(BOOL Correct)	
{
	int offset= Correct==TRUE ? m_pTable->NumPrintLines:1;
	if(Correct==TRUE)
	{
		if(p_Print->m_PrtDesc.pDC->GetMapMode()==MM_ANISOTROPIC)                     
			return (int)(m_pTable->PointSize*1.50)* offset;
		else
			return  (int) (((m_pTable->PointSize/72.0)* p_Print->m_PixPerInchY)*1.50)* offset;    
	}
	else
	{
		if(p_Print->m_PrtDesc.pDC->GetMapMode()==MM_ANISOTROPIC)                     
		{
			if(m_pTable->HeaderLines==1)
				return (int)(m_pTable->PointSize*1.5);
			else
				return (int)(m_pTable->PointSize*1.0);
		}
		else
		{		
			if(m_pTable->HeaderLines==1)
				return  (int) (((m_pTable->PointSize/72.0)* p_Print->m_PixPerInchY)*1.5);    
			else
				return  (int) (((m_pTable->PointSize/72.0)* p_Print->m_PixPerInchY)*1.0);    
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//	Desc:
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
CPrintTable::CPrintTable(TABLEHEADER* pTable,CPage* pPrint)
{
	m_ps=&pPrint->m_PrtDesc;;
	m_pTable=pTable;
	p_Print=pPrint;
}

CPrintTable::~CPrintTable()
{

}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Called to actually display the table on the display
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrintTable::PrintTable()
{   
	p_Print->SaveState();
	//	if autosize calc the size of each column and set the desciptor                             
	if(m_pTable->AutoSize==TRUE)          
	{
		//	(end-start) / number of col = average width of each column
		//	and roundoff errors will be taken care of later
		int val=(int) (m_pTable->EndCol-m_pTable->StartCol)/m_pTable->NumColumns;
		
		for(int y=0;y < m_pTable->NumColumns;++y)
			m_pTable->ColDesc[y].Width=val;
	}	
	PrintHeader();
	PrintBorder();
	PrintHLines();
	PrintVLines();
	for(int xx=0;xx < m_pTable->NumColumns;++xx)		
		for(int yy=0;yy < m_pTable->NumRows;++yy)
			FillColumn(yy,xx);
	p_Print->RestoreState();
}

////////////////////////////////////////////////////////////////////////////
//	Helper class CPrintRegion here
////////////////////////////////////////////////////////////////////////////

CPrintRegion::CPrintRegion()
{
}

CPrintRegion::~CPrintRegion()
{
}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Qualifies and creates a print region
//	params:	* to parent page and location in 4 points
//	Returns: TRUE=Success
///////////////////////////////////////////////////////////////////////////
BOOL CPrintRegion::Create(CPage * pPage,int ptop,int pleft,int pright, int pbottom,UINT Fill)
{
	UNREFERENCED_PARAMETER(Fill);
	pOwner=pPage;
	top=ptop;
	right=pright;
	left=pleft;
	bottom=pbottom;
	if(top < 0								||
	   right > (signed)pOwner->m_PrtDesc.n_maxWidth ||
	   left < 0								||
	   bottom > (signed)pOwner->m_PrtDesc.n_maxLength)
		return FALSE;
	if(bottom > (signed)pOwner->SetBottomMargin(0))
		bottom=(signed)pOwner->SetBottomMargin(0);
	if(right > (signed)pOwner->SetRightMargin(0))
		right=(signed)pOwner->SetRightMargin(0);
	FirstX=left+1;
	FirstY=top+1;
	pOwner->SaveState();
	pOwner->m_PrtDesc.rc.top=top;
	pOwner->m_PrtDesc.rc.bottom=bottom;
	pOwner->m_PrtDesc.rc.right=right;
	pOwner->m_PrtDesc.rc.left=left;
	pOwner->m_PrtDesc.uFillFlags=FillColor;
	pOwner->ThePrinter.FillRect(&pOwner->m_PrtDesc);
	pOwner->RestoreState();
	
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////
//	Desc:	Draws border around the print region
//	params:
//	returns:
///////////////////////////////////////////////////////////////////////////
void CPrintRegion::DrawBorder()
{
	pOwner->SaveState();
	pOwner->m_PrtDesc.rc.top=top;
	pOwner->m_PrtDesc.rc.bottom=bottom;
	pOwner->m_PrtDesc.rc.right=right;
	pOwner->m_PrtDesc.rc.left=left;
	
	pOwner->ThePrinter.DrawRect(&pOwner->m_PrtDesc,1);
	pOwner->RestoreState();
}

///////////////////////////////////////////////////////////////////////////
//	Desc:	Draws a title for the print region
//	params: The title, the size fomnt to use, the flags
//	returns:
///////////////////////////////////////////////////////////////////////////
void CPrintRegion::DrawTitle(LPCSTR Title,int PointSize,UINT TextFlags,UINT FillFlags)
{
	pOwner->SaveState();
	pOwner->m_PrtDesc.rc.top=top;
	pOwner->m_PrtDesc.rc.right=right;
	pOwner->m_PrtDesc.rc.left=left;
	pOwner->m_PrtDesc.rc.bottom=bottom;
	pOwner->m_PrtDesc.Text=Title;	
	pOwner->m_PrtDesc.PointSize=PointSize;
	if( (TextFlags & TEXT_RECT)==TEXT_RECT)
	{
		pOwner->m_PrtDesc.rc.bottom=0;
		pOwner->ThePrinter.GetPrintInfo(&pOwner->m_PrtDesc,1);		
		pOwner->m_PrtDesc.rc.bottom=pOwner->m_PrtDesc.LastPrintArea.bottom+10;
	}
	else
	{
		int temp=pOwner->ConvertToMappedUnits( (PointSize/72.0)*2,VERTRES);		
		pOwner->m_PrtDesc.rc.bottom=top+temp;
	}
	pOwner->m_PrtDesc.uTextFlags=TextFlags|TEXT_SINGLELINE|TEXT_VCENTER;
	pOwner->m_PrtDesc.uFillFlags=FillFlags;
	pOwner->ThePrinter.PrintText(&pOwner->m_PrtDesc,1);
	FirstY=pOwner->m_PrtDesc.rc.bottom+1;
	pOwner->RestoreState();
}
////////////////////////////////////////////////////////////////////////////
//	STATIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
//	Desc: static member function for changing print orientation
//	params: CDC* to be used and the mode to switch to
//	Returns: nada
///////////////////////////////////////////////////////////////////////////
void CPage::SetPrinterMode(CDC* pDC,int Mode)
{
	if(Mode !=DMORIENT_LANDSCAPE && Mode != DMORIENT_PORTRAIT)
		return;
	PRINTDLG* pPrintDlg = new PRINTDLG;
	afxGetApp()->GetPrinterDeviceDefaults(pPrintDlg);
	DEVMODE* lpDevMode = (DEVMODE*)::GlobalLock(pPrintDlg->hDevMode);		
	lpDevMode->dmOrientation = (short)Mode;
	pDC->ResetDC(lpDevMode);	
	::GlobalUnlock(pPrintDlg->hDevMode);	
	delete pPrintDlg;
}

////////////////////////////////////////////////////////////////////////////
//	Desc: static member function to find printer name
//	params: CDC* to be used 
//	Returns: The device name
///////////////////////////////////////////////////////////////////////////
LPCTSTR CPage::GetPrinterName(CDC* pDC)
{
	static TCHAR buff[30];
	UNREFERENCED_PARAMETER(pDC);
	PRINTDLG* pPrintDlg = new PRINTDLG;
	afxGetApp()->GetPrinterDeviceDefaults(pPrintDlg);
	DEVMODE* lpDevMode = (DEVMODE*)::GlobalLock(pPrintDlg->hDevMode);		
	_tcscpy(buff,(LPCTSTR)lpDevMode->dmDeviceName);
	::GlobalUnlock(pPrintDlg->hDevMode);	
	delete pPrintDlg;
	return buff;
}





///////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//	Desc:	Creates a moveable print region
//	params:	location of region in 4 points
//	Returns: Pointer to the new region or a NULL
///////////////////////////////////////////////////////////////////////////

CPrintRegion* CPage::CreateSubRegion(CPrintRegion* pParent,double ptop,double pleft,double pbottom, double pright,UINT Fill)
{
	ConvertArea(ptop,pleft,pbottom,pright);
	return CreateSubRegion(pParent,(int) ptop,(int) pleft,(int) pbottom, (int) pright,Fill);
}

////////////////////////////////////////////////////////////////////////////
//	Desc:  Create a CprintRegion structure and maintain a list of created structures
//  to free allocated memory when destructor runs
//	params: bounding rectangle for the region
//	Returns: pointer to the created structure
///////////////////////////////////////////////////////////////////////////
CPrintRegion* CPage::CreateSubRegion(CPrintRegion* pParent,int ptop,int pleft,int pbottom, int pright,UINT Fill)
{
	CPrintRegion*	pRegion= new CPrintRegion;
	if(pRegion==NULL || pParent==NULL)
		return NULL;
	//	adjust boundries to fit within parent region
	ptop=ptop+pParent->FirstY;
	pleft=pleft+pParent->FirstX;
	pright=pright+pParent->FirstX;
	if(pright > pParent->right)
		pright=pParent->right;
	pbottom=pbottom+pParent->top;
	if(pbottom > pParent->bottom)
		pbottom=pParent->bottom;
	if(Fill==FILL_NONE)
		Fill=pParent->FillColor;
	pRegion->FillColor=Fill;
	if( pRegion->Create(this,ptop,pleft,pright, pbottom,Fill)==FALSE)
	{
		delete pRegion;
		return (CPrintRegion*)0;
	}
	
	m_RegionList.Add(pRegion);
		
	return pRegion;
}






////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints rotated text 
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintRotatedText(int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,LPCSTR Text,int angle)
{
	SaveState();
	m_PrtDesc.Text=Text;
	if(PointSize > 0)
		m_PrtDesc.PointSize=PointSize;
	if(flags != IGNORE_PARAM)
		m_PrtDesc.uTextFlags=flags;	       

	m_PrtDesc.rc.left=(int)Left;
    m_PrtDesc.rc.top=(int)Top;
	m_PrtDesc.rc.right=(int)Right;
    m_PrtDesc.rc.bottom=(int)Bottom;


	ThePrinter.RotationAngle=angle;
	ThePrinter.PrintText(&m_PrtDesc,m_Spacing);                       
	m_nNextPos=m_PrtDesc.m_NextCharPos;
	RestoreState();

}

////////////////////////////////////////////////////////////////////////////
//	Desc:	Prints rotated text 
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPage::PrintRotatedText(double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,LPCSTR Text,int angle)
{
	SaveState();
	ConvertArea(Top,Left,Bottom,Right);
	PrintRotatedText( (int) Top,(int) Left,(int) Bottom,(int) Right,flags,PointSize,Text,angle);
	RestoreState();
}


