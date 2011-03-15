#ifndef _DEVICEFUN_H
#define _DEVICEFUN_H
#include "Winsock2.h"
#include "commfun.h"

struct sockaddr_irda {
	unsigned short 			irdaAddressFamily;
	unsigned char 			irdaDeviceID[4];
	char 				irdaServiceName[25];
};

typedef struct {
	/* Config file (or Registry or...) variables */
	char			Model[50];	   /* Model from config file 		*/
//	char			DebugLevel[50];    /* Debug level			*/
	char 			DeviceCOMport[100];	   /* Device name from config file 	*/
	char			Connection[100];	   /* Connection type as string		*/
	bool			SyncTime;	   /* Synchronize time on startup? 	*/
	bool			LockDevice;	   /* Lock device ? (Unix)		*/
//	char			*DebugFile;        /* Name of debug file		*/
//	char 			*Localize;	   /* Name of localisation file		*/
	bool			StartInfo;	   /* Display something during start ?  */
//	bool			UseGlobalDebugFile;/* Should we use global debug file?	*/
//	bool			DefaultModel;
//	bool			DefaultDebugLevel;
//	bool			DefaultDevice;
//	bool			DefaultConnection;
//	bool			DefaultSyncTime;
//	bool			DefaultLockDevice;
//	bool			DefaultDebugFile;
//	bool			DefaultLocalize;
//	bool			DefaultStartInfo;
} GSM_Config;
/*
typedef struct {
	HANDLE 		hPhone;
	DCB 		old_settings;
	OVERLAPPED 	osWrite,osRead;
} GSM_Device_SerialData;

typedef struct {
    int hPhone;
} GSM_Device_BlueToothData;
typedef struct {
	int 			hPhone;
	struct sockaddr_irda	peer;
} GSM_Device_IrdaData;
*/
typedef struct {
	GSM_ConnectionType 	ConnectionType;				/* Type of connection as int			*/
	GSM_Config		CurrentConfig;				/* Config file (or Registry or...) variables 	*/
//	GSM_Device_SerialData SerialData;
//	GSM_Device_BlueToothData BlueToothData;
//	GSM_Device_IrdaData	IrdaData;
	HANDLE 		hPhone;                      //for SerialData
	DCB 		old_settings;					//for SerialData
	OVERLAPPED 	osWrite,osRead;					//for SerialData
	SOCKET		hSocketPhone;					//for winsocket
	struct sockaddr_irda	peer;			//for winsocket //irda
	char		irdamodel[200];			//for winsocket //irda

} GSM_DeviceData;

#endif