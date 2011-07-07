#ifndef _MMSCODCFUN_H
#define _MMSCODCFUN_H
#include "interface.h"

int AnwMMSEncodeSendReq(CString strFileName, MMSEDIT *pSentmsg, MMSSET Set, char *pSendId,DWORD& dwsize);
int AnwMMSDecodeReveveData(MMSEDIT **pEditData,CString strFileName);
void AnwDestroyMmsEditInfo(MMSEDIT *pEditInfo);
CString AnwGetFirstImage(CString strFileName);
BOOL WINAPI	AnwGetMMSHeader(CString mmsFileName,CString &strTo,CString &strCC,CString &strSubject);
BOOL WINAPI	AnwGetFirstImageAndHeader(CString mmsFileName,CString& strFileImage,CString &strTo,CString &strCC,CString &strSubject);

#endif