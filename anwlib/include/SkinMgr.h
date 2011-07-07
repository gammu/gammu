//
// SkinMgr.h
//

#ifndef _SKINMGR_H_
#define _SKINMGR_H_

#define WM_CHANGESBCOLOR	WM_SYSCOLORCHANGE

extern "C" {

void SkinMgr_Startup(void);
void SkinMgr_Shutdown(void);
void SkinMgr_InstallHook(DWORD dwThreadID = -1, BOOL bDialogOnly = TRUE);
void SkinMgr_UninstallHook(DWORD dwThreadID = -1);
void SkinMgr_Enable(BOOL bEnable = TRUE);
BOOL SkinMgr_IsEnabled(void);
//void SkinMgr_UninstallRadioButton(BOOL bUninstall = TRUE);
void SkinMgr_UnhookRadioButton(BOOL bUnhook = true);
void SkinMgr_UnhookStaticCtrl(BOOL bUnhook = true);
COLORREF SkinMgr_GetColor(int nColor);
void SkinMgr_SetColor(int nColor, COLORREF color);
}

#endif	// _SKINMGR_H_