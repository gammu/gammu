#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>

GSM_StateMachine *s;

int main(int argc UNUSED, char **argv UNUSED)
{
	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}

	/* Free state machine */
	GSM_FreeStateMachine(s);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
