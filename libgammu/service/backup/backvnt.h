/**
 * vNote backup handling.
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */
/* Licensend under GNU GPL 2 */

#ifndef __gsm_backvnt_h
#define __gsm_backvnt_h

#include <gammu-error.h>
#include <gammu-backup.h>

#ifdef GSM_ENABLE_BACKUP
GSM_Error SaveVNT(char *FileName, GSM_Backup *backup);
GSM_Error LoadVNT(char *FileName, GSM_Backup *backup);
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
