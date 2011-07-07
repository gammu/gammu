///////////////////////////////////////////////////////////////////////////////
//	CBrowseFolder                                                                    
//
//	This is a simple class to wrap the SHBrowseForFolder function.

#pragma once
#include "_defControlext.h"

enum retCode 
{
	BIF_RET_OK = 0,
	BIF_RET_CANCEL,
	BIF_RET_NOPATH,
};

class CONTROL_EXT CBrowseFolder
{
public:
	CBrowseFolder();
	~CBrowseFolder();

protected:
	// Holds the current style
	DWORD m_style;

public:
	// Modifies the current style
	DWORD ModifyStyle(DWORD add, DWORD remove = 0);
	// Returns the current style
	DWORD GetStyle();
	// Displays the dialog
	int Show(HWND parent, LPTSTR pathBuffer);
	// Set the title of the dialog
	void SetTitle(LPTSTR title);

protected:
	// Buffer to receieve the display name of the selected object
	TCHAR m_displayName[MAX_PATH];
	// Root item to start browsing at
	LPITEMIDLIST m_root;
	// Text to display above the tree view control
	TCHAR m_title[MAX_PATH];
		
};
