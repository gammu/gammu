/*
 * serial interface threads for gnokii on win32
 * Based on the MS sample program 'tty.c'
 *
 * Roger Willcocks  16 Sept 99
 *
 * compile with:
 * cl -Zi -DWIN32 -DVERSION=\"win32\" -DMODEL=\"6110\" -DPORT=\"COM1\" \
 * gnokii.c winserial.c fbus-6110.c getopt.c gsm-api.c fbus-6110-auth.c \
 * fbus-6110-ringtones.c gsm-networks.c cfgreader.c 
 */

#ifdef WIN32

#define USECOMM      // yes, we need the COMM API

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <memory.h>

#include "devices/winserial.h"

#define sleep(x) Sleep((x) * 1000)
#define usleep(x) Sleep(((x) < 1000) ? 1 : ((x) / 1000))

#define BAUDRATE CBR_115200

DWORD FAR PASCAL CommWatchProc(LPSTR lpData);
DWORD FAR PASCAL KeepAliveProc(LPSTR lpData);
BOOL SetupConnection();
#define MAXBLOCK 1024

//---------------------------------------------------------------------------
//  BOOL OpenConnection(char *szPort)
//
//  Description:
//     Opens communication port.
//     It also sets the CommState and notifies the window via
//     the fConnected flag.
//
//  Parameters:
//     - szPort - name of com port eg "com1"
//
//---------------------------------------------------------------------------

OVERLAPPED osWrite, osRead;
HANDLE hPhone, hThread, hKAThread;
BOOL   isConnected;
DWORD  ThreadID, KAThreadID;

BOOL OpenConnection(char *szPort, sigcallback fn, keepalive ka)
{
    BOOL       fRetVal;
    HANDLE        hCommWatchThread;
    DWORD         dwThreadID;
    COMMTIMEOUTS  CommTimeOuts;

    if ((hPhone =
	 CreateFile(szPort, GENERIC_READ | GENERIC_WRITE,
		    0,                    // exclusive access
		    NULL,                 // no security attrs
		    OPEN_EXISTING,
		    FILE_ATTRIBUTE_NORMAL |
		    FILE_FLAG_OVERLAPPED, // overlapped I/O
		    NULL)) == (HANDLE) -1)
	return (FALSE);
    else {
	// get any early notifications

	SetCommMask(hPhone, EV_RXCHAR);

	// setup device buffers

	SetupComm(hPhone, 4096, 4096);

	// purge any information in the buffer

	PurgeComm(hPhone, PURGE_TXABORT | PURGE_RXABORT |
		  PURGE_TXCLEAR | PURGE_RXCLEAR);

	// set up for overlapped I/O

	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000;
#if 0
	// CBR_9600 is approximately 1byte/ms. For our purposes, allow
	// double the expected time per character for a fudge factor.
	CommTimeOuts.WriteTotalTimeoutMultiplier = 2*CBR_9600/BAUDRATE;
#else
	CommTimeOuts.WriteTotalTimeoutMultiplier = 10;
#endif
	CommTimeOuts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(hPhone, &CommTimeOuts);
    }

    fRetVal = SetupConnection();

    if (fRetVal) {
	isConnected = TRUE;

	// Create a secondary thread to issue keepAlive packets

/* Marcin Wiacek. In this moment there is NO method of communication,
   which require keepalive packets. I comment this thread, because it
   makes only CPU time (100%) and makes it more hot */
//	hKAThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,
//				0,
//				(LPTHREAD_START_ROUTINE) KeepAliveProc,
//				(LPVOID) ka,
//				0, &KAThreadID );
//	if (!hKAThread)
//	{
//	    isConnected = FALSE;
//	    CloseHandle(hPhone);
//	    return FALSE;
//	}
 
	// Create a secondary thread
	// to watch for an event.

	hCommWatchThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,
				  0,
				  (LPTHREAD_START_ROUTINE) CommWatchProc,
				  (LPVOID) fn,
				  0, &dwThreadID);
	if (!hCommWatchThread)
	{
	    isConnected = FALSE;
	    CloseHandle(hPhone);
	    fRetVal = FALSE;
	}
	else {
	    ThreadID = dwThreadID;
	    hThread  = hCommWatchThread;

	    // assert DTR

	    EscapeCommFunction(hPhone, SETDTR);
	}
    }
    else {
	isConnected = FALSE;
	CloseHandle(hPhone);
    }

    return (fRetVal);

} // end of OpenConnection()

//---------------------------------------------------------------------------
//  BOOL SetupConnection()
//
//  Description:
//     This routines sets up the DCB based on settings in the
//     TTY info structure and performs a SetCommState().
//
//  Parameters:
//
//---------------------------------------------------------------------------

