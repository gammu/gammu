#ifndef _gammu_common_h
#define _gammu_common_h

#include <gammu.h>

extern volatile bool gshutdown;
extern GSM_StateMachine *gsm;
extern INI_Section *cfg;
extern bool batch;
extern bool batchConn;
extern bool always_answer_yes;

const char *GetMonthName(const int month);
const char *GetDayName(const int day);
void Print_Error(GSM_Error error);
void GSM_Init(bool checkerror);
void GSM_Terminate(void);
void GetStartStop(int *start, int *stop, int num, int argc, char *argv[]);

/**
 * Requests response from user on single question.
 */
PRINTF_STYLE(1, 2)
bool answer_yes(const char *format, ...);

/**
 * Prints information about security status.
 */
void PrintSecurityStatus(void);

/**
 * Converts string to memory type.
 */
GSM_MemoryType MemoryTypeFromString(const char *type);

/**
 * Interrupt handler, sets shutdown flag.
 */
void interrupt(int sign);

/**
 * Downloads file from arbitrary URL.
 *
 * \param url URL to download.
 * \param file Storage for data.
 *
 * \returns true on success.
 */
bool GSM_ReadHTTPFile(const char *url, GSM_File *file);

/**
 * Cleanup performed at program termination.
 */
void Cleanup(void);

/**
 * Cleanups and terminates program.
 */
NORETURN
void Terminate(int code);

#ifdef GSM_ENABLE_BEEP
void GSM_PhoneBeep(void);
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
