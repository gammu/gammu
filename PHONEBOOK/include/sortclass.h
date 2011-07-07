

class CSortClass
{
public:
	CSortClass(CListCtrl * _pWnd, const int _iCol, const CString _DataType); //bool _bIsNumeric);
	//CSortClass(CListCtrl * _pWnd, const int _iCol, const bool _bIsNumeric, int iStartingItem, int iEndingItem );
	virtual ~CSortClass();		
	
	int iCol;	
	int m_iStartingItem;
	int m_iEndingItem;

	CListCtrl * pWnd;	
//	bool bIsNumeric;
//	int dataType;
	
	void Sort(const bool bAsc);	
	
	static int CALLBACK CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

public:	
	class CSortItem	
	{	
		public:		
			virtual  ~CSortItem();
			CSortItem(const DWORD _dw, const CString &_txt);		
			CString txt;		
			DWORD dw;	
	};
	class CSortItemInt	
	{	
		public:
//			virtual ~CSortItemInt();
			CSortItemInt(const DWORD _dw, const CString &_txt);		
			int iInt ;		
			DWORD dw;	
	};
	class CSortItemDate
	{
	public:
//		virtual ~CSortItemDate();
		CSortItemDate(const DWORD _dw, const CString &_txt);
		CTime dDate;
		DWORD dw;
	};
};

