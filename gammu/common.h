#include <gammu.h>

extern volatile bool gshutdown;
extern GSM_StateMachine *gsm;
extern bool phonedb;
extern bool batch;
extern bool batchConn;
extern bool always_answer_yes;

const char *GetMonthName(const int month);
const char *GetDayName(const int day);
void Print_Error(GSM_Error error);
void GSM_Init(bool checkerror);
void GSM_Terminate(void);
void GetStartStop(int *start, int *stop, int num, int argc, char *argv[]);
bool answer_yes(const char *text);

/**
 * Prints information about security status.
 */
void PrintSecurityStatus();

/**
 * Converts string to memory type.
 */
GSM_MemoryType MemoryTypeFromString(const char *type);

/**
 * Write error to user.
 */
PRINTF_STYLE(1, 2)
int printf_err(const char *format, ...);

/**
 * Write warning to user.
 */
PRINTF_STYLE(1, 2)
int printf_warn(const char *format, ...);

/**
 * Interrupt handler, sets shutdown flag.
 */
void interrupt(int sign);

#ifdef GSM_ENABLE_BEEP
void GSM_PhoneBeep(void);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
