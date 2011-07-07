#ifndef	__CPAGE__H_
#define __CPAGE__H_

#include "cprinter.h"

typedef LPCTSTR	(*PF_REMOTE)(int);

#ifndef	COLOR_RED
	#define	COLOR_RED		RGB(255,0,0)
	#define	COLOR_GREEN		RGB(0,255,0)
	#define	COLOR_BLUE		RGB(0,0,255)
	#define	COLOR_MAGENTA	RGB(255,0,255)
	#define	COLOR_YELLOW	RGB(255,255,0)
	#define	COLOR_WHITE		RGB(255,255,255)
	#define	COLOR_BLACK		RGB(0,0,0)
	#define	COLOR_CYAN		RGB(0,255,255)
	#define	COLOR_GRAY		RGB(192,192,192)
	#define COLOR_SILVER	COLOR_YELLOW|COLOR_GRAY
	#define	COLOR_LTGREEN	RGB(0,192,128)
#endif

class TABLEHEADER;
class CPrintRegion;
class CPrintTable;

class	CPage{
public:
	CPage(RECT rectDraw, CDC* p_Dc, int MapMode=MM_ANISOTROPIC);
	virtual	~CPage();
public:

	CPrintRegion* CreateSubRegion(CPrintRegion* pParent,int ptop,int pleft,int pbottom, int pright,UINT Fill=FILL_NONE);
	CPrintRegion* CreateSubRegion(CPrintRegion* pParent,double ptop,double pleft,double pbottom, double pright,UINT Fill=FILL_NONE);


	void	PrintRotatedText(double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,LPCSTR Text,int angle);
	void	PrintRotatedText(int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,LPCSTR Text,int angle);

	//	static functions
	static	void	SetPrinterMode(CDC* pDC,int Mode=DMORIENT_PORTRAIT);
	static	LPCTSTR	GetPrinterName(CDC* pDC);

	virtual	void	SetUserFunction(PF_REMOTE ptr);
	//	print functions
	//	these functions use device unit parameters
	virtual	int Print(int row,int col,UINT TextFlags,int PointSize,const TCHAR* fmt,...);
	virtual	int	Print(int row,int *col,UINT TextFlags,int PointSize,const TCHAR* fmt,...);
	//	print using data supplied from user function
	virtual	int Print(int row,int col,UINT TextFlags,int PointSize,int ID);
	//	these use inches
	virtual	double Print(double row,double col,UINT TextFlags,int PointSize,const TCHAR* fmt,...);
	virtual	double Print(double row,double *col,UINT TextFlags,int PointSize,const TCHAR* fmt,...);
	//	print using data supplied from user function
	virtual	double Print(double row,double col,UINT TextFlags,int PointSize,int ID);

	// simpler versions using default text and pointsized parameters
	virtual	int Print(int row,int col,const TCHAR* fmt,...);
	virtual	double Print(double row,double col,const TCHAR* fmt,...);
	virtual	double Print(double row,double col,CString strText);
	//	print using data supplied from user function
	virtual	int Print(int row,int col,int ID);
	virtual	double Print(double row,double col,int ID);
	//	print limited to print region 
	virtual	int Print(CPrintRegion* pRegion,int row,int col,UINT TextFlags,int PointSize,const TCHAR* fmt,...);
	virtual	double Print(CPrintRegion* pRegion,double row,double col,UINT TextFlags,int PointSize,const TCHAR* fmt,...);
	//	print using data supplied from user function
	virtual	int Print(CPrintRegion* pRegion,int row,int col,UINT TextFlags,int PointSize,int ID);
	virtual	double Print(CPrintRegion* pRegion,double row,double col,UINT TextFlags,int PointSize,int ID);
	//	print in newspaper columns
	virtual	void PrintColumn(int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,LPCSTR Text);
	virtual	void PrintColumn(double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,LPCSTR Text);
	//	print using data supplied from user function
	virtual	void PrintColumn(int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,int ID);
	virtual	void PrintColumn(double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,int ID);
	//	columns in regions
	virtual	void PrintColumn(CPrintRegion* pRegion,int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,LPCSTR Text);
	virtual	void PrintColumn(CPrintRegion* pRegion,double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,LPCSTR Text);
	//	print using data supplied from user function
	virtual	void PrintColumn(CPrintRegion* pRegion,int Top,int Left,int Bottom,int Right,UINT flags,int PointSize,int ID);
	virtual	void PrintColumn(CPrintRegion* pRegion,double Top,double Left,double Bottom,double Right,UINT flags,int PointSize,int ID);
	//	variable access routines
	virtual	double	SetLineSpacing(double Spacing);
	virtual	LPCTSTR	SetFont(LPCSTR FontName);
	virtual	double	GetNextLogicalColumn(BOOL Convert=TRUE,BOOL AddOffset=FALSE);
	virtual	COLORREF SetColor(COLORREF Color);
	virtual	int		SetFontSize(int sz);
	virtual	int		SetRightMargin(int width);
	virtual	int		SetBottomMargin(int length);
	virtual	double	SetRightMargin(double width);
	virtual	double	SetBottomMargin(double length);
	CDC*			GetDisplayContext();
	//	drawing routines
	virtual	void	Line(int top,int left,int bottom,int right,int LineSize=1,UINT flag=PEN_SOLID);
	virtual	void	Box(int top,int left,int bottom,int right,int LineSize=1,UINT Fillflags=FILL_NONE,UINT PenFlags=PEN_SOLID);
	virtual	void	Line(double top,double left,double bottom,double right,int LineSize=1,UINT flag=PEN_SOLID);
	virtual	void	Box(double top,double left,double bottom,double right,int LineSize=1,UINT Fillflags=FILL_NONE,UINT PenFlags=PEN_SOLID);

