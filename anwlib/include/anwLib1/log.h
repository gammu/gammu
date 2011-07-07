/*/////////////////////////////////////////////////////////////////////////////

Copyright (c) 2000-2002 Advanced & Wise Technology Corp.  All Rights Reserved.
File Name:		log.h
Author:			max huang
Description:	debug log file
Platform:		Win32
Compiler:		MS Visual C++ 6.0 in sp5 with Platform SDK Nov 2001.
Data:			2003/02/12 v0.1			max huang start develop

/////////////////////////////////////////////////////////////////////////////*/

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "_defanwlib1ext.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// DebugLog
class ANWLIB1_EXT CLog
{
	public:
		CLog();
		~CLog();

		//CLog::file( LPCTSTR czfile );
		inline void turn_on(BOOL bOn);		// off log
		inline void start();	// start log
		inline void stop();		// stop log
		void clear();			// clear log file
		void trace( const TCHAR* szFormat, ... );


	private:
		FILE *m_stream;
		TCHAR czfname[_MAX_PATH];
		BOOL m_bON;		// TRUE: on, FALSE: off
		BOOL m_bShow;	// show log

		BOOL xSystemVersion();

};

inline void CLog::turn_on(BOOL bOn)
{	m_bON = bOn;	
};

inline void CLog::start()
{	m_bShow = TRUE;	
};

inline void CLog::stop()
{	m_bShow = FALSE;
};

#endif //_LOG_H