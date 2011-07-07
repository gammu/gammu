// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__29C309AC_B65A_4BE5_AECC_408EC42613D9__INCLUDED_)
#define AFX_STDAFX_H__29C309AC_B65A_4BE5_AECC_408EC42613D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcview.h>
#include <Commctrl.h>
#include <afxole.h>
#include <Shlobj.h>
#include "import_lib.h"

#pragma	warning(disable: 4786)
#pragma warning(push,3)
#include <string>
#include <memory>
#include <deque>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#pragma warning(pop)

using namespace std;

#ifdef _UNICODE
typedef wstring Tstring;
#else
typedef string Tstring;
#endif
#include "global.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__29C309AC_B65A_4BE5_AECC_408EC42613D9__INCLUDED_)
