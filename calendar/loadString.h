#ifndef _LOADSTRING_H
#define _LOADSTRING_H

int CalendarStringMessageBox(HWND hWnd, LPCTSTR lpMessageText,UINT uType =MB_OK |MB_ICONINFORMATION);
int CalendarMessageBox(HWND hWnd, LPCTSTR lpSectionText,UINT uType=MB_OK|MB_ICONINFORMATION);
CString  LoadStringFromFile(CString strSection, CString strKey,CString strFilename="");

#endif