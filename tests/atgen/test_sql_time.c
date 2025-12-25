#include "test_helper.h"
#include "../../smsd/core.h"

#define TS20190715120000 1563192000
#define TS20191215120000 1576411200

void SMSDSQL_Time2String(GSM_SMSDConfig * Config, time_t timestamp, char *static_buff, size_t size);
time_t SMSDSQL_ParseDate(GSM_SMSDConfig * Config, const char *date);

int get_local_timezone_offset(time_t posix_time);

void print_time(const struct tm *tm)
{
	char buffer[128] = {0};
  strftime(buffer, 128, "'%Y-%m-%d %H:%M:%S'", tm);
  puts(buffer);
}

GSM_DateTime *mk_dt(
  int year, int month, int day,
  int hour, int minute, int second,
  int offset)
{
  static GSM_DateTime dt;

  dt.Year = year;
  dt.Month = month;
  dt.Day = day;
  dt.Hour = hour;
  dt.Minute = minute;
  dt.Second = second;
  dt.Timezone = offset*3600;

  return &dt;
}

void get_sql_string(char *dest, const char* driver_name, time_t posix_time)
{
  GSM_SMSDConfig config;
  memset(&config, 0, sizeof(GSM_SMSDConfig));
  config.driver = driver_name;
  SMSDSQL_Time2String(&config, posix_time, dest, 128);
}

/**************************************************/

void test_fill_time_t_gmt(void)
{
  GSM_DateTime dt = *mk_dt(2019, 12, 15, 12, 00, 00, 0);
  struct tm tm;
  time_t time;

  puts(__func__);

  time = Fill_Time_T(dt);
#ifndef WIN32
  putenv((char*)"TZ=:Europe/London");
#else
  putenv("TZ=GMT0");
  tzset();
#endif
  tm = *localtime(&time);

  test_result(tm.tm_hour == 12);
  test_result(time == TS20191215120000);
}

void test_fill_time_t_dst(void)
{
  GSM_DateTime dt = *mk_dt(2019, 7, 15, 12, 00, 00, 0);
  struct tm tm;
  time_t time;

  puts(__func__);

  time = Fill_Time_T(dt);
#ifndef WIN32
  putenv((char*)"TZ=:Europe/London");
#else
  putenv("TZ=GMT-1");
  tzset();
#endif
  tm = *localtime(&time);

  test_result(tm.tm_hour == 13);
  test_result(time == TS20190715120000);
}

void test_fill_time_t_cet(void)
{
  // origin timestamp +5 hours == GMT0700 == CET0800
  GSM_DateTime dt = *mk_dt(2019, 12, 15, 12, 00, 00, 5);
  struct tm tm;
  time_t time;

  puts(__func__);

  time = Fill_Time_T(dt);
#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-1");
  tzset();
#endif
  tm = *localtime(&time);

  test_result(tm.tm_hour == 8);
  test_result(time == TS20191215120000 - 5*3600);
}

void test_fill_time_t_cest(void)
{
  // origin timestamp -3 hours == GMT1500 == CEST1700
  GSM_DateTime dt = *mk_dt(2019, 7, 15, 12, 00, 00, -3);
  struct tm tm;
  time_t time;

  puts(__func__);

  time = Fill_Time_T(dt);
#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-2");
  tzset();
#endif
  tm = *localtime(&time);

  test_result(tm.tm_hour == 17);
  test_result(time == TS20190715120000 + 3*3600);
}

void test_local_tz_offset_cet(void)
{
  GSM_DateTime dt = *mk_dt(2019, 12, 15, 12, 00, 00, 0);
  int offset;

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-1");
  tzset();
#endif

  offset = get_local_timezone_offset(Fill_Time_T(dt));
  test_result(offset == 3600);
}

