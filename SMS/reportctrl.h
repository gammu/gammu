///////////////////////////////////////////////////////////////////////////////
// ReportCtrl.h
//
// CReportCtrl, a CListCtrl derived class that is specialized on "Report View"
// style. 
//
// Features:
//
// 1, Item sorting by clicking on column header.
// 2, Sub-item text edit.
// 3, Item repositioning.
// 4, Customizing checkbox styles, including "single" and "disabled".
// 5, Sending a message to parent window when user clicks on a checkbox.
// 6, Convenient item insertion, deletion, moving, and sub-item text changing.
// 7, Sub-item images and color
// 8, And much more... 
//
// This code may be used in compiled form in any way you desire. This file may be
// redistributed unmodified by any means PROVIDING it is not sold for profit without
// the authors written consent, and providing that this notice and the authors name 
// is included. If the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file .
//
// This file is provided "as is" with no expressed or implied warranty.
//
// Written by Bin Liu (abinn32@yahoo.com)
//
// History
//
// Nov. 26, 2003 - Initial release.
// Dec. 03, 2003 - Fixed a bug in "EndEdit" where item text were not preperly committed.
//                 Completed the implementation of the "Sort-Separator" feature.
// Jan. 01, 2004 - Fixed a bug in "SetItemData".
//               - Added message "WM_EDIT_COMMITTED" which is sent to the parent window
//                 when an item text editing is committed.
//               - Fixed a bug in "SetItemText"(double type).
//               - Fixed a bug where item sorting does not work properly when there
//                 are multiple CReportCtrl objects on same window.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __REPORTCTRL_H__
#define __REPORTCTRL_H__

// Sent to parent window when user clicked on the checkbox of an item:
// wParam: The item index in the list ctrl
// lParam: The mouse event type(WM_LBUTTONDOWN, WM_RBUTTONDOWN, etc) which generated this event. 
// Note: This message is not sent when the checkbox states were altered programmatically
//       by calling "SetItem", it is only sent when the user "physically" clicked the
//       checkbox using mouse or joystick etc.
#define WM_ON_CHKBOX	(WM_APP + 10000)

// Sent to parent window when a column of items were sorted
// wParam: The column index
// lParam: The sort method, either 0(descending) or 1(ascending)
#define WM_ITEM_SORTED	(WM_APP + 10001)

// Sent to parent window when an item text editing was committed
// wParam: The item index
// lParam: The column index
#define WM_EDIT_COMMITTED	(WM_APP + 10002)

// Checkbox styles.
#define RC_CHKBOX_NONE			0 // No checkbox displayed
#define	RC_CHKBOX_NORMAL		1 // Normal, multiple check allowed
#define	RC_CHKBOX_SINGLE		2 // Single check only
#define	RC_CHKBOX_DISABLED		3 // Disabled, cannot be checked/unchecked by user input,
								  // but can be by your code.

// Item state flags for selection, deletion, etc.
// Multiple flags can be combined together using the bit-or operator.
// Note: If RC_ITEM_ALL is set, all other flags are ignored
#define RC_ITEM_NONE		0x0000 // Void, indicates invalid items only
#define	RC_ITEM_ALL			0x0001 // All items regardless of states
#define	RC_ITEM_SELECTED	0x0002 // Selected items
#define	RC_ITEM_UNSELECTED	0x0004 // Unselected items
#define	RC_ITEM_CHECKED		0x0008 // Checked items
#define	RC_ITEM_UNCHECKED	0x0010 // Unchecked items
#define	RC_ITEM_FOCUSED		0x0020 // Focused item
#define	RC_ITEM_UNFOCUSED	0x0040 // Unfocused items

// Item inverting types
#define RC_INVERT_SELECTION	0 // Invert item selection
#define RC_INVERT_CHECKMARK	1 // Invert item check mark

// Removes any custom color from item text and item backgroun
#define COLOR_INVALID	0xffffffff

//////////////////////////////////////////////////////////////////////////
// The CReportCtrl Class Definition
//////////////////////////////////////////////////////////////////////////

class CReportCtrl : public CListCtrl
{
public:		
		
	//////////////////////////////////////////////////////////////////////
	//		Constructor & Destructor
	//////////////////////////////////////////////////////////////////////
	CReportCtrl();
	virtual ~CReportCtrl();

	//////////////////////////////////////////////////////////////////////
	//		Run-time Creation
	//////////////////////////////////////////////////////////////////////
	virtual BOOL Create(CWnd* pParentWnd, UINT nID, LPCRECT lpRect = NULL, DWORD dwStyle = WS_BORDER | WS_TABSTOP);

