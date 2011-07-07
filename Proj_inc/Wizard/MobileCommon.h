#if !defined(AFX_MOBILECOMMON_H)
#define AFX_MOBILECOMMON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MobileCommon.h : header file
//

enum	TaskMode_TAG	{
	NoneInitStatusMode = 0,
	Wizard_PhotoLab,	//From MobileTask, for Wallpaper
	Wizard_PhotoLab2,	//From MobileTask, for Sticker
	Wizard_PIM,			//not use
	Wizard_MMS,			//not use
	Wizard_Animation,	//From MobileTask, for Animation
	Wizard_Ring,		//From MobileTask, for Ring
	Wizard_Java,		//not use
	Mgr_PhotoLab,		//From PhotoMgr, for Photo Edit
	Mgr_Animation,		//not use
	Wizard_AVIAnim,		//From MobileTask, for AVI Animation
//		Mgr_PIM,	
} TaskMode;



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOBILECOMMON_H)
