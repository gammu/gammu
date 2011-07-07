#ifndef _MOBILEINFO_H
#define _MOBILEINFO_H
#include <afxtempl.h>
#include "commfun.h"
typedef struct 
{
	char ID[MAX_PATH];
	char Model[MAX_PATH];
	bool UseSPP;
} _PhoneModel;

BOOL GetSupportMobileInfoList(CPtrList *pSupportModelList);
#endif