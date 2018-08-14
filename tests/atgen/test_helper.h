#ifndef H_ATGEN_TEST_HELPER_INCLUDED
#define H_ATGEN_TEST_HELPER_INCLUDED

#include <stdlib.h>
#include <string.h>
#include "../../libgammu/phone/at/atgen.h"
#include "../common.h"

#if _MSC_VER < 1910
#define __func__ __FUNCTION__
#endif

#define UNNEEDED(x) (void)(x)

#define SET_RESPONSES(responses) \
  set_responses(responses, sizeof(responses) / sizeof(*(responses)))

GSM_StateMachine* setup_state_machine(void);
GSM_Phone_ATGENData* setup_at_engine(GSM_StateMachine *s);
void cleanup_state_machine(GSM_StateMachine *s);


void set_responses(const char **responses, size_t size);
void bind_response_handling(GSM_StateMachine *s);
const unsigned char* last_command(void);

size_t read_file(const char *filepath, void *buffer,size_t buffer_len);

#endif /* H_ATGEN_TEST_HELPER_INCLUDED */
