#include "stdafx.h"
#include "DeviceAPI.h"

CDeviceAPI::CDeviceAPI()
{
	m_hDll = NULL;
	m_DeviceData.ConnectionType = GCT_ERROR;
	m_bDeviceOpened = FALSE;
	m_pDebugInfo = NULL;
	m_strLibFileName.Empty();
}
CDeviceAPI::~CDeviceAPI()
{
	if(m_hDll)
	{
		if(m_bDeviceOpened)
			DeviceAPI_CloseDevice();

		FreeLibrary(m_hDll);
	}
}

BOOL CDeviceAPI::LoadDeviceApiLibrary(CString strDllName)
{
	if(!m_strLibFileName.IsEmpty())
	{
		if(m_strLibFileName.CompareNoCase(strDllName) == 0)
			if(m_hDll) return true;
		else
		{
			if(m_hDll)
			{
				if(m_bDeviceOpened)
					DeviceAPI_CloseDevice();

				FreeLibrary(m_hDll);
			}
		}

	}
	m_strLibFileName.Empty();

	m_hDll = ::LoadLibrary(strDllName);
	if(!m_hDll)
		return FALSE;

	m_strLibFileName.Format("%s",strDllName);
	OpenDevicefn = (OpenDeviceProc)GetProcAddress(m_hDll,"OpenDevice");
	CloseDevicefn = (CloseDeviceProc)GetProcAddress(m_hDll,"CloseDevice");
	DeviceSetParityfn = (DeviceSetParityProc)GetProcAddress(m_hDll,"DeviceSetParity");
	DeviceSetDtrRtsfn = (DeviceSetDtrRtsProc)GetProcAddress(m_hDll,"DeviceSetDtrRts");
	DeviceSetSpeedfn = (DeviceSetSpeedProc)GetProcAddress(m_hDll,"DeviceSetSpeed");
	ReadDevicefn = (ReadDeviceProc)GetProcAddress(m_hDll,"ReadDevice");
	WriteDevicefn = (WriteDeviceProc)GetProcAddress(m_hDll,"WriteDevice");
	return true;
}
int CDeviceAPI::FindSerialSpeed(char *buffer)
{
	switch (atoi(buffer)) {
		case 50		: return 50;
		case 75		: return 75;
		case 110	: return 110;
		case 134	: return 134;
		case 150	: return 150;
		case 200	: return 200;
		case 300	: return 300;
		case 600	: return 600;
		case 1200	: return 1200;
		case 1800	: return 1800;
		case 2400	: return 2400;
		case 4800	: return 4800;
		case 9600	: return 9600;
		case 19200	: return 19200;
		case 38400	: return 38400;
		case 57600	: return 57600;
		case 115200	: return 115200;
		case 230400	: return 230400;
		default		: return 0;	
	}
}
int CDeviceAPI::_mystrnicmp( const char *string1, const char *string2)
{
	size_t s1,s2;
	s1 = strlen(string1);
	s2 = strlen(string2);
	if(s1<s2)
		return _strnicmp (string1	,string2,s1);
	else
		return _strnicmp (string1	,string2,s2);
	return 0;

}
GSM_Error CDeviceAPI::InitDeviceData(Debug_Info	*pDebugInfo,GSM_Config ConfigInfo)
{
	char szConnectType[MAX_PATH];
	m_pDebugInfo = pDebugInfo;
	wsprintf(szConnectType,"%s",ConfigInfo.Connection);
	m_DeviceData.ConnectionType = GCT_ERROR;


//	if (_mystrnicmp ("mbus"	,szConnectType) == 0) m_DeviceData.ConnectionType = GCT_MBUS2;
	if (_mystrnicmp ("fbus"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2;
	if (_mystrnicmp ("fbusdlr3"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2DLR3;
	if (_mystrnicmp ("fbusdku5"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2DKU5;
	if (_mystrnicmp ("fbuspl2303"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2PL2303;
	if (_mystrnicmp ("fbusblue"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2BLUE;
	if (_mystrnicmp ("fbusirda"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2IRDA;
	if (_mystrnicmp ("phonetblue"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_PHONETBLUE;
//	if (_mystrnicmp ("mrouterblue"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_MROUTERBLUE;
	if (_mystrnicmp ("irdaphonet"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_IRDAPHONET;
	if (_mystrnicmp ("irdaat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_IRDAAT;
	if (_mystrnicmp ("irdaobex"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_IRDAOBEX;
	if (_mystrnicmp ("blueobex"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEOBEX;
	if (_mystrnicmp ("bluefbus"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEFBUS2;
	if (_mystrnicmp ("bluephonet"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEPHONET;
	if (_mystrnicmp ("blueat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEAT;
	if (_mystrnicmp ("bluerfobex"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEOBEX;
	if (_mystrnicmp ("bluerffbus"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEFBUS2;
	if (_mystrnicmp ("bluerfphonet",szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEPHONET;
	if (_mystrnicmp ("bluerfat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEAT;

	/* These are for compatibility only */
	if (_mystrnicmp ("atblue"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEAT;
	if (_mystrnicmp ("dlr3blue"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_BLUEFBUS2;
	if (_mystrnicmp ("irda"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_IRDAPHONET;
	if (_mystrnicmp ("dlr3"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2DLR3;
	if (_mystrnicmp ("infrared"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FBUS2IRDA;

	if (_mystrnicmp("at"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_AT;
	if (_mystrnicmp("cableat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_CABLEAT;
	if (_mystrnicmp("phoneat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_PHONEAT;
	if (_mystrnicmp("cablephoneat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_CABLEPHONEAT;
	if (_mystrnicmp("ftpat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_FTPAT;

	if (_mystrnicmp("syncmlat"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_SYNCMLAT;
	if (_mystrnicmp("syncmlbt"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_SYNCMLBT;
	if (_mystrnicmp("syncmlusb"	,szConnectType)== 0) m_DeviceData.ConnectionType = GCT_SYNCMLUSB;

	if (m_DeviceData.ConnectionType==0) return ERR_UNKNOWNCONNECTIONTYPESTRING;



//	m_DeviceData.CurrentConfig.Connection = (char *)malloc( strlen(pConnectMode)+1 );
//	if (m_DeviceData.CurrentConfig.Connection== NULL) return ERR_MOREMEMORY;
	strcpy(m_DeviceData.CurrentConfig.Connection,ConfigInfo.Connection);
	
//	m_DeviceData.CurrentConfig.Device 	= (char *)malloc( strlen(pConnectPort)+1 );
//	if (m_DeviceData.CurrentConfig.Device == NULL) return ERR_MOREMEMORY;
	strcpy(m_DeviceData.CurrentConfig.DeviceCOMport,ConfigInfo.DeviceCOMport);

	if(strlen(ConfigInfo.Model) >0)
		strcpy(m_DeviceData.CurrentConfig.Model,ConfigInfo.Model);
	else
		m_DeviceData.CurrentConfig.Model[0]='\0';

	return ERR_NONE;
}


GSM_Error CDeviceAPI::DeviceAPI_OpenDevice()
{
	GSM_Error error;
	if(OpenDevicefn)
	{
		error =  OpenDevicefn(&m_DeviceData,m_pDebugInfo);
		if (error==ERR_NONE)
			m_bDeviceOpened = TRUE;

		return error;
	}
		
	return ERR_BUG;
}
GSM_Error CDeviceAPI::DeviceAPI_CloseDevice()
{
	GSM_Error error;
	if(CloseDevicefn && m_bDeviceOpened)
	{
		error = CloseDevicefn(&m_DeviceData,m_pDebugInfo);
		m_bDeviceOpened = FALSE;
		return error;
	}
	return ERR_BUG;
}
GSM_Error CDeviceAPI::DeviceAPI_DeviceSetParity(bool parity)
{
	if(DeviceSetParityfn)
	{
		if(m_bDeviceOpened)
			return DeviceSetParityfn(&m_DeviceData,parity,m_pDebugInfo);
		else
			return	ERR_NOTCONNECTED;
	}
	return ERR_BUG;
}
GSM_Error CDeviceAPI::DeviceAPI_DeviceSetDtrRts(bool dtr, bool rts)
{
	if(DeviceSetDtrRtsfn)
	{
		if(m_bDeviceOpened)
			return DeviceSetDtrRtsfn(&m_DeviceData,dtr,rts,m_pDebugInfo);
		else
			return	ERR_NOTCONNECTED;
	}
	return ERR_BUG;
}
GSM_Error CDeviceAPI::DeviceAPI_DeviceSetSpeed(int speed)
{
	if(DeviceSetSpeedfn)
	{
		if(m_bDeviceOpened)
			return DeviceSetSpeedfn(&m_DeviceData,speed,m_pDebugInfo);
		else
			return	ERR_NOTCONNECTED;
	}
	return ERR_BUG;
}
int CDeviceAPI::DeviceAPI_ReadDevice(void *buf, size_t nbytes)
{
	if(ReadDevicefn)
	{
		if(m_bDeviceOpened)
			return ReadDevicefn(&m_DeviceData,buf, nbytes,m_pDebugInfo);
		else
			return	ERR_NOTCONNECTED;
	}
	return ERR_BUG;
}
int CDeviceAPI::DeviceAPI_WriteDevice(void *buf, size_t nbytes)
{
	if(WriteDevicefn)
	{
		if(m_bDeviceOpened)
			return WriteDevicefn(&m_DeviceData,buf, nbytes,m_pDebugInfo);
		else
			return	ERR_NOTCONNECTED;

	}
	return ERR_BUG;
}
