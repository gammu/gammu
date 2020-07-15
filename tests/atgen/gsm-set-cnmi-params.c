#include "gammu-error.h"
#include "test_helper.h"

GSM_Error GSM_ReadCNMIParams(int *out_params, const char *args);

int params_eq(const int *a, const int *b) {
  return memcmp(a, b, 5 * sizeof(int)) == 0;
}

void with_null_args_does_nothing(void)
{
  GSM_Error error;
  int params[5] = {5,5,5,5,5};
  const int expected[5] = {5,5,5,5,5};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, NULL);
  test_result(error == ERR_NONE);
  test_result(params_eq(params, expected));
}

void with_empty_args_does_nothing(void)
{
  GSM_Error error;
  int params[5] = {5,5,5,5,5};
  const int expected[5] = {5,5,5,5,5};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, "");
  test_result(error == ERR_NONE);
  test_result(params_eq(params, expected));
}

void with_partial_args_defaults_others(void)
{
  GSM_Error error;
  int params[5] = {5,5,5,5,5};
  const int expected[5] = {5,5,2,5,5};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, ",,2");
  test_result(error == ERR_NONE);
  test_result(params_eq(params, expected));
}

void ignores_spaces(void)
{
  GSM_Error error;
  int params[5] = {5,5,5,5,5};
  const int expected[5] = {1,5,7,5,5};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, "  1 , ,7, ");
  test_result(error == ERR_NONE);
  test_result(params_eq(params, expected));
}

void sets_all_params(void)
{
  GSM_Error error;
  int params[5] = {0};
  const int expected[5] = {1,2,3,4,5};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, "1,2,3,4,5");
  test_result(error == ERR_NONE);
  test_result(params_eq(params, expected));
}

void sets_some_params(void)
{
  GSM_Error error;
  int params[5] = {2,1,0,0,1};
  const int expected[5] = {2,1,0,1,0};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, "2,,,1,0");
  test_result(error == ERR_NONE);
  test_result(params_eq(params, expected));
}

void fails_on_invalid_input(void)
{
  GSM_Error error;
  int params[5] = {0};
  const int expected[5] = {1,2};

  puts(__func__);

  error = GSM_ReadCNMIParams(params, "\t1, 2;3,4");
  test_result(error == ERR_INVALIDDATA);
  test_result(params_eq(params, expected));
}

int main(void)
{
  with_null_args_does_nothing();
  with_empty_args_does_nothing();
  with_partial_args_defaults_others();
  ignores_spaces();
  sets_all_params();
  sets_some_params();
  fails_on_invalid_input();
}
