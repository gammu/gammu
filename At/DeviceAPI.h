#ifndef _DEVICEAPI_H
#define _DEVICEAPI_H
#include "DeviceDef.h"
#include "mbglobals.h"
#include "commFun.h"

typedef GSM_Error (WINAPI* OpenDeviceProc)(GSM_DeviceData *s,Debug_Info *debugInfo);
typedef GSM_Error (WINAPI* CloseDeviceProc)(GSM_DeviceData *s,Debug_Info *debugInfo);
typedef GSM_Error (WINAPI* DeviceSetParityProc)(GSM_DeviceData *s, bool parity,Debug_Info *debugInfo);
typedef GSM_Error (WINAPI* DeviceSetDtrRtsProc)(GSM_DeviceData *s, bool dtr, bool rts,Debug_Info *debugInfo);
typedef GSM_Error (WINAPI* DeviceSetSpeedProc)(GSM_DeviceData *s, int speed,Debug_Info *debugInfo);
typedef int (WINAPI* ReadDeviceProc)(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo);
typedef int (WINAPI* WriteDeviceProc)(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo);

class CDeviceAPI
{
public:
	CDeviceAPI();
	~CDeviceAPI();

	CString m_strLibFileName;
	BOOL LoadDeviceApiLibrary(CString strDllName);
	HINSTANCE m_hDll;
	OpenDeviceProc			OpenDevicefn;
	CloseDeviceProc			CloseDevicefn;
	DeviceSetParityProc			DeviceSetParityfn;
	DeviceSetDtrRtsProc			DeviceSetDtrRtsfn;
	DeviceSetSpeedProc		DeviceSetSpeedfn;
	ReadDeviceProc			ReadDevicefn;
	WriteDeviceProc		WriteDevicefn;

	GSM_DeviceData	m_DeviceData;
	BOOL            m_bDeviceOpened;
	Debug_Info	*m_pDebugInfo;

	int FindSerialSpeed(char *buffer);
	int _mystrnicmp( const char *string1, const char *string2);
	GSM_Error InitDeviceData(Debug_Info	*pDebugInfo,GSM_Config ConfigInfo);

	GSM_Error DeviceAPI_OpenDevice();
	GSM_Error DeviceAPI_CloseDevice();
	GSM_Error DeviceAPI_DeviceSetParity(bool parity);
	GSM_Error DeviceAPI_DeviceSetDtrRts(bool dtr, bool rts);
	GSM_Error DeviceAPI_DeviceSetSpeed(int speed);
	int DeviceAPI_ReadDevice(void *buf, size_t nbytes);
	int DeviceAPI_WriteDevice(void *buf, size_t nbytes);


};
#endif