BOOL SetupConnection()
{
    BOOL       fRetVal;
    DCB        dcb;

    dcb.DCBlength = sizeof(DCB);

    GetCommState(hPhone, &dcb);

   dcb.BaudRate = BAUDRATE;
   dcb.ByteSize = 8;
   dcb.Parity = NOPARITY;
   dcb.StopBits = ONESTOPBIT;

    // set DTS

    dcb.fOutxDsrFlow = 0;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    // clear RTS

    dcb.fOutxCtsFlow = 0;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    // no software flow control

    dcb.fInX = dcb.fOutX = 0;

    fRetVal = SetCommState(hPhone, &dcb);

    if (fRetVal == 0)
	fRetVal = GetLastError();

    return (fRetVal);

} // end of SetupConnection()

//---------------------------------------------------------------------------
//  BOOL CloseConnection()
//
//  Description:
//     Closes the connection to the port.  Resets the connect flag
//
//  Parameters:
//
//---------------------------------------------------------------------------

BOOL CloseConnection()
{
    isConnected = FALSE;

    // block until keepalive thread terminates (should wake it really)

/* Marcin Wiacek. In this moment there is NO method of communication,
   which require keepalive packets. I comment this thread, because it
   makes only CPU time (100%) and makes it more hot */
//    WaitForSingleObject( hKAThread, INFINITE );
#ifdef _NEVER_SAY_NEVER_AGAIN_
//    while (KAThreadID != 0)
//		Sleep(250);
#endif

    // disable event notification and wait for thread
    // to halt

    SetCommMask(hPhone, 0);

    // block until thread has been halted

    WaitForSingleObject( hThread, INFINITE );
#ifdef _NEVER_SAY_NEVER_AGAIN_
    while(ThreadID != 0)
		Sleep(250);
#endif

    // drop DTR

    EscapeCommFunction(hPhone, CLRDTR);

    // purge any outstanding reads/writes and close device handle

    PurgeComm(hPhone, PURGE_TXABORT | PURGE_RXABORT |
	      PURGE_TXCLEAR | PURGE_RXCLEAR);

    CloseHandle(hPhone);
    return (TRUE);

} // end of CloseConnection()

//---------------------------------------------------------------------------
//  int ReadCommBlock(LPSTR lpszBlock, int nMaxLength)
//
//  Description:
//     Reads a block from the COM port and stuffs it into
//     the provided buffer.
//
//  Parameters:
//
//     LPSTR lpszBlock
//        block used for storage
//
//     int nMaxLength
//        max length of block to read
//
//---------------------------------------------------------------------------

int ReadCommBlock(LPSTR lpszBlock, int nMaxLength)
{
    BOOL       fReadStat;
    COMSTAT    ComStat;
    DWORD      dwErrorFlags;
    DWORD      dwLength;
    DWORD      dwError;

    // only try to read number of bytes in queue
    ClearCommError(hPhone, &dwErrorFlags, &ComStat);
    dwLength = min((DWORD) nMaxLength, ComStat.cbInQue);

    if (dwLength > 0) {
	fReadStat = ReadFile(hPhone, lpszBlock,
			     dwLength, &dwLength, &osRead);
	if (!fReadStat) {
	    if (GetLastError() == ERROR_IO_PENDING) {
		fprintf(stderr, "\n\rIO Pending");
		// We have to wait for read to complete.
		// This function will timeout according to the
		// CommTimeOuts.ReadTotalTimeoutConstant variable
		// Every time it times out, check for port errors
		while(!GetOverlappedResult(hPhone,
					   &osRead, &dwLength, TRUE)) {
		    dwError = GetLastError();
		    if(dwError == ERROR_IO_INCOMPLETE)
			// normal result if not finished
			continue;
		    else {
			// an error occurred, try to recover
			fprintf(stderr, "<CE-%u>", dwError);
			ClearCommError(hPhone, &dwErrorFlags, &ComStat);
			if (dwErrorFlags > 0)
			    fprintf(stderr, "<CE-%u>", dwErrorFlags);
			break;
		    }

		}

	    }
	    else {
		// some other error occurred
		dwLength = 0;
		ClearCommError(hPhone, &dwErrorFlags, &ComStat);
		if (dwErrorFlags > 0)
	            fprintf(stderr, "<CE-%u>", dwErrorFlags);
	    }
	}
    }

    return (dwLength);

} // end of ReadCommBlock()

//---------------------------------------------------------------------------
//  BOOL WriteCommBlock(BYTE *pByte, DWORD dwBytesToWrite)
//
//  Description:
//     Writes a block of data to the COM port specified in the associated
//     TTY info structure.
//
//  Parameters:
//
//     BYTE *pByte
//        pointer to data to write to port
//
//---------------------------------------------------------------------------

