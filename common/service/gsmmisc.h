/* (c) 2001-2005 by Marcin Wiacek, Walek and Michal Cihar */

#ifndef __gsm_misc_h
#define __gsm_misc_h

#include "../gsmcomon.h"

#include <gammu-file.h>
#include <gammu-info.h>

/* --------------------------- resetting phone settings  ------------------- */


/* --------------------------- security codes ------------------------------ */


/* ---------------------------- keyboard ----------------------------------- */


/* ------------------------------- display features ------------------------ */



/**
 * Resets all members of GSM_BatteryCharge structure.
 */
void GSM_ClearBatteryCharge(GSM_BatteryCharge *bat);

/* ------------------------------ categories ------------------------------- */


/* ------------------- radio FM stations ---------------------------------- */


/* ----------------------- filesystem ------------------------------------- */


GSM_Error GSM_ReadFile(char *FileName, GSM_File *File);

GSM_Error GSM_JADFindData(GSM_File File, char *Vendor, char *Name, char *JAR, char *Version, int *Size);

void GSM_IdentifyFileFormat(GSM_File *File);

bool GSM_ReadHTTPFile(unsigned char *server, unsigned char *filename, GSM_File *file);

/* ----------------------------- GPRS access points ----------------------- */


/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */

bool ReadVCALDateTime(const char *Buffer, GSM_DateTime *dt);
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);

bool ReadVCALDate(char *Buffer, char *Start, GSM_DateTime *Date, bool *is_date_only);
void SaveVCALDate(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);

/**
 * Stores text in vCalendar/vCard formatted buffer.
 *
 * @param Buffer Buffer where output will be stored.
 * @param Length Current position in output buffer (will be updated).
 * @param Text Value to be stored.
 * @param Start Name of field which is being stored.
 * @param UTF8 Whether text should be stored in UTF-8 without prefix.
 */
void SaveVCALText(char *Buffer, int *Length, unsigned char *Text, char *Start, bool UTF8);

/**
 * Reads text from vCalendar/vCard formatted buffer.
 *
 * @param Buffer Buffer where input is stored.
 * @param Start Name of field which is being read.
 * @param Value Storage for read value.
 * @param UTF8 Whether text should be read in UTF-8 without prefix.
 */
bool ReadVCALText(char *Buffer, char *Start, unsigned char *Value, bool UTF8);

bool ReadVCALInt(char *Buffer, char *Start, int *Value);

unsigned char *VCALGetTextPart(unsigned char *Buff, int *pos);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