void test_local_tz_offset_cest(void)
{
  GSM_DateTime dt = *mk_dt(2019, 7, 15, 12, 00, 00, 0);
  int offset;

  puts(__func__);

#ifndef WIN32
  putenv((char *) "TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-2");
  tzset();
#endif

  offset = get_local_timezone_offset(Fill_Time_T(dt));
  test_result(offset == 7200);
}

void test_sql_parse_date_no_dst(void)
{
  const char* expected = "2019-12-15 12:00:00";
  char actual[128];
  time_t time;
  struct tm tm;

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-1");
  tzset();
#endif
  time = SMSDSQL_ParseDate(NULL, expected);
  tm = *localtime(&time);

  strftime(actual, 128, "%Y-%m-%d %H:%M:%S", &tm);
  test_result(strcmp(expected, actual) == 0);
}

void test_sql_parse_date_with_dst(void)
{
  const char* expected = "2019-07-15 12:00:00";
  char actual[128];
  time_t time;
  struct tm tm;

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-2");
  tzset();
#endif
  time = SMSDSQL_ParseDate(NULL, expected);
  tm = *localtime(&time);

  strftime(actual, 128, "%Y-%m-%d %H:%M:%S", &tm);
  test_result(strcmp(expected, actual) == 0);
}

void neg_timestamp(void)
{
  char actual[128];

  puts(__func__);

  get_sql_string(actual, "pgsql", -2);

  test_result(strcmp("0000-00-00 00:00:00", actual) == 0);
}

void pgsql_timestamp(void)
{
  GSM_DateTime dt = *mk_dt(2019, 5, 8, 11, 48, 44, 2);
  char actual[128];

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-2");
  tzset();
#endif
  get_sql_string(actual, "pgsql", Fill_Time_T(dt));

  test_result(strcmp("2019-05-08 11:48:44", actual) == 0);
}

void mysql_timestamp(void)
{
  GSM_DateTime dt = *mk_dt(2019, 5, 8, 11, 48, 44, 0);
  char actual[128];

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-2");
  tzset();
#endif
  get_sql_string(actual, "mysql", Fill_Time_T(dt));

  test_result(strcmp("2019-05-08 13:48:44", actual) == 0);
}

void oracle_timestamp(void)
{
  GSM_DateTime dt = *mk_dt(2019, 5, 8, 11, 48, 44, -3);
  char actual[128];

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/London");
#else
  putenv("TZ=GMT-1");
  tzset();
#endif
  get_sql_string(actual, "oracle", Fill_Time_T(dt));

  test_result(strcmp("TIMESTAMP '2019-05-08 15:48:44'", actual) == 0);
}

void odbc_timestamp(void)
{
  GSM_DateTime dt = *mk_dt(2019, 12, 8, 11, 48, 44, 0);
  char actual[128];

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/London");
#else
  putenv("TZ=GMT0");
  tzset();
#endif
  get_sql_string(actual, "odbc", Fill_Time_T(dt));

  test_result(strcmp("{ ts '2019-12-08 11:48:44' }", actual) == 0);
}

void access_timestamp(void)
{
  GSM_DateTime dt = *mk_dt(2019, 5, 8, 11, 48, 44, 0);
  char actual[128];

  puts(__func__);

#ifndef WIN32
  putenv((char*)"TZ=:Europe/Warsaw");
#else
  putenv("TZ=CET-2");
  tzset();
#endif
  get_sql_string(actual, "access", Fill_Time_T(dt));

  test_result(strcmp("'2019-05-08 13:48:44'", actual) == 0);
}

int main(void)
{
  test_fill_time_t_gmt();
  test_fill_time_t_dst();

  test_fill_time_t_cet();
  test_fill_time_t_cest();

  test_local_tz_offset_cet();
  test_local_tz_offset_cest();

  test_sql_parse_date_no_dst();
  test_sql_parse_date_with_dst();

  neg_timestamp();

  pgsql_timestamp();
  mysql_timestamp();
  oracle_timestamp();

  odbc_timestamp();
  access_timestamp();
}
