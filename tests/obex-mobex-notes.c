/* Unsupported note reads must not disrupt an m-OBEX session. */

#include <gammu.h>
#include <stdio.h>

#include "../libgammu/gsmstate.h"
#include "../libgammu/phone/obex/obexfunc.h"
#include "common.h"

static GSM_Error UnexpectedWrite(GSM_StateMachine *s, const unsigned char *buffer,
				size_t length, int type)
{
	fprintf(stderr, "Unexpected OBEX request: 0x%02x\n", type);
	test_result(FALSE);
	return ERR_BUG;
}

int main(void)
{
	GSM_StateMachine *s;
	GSM_Protocol_Functions protocol = {0};
	GSM_NoteEntry note = {0};
	GSM_ToDoStatus status = {0};
	GSM_Error error;
	int count;

	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	s->CurrentConfig = GSM_GetConfig(s, 0);
	s->ReplyNum = 1;
	protocol.WriteMessage = UnexpectedWrite;
	s->Protocol.Functions = &protocol;
	s->Phone.Data.Priv.OBEXGEN.Service = OBEX_m_OBEX;

	error = OBEXGEN_GetNoteStatus(s, &status);
	gammu_test_result_code(error, "m-OBEX note status", ERR_NOTSUPPORTED);
	test_result(s->Phone.Data.Priv.OBEXGEN.Service == OBEX_m_OBEX);

	note.Location = 1;
	error = OBEXGEN_GetNote(s, &note);
	gammu_test_result_code(error, "m-OBEX note read", ERR_NOTSUPPORTED);
	test_result(s->Phone.Data.Priv.OBEXGEN.Service == OBEX_m_OBEX);

	/* Reject iteration with unknown, empty, and populated note counts. */
	for (count = -1; count <= 1; count++) {
		s->Phone.Data.Priv.OBEXGEN.NoteCount = count;
		error = OBEXGEN_GetNextNote(s, &note, TRUE);
		gammu_test_result_code(error, "m-OBEX first note", ERR_NOTSUPPORTED);
		test_result(s->Phone.Data.Priv.OBEXGEN.Service == OBEX_m_OBEX);

		error = OBEXGEN_GetNextNote(s, &note, FALSE);
		gammu_test_result_code(error, "m-OBEX next note", ERR_NOTSUPPORTED);
		test_result(s->Phone.Data.Priv.OBEXGEN.Service == OBEX_m_OBEX);
	}

	GSM_FreeStateMachine(s);
	return 0;
}
