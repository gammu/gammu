#ifndef _BT_MAIN_H_
#define _BT_MAIN_H_

#pragma once
#include "BtIfDefinitions.h"
typedef struct
{
	BD_ADDR		bd_address;  // UINT8 [6]. Address of device, use to pair or select device.
	DEV_CLASS	dev_class;	// UINT8 [3]. Class of device. If dev_class[1] = 2, the device is phone class.
	BD_NAME		bd_name;	// UINT8 [248]. Device name, NULL terminated.
	BOOL			connect;	// Indicate if device is connected to local PC
	BD_MODEL	model_name; 
} DeviceTable;

typedef enum
{
    SUCCESS,
    BAD_PARAMETER, //pin_code is NULL or too long
    NO_BT_SERVER, //can't access the device
    ALREADY_BONDED,
    FAIL	//timeout or reject by device
} BOND_RETURN_CODE;

typedef enum
{
 BT_Error_None = 0,
 BT_Error_NoDriver = 30000,
 BT_Error_StackServerDown,
 BT_Error_DeviceNotReady,
 BT_Error_BaseModuleFail,
 BT_Error_DeviceNotSupport,
 BT_Error_DeviceBusy, 
 BT_Error_AudioModuleFail,
 BT_Error_DeviceNotPaired
} BTError;



//-- get the number of  the paired device count
unsigned char WINAPI BTGetPairNum(unsigned char *PairNums);
typedef unsigned char (WINAPI *BTGetPairNumAPI)(unsigned char *PairNums);
extern BTGetPairNumAPI BTGetPairNumFunc;

//-- get paired device list
unsigned char WINAPI BTGetPairDevice(DeviceTable *TableList);
typedef unsigned char (WINAPI *BTGetPairDeviceAPI)(DeviceTable *TableList);
extern BTGetPairDeviceAPI BTGetPairDeviceFunc;

//-- search
typedef unsigned char (WINAPI *FnOnInquiryResponse)(BD_ADDR bda, DEV_CLASS dev_class, BD_NAME bd_name, BOOL b_connect, BOOL bComplete);

unsigned char WINAPI BTDeviceInquiry(FnOnInquiryResponse OnInquiryResponse);
typedef unsigned char (WINAPI *BTDeviceInquiryAPI)(FnOnInquiryResponse OnInquiryResponse);
extern BTDeviceInquiryAPI BTDeviceInquiryFunc;

//-- pairing
BOND_RETURN_CODE WINAPI BTDevicePairing(BD_ADDR bda, LPTSTR pin_code);
typedef BOND_RETURN_CODE (WINAPI *BTDevicePairingAPI)(BD_ADDR bda, LPTSTR pin_code);
extern BTDevicePairingAPI BTDevicePairingFunc;

//-- unpairing
unsigned char WINAPI BTDeviceUnPairing(BD_ADDR bda);
typedef unsigned char (WINAPI *BTDeviceUnPairingAPI)(BD_ADDR bda);
extern BTDeviceUnPairingAPI BTDeviceUnPairingFunc;

//-- select device
unsigned char WINAPI BTSelectDevice(BD_ADDR bda);
typedef unsigned char (WINAPI *BTSelectDeviceAPI)(BD_ADDR bda);
extern BTSelectDeviceAPI BTSelectDeviceFunc;

//-- stop search
void WINAPI BTStopInquiry(void);
typedef void (WINAPI *BTStopInquiryAPI)(void);
extern BTStopInquiryAPI BTStopInquiryFunc;

//-- Model Name
BTError WINAPI BTGetDeviceModel(LPTSTR DeviceModel);
typedef BTError (WINAPI *BTGetDeviceModelAPI)(LPTSTR DeviceModel);
extern BTGetDeviceModelAPI BTGetDeviceModelFunc;

//-- COM port
unsigned char WINAPI BTGetComPort(unsigned char* comport);
typedef unsigned char (WINAPI *BTGetComPortAPI)(unsigned char* comport);
extern BTGetComPortAPI BTGetComPortFunc;

typedef unsigned char (WINAPI *BTGetComPortsAPI)(UINT8 *comport, UINT8* BufLen);
extern BTGetComPortsAPI BTGetComPortsFunc;
typedef UINT8  (WINAPI *BTGetDeviceIMEIAPI)(LPTSTR IMEI);
extern BTGetDeviceIMEIAPI BTGetDeviceIMEIFunc;

//--- terminate
unsigned char WINAPI BTTerminate();
typedef unsigned char( WINAPI *BTTerminateAPI)();
extern BTTerminateAPI BTTerminateFunc;

//--- check status
BTError WINAPI BTCheckStatus();
typedef BTError (WINAPI *BTCheckStatusAPI)();
extern BTCheckStatusAPI BTCheckStatusFunc;

//--- IVT
bool WINAPI BTCreateCOMTunnel();
typedef bool (WINAPI *BTCreateCOMTunnelAPI)();
extern BTCreateCOMTunnelAPI BTCreateCOMTunnelFunc;

bool WINAPI BTDeleteCOMTunnel();
typedef bool (WINAPI *BTDeleteCOMTunnelAPI)();
extern BTDeleteCOMTunnelAPI BTDeleteCOMTunnelFunc;

bool WINAPI BTCreateDUNTunnel(BOOL  bCreate);
typedef bool (WINAPI *BTCreateDUNTunnelAPI)(BOOL  bCreate);
extern BTCreateDUNTunnelAPI BTCreateDUNTunnelFunc;


void WINAPI BTGetStackDriverVersion(LPTSTR pszStackVersion);
typedef void (WINAPI *BTGetStackDriverVersionAPI)(LPTSTR pszStackVersion);
extern BTGetStackDriverVersionAPI BTGetStackDriverVersionFunc;

#endif