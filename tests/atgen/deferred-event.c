#include <gammu-message.h>
#include "test_helper.h"
#include "../../libgammu/phone/at/atgen.h"
#include "../../libgammu/gsmstate.h"
#include "../../libgammu/phone/at/atfunc.h"

GSM_Error EventQueue_Pop(GSM_StateMachine *s, EventBinding *binding);
GSM_Error EventQueue_Push(GSM_StateMachine *s, EventBinding *binding);
GSM_Error ATGEN_BeforeDeferredEventHook(GSM_StateMachine *s);

void IncomingCallHandler(GSM_StateMachine *s UNUSED, GSM_Call *call UNUSED, void *user_data)
{
  *(int*)user_data += 1;
  smprintf(s, "IncomingCallHandler: %s\n", call->PhoneNumber);
}

void calls_all_handlers(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SecurityCodeType status;
  EventBinding binding;
  GSM_Call call;
  int event_count = 0;
  int i;

  const char *responses[] = {
    "+CPIN: READY\r",
    "OK\r\n"
  };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  UNNEEDED(Priv);

  s->Phone.Data.EnableIncomingCall = TRUE;
  s->User.IncomingCall = &IncomingCallHandler;
  s->User.IncomingCallUserData = &event_count;

  memset(&call, 0, sizeof(call));
  s->Phone.Data.RequestID = ID_GetSecurityStatus;

  memset(
    &s->Phone.Data.DeferredEvents.event_bindings[MAX_DEFERRED_EVENTS],
    0xFF,
    sizeof(EventBinding));

  for(i = 1; i <= MAX_DEFERRED_EVENTS; ++i) {
    call.PhoneNumber[0] = '0' + i;
    GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);
  }

  test_result(EventQueue_Push(s, &binding) == ERR_FULL)

  EventQueue_Pop(s, &binding);

  call.PhoneNumber[1] = '1';
  GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);

  test_result(EventQueue_Push(s, &binding) == ERR_FULL)
  test_result(event_count == 0)

  s->Phone.Data.RequestID = ID_None;

  // not deferred
  call.PhoneNumber[1] = '2';
  GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);

  test_result(event_count == 1)

  EventQueue_Pop(s, &binding);

  error = ATGEN_GetSecurityStatus(s, &status);

  test_result(error == ERR_NONE)
  test_result(event_count == MAX_DEFERRED_EVENTS)
  test_result(EventQueue_Pop(s, &binding) == ERR_EMPTY)

  cleanup_state_machine(s);
}

void does_not_call_cancelled_handlers(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SecurityCodeType status;
  EventBinding binding;
  GSM_Call call;
  int event_count = 0;
  int i;

  const char *responses[] = {
    "+CPIN: READY\r",
    "OK\r\n"
  };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  UNNEEDED(Priv);

  s->Phone.Data.EnableIncomingCall = TRUE;
  s->User.IncomingCall = &IncomingCallHandler;
  s->User.IncomingCallUserData = &event_count;

  memset(&call, 0, sizeof(call));
  s->Phone.Data.RequestID = ID_GetSecurityStatus;

  memset(
    &s->Phone.Data.DeferredEvents.event_bindings[MAX_DEFERRED_EVENTS],
    0xFF,
    sizeof(EventBinding));
  for(i = 1; i <= MAX_DEFERRED_EVENTS; ++i) {
    call.PhoneNumber[0] = '0' + i;
    GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);
  }

  EventQueue_Pop(s, &binding);
  EventQueue_Pop(s, &binding);
  call.PhoneNumber[1] = 'Q';
  GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);

  GSM_CancelEventsOfType(s, GSM_EV_CALL);

  call.PhoneNumber[1] = 'X';
  GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);

  s->Phone.Data.RequestID = ID_None;

  error = ATGEN_GetSecurityStatus(s, &status);

  test_result(error == ERR_NONE)
  test_result(event_count == 1)
  test_result(EventQueue_Pop(s, &binding) == ERR_EMPTY)

  cleanup_state_machine(s);
}

int main(void)
{
  calls_all_handlers();
  does_not_call_cancelled_handlers();
}
