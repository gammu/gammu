#ifndef __ERRORCODE_H__
#define __ERRORCODE_H__

enum
{
    Anw_SUCCESS=         	        1		,   // Success
    Anw_MOBILE_BUSY=     			2000	,   // MOBILE Busy.
    Anw_TIMEOUT,      							// Time Out.
    Anw_MOBILE_CONNECT_FAILED,					// Mobile Connect Failed.
    Anw_MOBILE_WRITE_FAILED,					// Mobile WRITE Failed.
    Anw_MOBILE_READ_FAILED,						// Mobile READ Failed.
	Anw_MOBILE_DELETE_FAILED,
    Anw_PARAMETER_CHECK_ERROR,					// Unknown connection type given by user 
    Anw_ALLOCATE_MEMORY_FAILED,					// Allocate Memory Failed.
//    Anw_MEMORY_RW_FAILED,						// Memory R/W Verify Failed.
//    Anw_MOBILE_POWER_OFF,						// MOBILE power off.
//    Anw_NOT_FOUNT_WINASPI,						// Can't find winaspi
//    Anw_NOT_FOUND_MOBILE,						// Can't find mobile. 
	Anw_NOT_LINK_DRV,						    // Can't LINK DRIVER. 
	Anw_NOTSUPPORTED,	                   	    // Not supported by phone 
	Anw_EMPTY,		 	                        // Empty phonebook entry, ... 
	Anw_CANTOPENFILE, 	                    	// Error during opening file 
	Anw_FILENOTSUPPORTED,                       // File format not supported by SyncDrv 
	Anw_INVALIDDATETIME,		                // Invalid date/time 
	Anw_Phone_MEMORY,			                // Phone memory error, maybe it is read only 
	Anw_INVALIDDATA,	                   	    //Invalid data
	
	Anw_NOT_LINK_MBdrv,						    // Can't LINK MBdrv.DLL. 
	Anw_FileName_FAILED,						// File Name FAILED.
	Anw_Thread_Terminate,
    Anw_NOSIM_ERROR,                            //Compare; added by mingfa , v1.0.2.1
	Anw_TRIAL_LIMITEDFUNCTION,
	Anw_MOBILENOTRESPONSE,
    Anw_UNKNOW_ERROR=				2999	,	// unknow error
};

/*
enum
{
    Anw_SUCCESS=         	        1		,   // Success
    Anw_MOBILE_BUSY=     			2000	,   // MOBILE Busy.
    Anw_TIMEOUT,      							// Time Out.
    Anw_MOBILE_CONNECT_FAILED,					// Mobile Connect Failed.
    Anw_MOBILE_WRITE_FAILED,					// Mobile WRITE Failed.
    Anw_MOBILE_READ_FAILED,						// Mobile READ Failed.
    Anw_MOBILE_DELETE_FAILED,						// Mobile READ Failed.
    Anw_PARAMETER_CHECK_ERROR,					// Parameter Check Error.
    Anw_ALLOCATE_MEMORY_FAILED,					// Allocate Memory Failed.
    Anw_MEMORY_RW_FAILED,						// Memory R/W Verify Failed.
    Anw_MOBILE_POWER_OFF,						// MOBILE power off.
    Anw_NOT_FOUNT_WINASPI,						// Can't find winaspi
    Anw_NOT_FOUND_MOBILE,						// Can't find mobile. 
	Anw_NOT_LINK_MBdrv,						    // Can't LINK MBdrv.DLL. 
	Anw_FileName_FAILED,						// File Name FAILED.
	Anw_Thread_Terminate,
    Anw_UNKNOW_ERROR=				2999	,	// unknow error
};
*/
#endif