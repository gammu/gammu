#include "test_helper.h"

int main(void)
{
  GSM_Phone_ATGENData data;
  memset(&data, 0, sizeof(GSM_Phone_ATGENData));

  test_result(ATGEN_IsMemoryWriteable(&data, MEM_ME) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_MT) == FALSE);

  data.PhoneSaveSMS = AT_NOTAVAILABLE;
  data.SRSaveSMS = AT_NOTAVAILABLE;
  data.SIMSaveSMS = AT_NOTAVAILABLE;
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_ME) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_MT) == FALSE);

  data.PhoneSaveSMS = AT_AVAILABLE;
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_MT) == TRUE);

  data.SIMSaveSMS = AT_AVAILABLE;
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SR) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SM) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_MT) == TRUE);

  data.SRSaveSMS = AT_AVAILABLE;
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SR) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SM) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_MT) == TRUE);

  data.SIMSaveSMS = AT_NOTAVAILABLE;
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_ME) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SR) == TRUE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_SM) == FALSE);
  test_result(ATGEN_IsMemoryWriteable(&data, MEM_MT) == TRUE);
}
