/*#############################################################################
# DLLLOAD.CPP
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

#include "stdafx.h"
#include "DllLoad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=============================================================================
// class TDllModule 
//
//=============================================================================

TDllModule::TDllModule(LPCTSTR szName) 
{
  m_strName = szName;
  m_hHandle = NULL;
  m_nErrorCode = ERROR_NOTCREATED;
}

TDllModule::~TDllModule()
{
  Destroy();
}

// Dynamically load a DLL
// Returns TRUE if loaded
BOOL TDllModule::Create(void)
{
  m_hHandle = ::LoadLibrary(m_strName);
  if (m_hHandle == NULL) {
    m_nErrorCode = ERROR_DLLNOTFOUND;
    return FALSE;
  }
  m_nErrorCode = ERROR_OK;
  return TRUE;
}

// Unload the DLL
void TDllModule::Destroy(void)
{
  if (m_hHandle) {
    ::FreeLibrary(m_hHandle);
    m_hHandle = NULL;
    m_nErrorCode = ERROR_NOTCREATED;
  }
}

/*#############################################################################
# End of file DLLLOAD.CPP
#############################################################################*/