	virtual	void	Line(CPrintRegion* pRegion,int top,int left,int bottom,int right,int LineSize=1,UINT flag=PEN_SOLID);
	virtual	void	Box(CPrintRegion* pRegion,int top,int left,int bottom,int right,int LineSize=1,UINT Fillflags=FILL_NONE,UINT PenFlags=PEN_SOLID);
	virtual	void	Line(CPrintRegion* pRegion,double top,double left,double bottom,double right,int LineSize=1,UINT flag=PEN_SOLID);
	virtual	void	Box(CPrintRegion* pRegion,double top,double left,double bottom,double right,int LineSize=1,UINT Fillflags=FILL_NONE,UINT PenFlags=PEN_SOLID);
	
	virtual	void	CheckBox(LPCTSTR Caption,BOOL Data,int top,int left,int FontSize,int Direction=LABEL_RIGHT,int LineSize=1,UINT Fillflags=FILL_NONE,UINT TextFlags=TEXT_NORMAL|TEXT_NOCLIP|TEXT_SINGLELINE);
	virtual	void	CheckBox(LPCTSTR Caption,BOOL Data,double top,double left,int FontSize,int Direction=LABEL_RIGHT,int LineSize=1,UINT Fillflags=FILL_NONE,UINT TextFlags=TEXT_NORMAL|TEXT_NOCLIP|TEXT_SINGLELINE);
	virtual void	CheckBox(CPrintRegion* pRegion,LPCTSTR Caption,BOOL Data,int top,int left,int FontSize,int Direction=LABEL_RIGHT,int LineSize=1,UINT Fillflags=FILL_NONE,UINT TextFlags=TEXT_NORMAL|TEXT_NOCLIP|TEXT_SINGLELINE);
	virtual	void	CheckBox(CPrintRegion* pRegion,LPCTSTR Caption,BOOL Data,double top,double left,int FontSize,int Direction=LABEL_RIGHT,int LineSize=1,UINT Fillflags=FILL_NONE,UINT TextFlags=TEXT_NORMAL|TEXT_NOCLIP|TEXT_SINGLELINE);

	// table stuff
	virtual	void Table(TABLEHEADER* TheTable);
	virtual	void Print(TABLEHEADER* TheTable,int row,int col,int PointSize,UINT TextFlags,TCHAR* fmt,...);

	// region stuff
	virtual  CPrintRegion* CPage::CreateRegion(double ptop,double pleft,double pbottom, double pright,UINT Fill=FILL_NONE);
	virtual  CPrintRegion* CPage::CreateRegion(int ptop,int pleft,int pbottom, int pright,UINT Fill=FILL_NONE);

	virtual void PrintBitMap(int top,int left,int bottom,int right,LPCSTR name);
	virtual	void PrintBitMap(double top,double left,double bottom,double right,LPCSTR name);
protected:
	void	SaveState();
	void	RestoreState();
	int		ConvertToMappedUnits(double dwInch,int bWidth);
	double	ConvertToInches(int value,int bWidth);
	void	ConvertPosition(double& Row,double& Col);
	virtual	int Print(LPCTSTR Text,int StartPos,UINT flags,int PointSize);
	void	ConvertArea(double& top,double& left,double& bottom,double& right);
	int		GetPrinterMode(CDC* pDC);

