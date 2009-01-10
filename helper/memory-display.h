#ifndef __helper_memory_display_h__
#define __helper_memory_display_h__

#include <gammu-memory.h>
#include <gammu-statemachine.h>

GSM_Error PrintMemoryEntry(GSM_MemoryEntry *entry, GSM_StateMachine *sm);
GSM_Error PrintMemorySubEntry(GSM_SubMemoryEntry *entry, GSM_StateMachine *sm);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

