
typedef struct {
	char	*parameter;
	int	min_arg;
	int	max_arg;
	void	(*Function) (int argc, char *argv[]);	
} GSM_Parameters;

#include "../common/misc/misc.h"
#include "../common/gsmstate.h"

void	Print_Error		(GSM_Error error);
void	GSM_Init		(bool checkerror);
void	GSM_Terminate		(void);

extern	GSM_StateMachine	s;
extern	GSM_Phone_Functions	*Phone;
extern	GSM_Error		error;

extern volatile bool 		bshutdown;

void interrupted(int sig);

#ifdef GSM_ENABLE_BEEP
void	GSM_PhoneBeep		(void);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
