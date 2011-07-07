/* (c) 2003 by Michal Cihar */

/*
 * This file includes some core MBdrv headers, that you will probably need.
 */
#ifndef _MBDRV_
#define _MBDRV_
#pragma once

/* --------------------------- security codes ------------------------------ */


#define GSM_SECURITY_CODE_LEN	15

#include "coding.h"
#include "gsmpbk.h"
#include "gsmlogo.h"
#include "gsmnet.h"
#include "commFun.h"
#include "gsmcal.h"
#include "gsmsms.h"
#include "gsmmulti.h"
typedef struct {
	/**
	 * Actual code.
	 */
	char 			Code[GSM_SECURITY_CODE_LEN+1];
	/**
	 * Type of the code.
	 */
	GSM_SecurityCodeType 	Type;
} GSM_SecurityCode;


#endif
/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
