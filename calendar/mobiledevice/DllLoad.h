/*#############################################################################
# DLLLOAD.H
#
# SCA Software International S.A.
# http://www.scasoftware.com
# scaadmin@scasoftware.com
#
# Copyright (c) 1999 SCA Software International S.A.
#
# Date: 21.12.1999.
# Author: Zoran M.Todorovic
#
# This software is provided "AS IS", without a warranty of any kind.
# You are free to use/modify this code but leave this header intact.
#
#############################################################################*/

#ifndef __DLL_LOAD_H__
#define __DLL_LOAD_H__

//=============================================================================
// class TDllModule
// Base class for dynamically loaded DLL's
//=============================================================================

class TDllModule {
  public:
    enum TErrorCode {
      ERROR_OK = 0,
      ERROR_DLLNOTFOUND,
      ERROR_INVALIDEXPORT,
      ERROR_NOTCREATED
    };
  
  protected:
    int m_nErrorCode;
    CString m_strName;
    HINSTANCE m_hHandle;

  public:
    TDllModule(LPCTSTR szName);
    virtual ~TDllModule();
    
    virtual BOOL Create(void);
    virtual void Destroy(void);
    
    int GetErrorCode(void)      { return m_nErrorCode; }
    CString GetDLLName(void)    { return m_strName; }
};

#endif

/*#############################################################################
# End of file DLLLOAD.H
#############################################################################*/

