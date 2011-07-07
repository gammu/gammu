//
// WORKER.h
//

#ifndef _WORKER_H_
#define _WORKER_H_

class CWorker;

DWORD WINAPI WorkerProc(CWorker *pWorker);

// Callback function prototype:
typedef bool (CALLBACK *PROGRESSPROC)(BOOL, DWORD, DWORD);

class CWorker  
{
	friend DWORD WINAPI WorkerProc(CWorker *pWorker);	// Worker thread

// Construction
public:
	CWorker();
	~CWorker();

// Operations
public:
	BOOL Start(PROGRESSPROC pCallback, DWORD uUserData1, DWORD uUserData2);
	BOOL Stop();

	void SetLoopCount(UINT nCount) { m_nLoopCount = nCount; }
	
	virtual BOOL Do();

// Attributes
public:
	BOOL InProgress();

// Implementation
protected:
	DWORD  m_uUserData1, m_uUserData2;
	HANDLE m_hWorkerThread;

	UINT   m_nLoopCount;
	BOOL   m_bStop;
	
	PROGRESSPROC m_pCallback;
};

#endif // _WORKER_H_
