#include "test_helper.h"

int main(void)
{
  GSM_Phone_ATGENData data;
  memset(&data, 0, sizeof(GSM_Phone_ATGENData));

  test_result(ATGEN_IsMemoryAvailable(&data, MEM_ME) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_MT) == FALSE);

  data.PhoneSMSMemory = AT_NOTAVAILABLE;
  data.SIMSMSMemory = AT_NOTAVAILABLE;
  data.SRSMSMemory = AT_NOTAVAILABLE;
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_ME) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_MT) == FALSE);

  data.PhoneSMSMemory = AT_AVAILABLE;
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_MT) == TRUE);

  data.SIMSMSMemory = AT_AVAILABLE;
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SM) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_MT) == TRUE);

  data.SRSMSMemory = AT_AVAILABLE;
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SR) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SM) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_MT) == TRUE);

  data.PhoneSMSMemory = AT_NOTAVAILABLE;
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_ME) == FALSE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SR) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_SM) == TRUE);
  test_result(ATGEN_IsMemoryAvailable(&data, MEM_MT) == TRUE);
}
