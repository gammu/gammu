#include <gammu-error.h>
#include <gammu-statemachine.h>
#include <gammu-debug.h>
#include <assert.h>
#include "../../libgammu/gsmstate.h"
#include "../../libgammu/gsmphones.h"

#include "test_helper.h"

ssize_t _ResponseReadDevice(GSM_StateMachine *s, void *buf, size_t nbytes);
GSM_Error _ResponseWriteMessage(GSM_StateMachine *s, const unsigned char *buffer, size_t length, int type);

#define  _BUFFER_SIZE 512

static struct {
  int echoed;
  size_t echo_len;
  unsigned char echo[_BUFFER_SIZE];
} _echo_buffer;

static struct {
  GSM_StateMachine *stateMachine;
  size_t pos;
  size_t queue_size;
  const char **queue;
} _response_queue;

static GSM_Device_Functions _response_dev_funcs;

void bind_response_handling(GSM_StateMachine *s)
{
  _response_queue.stateMachine = s;
  _response_dev_funcs.ReadDevice = &_ResponseReadDevice;
  memset(&_echo_buffer, 0, sizeof(_echo_buffer));

  if(s->Device.Functions == NULL)
    s->Device.Functions = &_response_dev_funcs;
  else
    s->Device.Functions->ReadDevice = _response_dev_funcs.ReadDevice;

  s->Protocol.Functions = &ATProtocol;
  s->Protocol.Functions->WriteMessage = &_ResponseWriteMessage;
  s->ReplyNum = 1;
  s->opened = TRUE;
}

void set_responses(const char **responses, size_t size)
{
  _response_queue.queue = responses;
  _response_queue.pos = 0;
  _response_queue.queue_size = size;
}

const unsigned char* last_command(void)
{
  return _echo_buffer.echo;
}

void set_echo(const unsigned char *buf, const size_t len)
{
  if(buf && len > 0) {
    memccpy(_echo_buffer.echo, buf, sizeof(*buf), len);
    _echo_buffer.echo[len] = '\0';
    _echo_buffer.echo_len = len;
    _echo_buffer.echoed = FALSE;
  }
}

ssize_t _ResponseReadDevice(GSM_StateMachine *s UNUSED, void *buf, size_t nbytes)
{
  size_t read_len = 0;

  if(_response_queue.stateMachine->Phone.Data.SentMsg == NULL)
    return 0;

  if(_echo_buffer.echoed == FALSE && _echo_buffer.echo_len > 0) {
    read_len = _echo_buffer.echo_len;
    if(read_len > nbytes) {
      // shouldn't happen in current design, so truncate for now.
      memcpy(buf, _echo_buffer.echo, nbytes);
    } else {
      memcpy(buf, _echo_buffer.echo, read_len);
    }

    _echo_buffer.echoed = TRUE;
  }
  else if(_response_queue.pos < _response_queue.queue_size) {
    read_len = strlen(_response_queue.queue[_response_queue.pos]);
    if(read_len > nbytes)
      return 0;

    memcpy(buf, _response_queue.queue[_response_queue.pos], read_len);
    _response_queue.pos++;
  }

  return read_len;
}

GSM_Error _ResponseWriteMessage(GSM_StateMachine *s UNUSED, const unsigned char *buffer, size_t length, int type UNUSED)
{
  if(length) {
    GSM_DumpMessageText(s, buffer, length, type);
    GSM_DumpMessageBinary(s, buffer, length, type);

    set_echo(buffer, length);
  }
  return ERR_NONE;
}

GSM_StateMachine* setup_state_machine(void)
{
  GSM_Debug_Info *debug_info;
  GSM_StateMachine *s;

  /* Configure state machine */
  debug_info = GSM_GetGlobalDebug();
  GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
  GSM_SetDebugLevel("textall", debug_info);

  /* Allocates state machine */
  s = GSM_AllocStateMachine();
  test_result(s != NULL);
  debug_info = GSM_GetDebug(s);
  GSM_SetDebugGlobal(TRUE, debug_info);

  s->ReplyNum = 1;

  return s;
}

void cleanup_state_machine(GSM_StateMachine *s)
{
  if(s->Phone.Data.Priv.ATGEN.Lines.allocated > 0) {
    FreeLines(&s->Phone.Data.Priv.ATGEN.Lines);
    GetLineString(NULL, NULL, 0);
  }

  GSM_FreeStateMachine(s);
}

GSM_Phone_ATGENData* setup_at_engine(GSM_StateMachine *s)
{
  GSM_Phone_Data *Data;
  GSM_Phone_ATGENData *Priv;
  GSM_Protocol_ATData *d;

  /* Initialize AT engine */
  Data = &s->Phone.Data;
  Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
  Data->RequestID = ID_None;

  Priv = &s->Phone.Data.Priv.ATGEN;
  Priv->ReplyState = AT_Reply_OK;
  Priv->SMSMode = SMS_AT_PDU;
  Priv->Charset = AT_CHARSET_GSM;
  s->Phone.Functions = &ATGENPhone;

  d = &s->Protocol.Data.AT;
  d->Msg.Buffer 		= NULL;
  d->Msg.BufferUsed	= 0;
  d->Msg.Length		= 0;
  d->Msg.Type		= 0;
  d->SpecialAnswerLines	= 0;
  d->LineStart		= -1;
  d->LineEnd		= -1;
  d->wascrlf 		= FALSE;
  d->EditMode		= FALSE;
  d->FastWrite		= FALSE;
  d->CPINNoOK		= FALSE;

  return Priv;
}

size_t read_file(const char *filepath,void *buffer,  const size_t buffer_len)
{
  FILE *f;
  size_t bytes_read = 0;

  assert(NULL != buffer);

  f = fopen(filepath, "r");
  if (f == NULL) {
    printf("Could not open %s\n", filepath);
    return 0;
  }

  bytes_read = fread(buffer, 1, buffer_len, f);

  if (!feof(f))
    printf("Could not read whole file %s\n", filepath);

  fclose(f);
  return bytes_read;
}