	///////////////////////////////////////////////////////////////////////
	//		Column Header attributes
	///////////////////////////////////////////////////////////////////////
	
	BOOL SetColumnHeader(const CString& strHeadings); // Set columns and their formats.
	int GetColumnCount() const; // Get the column count.
	BOOL DeleteAllColumns();
	CString GetHeaderText(int nColumn) const;
	BOOL SetHeaderText(int nColumn, LPCTSTR lpText);	
	BOOL HasColumnHeader() const; // FALSE if the list control has LVS_NOCOLUMNHEADER flag
	const CHeaderCtrl* GetHeaderCtrl() const;

	///////////////////////////////////////////////////////////////////////
	//		Images & Color
	///////////////////////////////////////////////////////////////////////
	
	// Column header images
	BOOL SetHeaderImage(int nColumn, int nImageIndex, BOOL bLeftSide = TRUE);
	int GetHeaderImage(int nColumn) const;
	CImageList* SetHeaderImageList(UINT nBitmapID, COLORREF crMask = RGB(255, 0, 255));
	CImageList* SetHeaderImageList(CImageList* pImageList);
	
	// Sub-item images
	BOOL SetItemImage(int nItem, int nSubItem, int nImageIndex);
	int GetItemImage(int nItem, int nSubItem) const;
	CImageList* SetImageList(UINT nBitmapID, COLORREF crMask = RGB(255, 0, 255));
	CImageList* SetImageList(CImageList* pImageList);
	CImageList* GetImageList() const;

	// Sub-item Text & Background Color
	void SetItemTextColor(int nItem = -1, int nSubItem = -1, COLORREF color = COLOR_INVALID, BOOL bRedraw = TRUE);
	COLORREF GetItemTextColor(int nItem, int nSubItem) const;
	void SetItemBkColor(int nItem = -1, int nSubItem = -1, COLORREF color = COLOR_INVALID, BOOL bRedraw = TRUE);
	COLORREF GetItemBkColor(int nItem, int nSubItem) const;

	//////////////////////////////////////////////////////////////////////
	//		Control Styles & Attributes
	//////////////////////////////////////////////////////////////////////
	
	void ResizeToFitParent(); // Resize the list control to occupy parent's client area
	void SetGridLines(BOOL bSet = TRUE); // Show grid lines.
	void SetCheckboxeStyle(int nStyle = RC_CHKBOX_NORMAL); // Set checkbox styles.	
	int GetCheckboxStyle() const;	
	BOOL IsSortable() const; // Is sort allowed?
	BOOL SetSortable(BOOL bSet); // Allow/disallow sorting
	BOOL IsEditable() const; // Is Item text editable?
	void SetEditable(BOOL bSet = TRUE); // Allow item text editting
	
	///////////////////////////////////////////////////////////////////////
	//		Item attributes & operations
	///////////////////////////////////////////////////////////////////////	

	// Item states operation
	int GetFirstItem(DWORD dwStates = RC_ITEM_ALL, int nStartAfter = -1) const;
	int GetLastItem(DWORD dwStates = RC_ITEM_ALL, int nStartBefore = -1) const;
	int GetItemCount(DWORD dwStates = RC_ITEM_ALL) const;	
	DWORD GetItemStates(int nItem) const;
	BOOL ExamItemStates(int nItem, DWORD dwStates) const;
	BOOL SetItemStates(int nItem, DWORD dwNewStates);
	int SetAllItemStates(DWORD dwOldStates, DWORD dwNewStates);
	void InvertItems(int nType); // RC_INVERT_SELECTION or RC_INVERT_CHECKMARK

	// Item Insertion & Deletion	
	int InsertItemEx(int nItem, LPCTSTR lpText, ...); 	
	BOOL DeleteItem(int nItem, BOOL bSelectNextItem = FALSE); // Delete an item.
	int DeleteAllItems(DWORD dwStates = RC_ITEM_ALL); // Delete all qualified items.

	// Item positioning	
	int MoveUp(int nItem, int nCount = 1); // Move an item upwards by "nCount" positions.
	int MoveDown(int nItem, int nCount = 1); // Move an item downwards by "nCount" positions.
	int MoveToTop(int nItem); // Move an item up to the top.
	int MoveToBottom(int nItem); // Move an item down to the bottom.
	int MoveTo(int nItem, int nNewPosition); // Move an item to a particular position 
	BOOL SwapItems(int nItem1, int nItem2);	// Swap two items in the list, including all attributes.