BOOL WriteCommBlock(LPSTR lpByte , DWORD dwBytesToWrite)
{

    BOOL        fWriteStat;
    DWORD       dwBytesWritten;
    DWORD       dwErrorFlags;
    DWORD   	dwError;
    DWORD       dwBytesSent=0;
    COMSTAT     ComStat;

    fWriteStat = WriteFile(hPhone, lpByte, dwBytesToWrite,
                           &dwBytesWritten, &osWrite);

    // Note that normally the code will not execute the following
    // because the driver caches write operations. Small I/O requests
    // (up to several thousand bytes) will normally be accepted
    // immediately and WriteFile will return true even though an
    // overlapped operation was specified

    if (!fWriteStat) {

	if(GetLastError() == ERROR_IO_PENDING) {
	    // We should wait for the completion of the write operation
	    // so we know if it worked or not

	    // This is only one way to do this. It might be beneficial to
	    // place the write operation in a separate thread
	    // so that blocking on completion will not negatively
	    // affect the responsiveness of the UI

	    // If the write takes too long to complete, this
	    // function will timeout according to the
	    // CommTimeOuts.WriteTotalTimeoutMultiplier variable.
	    // This code logs the timeout but does not retry
	    // the write.

	    while(!GetOverlappedResult(hPhone,
				       &osWrite, &dwBytesWritten, TRUE)) {
		dwError = GetLastError();
		if(dwError == ERROR_IO_INCOMPLETE)
		{
		    // normal result if not finished
		    dwBytesSent += dwBytesWritten;
		    continue;
		}
		else {
		    // an error occurred, try to recover
		    fprintf(stderr, "<CE-%u>", dwError);
		    ClearCommError(hPhone, &dwErrorFlags, &ComStat);
		    if (dwErrorFlags > 0)
			fprintf(stderr, "<CE-%u>", dwErrorFlags);
		    break;
		}
	    }

	    dwBytesSent += dwBytesWritten;
#if 0
	    if(dwBytesSent != dwBytesToWrite)
		fprintf(stderr, "\nProbable Write Timeout: Total of %ld bytes sent (%ld)", dwBytesSent, dwBytesToWrite);
	    else
		fprintf(stderr, "\n%ld bytes written", dwBytesSent);
#endif
	}
	else {
	    // some other error occurred
	    ClearCommError(hPhone, &dwErrorFlags, &ComStat);
	    if (dwErrorFlags > 0)
		fprintf(stderr, "<CE-%u>", dwErrorFlags);
	    return (FALSE);
	}
    }
    return (TRUE);

} // end of WriteCommBlock()


//************************************************************************
//  DWORD FAR PASCAL CommWatchProc(LPSTR lpData)
//
//  Description:
//     A secondary thread that will watch for COMM events.
//
//  Parameters:
//     LPSTR lpData
//        32-bit pointer argument
//
//************************************************************************

DWORD FAR PASCAL CommWatchProc(LPSTR lpData)
{
    DWORD       dwEvtMask;
    OVERLAPPED  os;
    int        nLength;
    BYTE       abIn[ MAXBLOCK + 1];

    sigcallback fn = (sigcallback)lpData;

    memset(&os, 0, sizeof(OVERLAPPED));

    // create I/O event used for overlapped read

    os.hEvent = CreateEvent(NULL,    // no security
                            TRUE,    // explicit reset req
                            FALSE,   // initial event reset
                            NULL); // no name
    if (os.hEvent == NULL)
	return (FALSE);

    if (!SetCommMask(hPhone, EV_RXCHAR))
	return (FALSE);

    while (isConnected) {
	dwEvtMask = 0;

	WaitCommEvent(hPhone, &dwEvtMask, NULL);

	if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR) {
	    do {
		if ((nLength = ReadCommBlock((LPSTR) abIn, MAXBLOCK))) {
		    int i;

		    for (i = 0; i < nLength; i++)
			(*fn)(abIn[i]);
		}
	    }
	    while (nLength > 0);
	}
    }

    // get rid of event handle

    CloseHandle(os.hEvent);

    // clear information in structure (kind of a "we're done flag")

    ThreadID = 0;
    hThread = NULL;

    return(TRUE);

} // end of CommWatchProc()


DWORD FAR PASCAL KeepAliveProc(LPSTR lpData)
{
/* Marcin Wiacek. In this moment there is NO method of communication,
   which require keepalive packets. I comment this thread, because it
   makes only CPU time (100%) and makes it more hot */
//    keepalive ka = (keepalive)lpData;
//    while (isConnected)
//	(*ka)();
//    KAThreadID = 0;
    return 0;
}

#endif
/* end */