	friend	CPrintRegion;
	friend CPrintTable;
public:
	CPrinter ThePrinter;		//	low level printer functions
	PRTTYPE	m_PrtDesc;			// print type object descriptor
	PRTTYPE	m_SaveState[10];			// print type object descriptor
	int		StateInd;
	int		m_PixPerInchX;		//	for output device
	int		m_PixPerInchY;
	int		m_nNextPos;
	TEXTMETRIC	tm;
	int		m_PrintMode;
protected:
	PF_REMOTE	pUserFunc;
	int		m_nMaxWidth;
	int		m_nMaxLength;
	CRect	m_DrawRect;
	int		m_nMinLineOffset;
	int		m_nNextLine;                                 
	
	CRect	m_LastPrintExtent;	
	double	m_Spacing;
	double	m_WidthInches;
	double	m_LengthInches;
	
	RECT	m_TempRect;  
	CPtrArray m_RegionList;
private:
	TCHAR	Buffer[1024*5];
};


////////////////////////////////////////////////////////////
//	helper classes to describe table layouts for CPrintTable
//	to print. CPrinttablemis declared in prntcls.cpp because it
//	is local to that module but these classes are part of the UI 
//	so they are exposed here
////////////////////////////////////////////////////////////
class COLUMNDATA
{
	//	this class describes a single column it is Width units wide
	//	and has a caption of Text
public:
	double	Width;
	CString	Text;
	
	COLUMNDATA() {Width=0;Text.Empty();FillFlag=FILL_NONE;};
	void	Init(double nWidth,LPCSTR lpzText,UINT Flag=FILL_NONE) {Width=nWidth;Text=lpzText;FillFlag=Flag;};
	UINT	FillFlag;
};				


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//	helper class TABLEHEADER
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class TABLEHEADER{           
	//	This class describes a table and its attributes
public:
		~TABLEHEADER();
		TABLEHEADER();
		int				SetSkip;
		BOOL			UseInches;	//	true if units passed are in inches
		BOOL			AutoSize;  // if true make all cols equal size
		UINT			FillFlag;  // set to a value to fill header 
		int				PointSize; // font point size to use.all measuring done with this size
		int				LineSize;  // width of line used in drawing boxes and lines
		int				NumColumns; // number of col in table
		int				NumRows;	// num rows (ignored if headeronly=true)
		BOOL			Border;    // if true draw a border
		BOOL			VLines;		// if true draw vertical seperator lines
		BOOL			HLines;    // ditto on horizontal lines
		BOOL			HeaderOnly;  // if true display header only (virtual table)                        
		BOOL			NoHeader;  // if true only draw the boxes no headers
		int				HeaderLines;	//	how many lines of header text
		int				NumPrintLines;	// how many logical lines of text per cell
		double			StartRow;  // vertical position for top left of table
		double			StartCol;  // horizontal position for top left of table
		double			EndCol;		// horizontal position of right side of table
		double			EndRow;		//	bottom of chart; ( do not set will be calculated)
		COLUMNDATA	 	ColDesc[25]; // see above
		CPrintTable*	pClsTable;	// ptr to object ownibg this header
// the constructor will init data and the destructor will free data
// that ia all this class dies as it just basically holds information
	};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//	Helper Class CPrintTable that handles all table related printing
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
class CPrintTable
{
public:
	friend CPage;

	CPrintTable(TABLEHEADER* pTable,CPage* pPrint);
	~CPrintTable();
// oerations
protected:
		virtual void	PrintTable();
		virtual	void	InsertItem(LPCTSTR Text,int row,int col,int PS,UINT Flags);
		virtual	void	InsertVirtualItem(LPCTSTR Text,int row,int col,int PointSize,UINT TextFlags);
		virtual	void 	PrintHeader();
		virtual	void	PrintBorder();
		virtual	void	PrintHLines();
		virtual	void	PrintVLines();
		virtual	int 	GetVerticalSpacing(BOOL Correct=TRUE);
		virtual	void	FillColumn(int row,int col);
public:         

// Implementation
public:
protected:
	PRTTYPE*	m_ps;
	TABLEHEADER*	m_pTable;    // see prtcls.h
	CPage*		p_Print;
};



class CPrintRegion{
	friend CPage;
public:
	CPrintRegion();
	virtual ~CPrintRegion();
	virtual BOOL Create(CPage * pPage,int top,int left,int bottom, int right,UINT Fill=FILL_NONE);
	
	virtual	void DrawTitle(LPCSTR Title,int PointSize=8,UINT TextFlags=TEXT_BOLD|TEXT_CENTER|TEXT_RECT,UINT FillFlags=FILL_NONE);
	virtual	void DrawBorder();
protected:
	int FirstY;
	int FirstX;
	int	top;
	int	left;
	int	right;
	int	bottom;
	UINT	FillColor;
protected:
	CPage* pOwner;
	
};

#endif