	// Convenient versions of "CListCtrl::SetItemText"	
	BOOL SetItemText(int nItem, int nSubItem, INT val);
	BOOL SetItemText(int nItem, int nSubItem, UINT val);
	BOOL SetItemText(int nItem, int nSubItem, LONG val);
	BOOL SetItemText(int nItem, int nSubItem, ULONG val);
	BOOL SetItemText(int nItem, int nSubItem, TCHAR val);
	BOOL SetItemText(int nItem, int nSubItem, DOUBLE val, int nPrecision = -1);
	BOOL SetItemText(int nItem, int nSubItem, const COleDateTime& dateTime, DWORD dwFlags = 0);

	// Sorting	
	BOOL IsSortAscending() const;
	int GetSortedColumn() const;
	void SortItems(int nColumn, BOOL bAscending); // Sort a specified column.
	void SetSortSeparator(LPCTSTR lpSortSeparator = NULL); // Sort-separator, NULL=disabled
	LPCTSTR GetSortSeparator() const;

	// Item text edit	
	BOOL StartEdit(int nItem, int nSubItem); // Display the editbox, previous edit are committed
	BOOL EndEdit(BOOL bCommit = TRUE); // Commit/cancel text edit, hide the editbox
	CEdit* GetEditControl();	

	///////////////////////////////////////////////////////////////////////
	//		Necessary overloading but Irrelevant to Users
	///////////////////////////////////////////////////////////////////////
	
	int InsertColumn(int nCol, LPCTSTR lpColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);
	int InsertColumn(int nCol, const LVCOLUMN* pColumn);
	BOOL DeleteColumn(int nCol);
	BOOL SetCheck(int nItem, BOOL bCheck = TRUE); // overloaded to guard the "single" checkbox style
	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam);
	BOOL SetItem(const LVITEM* pItem);
	BOOL GetItem(LVITEM* pItem) const;
	DWORD GetItemData(int nItem) const;
	BOOL SetItemData(int nItem, DWORD dwData);
	DWORD SetExtendedStyle(DWORD dwNewStyle);
	BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	int InsertItem(UINT nMask, int nItem, LPCTSTR lpItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam);
	int InsertItem(int nItem, LPCTSTR lpItem, int nImage);
	int InsertItem(const LVITEM* pItem);
	int InsertItem(int nIndex, LPCTSTR lpText);
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpText);

protected:	
	
	// Helper functions for internal use
	BOOL _IsValidIndex(int nIndex) const;
	BOOL _ItemCheckMonitor(int nIndex, BOOL bBefore, BOOL bAfter, UINT nMsg);
	BOOL _IsEditVisible() const;
	void _MouseClkMonitor(UINT nMsg, UINT nFlags, CPoint point, BOOL bTriggerEdit);
	static void _StringSplit(const CString &str, CStringArray& arr, TCHAR chDelimitior);
	void _UnsetSortedColumn();
	BOOL _SetItemStatesNoVerify(int nItem, DWORD dwNewStates);
	void _EnsureSingleCheck(int nItem);
	DWORD _GetHeaderTextFormat(int nColumn) const;
	void _UpdateColumn(int nColumn, BOOL bInsert);
	void _AllocItemMemory(int nItem);
	void _FreeItemMemory(int nItem);
	BOOL _PartialSort(int nStart, int nEnd);
	int _FindSeparator(int nStartAfter, int nColumn) const;
	void _QuickSortRecursive(int* pArr, int nLow, int nHigh);
	int _CompareItems(int nItem1, int nItem2);

	// Member data	
	CEdit* m_pWndEdit; // Edit control, for subitem edit
	LPTSTR m_pszSeparator; // Sort separator
	BOOL m_bAllowEdit; // Is subitem edit allowed?
	POINT m_ptEditting; // Position of the subitem that is currently being edited
	int m_nChkStyle; // Checkbox style
	DWORD m_dwPrevEditFmt; // Format of previously edited subitem
	CImageList m_imgList; // Image list for items
	CImageList m_headerImgList; // Image list for the header control
	int m_nSortCol; // The sorted column, -1 if none
	BOOL m_bSortAscending; // Is sort ascending?

	//////////////////////////////////////////////////////////////////////
	// Wizard Generated Stuff
	//////////////////////////////////////////////////////////////////////

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Generated message map functions

protected:	

	//{{AFX_MSG(CReportCtrl)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////
// End of CReportCtrl Class Definition
//////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __REPORTCTRL_H__
