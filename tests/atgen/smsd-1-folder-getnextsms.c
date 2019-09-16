#include <assert.h>
#include "test_helper.h"
#include "../../libgammu/gsmstate.h"
#include "../../smsd/core.h"

GSM_Error SMSD_Init(GSM_SMSDConfig *Config);
gboolean SMSD_CheckSMSStatus(GSM_SMSDConfig *Config);

/*
 * Test against #378 - All SMS received twice
 */
int main(void)
{
  GSM_Error error;
  GSM_StateMachine *s = NULL;
  GSM_Phone_ATGENData *Priv;
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");

  const char *responses[] = {
    // AT+CPMS=?
    "+CPMS: (\"ME\"),(\"ME\")\r",
    "OK\r\n",
    // AT+CPMS="ME","ME"
    "+CPMS: 1,99,1,99\r",
    "OK\r\n",
    // AT+CPMS="ME","ME"
    "+CPMS: 1,99,1,99\r",
    "OK\r\n",
    // AT+CMGL=4
    "+CMGL: 1,0,\"\",23\r",
    "07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r",
    "OK\r\n",
    // AT+CPMS="ME"
    "+CPMS: 1,99,1,99\r",
    "OK\r\n",
    // AT+CMGR=1
    "+CMGR: 1,\"\",23\r",
    "07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r",
    "OK\r\n",
    // AT+CPMS="ME","ME"
    "+CPMS: 1,99,1,99\r",
    "OK\r\n",
    // AT+CPMS="ME"
    "+CPMS: 1,99,1,99\r",
    "OK\r\n",
    // AT+CMGD=1
    "OK\r\n",
    // AT+CMGR=1
    "ERROR\r\n",

    // Sentinel
    "ERROR\r\n"
  };
  SET_RESPONSES(responses);

  error = SMSD_ReadConfig("smsd.cfg", config, TRUE);
  test_result(error == ERR_NONE);
  SMSD_EnableGlobalDebug(config);

  s = config->gsm;
  Priv = &s->Phone.Data.Priv.ATGEN;

  GSM_SetDebugGlobal(TRUE, GSM_GetDebug(s));
  setup_at_engine(s);
  bind_response_handling(s);

  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  Priv->SMSMemory = MEM_ME;

  error = SMSD_Init(config);
  test_result(error == ERR_NONE);

  error = SMSD_CheckSMSStatus(config);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}
