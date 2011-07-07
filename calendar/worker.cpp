//
// WORKER.cpp
//

#include "StdAfx.h"
//#include "Event.h"
#include "Worker.h"

CWorker::CWorker()
{
	m_hWorkerThread = NULL;
	m_pCallback = NULL;
	m_uUserData1 = 0;
	m_uUserData2 = 0;
	m_nLoopCount = 0;

	m_bStop = FALSE;
}

CWorker::~CWorker()
{
	Stop();
}

// Abort
BOOL CWorker::Stop()
{
	if(InProgress())
	{	
		m_bStop = TRUE;

		return TRUE;
	}

	return FALSE;
}

BOOL CWorker::Do()
{
	for(UINT i = 0; i < m_nLoopCount; i++)
	{
		if(m_bStop == TRUE)
		{
			m_nLoopCount = 0;
			return FALSE;
		}

		//TRACE("Loop = %u/%u -- ", i, m_nLoopCount);
		m_pCallback(FALSE, m_uUserData1, m_uUserData2);
		//TRACE("OK\n");
	}

	return TRUE;
}

// worker thread.
DWORD WINAPI WorkerProc(CWorker *pWorker)
{
	BOOL ret = pWorker->Do();
	
	if(pWorker->m_pCallback)
	{
		if(ret == TRUE)
			pWorker->m_pCallback(TRUE, pWorker->m_uUserData1, pWorker->m_uUserData2);
		else
			pWorker->m_pCallback(TRUE, NULL, pWorker->m_uUserData2);
	}

	return 0;
}

// Start
BOOL CWorker::Start(PROGRESSPROC pCallback, DWORD uUserData1, DWORD uUserData2)
{
	if(InProgress())
		return FALSE;

	if(m_hWorkerThread)
	{
		CloseHandle(m_hWorkerThread);
	}

	m_pCallback  = pCallback;
	m_uUserData1 = uUserData1;
	m_uUserData2 = uUserData2;
	
	m_bStop = FALSE;

    // 2002/07/01, Jesse Modify
	//m_hWorkerThread = CreateThread(NULL, NULL, (PTHREAD_START_ROUTINE)WorkerProc, (void *)this, 0, NULL);
    DWORD dwThreadId;
	m_hWorkerThread = CreateThread(NULL, 512, (PTHREAD_START_ROUTINE)WorkerProc, (void *)this, 0, &dwThreadId);
	if(m_hWorkerThread == NULL)
	{
		return FALSE;
	}

	return TRUE;	
}

// Check if the work is in progress.
BOOL CWorker::InProgress()
{
	return (m_hWorkerThread && WaitForSingleObject(m_hWorkerThread, 1000) == WAIT_TIMEOUT) ? TRUE : FALSE;
}
