/**
 * SQL database service
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009 - 2018 Michal Čihař
 *           (c) 2010 Miloslav Semler
 *
 * Licensed under GNU GPL version 2 or later
 */

#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <time.h>
#include <gammu.h>
#include "../../libgammu/misc/strptime.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#endif

#include "../core.h"
#include "../../libgammu/misc/string.h"

/**
 * Returns name of the SQL dialect to use.
 */
const char *SMSDSQL_SQLName(GSM_SMSDConfig * Config)
{
	if (Config->sql != NULL) {
		return Config->sql;
	} else {
		return Config->driver;
	}
}

/* FIXME: I know this is broken, need to figure out better way */
const char now_plus_odbc[] = "{fn CURRENT_TIMESTAMP()} + INTERVAL %d SECOND";
const char now_plus_mysql[] = "(NOW() + INTERVAL %d SECOND) + 0";
const char now_plus_pgsql[] = "now() + interval '%d seconds'";
const char now_plus_sqlite[] = "datetime('now', '+%d seconds', 'localtime')";
const char now_plus_freetds[] = "DATEADD(second, %d, CURRENT_TIMESTAMP)";
const char now_plus_access[] = "DateAdd('s', %d, Now())";
const char now_plus_oracle[] = "CURRENT_TIMESTAMP + NUMTODSINTERVAL(%d, 'SECOND')";
const char now_plus_fallback[] = "NOW() + INTERVAL %d SECOND";

const char now_minus_odbc[] = "{fn TIMESTAMPADD(SQL_TSI_SECOND, -%d, CURRENT_TIMESTAMP)}";
const char now_minus_mysql[] = "NOW() - INTERVAL %d SECOND";
const char now_minus_pgsql[] = "now() - interval '%d seconds'";
const char now_minus_sqlite[] = "datetime('now', '-%d seconds', 'localtime')";
const char now_minus_freetds[] = "DATEADD(second, -%d, CURRENT_TIMESTAMP)";
const char now_minus_access[] = "DateAdd('s', -%d, Now())";
const char now_minus_oracle[] = "CURRENT_TIMESTAMP - NUMTODSINTERVAL(%d, 'SECOND')";
const char now_minus_fallback[] = "NOW() - INTERVAL %d SECOND";


const char *SMSDSQL_NowPlus(GSM_SMSDConfig * Config, int seconds)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		snprintf(result, sizeof(result), now_plus_mysql, seconds);
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		snprintf(result, sizeof(result), now_plus_pgsql, seconds);
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		snprintf(result, sizeof(result), now_plus_sqlite, seconds);
	} else if (strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0) {
		snprintf(result, sizeof(result), now_plus_freetds, seconds);
	} else if (strcasecmp(driver_name, "access") == 0) {
		snprintf(result, sizeof(result), now_plus_access, seconds);
	} else if (strcasecmp(driver_name, "oracle") == 0) {
		snprintf(result, sizeof(result), now_plus_oracle, seconds);
	} else if (strcasecmp(driver_name, "odbc") == 0) {
		snprintf(result, sizeof(result), now_plus_odbc, seconds);
	} else {
		snprintf(result, sizeof(result), now_plus_fallback, seconds);
	}
	return result;
}

int SMSDSQL_PhoneStatusTimeout(unsigned int status_frequency)
{
	if (status_frequency > INT_MAX - 10) {
		return INT_MAX;
	}
	return (int)status_frequency + 10;
}

static const char *SMSDSQL_NowMinus(GSM_SMSDConfig *Config, int seconds)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		snprintf(result, sizeof(result), now_minus_mysql, seconds);
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		snprintf(result, sizeof(result), now_minus_pgsql, seconds);
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		snprintf(result, sizeof(result), now_minus_sqlite, seconds);
	} else if (strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0) {
		snprintf(result, sizeof(result), now_minus_freetds, seconds);
	} else if (strcasecmp(driver_name, "access") == 0) {
		snprintf(result, sizeof(result), now_minus_access, seconds);
	} else if (strcasecmp(driver_name, "oracle") == 0) {
		snprintf(result, sizeof(result), now_minus_oracle, seconds);
	} else if (strcasecmp(driver_name, "odbc") == 0) {
		snprintf(result, sizeof(result), now_minus_odbc, seconds);
	} else {
		snprintf(result, sizeof(result), now_minus_fallback, seconds);
	}
	return result;
}

const char escape_char_odbc[] = "";
const char escape_char_mysql[] = "`";
const char escape_char_pgsql[] = "\"";
const char escape_char_sqlite[] = "";
const char escape_char_freetds[] = "\"";
const char escape_char_fallback[] = "";

static const char *SMSDSQL_EscapeChar(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		return escape_char_mysql;
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		return escape_char_pgsql;
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		return escape_char_sqlite;
	} else if (strcasecmp(driver_name, "oracle") == 0) {
		return escape_char_fallback;
	} else if (strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0 || strcasecmp(driver_name, "sybase") == 0) {
		return escape_char_freetds;
	} else if (strcasecmp(Config->driver, "odbc") == 0 || strcasecmp(Config->driver, "mssql") == 0) {
		return escape_char_odbc;
	} else {
		return escape_char_fallback;
	}
}

const char *SMSDSQL_TimeDiff(GSM_SMSDConfig *Config, const char *field)
{
	const char *driver_name;
	const char *escape_char;
	static char result[256];

	driver_name = SMSDSQL_SQLName(Config);
	escape_char = SMSDSQL_EscapeChar(Config);

	if (strcasecmp(driver_name, "mysql") == 0 ||
	    strcasecmp(driver_name, "native_mysql") == 0) {
		snprintf(result, sizeof(result),
			"TIMESTAMPDIFF(SECOND, %s%s%s, NOW())",
			escape_char, field, escape_char);
	} else if (strcasecmp(driver_name, "pgsql") == 0 ||
		   strcasecmp(driver_name, "native_pgsql") == 0) {
		snprintf(result, sizeof(result),
			"CAST(EXTRACT(EPOCH FROM (now() - %s%s%s)) AS BIGINT)",
			escape_char, field, escape_char);
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		snprintf(result, sizeof(result),
			"CAST((julianday('now', 'localtime') - "
			"julianday(%s%s%s)) * 86400 AS INTEGER)",
			escape_char, field, escape_char);
	} else if (strcasecmp(driver_name, "oracle") == 0) {
		snprintf(result, sizeof(result),
			"ROUND((CAST(CURRENT_TIMESTAMP AS DATE) - "
			"CAST(%s%s%s AS DATE)) * 86400)",
			escape_char, field, escape_char);
	} else if (strcasecmp(driver_name, "freetds") == 0 ||
		   strcasecmp(driver_name, "mssql") == 0 ||
		   strcasecmp(driver_name, "sybase") == 0) {
		snprintf(result, sizeof(result),
			"DATEDIFF(second, %s%s%s, CURRENT_TIMESTAMP)",
			escape_char, field, escape_char);
	} else if (strcasecmp(driver_name, "access") == 0) {
		snprintf(result, sizeof(result),
			"DateDiff('s', %s%s%s, Now())",
			escape_char, field, escape_char);
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		snprintf(result, sizeof(result),
			"{fn TIMESTAMPDIFF(SQL_TSI_SECOND, %s%s%s, "
			"CURRENT_TIMESTAMP)}",
			escape_char, field, escape_char);
	} else {
		snprintf(result, sizeof(result),
			"TIMESTAMPDIFF(SECOND, %s%s%s, NOW())",
			escape_char, field, escape_char);
	}
	return result;
}

const char *SMSDSQL_DayMaskPredicate(GSM_SMSDConfig *Config)
{
	const char *driver_name;
	const char *escape_char;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);
	escape_char = SMSDSQL_EscapeChar(Config);

	if (strcasecmp(driver_name, "oracle") == 0) {
		snprintf(result, sizeof(result), "BITAND(%sSendDays%s, %%2) <> 0",
			escape_char, escape_char);
	} else if (strcasecmp(driver_name, "access") == 0) {
		snprintf(result, sizeof(result), "(%sSendDays%s AND %%2) <> 0",
			escape_char, escape_char);
	} else {
		snprintf(result, sizeof(result), "(%sSendDays%s & %%2) <> 0",
			escape_char, escape_char);
	}
	return result;
}

int SMSDSQL_DayMask(int wday)
{
	if (wday < 0 || wday > 6) {
		return 127;
	}
	return 1 << ((wday + 6) % 7);
}

static int SMSDSQL_CurrentDayMask(void)
{
	time_t now = time(NULL);
	struct tm current;

#ifdef WIN32
	if (localtime_s(&current, &now) != 0) {
		return 127;
	}
#else
	if (localtime_r(&now, &current) == NULL) {
		return 127;
	}
#endif
	return SMSDSQL_DayMask(current.tm_wday);
}

const char rownum_clause_fallback[] = "";


static const char *SMSDSQL_RownumClause(GSM_SMSDConfig * Config, const char *count, gboolean in_where)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "oracle") == 0 || strcasecmp(driver_name, "freetds") == 0) {
		const char *prefix = in_where ? " AND " : " WHERE ";

		snprintf(result, sizeof(result), "%sROWNUM <= %s ", prefix, count);

		return result;
	} else {
		return rownum_clause_fallback;
	}
}


const char top_clause_access[] = "TOP";
const char top_clause_fallback[] = "";

static const char *SMSDSQL_TopClause(GSM_SMSDConfig * Config, const char *count)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "access") == 0 || strcasecmp(driver_name, "mssql") == 0) {
		snprintf(result, sizeof(result), "%s %s ", top_clause_access, count);
		return result;
	} else {
		return top_clause_fallback;
	}
}

const char limit_clause_access[] = "";
const char limit_clause_fallback[] = "LIMIT";

static const char *SMSDSQL_LimitClause(GSM_SMSDConfig * Config, const char *count)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mssql") == 0 || strcasecmp(driver_name, "access") == 0 || strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "oracle") == 0) {
		return limit_clause_access;
	} else {
		strcpy(result, limit_clause_fallback);
		strcat(result, " ");
		strcat(result, count);
		return result;
	}
}

const char now_odbc[] = "{fn CURRENT_TIMESTAMP()}";
const char now_mysql[] = "NOW()";
const char now_pgsql[] = "now()";
const char now_sqlite[] = "datetime('now', 'localtime')";
const char now_freetds[] = "CURRENT_TIMESTAMP";
const char now_access[] = "now()";
const char now_fallback[] = "NOW()";

const char currtime_odbc[] = "{fn CURTIME()}";
const char currtime_mysql[] = "CURTIME()";
const char currtime_pgsql[] = "localtime";
const char currtime_sqlite[] = "time('now', 'localtime')";
const char currtime_freetds[] = "CURRENT_TIME";
const char currtime_mssql[] = "CAST(CURRENT_TIMESTAMP AS time)";
const char currtime_fallback[] = "CURTIME()";

static const char *SMSDSQL_CurrentTime(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		return currtime_mysql;
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		return currtime_pgsql;
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		return currtime_sqlite;
	} else if (strcasecmp(driver_name, "mssql") == 0) {
		return currtime_mssql;
	} else if (strcasecmp(Config->driver, "oracle") == 0 || strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "sybase") == 0) {
		return currtime_freetds;
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		return currtime_odbc;
	} else {
		return currtime_fallback;
	}
}
static const char *SMSDSQL_Now(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		return now_mysql;
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		return now_pgsql;
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		return now_sqlite;
	} else if (strcasecmp(driver_name, "oracle") == 0 || strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0 || strcasecmp(driver_name, "sybase") == 0) {
		return now_freetds;
	} else if (strcasecmp(driver_name, "access") == 0) {
		return now_access;
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		return now_odbc;
	} else {
		return now_fallback;
	}
}

static GSM_Error SMSDSQL_Reconnect(GSM_SMSDConfig * Config)
{
	GSM_Error error = ERR_DB_TIMEOUT;
	int attempts;
	struct GSM_SMSDdbobj *db = Config->db;

	SMSD_Log(DEBUG_INFO, Config, "Reconnecting to the database!");
	for (attempts = 1; attempts <= Config->backend_retries; attempts++) {
		SMSD_Log(DEBUG_INFO, Config, "Reconnecting after %d seconds...", attempts * attempts);
		sleep(attempts * attempts);
		db->Free(Config);
		error = db->Connect(Config);
		if (error == ERR_NONE) {
			return ERR_NONE;
		}
	}
	return error;
}

static GSM_Error SMSDSQL_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * res)
{
	GSM_Error error = ERR_DB_TIMEOUT;
	int attempts;
	struct GSM_SMSDdbobj *db = Config->db;

	for (attempts = 1; attempts <= Config->backend_retries; attempts++) {
		SMSD_Log(DEBUG_SQL, Config, "Execute SQL: %s", query);
		error = db->Query(Config, query, res);
		if (error == ERR_NONE) {
			return ERR_NONE;
		}

		if (error != ERR_DB_TIMEOUT){
			SMSD_Log(DEBUG_INFO, Config, "SQL failure: %d", error);
			return error;
		}

		SMSD_Log(DEBUG_INFO, Config, "SQL failed (timeout): %s", query);
		/* We will try to reconnect */
		error = SMSDSQL_Reconnect(Config);
		if (error != ERR_NONE) {
			return ERR_DB_TIMEOUT;
		}
	}
	return error;
}

/*
 * generates a timestamp string suitable for inserting into a database, the timestamp
 * argument must be a valid POSIX calendar time.
 *
 * pgsql: https://www.postgresql.org/docs/9.1/datatype-datetime.html
 *   - "For timestamp with time zone, the internally stored value is always in UTC
 *      (Universal Coordinated Time, traditionally known as Greenwich Mean Time, GMT).
 *      An input value that has an explicit time zone specified is converted to UTC
 *      using the appropriate offset for that time zone.
 *
 *      If no time zone is stated in the input string, then it is assumed to be in the
 *      time zone indicated by the system's timezone parameter, and is converted to
 *      UTC using the offset for the timezone zone. When a timestamp with time zone
 *      value is output, it is always converted from UTC to the current timezone zone,
 *      and displayed as local time in that zone. To see the time in another time zone,
 *      either change timezone or use the AT TIME ZONE construct (see Section 9.9.3).
 *
 *      Conversions between timestamp without time zone and timestamp with time zone
 *      normally assume that the timestamp without time zone value should be taken or
 *      given as timezone local time."
 *
 * mysql: https://dev.mysql.com/doc/refman/8.0/en/datetime.html
 *  - "MySQL converts TIMESTAMP values from the current time zone to UTC for
 *     storage, and back from UTC to the current time zone for retrieval."
 *
 * oracle: https://docs.oracle.com/cd/B19306_01/server.102/b14225/ch4datetime.htm#i1006050
 *  - a TIMESTAMP literal without tz info is interpreted as local time zone.
 */
void SMSDSQL_Time2String(GSM_SMSDConfig * Config, time_t timestamp, char *static_buff, size_t size)
{
	const char *driver_name = SMSDSQL_SQLName(Config);
	struct tm *tm = localtime(&timestamp);

  if(timestamp == -2) {
    strcpy(static_buff, "0000-00-00 00:00:00");
  }
  else if (strcasecmp(driver_name, "oracle") == 0) {
    strftime(static_buff, size, "TIMESTAMP '%Y-%m-%d %H:%M:%S'", tm);
  }
  else if (strcasecmp(driver_name, "mssql") == 0) {
    strftime(static_buff, size, "%Y-%m-%dT%H:%M:%S", tm);
  }
  else if (strcasecmp(driver_name, "odbc") == 0) {
    strftime(static_buff, size, "{ ts '%Y-%m-%d %H:%M:%S' }", tm);
  }
  else if (strcasecmp(driver_name, "access") == 0) {
    strftime(static_buff, size, "'%Y-%m-%d %H:%M:%S'", tm);
  }
  else {
    strftime(static_buff, size, "%Y-%m-%d %H:%M:%S", tm);
  }
}

static void SMSDSQL_EncodeCanonicalNumber(
	char *destination,
	const unsigned char *number)
{
	EncodeUTF8(destination, number);
	if (destination[0] == '0' && destination[1] == '0') {
		destination[0] = '+';
		memmove(
			destination + 1,
			destination + 2,
			strlen(destination + 2) + 1);
	}
}

static GSM_Error SMSDSQL_NamedQuery(GSM_SMSDConfig * Config, const char *sql_query, GSM_SMSMessage *sms,
	GSM_MultiSMSMessage * smsmulti, const SQL_Var *params, SQL_result * res, gboolean retry)
{
	char buff[65536], *ptr, c, static_buff[8192];
	char *buffer2, *end;
	const char *to_print, *q = sql_query;
	int int_to_print;
	int numeric;
	int n, argc = 0, i;
	struct GSM_SMSDdbobj *db = Config->db;
	GSM_MultiPartSMSInfo SMSInfo;

	if (params != NULL) {
		while (params[argc].type != SQL_TYPE_NONE) argc++;
	}

	ptr = buff;

	do {
		if (*q != '%') {
			*ptr++ = *q;
			continue;
		}
		c = *(++q);
		if( c >= '0' && c <= '9'){
			n = strtoul(q, &end, 10) - 1;
			if (n < argc && n >= 0) {
				switch(params[n].type){
					case SQL_TYPE_INT:
						ptr += sprintf(ptr, "%lli", params[n].v.i);
						break;
					case SQL_TYPE_STRING:
						buffer2 = db->QuoteString(Config, params[n].v.s);
						memcpy(ptr, buffer2, strlen(buffer2));
						ptr += strlen(buffer2);
						free(buffer2);
						break;
					default:
						SMSD_Log(DEBUG_ERROR, Config, "SQL: unknown type: %i (application bug) in query: `%s`", params[n].type, sql_query);
						return ERR_BUG;
						break;
				}
			} else {
				SMSD_Log(DEBUG_ERROR, Config, "SQL: wrong number of parameter: %i (max %i) in query: `%s`", n+1, argc, sql_query);
				return ERR_BUG;
			}
			q = end - 1;
			continue;
		}
		numeric = 0;
		to_print = NULL;
		switch (c) {
			case 'I':
				to_print = Config->Status->IMEI;
				break;
			case 'S':
				to_print = Config->Status->IMSI;
				break;
			case 'P':
				to_print = Config->PhoneID;
				break;
			case 'O':
				to_print = Config->Status->NetInfo.NetworkCode;
				break;
			case 'M':
				to_print = DecodeUnicodeConsole(Config->Status->NetInfo.NetworkName);
				break;
			case 'N':
				snprintf(static_buff, sizeof(static_buff), "Gammu %s, %s, %s", GAMMU_VERSION, GetOS(), GetCompiler());
				to_print = static_buff;
				break;
			case 'A':
				to_print = Config->CreatorID;
				break;
			default:
				if (sms != NULL) {
					switch (c) {
						case 'R':
							/*
							 * Always store international numbers with + prefix
							 * to allow easy matching later.
							 */
							SMSDSQL_EncodeCanonicalNumber(static_buff, sms->Number);
							to_print = static_buff;
							break;
						case 'F':
							EncodeUTF8(static_buff, sms->SMSC.Number);
							to_print = static_buff;
							break;
						case 'u':
							if (sms->UDH.Type != UDH_NoUDH) {
								EncodeHexBin(static_buff, sms->UDH.Text, sms->UDH.Length);
								to_print = static_buff;
							}else{
								to_print = "";
							}
							break;
						case 'x':
							int_to_print =  sms->Class;
							numeric = 1;
							break;
						case 'c':
							to_print = GSM_SMSCodingToString(sms->Coding);
							break;
						case 't':
							int_to_print =  sms->MessageReference;
							numeric = 1;
							break;
						case 'E':
							switch (sms->Coding) {
								case SMS_Coding_Unicode_No_Compression:
								case SMS_Coding_Default_No_Compression:
									EncodeHexUnicode(static_buff, sms->Text, UnicodeLength(sms->Text));
									break;
								case SMS_Coding_8bit:
									EncodeHexBin(static_buff, sms->Text, sms->Length);
									break;
								default:
									*static_buff = '\0';
									break;
							}
							to_print = static_buff;
							break;
						case 'T':
							/*
							 * Print empty string on retry as the error is quite likely in
							 * corrupted text.
							 */
							if (retry) {
								to_print = "";
							} else {
								if (smsmulti != NULL) {
									if (!smsmulti->Processed && sms == &smsmulti->SMS[0]) {
										static_buff[0] = 0;
										if (GSM_DecodeMultiPartSMS(GSM_GetDebug(Config->gsm), &SMSInfo, smsmulti, TRUE)) {
											for (i = 0; i < SMSInfo.EntriesNum; i++) {
												switch (SMSInfo.Entries[i].ID) {
													case SMS_ConcatenatedTextLong:
													case SMS_ConcatenatedAutoTextLong:
													case SMS_ConcatenatedTextLong16bit:
													case SMS_ConcatenatedAutoTextLong16bit:
														EncodeUTF8(static_buff + strlen(static_buff), SMSInfo.Entries[i].Buffer);
														break;
													default:
														break;
												}
											}
										}
										GSM_FreeMultiPartSMSInfo(&SMSInfo);
										if (static_buff[0] != 0) {
											to_print = static_buff;
											smsmulti->Processed = TRUE;
										}
									} else if (smsmulti->Processed) {
										to_print = "";
									}
								}
								if (to_print == NULL) {
								       switch (sms->Coding) {
									       case SMS_Coding_Unicode_No_Compression:
									       case SMS_Coding_Default_No_Compression:
										       EncodeUTF8(static_buff, sms->Text);
										       to_print = static_buff;
										       break;
									       default:
										       to_print = "";
										       break;
								       }
								}
							}
							break;
						case 'V':
							if (sms->SMSC.Validity.Format == SMS_Validity_RelativeFormat) {
								int_to_print = sms->SMSC.Validity.Relative;
							} else {
								int_to_print =  -1;
							}
							numeric = 1;
							break;
						case 'C':
							SMSDSQL_Time2String(Config, Fill_Time_T(sms->SMSCTime), static_buff, sizeof(static_buff));
							to_print = static_buff;
							break;
						case 'd':
							SMSDSQL_Time2String(Config, Fill_Time_T(sms->DateTime), static_buff, sizeof(static_buff));
							to_print = static_buff;
							break;
						case 'e':
							int_to_print = sms->DeliveryStatus;
							numeric = 1;
							break;
						default:
							SMSD_Log(DEBUG_ERROR, Config, "SQL: uexpected char '%c' in query: %s", c, sql_query);
							return ERR_BUG;

					} /* end of switch */
				} else {
					SMSD_Log(DEBUG_ERROR, Config, "Syntax error in query.. uexpected char '%c' in query: %s", c, sql_query);
					return ERR_BUG;
				}
				break;
		} /* end of switch */
		if (numeric) {
			ptr += sprintf(ptr, "%i", int_to_print);
		} else if (to_print != NULL) {
			buffer2 = db->QuoteString(Config, to_print);
			memcpy(ptr, buffer2, strlen(buffer2));
			ptr += strlen(buffer2);
			free(buffer2);
		} else {
			memcpy(ptr, "NULL", 4);
			ptr += 4;
		}
	} while (*(++q) != '\0');
	*ptr = '\0';
	return SMSDSQL_Query(Config, buff, res);

}

static GSM_Error SMSDSQL_CheckTable(GSM_SMSDConfig * Config, const char *table)
{
	SQL_result res;
	char buffer[200];
	GSM_Error error;
	struct GSM_SMSDdbobj *db = Config->db;

	sprintf(
		buffer,
		"SELECT %s * FROM %s %s %s",
		SMSDSQL_TopClause(Config, "1"),
		table,
		SMSDSQL_RownumClause(Config, "1", FALSE),
		SMSDSQL_LimitClause(Config, "1")
	);
	error = SMSDSQL_Query(Config, buffer, &res);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Table %s not found, disconnecting!", table);
		db->Free(Config);
		return error;
	}
	db->FreeResult(Config, &res);
	return ERR_NONE;
}

/* Disconnects from a database */
static GSM_Error SMSDSQL_Free(GSM_SMSDConfig * Config)
{
	int i;
	SMSD_SQLInboxGroup *group, *next;

	SMSD_Log(DEBUG_SQL, Config, "Disconnecting from SQL database.");
	Config->db->Free(Config);
	group = Config->inbox_groups;
	while (group != NULL) {
		next = group->next;
		free(group);
		group = next;
	}
	Config->inbox_groups = NULL;
	/* free configuration */
	for(i = 0; i < SQL_QUERY_LAST_NO; i++){
		free(Config->SMSDSQL_queries[i]);
		Config->SMSDSQL_queries[i] = NULL;
	}
	return ERR_NONE;
}

/* Connects to database */
static GSM_Error SMSDSQL_Init(GSM_SMSDConfig * Config)
{
	SQL_result res;
	int version;
	GSM_Error error;
	struct GSM_SMSDdbobj *db;
	const char *escape_char;
	char buffer[100];

#ifdef WIN32
	_tzset();
#else
	tzset();
#endif

	db = Config->db;

	error = db->Connect(Config);
	if (error != ERR_NONE)
		return error;

	error = SMSDSQL_CheckTable(Config, Config->table_gammu);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to open table %s", Config->table_gammu);
		return error;
	}

	error = SMSDSQL_CheckTable(Config, Config->table_inbox);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to open table %s", Config->table_inbox);
		return error;
	}

	error = SMSDSQL_CheckTable(Config, Config->table_sentitems);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to open table %s", Config->table_sentitems);
		return error;
	}

	error = SMSDSQL_CheckTable(Config, Config->table_outbox);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to open table %s", Config->table_outbox);
		return error;
	}

	error = SMSDSQL_CheckTable(Config, Config->table_outbox_multipart);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to open table %s", Config->table_outbox_multipart);
		return error;
	}

	error = SMSDSQL_CheckTable(Config, Config->table_phones);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to open table %s", Config->table_phones);
		return error;
	}

	escape_char = SMSDSQL_EscapeChar(Config);

	sprintf(buffer, "SELECT %sVersion%s FROM %s", escape_char, escape_char, Config->table_gammu);
	error = SMSDSQL_Query(Config, buffer, &res);
	if (error != ERR_NONE) {
		db->Free(Config);
		return error;
	}
	if (db->NextRow(Config, &res) != 1) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to seek to first row!");
		db->FreeResult(Config, &res);
		db->Free(Config);
		return ERR_UNKNOWN;
	}
	version = (int)db->GetNumber(Config, &res, 0);
	db->FreeResult(Config, &res);
	error = SMSD_CheckDBVersion(Config, version);
	if (error != ERR_NONE) {
		db->Free(Config);
		return error;
	}

	SMSD_Log(DEBUG_INFO, Config, "Connected to Database %s: %s on %s", Config->driver, Config->database, Config->host);

	return ERR_NONE;
}

static GSM_Error SMSDSQL_InitAfterConnect(GSM_SMSDConfig * Config)
{
	SQL_result res;
	GSM_Error error;
	struct GSM_SMSDdbobj *db = Config->db;
	SQL_Var vars[3] = {{SQL_TYPE_STRING, {NULL}}, {SQL_TYPE_STRING, {NULL}}, {SQL_TYPE_NONE, {NULL}}};

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_DELETE_PHONE], NULL, NULL, NULL, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	SMSD_Log(DEBUG_INFO, Config, "Inserting phone info");
	vars[0].v.s = Config->enable_send ? "yes" : "no";
	vars[1].v.s = Config->enable_receive ? "yes" : "no";

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_INSERT_PHONE], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error inserting into database (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	return SMSDSQL_RestoreInboxGroups(Config);
}

static gboolean SMSDSQL_IsGroupedInboxPart(const GSM_SMSMessage *sms)
{
	gboolean supported;

	switch (sms->UDH.Type) {
		case UDH_ConcatenatedMessages:
		case UDH_ConcatenatedMessages16bit:
		case UDH_NokiaRingtoneLong:
		case UDH_NokiaOperatorLogoLong:
		case UDH_NokiaWAPLong:
		case UDH_NokiaCalendarLong:
		case UDH_NokiaProfileLong:
		case UDH_NokiaPhonebookLong:
		case UDH_MMSIndicatorLong:
		case UDH_UserUDH:
			supported = TRUE;
			break;
		default:
			supported = FALSE;
			break;
	}

	return (
		supported &&
		sms->UDH.PartNumber > 0 &&
		sms->UDH.AllParts > 1 &&
		sms->UDH.PartNumber <= sms->UDH.AllParts &&
		sms->UDH.AllParts <= GSM_MAX_MULTI_SMS);
}

static gboolean SMSDSQL_DecodeUserUDHConcat(GSM_UDHHeader *udh)
{
	size_t offset, length, ie_length;
	int id8 = -1, id16 = -1, part_number = -1, all_parts = -1;
	gboolean found = FALSE;

	if (udh->Type != UDH_UserUDH ||
	    udh->Length <= 1 ||
	    udh->Length > GSM_MAX_UDH_LENGTH ||
	    udh->Text[0] + 1 != udh->Length) {
		return FALSE;
	}

	length = udh->Length;
	offset = 1;
	while (offset < length) {
		if (length - offset < 2) {
			return FALSE;
		}
		ie_length = udh->Text[offset + 1];
		if (ie_length > length - offset - 2) {
			return FALSE;
		}

		if (udh->Text[offset] == 0x00) {
			if (ie_length != 3 || found) {
				return FALSE;
			}
			id8 = udh->Text[offset + 2];
			all_parts = udh->Text[offset + 3];
			part_number = udh->Text[offset + 4];
			found = TRUE;
		} else if (udh->Text[offset] == 0x08) {
			if (ie_length != 4 || found) {
				return FALSE;
			}
			id16 = udh->Text[offset + 2] * 256 +
				udh->Text[offset + 3];
			all_parts = udh->Text[offset + 4];
			part_number = udh->Text[offset + 5];
			found = TRUE;
		}
		offset += ie_length + 2;
	}

	if (!found) {
		return FALSE;
	}
	udh->ID8bit = id8;
	udh->ID16bit = id16;
	udh->PartNumber = part_number;
	udh->AllParts = all_parts;
	return TRUE;
}

static int SMSDSQL_InboxReference(const GSM_SMSMessage *sms)
{
	if (sms->UDH.Type == UDH_ConcatenatedMessages16bit ||
	    (sms->UDH.Type == UDH_UserUDH && sms->UDH.ID16bit >= 0)) {
		return sms->UDH.ID16bit;
	}
	return sms->UDH.ID8bit;
}

static gboolean SMSDSQL_InboxReferenceIs16bit(const GSM_SMSMessage *sms)
{
	return sms->UDH.Type == UDH_ConcatenatedMessages16bit ||
		(sms->UDH.Type == UDH_UserUDH && sms->UDH.ID16bit >= 0);
}

static gboolean SMSDSQL_UnicodeEqual(const unsigned char *first, const unsigned char *second)
{
	size_t first_length = UnicodeLength(first);
	size_t second_length = UnicodeLength(second);

	return first_length == second_length &&
		memcmp(first, second, first_length * 2) == 0;
}

static gboolean SMSDSQL_NumberEqual(
	const unsigned char *first,
	const unsigned char *second)
{
	char first_number[GSM_MAX_NUMBER_LENGTH * 3 + 1];
	char second_number[GSM_MAX_NUMBER_LENGTH * 3 + 1];

	SMSDSQL_EncodeCanonicalNumber(first_number, first);
	SMSDSQL_EncodeCanonicalNumber(second_number, second);
	return strcmp(first_number, second_number) == 0;
}

static gboolean SMSDSQL_InboxGroupMatches(
	const SMSD_SQLInboxGroup *group,
	const GSM_SMSMessage *sms)
{
	return (
		group->type == sms->UDH.Type &&
		group->reference == SMSDSQL_InboxReference(sms) &&
		group->reference_16bit == SMSDSQL_InboxReferenceIs16bit(sms) &&
		group->part_count == sms->UDH.AllParts &&
		SMSDSQL_NumberEqual(group->sender, sms->Number) &&
		SMSDSQL_UnicodeEqual(group->smsc, sms->SMSC.Number));
}

static void SMSDSQL_ExpireInboxGroups(GSM_SMSDConfig *Config, time_t now)
{
	SMSD_SQLInboxGroup *group, **link;

	link = &Config->inbox_groups;
	while ((group = *link) != NULL) {
		if (difftime(now, group->created) >= Config->multiparttimeout) {
			*link = group->next;
			free(group);
			continue;
		}
		link = &group->next;
	}
}

static SMSD_SQLInboxGroup *SMSDSQL_FindInboxGroup(
	GSM_SMSDConfig *Config,
	const GSM_SMSMessage *sms)
{
	SMSD_SQLInboxGroup *group;

	if (!SMSDSQL_IsGroupedInboxPart(sms) || SMSDSQL_InboxReference(sms) < 0) {
		return NULL;
	}

	for (group = Config->inbox_groups; group != NULL; group = group->next) {
		if (SMSDSQL_InboxGroupMatches(group, sms)) {
			return group;
		}
	}
	return NULL;
}

static SMSD_SQLInboxGroup *SMSDSQL_FindInboxGroupByMessageID(
	GSM_SMSDConfig *Config,
	unsigned long long message_id)
{
	SMSD_SQLInboxGroup *group;

	for (group = Config->inbox_groups; group != NULL; group = group->next) {
		if (group->message_id == message_id) {
			return group;
		}
	}
	return NULL;
}

static SMSD_SQLInboxGroup *SMSDSQL_NewInboxGroup(
	GSM_SMSDConfig *Config,
	const GSM_SMSMessage *sms,
	unsigned long long message_id,
	time_t created)
{
	SMSD_SQLInboxGroup *group;
	size_t length;

	group = calloc(1, sizeof(*group));
	if (group == NULL) {
		return NULL;
	}

	group->type = sms->UDH.Type;
	group->reference = SMSDSQL_InboxReference(sms);
	group->reference_16bit = SMSDSQL_InboxReferenceIs16bit(sms);
	group->part_count = sms->UDH.AllParts;
	group->message_id = message_id;
	group->created = created;

	length = UnicodeLength(sms->Number);
	memcpy(group->sender, sms->Number, (length + 1) * 2);
	length = UnicodeLength(sms->SMSC.Number);
	memcpy(group->smsc, sms->SMSC.Number, (length + 1) * 2);

	group->next = Config->inbox_groups;
	Config->inbox_groups = group;
	return group;
}

static void SMSDSQL_RemoveInboxGroup(
	GSM_SMSDConfig *Config,
	SMSD_SQLInboxGroup *group)
{
	SMSD_SQLInboxGroup **link = &Config->inbox_groups;

	while (*link != NULL) {
		if (*link == group) {
			*link = group->next;
			free(group);
			return;
		}
		link = &(*link)->next;
	}
}

static gboolean SMSDSQL_InboxGroupIsComplete(
	const SMSD_SQLInboxGroup *group)
{
	int i;

	for (i = 0; i < group->part_count; i++) {
		if (!group->received_parts[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

static void SMSDSQL_RecordInboxPart(
	GSM_SMSDConfig *Config,
	SMSD_SQLInboxGroup *group,
	int sequence_position)
{
	group->received_parts[sequence_position - 1] = TRUE;
	if (SMSDSQL_InboxGroupIsComplete(group)) {
		SMSDSQL_RemoveInboxGroup(Config, group);
	}
}

static void SMSDSQL_RemoveCompleteInboxGroups(GSM_SMSDConfig *Config)
{
	SMSD_SQLInboxGroup *group, **link;

	link = &Config->inbox_groups;
	while ((group = *link) != NULL) {
		if (SMSDSQL_InboxGroupIsComplete(group)) {
			*link = group->next;
			free(group);
			continue;
		}
		link = &group->next;
	}
}

GSM_Error SMSDSQL_RestoreInboxGroups(GSM_SMSDConfig *Config)
{
	SQL_result res;
	GSM_SMSMessage sms;
	SMSD_SQLInboxGroup *group;
	GSM_Error error;
	struct GSM_SMSDdbobj *db = Config->db;
	const char *sender, *smsc, *udh;
	unsigned long long message_id;
	long long stored_message_id;
	long long sequence_position, part_count, age;
	time_t created;
	time_t restore_time = time(NULL);
	size_t sender_length, smsc_length, udh_length;
	gboolean valid;

	error = SMSDSQL_NamedQuery(
		Config,
		Config->SMSDSQL_queries[SQL_QUERY_RESTORE_INBOX_GROUPS],
		NULL, NULL, NULL, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error restoring multipart inbox groups (%s)", __FUNCTION__);
		return error;
	}

	while (db->NextRow(Config, &res)) {
		GSM_SetDefaultSMSData(&sms);
		valid = TRUE;

		stored_message_id = db->GetNumber(Config, &res, 0);
		sender = db->GetString(Config, &res, 1);
		if (sender == NULL) {
			sender = "";
		}
		sender_length = strlen(sender);
		if (sender_length > GSM_MAX_NUMBER_LENGTH) {
			valid = FALSE;
		} else {
			EncodeUnicode(sms.Number, sender, sender_length);
		}
		smsc = db->GetString(Config, &res, 2);
		if (smsc == NULL) {
			smsc = "";
		}
		smsc_length = strlen(smsc);
		if (smsc_length > GSM_MAX_NUMBER_LENGTH) {
			valid = FALSE;
		} else {
			EncodeUnicode(sms.SMSC.Number, smsc, smsc_length);
		}
		udh = db->GetString(Config, &res, 3);
		sequence_position = db->GetNumber(Config, &res, 4);
		part_count = db->GetNumber(Config, &res, 5);
		age = db->GetNumber(Config, &res, 6);

		if (!valid || stored_message_id <= 0 || udh == NULL) {
			continue;
		}
		if (age < 0) {
			age = 0;
		}
		if ((unsigned long long)age > (unsigned long long)restore_time) {
			created = 0;
		} else {
			created = restore_time - (time_t)age;
		}
		message_id = (unsigned long long)stored_message_id;
		udh_length = strlen(udh);
		if (udh_length == 0 || udh_length % 2 != 0 ||
		    udh_length >= 2 * GSM_MAX_UDH_LENGTH ||
		    !DecodeHexBin(sms.UDH.Text, (const unsigned char *)udh, udh_length)) {
			continue;
		}
		sms.UDH.Length = udh_length / 2;
		GSM_DecodeUDHHeader(NULL, &sms.UDH);
		SMSDSQL_DecodeUserUDHConcat(&sms.UDH);
		if (!SMSDSQL_IsGroupedInboxPart(&sms) ||
		    sms.UDH.PartNumber != sequence_position ||
		    sms.UDH.AllParts != part_count ||
		    SMSDSQL_InboxReference(&sms) < 0) {
			continue;
		}

		group = SMSDSQL_FindInboxGroupByMessageID(Config, message_id);
		if (group == NULL) {
			group = SMSDSQL_NewInboxGroup(
				Config, &sms, message_id, created);
			if (group == NULL) {
				db->FreeResult(Config, &res);
				return ERR_MOREMEMORY;
			}
		} else if (!SMSDSQL_InboxGroupMatches(group, &sms)) {
			continue;
		} else if (created < group->created) {
			group->created = created;
		}
		group->received_parts[sequence_position - 1] = TRUE;
	}
	db->FreeResult(Config, &res);
	SMSDSQL_RemoveCompleteInboxGroups(Config);
	SMSDSQL_ExpireInboxGroups(Config, time(NULL));
	return ERR_NONE;
}

static const char *SMSDSQL_DeliveryStatus(unsigned char delivery_status)
{
	if (delivery_status < 0x03) {
		return "DeliveryOK";
	}
	if (delivery_status & 0x40) {
		return "DeliveryFailed";
	}
	if (delivery_status & 0x20) {
		return "DeliveryPending";
	}
	return "DeliveryUnknown";
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDSQL_SaveInboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, GSM_StringArray *Locations, GSM_StringArray *SentIDs)
{
	SQL_result res, res2;
	SQL_Var vars[6];
	GSM_Error error;
	struct GSM_SMSDdbobj *db = Config->db;
	const char *q, *status;
	SMSD_SQLInboxGroup *inbox_group;
	gboolean new_inbox_group;

	char smstext[3 * GSM_MAX_SMS_LENGTH + 1];
	char destinationnumber[3 * GSM_MAX_NUMBER_LENGTH + 1];
	char smsc_message[3 * GSM_MAX_NUMBER_LENGTH + 1];
	int i;
	time_t t_time1, t_time2;
	gboolean found;
	long diff;
	unsigned long long message_id = 0, new_id, sent_id;
	const char *state, *smsc;
	char location[50];
	char sent_id_string[50];
	int deliver_count = 0, part_count, sequence_position, stored_part = 0;

	sms->Processed = FALSE;
	SMSDSQL_ExpireInboxGroups(Config, time(NULL));
	for (i = 0; i < sms->Number; i++) {
		if (sms->SMS[i].PDU == SMS_Deliver) {
			deliver_count++;
		}
	}

	for (i = 0; i < sms->Number; i++) {
		sent_id_string[0] = 0;
		EncodeUTF8(destinationnumber, sms->SMS[i].Number);
		EncodeUTF8(smsc_message, sms->SMS[i].SMSC.Number);
		if (sms->SMS[i].PDU == SMS_Status_Report) {
			EncodeUTF8(smstext, sms->SMS[i].Text);
			SMSD_Log(DEBUG_INFO, Config, "Delivery report: %s to %s", smstext, destinationnumber);

			error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_SELECT], &sms->SMS[i], sms, NULL, &res, FALSE);
			if (error != ERR_NONE) {
				SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
				return error;
			}

			found = FALSE;
			while (db->NextRow(Config, &res)) {
				sent_id = db->GetNumber(Config, &res, 0);
				state = db->GetString(Config, &res, 1);
				t_time1 = db->GetDate(Config, &res, 2);
				smsc = db->GetString(Config, &res, 4);
				SMSD_Log(DEBUG_NOTICE, Config, "Checking for delivery report, SMSC=%s, state=%s", smsc, state);

				if (strcmp(smsc, smsc_message) != 0) {
					if (Config->skipsmscnumber[0] == 0 || strcmp(Config->skipsmscnumber, smsc)) {
						SMSD_Log(DEBUG_ERROR, Config, "Failed to match SMSC, you might want to use SkipSMSCNumber (sent: %s, received: %s)", smsc_message, smsc);
						continue;
					}
				}

				if (strcmp(state, "SendingOK") == 0 || strcmp(state, "DeliveryPending") == 0) {
					if (t_time1 < 0) {
						SMSD_Log(DEBUG_ERROR, Config, "Invalid SendingDateTime -1 for SMS TPMR=%i", sms->SMS[i].MessageReference);
						return ERR_UNKNOWN;
					}
					t_time2 = Fill_Time_T(sms->SMS[i].DateTime);
					diff = t_time2 - t_time1;

					if (diff > -Config->deliveryreportdelay && diff < Config->deliveryreportdelay) {
						found = TRUE;
						break;
					} else {
						SMSD_Log(DEBUG_NOTICE, Config,
							 "Delivery report would match, but time delta is too big (%ld), consider increasing DeliveryReportDelay", diff);
					}
				}
			}

			if (found) {
				status = SMSDSQL_DeliveryStatus(sms->SMS[i].DeliveryStatus);
				if (!strcmp(status, "DeliveryOK")) {
					q = Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED];
				} else {
					q = Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE];
				}

				vars[0].type = SQL_TYPE_STRING;
				vars[0].v.s = status;			/* Status */
				vars[1].type = SQL_TYPE_INT;
				vars[1].v.i = (long)sent_id; /* ID */
				vars[2].type = SQL_TYPE_NONE;

				error = SMSDSQL_NamedQuery(Config, q, &sms->SMS[i], sms, vars, &res2, FALSE);
				if (error != ERR_NONE) {
					SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
					return error;
				}
				db->FreeResult(Config, &res2);
				snprintf(sent_id_string, sizeof(sent_id_string), "%llu", sent_id);
			} else {
				SMSD_Log(DEBUG_ERROR, Config, "Failed to find SMS for TPMR=%i, Number=%s", sms->SMS[i].MessageReference, destinationnumber);
			}
			db->FreeResult(Config, &res);
			if (SentIDs != NULL && !GSM_StringArray_Add(SentIDs, sent_id_string)) {
				return ERR_MOREMEMORY;
			}
			continue;
		}

		if (sms->SMS[i].PDU != SMS_Deliver) {
			if (SentIDs != NULL && !GSM_StringArray_Add(SentIDs, "")) {
				return ERR_MOREMEMORY;
			}
			continue;
		}
		stored_part++;

		SMSDSQL_DecodeUserUDHConcat(&sms->SMS[i].UDH);
		if (sms->SMS[i].UDH.PartNumber > 0) {
			sequence_position = sms->SMS[i].UDH.PartNumber;
		} else {
			sequence_position = stored_part;
		}
		if (sms->SMS[i].UDH.AllParts > 0) {
			part_count = sms->SMS[i].UDH.AllParts;
		} else {
			part_count = deliver_count;
		}

		inbox_group = SMSDSQL_FindInboxGroup(Config, &sms->SMS[i]);
		new_inbox_group = FALSE;
		if (inbox_group != NULL) {
			message_id = inbox_group->message_id;
		} else if (SMSDSQL_IsGroupedInboxPart(&sms->SMS[i]) &&
			   SMSDSQL_InboxReference(&sms->SMS[i]) >= 0 &&
			   !Config->ProcessingIncompleteMessage) {
			inbox_group = SMSDSQL_NewInboxGroup(
				Config, &sms->SMS[i], message_id, time(NULL));
			if (inbox_group == NULL) {
				return ERR_MOREMEMORY;
			}
			new_inbox_group = TRUE;
		}

		vars[0].type = SQL_TYPE_INT;
		vars[0].v.i = (long long)message_id;       /* MessageID */
		vars[1].type = SQL_TYPE_INT;
		vars[1].v.i = sequence_position;           /* SequencePosition */
		vars[2].type = SQL_TYPE_INT;
		vars[2].v.i = part_count;                  /* PartCount */
		vars[3].type = SQL_TYPE_STRING;
		vars[3].v.s = "true";                      /* Processed */
		vars[4].type = SQL_TYPE_NONE;

		error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_INSERT], &sms->SMS[i], sms, vars, &res, FALSE);
		if (error != ERR_NONE) {
			if (error != ERR_DB_TIMEOUT) {
				error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_INSERT], &sms->SMS[i], sms, vars, &res, TRUE);
			}
			if (error != ERR_NONE) {
				if (new_inbox_group) {
					SMSDSQL_RemoveInboxGroup(Config, inbox_group);
				}
				SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
				return error;
			}
		}

		new_id = db->SeqID(Config, "inbox_ID_seq");
		if (new_id == 0) {
			if (new_inbox_group) {
				SMSDSQL_RemoveInboxGroup(Config, inbox_group);
			}
			SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		SMSD_Log(DEBUG_NOTICE, Config, "Inserted message id %llu", new_id);

		db->FreeResult(Config, &res);

		if (message_id == 0) {
			message_id = new_id;
		}
		if (new_inbox_group) {
			inbox_group->message_id = message_id;
		}

		vars[0].type = SQL_TYPE_INT;
		vars[0].v.i = (long long)message_id;       /* MessageID */
		vars[1].type = SQL_TYPE_INT;
		vars[1].v.i = sequence_position;           /* SequencePosition */
		vars[2].type = SQL_TYPE_INT;
		vars[2].v.i = part_count;                  /* PartCount */
		vars[3].type = SQL_TYPE_INT;
		vars[3].v.i = (long long)new_id;           /* ID */
		vars[4].type = SQL_TYPE_STRING;
		vars[4].v.s = "false";                     /* Processed */
		vars[5].type = SQL_TYPE_NONE;

		error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE_METADATA], &sms->SMS[i], sms, vars, &res2, FALSE);
		if (error != ERR_NONE) {
			if (new_inbox_group) {
				SMSDSQL_RemoveInboxGroup(Config, inbox_group);
			}
			SMSD_Log(DEBUG_INFO, Config, "Error updating received message metadata (%s)", __FUNCTION__);
			return error;
		}
		db->FreeResult(Config, &res2);

		snprintf(location, sizeof(location), "%llu", new_id);
		if (!GSM_StringArray_Add(Locations, location)) {
			return ERR_MOREMEMORY;
		}

		error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_UPDATE_RECEIVED], &sms->SMS[i], sms, NULL, &res2, FALSE);
		if (error != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error updating number of received messages (%s)", __FUNCTION__);
			return error;
		}
		db->FreeResult(Config, &res2);

		if (SentIDs != NULL && !GSM_StringArray_Add(SentIDs, "")) {
			return ERR_MOREMEMORY;
		}

		if (inbox_group != NULL) {
			SMSDSQL_RecordInboxPart(
				Config, inbox_group, sequence_position);
		}
	}

	return ERR_NONE;
}

static GSM_Error SMSDSQL_RefreshSendStatus(GSM_SMSDConfig * Config, char *ID)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	GSM_Error error;
	SQL_Var vars[2] = {
		{SQL_TYPE_STRING, {ID}},
		{SQL_TYPE_NONE, {NULL}}};

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_REFRESH_SEND_STATUS], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return error;
	}

	if (db->AffectedRows(Config, &res) == 0) {
		db->FreeResult(Config, &res);
		return ERR_UNKNOWN;
	}

	db->FreeResult(Config, &res);
	return ERR_NONE;
}



static GSM_Error SMSDSQL_UpdateRetries(GSM_SMSDConfig * Config, char *ID)
{
	SQL_result res;
	GSM_Error error;
	size_t query_type;
	struct GSM_SMSDdbobj *db = Config->db;
	/* 1 = ID, 2 = Retries, 3 = StatusCode, 4 = SequencePosition */
	SQL_Var vars[5] = {
		{SQL_TYPE_STRING, {ID}},
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_NONE, {NULL}}};
	vars[1].v.i = Config->retries;
	vars[2].v.i = Config->StatusCode;
	vars[3].v.i = Config->Part;

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_UPDATE_RETRIES], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return error;
	}

	if (db->AffectedRows(Config, &res) == 0) {
		db->FreeResult(Config, &res);
		return ERR_UNKNOWN;
	}

	db->FreeResult(Config, &res);

	if (Config->StatusCode != -1) {
		query_type = (Config->Part == 1) ? SQL_QUERY_UPDATE_OUTBOX_STATUSCODE : SQL_QUERY_UPDATE_OUTBOX_MULTIPART_STATUSCODE;
		error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[query_type], NULL, NULL, vars, &res, FALSE);
		if (error != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error updating StatusCode (%s)", __FUNCTION__);
			return error;
		}
		db->FreeResult(Config, &res);
	}

	return ERR_NONE;
}

static gboolean SMSDSQL_StringEquals(const char *actual, const char *expected)
{
	return actual != NULL && strcmp(actual, expected) == 0;
}

static gboolean SMSDSQL_StatusWasSent(const char *status)
{
	if (status == NULL) {
		return FALSE;
	}

	return strcmp(status, "SendingOK") == 0 ||
		strcmp(status, "SendingOKNoReport") == 0 ||
		strcmp(status, "DeliveryOK") == 0 ||
		strcmp(status, "DeliveryFailed") == 0 ||
		strcmp(status, "DeliveryPending") == 0 ||
		strcmp(status, "DeliveryUnknown") == 0;
}

static gboolean SMSDSQL_StatusWasSendingError(const char *status)
{
	return status != NULL && strcmp(status, "SendingError") == 0;
}

/*
 * Checks whether a sentitems row belongs to the same outbox message.
 *
 * A matching successful row means the modem send and sentitems insert
 * completed previously, but removing the outbox row did not. A matching
 * SendingError row means max-retry handling recorded the failure, but did not
 * remove the outbox row. A different row with the same key means the outbox ID
 * was reused. All cases must be handled before talking to the modem.
 */
static GSM_Error SMSDSQL_ReconcileSentItem(
	GSM_SMSMessage *sms,
	GSM_SMSDConfig *Config,
	char *ID,
	int Part,
	time_t InsertIntoDB,
	gboolean *SkipSend)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	GSM_Error error;
	SQL_Var vars[3] = {
		{SQL_TYPE_STRING, {ID}},
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_NONE, {NULL}}};
	char expected_text[(GSM_MAX_SMS_LENGTH + 1) * 4 + 1];
	char expected_decoded[(GSM_MAX_SMS_LENGTH + 1) * 4 + 1];
	char expected_udh[GSM_MAX_UDH_LENGTH * 2 + 1];
	char expected_destination[GSM_MAX_NUMBER_LENGTH * 3 + 1];
	const char *status;
	long long sent_validity;
	gboolean matches = TRUE;
	gboolean was_sent;

	*SkipSend = FALSE;
	vars[1].v.i = Part;

	error = SMSDSQL_NamedQuery(
		Config,
		Config->SMSDSQL_queries[SQL_QUERY_FIND_SENT_ITEM],
		NULL,
		NULL,
		vars,
		&res,
		FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Error checking sent message state (%s)", __FUNCTION__);
		return ERR_BUSY;
	}
	if (db->NextRow(Config, &res) != 1) {
		db->FreeResult(Config, &res);
		return ERR_EMPTY;
	}

	expected_text[0] = 0;
	expected_decoded[0] = 0;
	switch (sms->Coding) {
		case SMS_Coding_Unicode_No_Compression:
		case SMS_Coding_Default_No_Compression:
			EncodeHexUnicode(expected_text, sms->Text, UnicodeLength(sms->Text));
			EncodeUTF8(expected_decoded, sms->Text);
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(expected_text, sms->Text, sms->Length);
			break;
		default:
			break;
	}

	if (sms->UDH.Type != UDH_NoUDH) {
		EncodeHexBin(expected_udh, sms->UDH.Text, sms->UDH.Length);
	} else {
		expected_udh[0] = 0;
	}

	SMSDSQL_EncodeCanonicalNumber(expected_destination, sms->Number);

	if (!SMSDSQL_StringEquals(db->GetString(Config, &res, 0), expected_text)) {
		matches = FALSE;
	}
	if (!SMSDSQL_StringEquals(db->GetString(Config, &res, 1), GSM_SMSCodingToString(sms->Coding))) {
		matches = FALSE;
	}
	if (!SMSDSQL_StringEquals(db->GetString(Config, &res, 2), expected_udh)) {
		matches = FALSE;
	}
	if (db->GetNumber(Config, &res, 3) != sms->Class) {
		matches = FALSE;
	}
	if (!SMSDSQL_StringEquals(db->GetString(Config, &res, 4), expected_decoded)) {
		matches = FALSE;
	}
	if (!SMSDSQL_StringEquals(db->GetString(Config, &res, 5), expected_destination)) {
		matches = FALSE;
	}
	if (db->GetDate(Config, &res, 6) != InsertIntoDB) {
		matches = FALSE;
	}
	sent_validity = db->GetNumber(Config, &res, 7);
	/*
	 * -1 means the SMSC validity is inherited. In that case add_sent_info
	 * stores the effective value obtained from the configured or phone SMSC,
	 * which is not available while reconciling the outbox row.
	 */
	if (Config->relativevalidity != -1 && sent_validity != Config->relativevalidity) {
		matches = FALSE;
	}
	if (!SMSDSQL_StringEquals(db->GetString(Config, &res, 8), Config->CreatorID)) {
		matches = FALSE;
	}
	status = db->GetString(Config, &res, 9);
	was_sent = SMSDSQL_StatusWasSent(status);

	if (!matches) {
		SMSD_Log(
			DEBUG_ERROR,
			Config,
			"Refusing to send %s:%d: sentitems contains different message data for this key",
			ID,
			Part);
		error = ERR_FILEALREADYEXIST;
	} else if (SMSDSQL_StatusWasSendingError(status)) {
		SMSD_Log(
			DEBUG_NOTICE,
			Config,
			"Found matching sentitems failure for %s:%d, completing interrupted error cleanup",
			ID,
			Part);
		*SkipSend = TRUE;
		error = ERR_NONE;
	} else if (!was_sent) {
		SMSD_Log(
			DEBUG_ERROR,
			Config,
			"Refusing to send %s:%d: sentitems already contains non-success status %s",
			ID,
			Part,
			status == NULL ? "(null)" : status);
		error = ERR_FILEALREADYEXIST;
	} else {
		SMSD_Log(
			DEBUG_NOTICE,
			Config,
			"Found matching sentitems row for %s:%d with status %s, skipping duplicate transmission",
			ID,
			Part,
			status);
		*SkipSend = TRUE;
		error = ERR_NONE;
	}

	db->FreeResult(Config, &res);
	return error;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDSQL_FindOutboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *ID)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	int i;
	gboolean last;
	time_t timestamp;
	const char *coding;
	const char *text;
	size_t text_len;
	const char *text_decoded;
	const char *destination;
	const char *udh;
	const char *q;
	const char *status;
	size_t udh_len;
	gboolean skip_send;
	SQL_Var vars[3];
	GSM_Error error;

	vars[0].type = SQL_TYPE_INT;
	vars[0].v.i = 1;
	vars[1].type = SQL_TYPE_INT;
	vars[1].v.i = SMSDSQL_CurrentDayMask();
	vars[2].type = SQL_TYPE_NONE;

	while (TRUE) {
		error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_SMS_ID], NULL, NULL, vars, &res, FALSE);
		if (error != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
			return error;
		}

		if (db->NextRow(Config, &res) != 1) {
			db->FreeResult(Config, &res);
			return ERR_EMPTY;
		}

		sprintf(ID, "%ld", (long)db->GetNumber(Config, &res, 0));
		timestamp = db->GetDate(Config, &res, 1);

		db->FreeResult(Config, &res);

		if (timestamp == -1) {
			SMSD_Log(DEBUG_INFO, Config, "Invalid date for InsertIntoDB.");
			return ERR_UNKNOWN;
		}

		SMSDSQL_Time2String(Config, timestamp, Config->DT, sizeof(Config->DT));
		if (SMSDSQL_RefreshSendStatus(Config, ID) == ERR_NONE) {
			break;
		}
	}

	sms->Number = 0;
	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		GSM_SetDefaultSMSData(&sms->SMS[i]);
		/* Force using default SMSC */
		sms->SMS[i].SMSC.Location = 0;
	}

	for (i = 1; i < GSM_MAX_MULTI_SMS + 1; i++) {
		vars[0].type = SQL_TYPE_STRING;
		vars[0].v.s = ID;
		vars[1].type = SQL_TYPE_INT;
		vars[1].v.i = i;
		vars[2].type = SQL_TYPE_NONE;
		if (i == 1) {
			q = Config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_BODY];
		} else {
			q = Config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_MULTIPART];
		}
		error = SMSDSQL_NamedQuery(Config, q, NULL, NULL, vars, &res, FALSE);
		if (error != ERR_NONE) {
			SMSD_Log(DEBUG_ERROR, Config, "Error reading from database (%s)", __FUNCTION__);
			return error;
		}

		if (db->NextRow(Config, &res) != 1) {
			db->FreeResult(Config, &res);
			return ERR_NONE;
		}

		text = db->GetString(Config, &res, 0);
		coding = db->GetString(Config, &res, 1);
		if (text == NULL) {
			text_len = 0;
		} else {
			text_len = strlen(text);
		}
		udh = db->GetString(Config, &res, 2);
		sms->SMS[sms->Number].Class = (int)db->GetNumber(Config, &res, 3);
		text_decoded = db->GetString(Config, &res, 4);
		if (udh == NULL) {
			udh_len = 0;
		} else {
			udh_len = strlen(udh);
		}

		/* ID, we don't need it, but some ODBC backend need to fetch all values */
		db->GetNumber(Config, &res, 5);

		sms->SMS[sms->Number].Coding = GSM_StringToSMSCoding(coding);
		if (sms->SMS[sms->Number].Coding == 0) {
			if (text == NULL || text_len == 0) {
				SMSD_Log(DEBUG_NOTICE, Config, "Assuming default coding for text message");
				sms->SMS[sms->Number].Coding = SMS_Coding_Default_No_Compression;
			} else {
				SMSD_Log(DEBUG_NOTICE, Config, "Assuming 8bit coding for binary message");
				sms->SMS[sms->Number].Coding = SMS_Coding_8bit;
			}
		}

		if (text == NULL || text_len == 0) {
			if (text_decoded == NULL) {
				SMSD_Log(DEBUG_ERROR, Config, "Message without text!");
				return ERR_UNKNOWN;
			} else {
				SMSD_Log(DEBUG_NOTICE, Config, "Message: %s", text_decoded);
				DecodeUTF8(sms->SMS[sms->Number].Text, text_decoded, strlen(text_decoded));
			}
		} else {
			switch (sms->SMS[sms->Number].Coding) {
				case SMS_Coding_Unicode_No_Compression:

				case SMS_Coding_Default_No_Compression:
					if (! DecodeHexUnicode(sms->SMS[sms->Number].Text, text, text_len)) {
						SMSD_Log(DEBUG_ERROR, Config, "Failed to decode Text HEX string: %s", text);
						return ERR_UNKNOWN;
					}
					break;

				case SMS_Coding_8bit:
					if (! DecodeHexBin(sms->SMS[sms->Number].Text, text, text_len)) {
						SMSD_Log(DEBUG_ERROR, Config, "Failed to decode Text HEX string: %s", text);
						return ERR_UNKNOWN;
					}
					sms->SMS[sms->Number].Length = text_len / 2;
					break;

				default:
					break;
			}
		}

		if (i == 1) {
			destination = db->GetString(Config, &res, 6);
			if (destination == NULL) {
				SMSD_Log(DEBUG_ERROR, Config, "Message without recipient!");
				return ERR_UNKNOWN;
			}
			DecodeUTF8(sms->SMS[sms->Number].Number, destination, strlen(destination));
		} else {
			CopyUnicodeString(sms->SMS[sms->Number].Number, sms->SMS[0].Number);
		}

		sms->SMS[sms->Number].UDH.Type = UDH_NoUDH;
		if (udh != NULL && udh_len != 0) {
			sms->SMS[sms->Number].UDH.Type = UDH_UserUDH;
			sms->SMS[sms->Number].UDH.Length = udh_len / 2;
			if (! DecodeHexBin(sms->SMS[sms->Number].UDH.Text, udh, udh_len)) {
				SMSD_Log(DEBUG_ERROR, Config, "Failed to decode UDH HEX string: %s", udh);
				return ERR_UNKNOWN;
			}
		}

		sms->SMS[sms->Number].PDU = SMS_Submit;

		if (i == 1) {
			/* Is this a multipart message? */
			last = !db->GetBool(Config, &res, 7);
			Config->relativevalidity = (int)db->GetNumber(Config, &res, 8);

			Config->currdeliveryreport = db->GetBool(Config, &res, 9);
			strncpy(Config->CreatorID, db->GetString(Config, &res, 10), sizeof(Config->CreatorID));
			Config->CreatorID[sizeof(Config->CreatorID) - 1] = 0;
			Config->retries = (int)db->GetNumber(Config, &res, 11);
		}

		status = db->GetString(Config, &res, i == 1 ? 12 : 7);
		if (status != NULL && strncmp(status, "SendingOK", 9) == 0) {
			SMSD_Log(DEBUG_NOTICE, Config, "Marking %s:%d message for skip", ID, i);
			Config->SkipMessage[sms->Number] = TRUE;
		} else {
			Config->SkipMessage[sms->Number] = FALSE;
		}

		db->FreeResult(Config, &res);
		if (Config->SkipMessage[sms->Number] == FALSE) {
			error = SMSDSQL_ReconcileSentItem(
				&sms->SMS[sms->Number],
				Config,
				ID,
				i,
				timestamp,
				&skip_send);
			if (error == ERR_NONE && skip_send) {
				Config->SkipMessage[sms->Number] = TRUE;
			} else if (error != ERR_EMPTY) {
				return error;
			}
		}
		sms->Number++;
		if (last) {
			last = FALSE;
			break;
		}
	}

	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDSQL_MoveSMS(GSM_MultiSMSMessage * sms UNUSED, GSM_SMSDConfig * Config, char *ID, gboolean alwaysDelete UNUSED, gboolean sent UNUSED)
{
	SQL_result res;
	SQL_Var vars[2];
	struct GSM_SMSDdbobj *db = Config->db;
	GSM_Error error;

	vars[0].type = SQL_TYPE_STRING;
	vars[0].v.s = ID;
	vars[1].type = SQL_TYPE_NONE;

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_DELETE_OUTBOX], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_DELETE_OUTBOX_MULTIPART], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDSQL_CreateOutboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *NewID)
{
	char creator[200];
	int i;
	unsigned long long ID = 0;
	SQL_result res;
	SQL_Var vars[6];
	struct GSM_SMSDdbobj *db = Config->db;
	const char *report, *multipart, *q;
	GSM_Error error;

	sprintf(creator, "Gammu %s",GAMMU_VERSION); /* %1 */
	multipart = (sms->Number == 1) ? "FALSE" : "TRUE"; /* %3 */

	for (i = 0; i < sms->Number; i++) {
		report = (sms->SMS[i].PDU == SMS_Status_Report) ? "yes": "default"; /* %2 */
		if (i == 0) {
			q = Config->SMSDSQL_queries[SQL_QUERY_CREATE_OUTBOX];
		} else {
			q = Config->SMSDSQL_queries[SQL_QUERY_CREATE_OUTBOX_MULTIPART];
		}

		vars[0].type = SQL_TYPE_STRING;
		vars[0].v.s = creator;
		vars[1].type = SQL_TYPE_STRING;
		vars[1].v.s = report;
		vars[2].type = SQL_TYPE_STRING;
		vars[2].v.s = multipart;
		vars[3].type = SQL_TYPE_INT;
		vars[3].v.i = i+1;
		vars[4].type = SQL_TYPE_INT;
		vars[4].v.i = ID;
		vars[5].type = SQL_TYPE_NONE;

		error = SMSDSQL_NamedQuery(Config, q, &sms->SMS[i], NULL, vars, &res, FALSE);
		if (error != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
			return error;
		}
		if (i == 0) {
			ID = db->SeqID(Config, "outbox_ID_seq");
			if (ID == 0) {
				SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}
		}
		db->FreeResult(Config, &res);
	}
	SMSD_Log(DEBUG_INFO, Config, "Written message with ID %llu", ID);
	if (NewID != NULL)
		sprintf(NewID, "%llu", ID);
	return ERR_NONE;
}

static GSM_Error SMSDSQL_AddSentSMSInfo(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	GSM_Error error;
	size_t query_type;

	const char *message_state;
	SQL_Var vars[7];
	char smsc[GSM_MAX_NUMBER_LENGTH + 1];
	char destination[GSM_MAX_NUMBER_LENGTH + 1];

	EncodeUTF8(smsc, sms->SMS[Part - 1].SMSC.Number);
	EncodeUTF8(destination, sms->SMS[Part - 1].Number);

	if (err == SMSD_SEND_OK) {
		SMSD_Log(DEBUG_NOTICE, Config, "Transmitted %s (%s: %i) to %s", Config->SMSID,
			 (Part == sms->Number ? "total" : "part"), Part, DecodeUnicodeString(sms->SMS[0].Number));

		if (sms->SMS[Part - 1].PDU == SMS_Status_Report) {
			message_state = "SendingOK";
		} else {
			message_state = "SendingOKNoReport";
		}
	} else if (err == SMSD_SEND_SENDING_ERROR) {
		message_state = "SendingError";
	} else if (err == SMSD_SEND_ERROR) {
		message_state = "Error";
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Unknown SMS state: %d, assuming Error", err);
		message_state = "Error";
	}

	/* 1 = ID, 2 = SequencePosition, 3 = Status, 4 = TPMR, 5 = insertintodb */
	vars[0].type = SQL_TYPE_STRING;
	vars[0].v.s = ID;
	vars[1].type = SQL_TYPE_INT;
	vars[1].v.i = Part;
	vars[2].type = SQL_TYPE_STRING;
	vars[2].v.s = message_state;
	vars[3].type = SQL_TYPE_INT;
	vars[3].v.i = TPMR;
	vars[4].type = SQL_TYPE_STRING;
	vars[4].v.s = Config->DT;
	vars[5].type = SQL_TYPE_NONE;

	query_type = (Part == 1) ? SQL_QUERY_FIND_OUTBOX_BODY : SQL_QUERY_FIND_OUTBOX_MULTIPART;
	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[query_type], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Error reading from database (%s)", __FUNCTION__);
		return error;
	}
	if (db->NextRow(Config, &res) != 1) {
		db->FreeResult(Config, &res);
		return ERR_NONE;
	}
	/* 6 = StatusCode */
	vars[5].type = SQL_TYPE_INT;
	vars[5].v.i = (int)db->GetNumber(Config, &res, Part == 1 ? 13 : 8);
	vars[6].type = SQL_TYPE_NONE;
	db->FreeResult(Config, &res);

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_ADD_SENT_INFO], &sms->SMS[Part - 1], NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_UPDATE_SENT], &sms->SMS[Part - 1], NULL, NULL, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error updating number of sent messages (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	if (sms->Number != 1) {
		query_type = (Part == 1) ? SQL_QUERY_UPDATE_OUTBOX : SQL_QUERY_UPDATE_OUTBOX_MULTIPART;
		error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[query_type], &sms->SMS[Part - 1], NULL, vars, &res, FALSE);
		if (error != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error updating status of multipart messages (%s)", __FUNCTION__);
			return error;
		}
		db->FreeResult(Config, &res);
	}

	return ERR_NONE;
}

static GSM_Error SMSDSQL_RefreshPhoneStatus(GSM_SMSDConfig * Config)
{
	SQL_result res;
	SQL_Var vars[3] = {
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_NONE, {NULL}}};
	struct GSM_SMSDdbobj *db = Config->db;
	GSM_Error error;

	vars[0].v.i = Config->Status->Charge.BatteryPercent;
	vars[1].v.i = Config->Status->Network.SignalPercent;

	error = SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_REFRESH_PHONE_STATUS], NULL, NULL, vars, &res, FALSE);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return error;
	}
	db->FreeResult(Config, &res);

	return ERR_NONE;
}

/*
 * better strcat... shows where is the bug
 */
#define STRCAT_MAX 80
GSM_Error SMSDSQL_option(GSM_SMSDConfig *Config, int optint, const char *option, ...)
{
	size_t len[STRCAT_MAX], to_alloc = 0;
	int i, j;
	va_list ap;
	const char *arg;
	const char *args[STRCAT_MAX];
	char *buffer, *ptr;

	/* read from config */
	buffer = INI_GetValue(Config->smsdcfgfile, "sql", option, FALSE);
	/* found? */
	if (buffer != NULL){
		Config->SMSDSQL_queries[optint] = strdup(buffer); /* avoid to double free */
		return ERR_NONE;
	}

	/* not found.. we use default query */
	va_start(ap, option);
	for(i = 0; i < STRCAT_MAX; i++){
		arg = va_arg(ap, const char *);
		if (arg == NULL)
			break;
		len[i] = strlen(arg);
		args[i] = arg;
		to_alloc += len[i];
	}
	va_end(ap);

	if (i == STRCAT_MAX) {
		SMSD_Log(DEBUG_ERROR, Config, "STRCAT_MAX too small.. consider increase this value for option %s", option);
		return ERR_UNKNOWN;
	}

	buffer = malloc(to_alloc+1);
	if (buffer == NULL){
		SMSD_Log(DEBUG_ERROR, Config, "Insufficient memory problem for option %s", option);
		return ERR_UNKNOWN;
	}
	ptr = buffer;
	for (j = 0; j < i; j++) {
		memcpy(ptr, args[j], len[j]);
		ptr += len[j];
	}
	*ptr = '\0';
	Config->SMSDSQL_queries[optint] = buffer;
	return ERR_NONE;
}


/**
 * Reads common options for database backends.
 */
GSM_Error SMSDSQL_ReadConfiguration(GSM_SMSDConfig *Config)
{
	int locktime, phone_timeout;
	const char *escape_char;

	Config->inbox_groups = NULL;

	Config->user = INI_GetValue(Config->smsdcfgfile, "smsd", "user", FALSE);
	if (Config->user == NULL) {
		Config->user="root";
	}

	Config->password = INI_GetValue(Config->smsdcfgfile, "smsd", "password", FALSE);
	if (Config->password == NULL) {
		Config->password="";
	}

	Config->host = INI_GetValue(Config->smsdcfgfile, "smsd", "host", FALSE);
	if (Config->host == NULL) {
		/* Backward compatibility */
		Config->host = INI_GetValue(Config->smsdcfgfile, "smsd", "pc", FALSE);
	}
	if (Config->host == NULL) {
		Config->host="localhost";
	}

	Config->database = INI_GetValue(Config->smsdcfgfile, "smsd", "database", FALSE);
	if (Config->database == NULL) {
		Config->database="sms";
	}

	Config->table_gammu = INI_GetValue(Config->smsdcfgfile, "tables", "gammu", FALSE);
	if (Config->table_gammu == NULL){
		Config->table_gammu = "gammu";
	}
	Config->table_inbox = INI_GetValue(Config->smsdcfgfile, "tables", "inbox", FALSE);
	if (Config->table_inbox == NULL){
		Config->table_inbox = "inbox";
	}
	Config->table_sentitems = INI_GetValue(Config->smsdcfgfile, "tables", "sentitems", FALSE);
	if (Config->table_sentitems == NULL){
		Config->table_sentitems = "sentitems";
	}
	Config->table_outbox = INI_GetValue(Config->smsdcfgfile, "tables", "outbox", FALSE);
	if (Config->table_outbox == NULL){
		Config->table_outbox = "outbox";
	}
	Config->table_outbox_multipart = INI_GetValue(Config->smsdcfgfile, "tables", "outbox_multipart", FALSE);
	if (Config->table_outbox_multipart == NULL){
		Config->table_outbox_multipart = "outbox_multipart";
	}
	Config->table_phones = INI_GetValue(Config->smsdcfgfile, "tables", "phones", FALSE);
	if (Config->table_phones == NULL){
		Config->table_phones = "phones";
	}

	Config->driverspath = INI_GetValue(Config->smsdcfgfile, "smsd", "driverspath", FALSE);

	Config->sql = INI_GetValue(Config->smsdcfgfile, "smsd", "sql", FALSE);

	Config->dbdir = INI_GetValue(Config->smsdcfgfile, "smsd", "dbdir", FALSE);

	if (Config->driver == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "No database driver selected. Must be native_mysql, native_pgsql, ODBC or DBI one.");
		return ERR_UNKNOWN;
	}

	Config->db = NULL;
	if (!strcasecmp(Config->driver, "native_mysql")) {
#ifdef HAVE_MYSQL_MYSQL_H
		Config->db = &SMSDMySQL;
#else
		SMSD_Log(DEBUG_ERROR, Config, "The %s driver was not compiled in!", Config->driver);
		return ERR_DISABLED;
#endif
	}
	if (!strcasecmp(Config->driver, "native_pgsql")) {
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
		Config->db = &SMSDPgSQL;
#else
		SMSD_Log(DEBUG_ERROR, Config, "The %s driver was not compiled in!", Config->driver);
		return ERR_DISABLED;
#endif
	}
	if (!strcasecmp(Config->driver, "odbc")) {
#ifdef ODBC_FOUND
		Config->db = &SMSDODBC;
		if (Config->sql == NULL) {
			SMSD_Log(DEBUG_INFO, Config, "Using generic SQL for ODBC, this might fail. In such case please set SQL configuration option.");
		}
#else
		SMSD_Log(DEBUG_ERROR, Config, "The %s driver was not compiled in!", Config->driver);
		return ERR_DISABLED;
#endif
	}
	if (Config->db == NULL) {
#ifdef LIBDBI_FOUND
		Config->db = &SMSDDBI;
#else
		SMSD_Log(DEBUG_ERROR, Config, "Unknown DB driver");
		return ERR_UNKNOWN;
#endif
	}

	escape_char = SMSDSQL_EscapeChar(Config);
#define ESCAPE_FIELD(x) escape_char, x, escape_char

	locktime = Config->loopsleep * 8; /* reserve 8 sec per message */
	locktime = locktime < 60 ? 60 : locktime; /* Minimum time reserve is 60 sec */
	phone_timeout = SMSDSQL_PhoneStatusTimeout(Config->statusfrequency);

	if (SMSDSQL_option(Config, SQL_QUERY_DELETE_PHONE, "delete_phone",
		"DELETE FROM ", Config->table_phones, " WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_INSERT_PHONE, "insert_phone",
		"INSERT INTO ", Config->table_phones, " (",
			ESCAPE_FIELD("IMEI"),
			", ", ESCAPE_FIELD("IMSI"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("NetCode"),
			", ", ESCAPE_FIELD("NetName"),
			", ", ESCAPE_FIELD("Send"),
			", ", ESCAPE_FIELD("Receive"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("TimeOut"),
			", ", ESCAPE_FIELD("Client"),
			", ", ESCAPE_FIELD("Battery"),
			", ", ESCAPE_FIELD("Signal"),
			") VALUES (%I, %S, %P, %O, %M, %1, %2, ",
			SMSDSQL_Now(Config),
			", ",
			SMSDSQL_NowPlus(Config, phone_timeout),
			", %N, -1, -1)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_SELECT, "save_inbox_sms_select",
		"SELECT ",
			ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("SendingDateTime"),
			", ", ESCAPE_FIELD("DeliveryDateTime"),
			", ", ESCAPE_FIELD("SMSCNumber"), " "
			"FROM ", Config->table_sentitems, " WHERE ",
			ESCAPE_FIELD("DeliveryDateTime"), " IS NULL AND ",
			ESCAPE_FIELD("SenderID"), " = %P AND ",
			ESCAPE_FIELD("TPMR"), " = %t AND ",
			ESCAPE_FIELD("DestinationNumber"), " = %R", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED, "save_inbox_sms_update_delivered",
		"UPDATE ", Config->table_sentitems, " "
			"SET ", ESCAPE_FIELD("DeliveryDateTime"), " = %C"
			", ", ESCAPE_FIELD("Status"), " = %1"
			", ", ESCAPE_FIELD("StatusError"), " = %e"
			" WHERE ", ESCAPE_FIELD("ID"), " = %2"
			" AND ", ESCAPE_FIELD("TPMR"), " = %t", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_UPDATE, "save_inbox_sms_update",
		"UPDATE ", Config->table_sentitems, " "
			"SET ",	ESCAPE_FIELD("Status"), " = %1"
			", ", ESCAPE_FIELD("StatusError"), " = %e"
			" WHERE ", ESCAPE_FIELD("ID"), " = %2"
			" AND ", ESCAPE_FIELD("TPMR"), " = %t", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_INSERT, "save_inbox_sms_insert",
		"INSERT INTO ", Config->table_inbox, " "
			"(", ESCAPE_FIELD("ReceivingDateTime"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("SenderNumber"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("SMSCNumber"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("RecipientID"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("MessageID"),
			", ", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("PartCount"),
			", ", ESCAPE_FIELD("Processed"), ")",
			" VALUES (%d, ", SMSDSQL_Now(Config),
			", %E, %R, %c, %F, %u, %x, %T, %P, %e, %1, %2, %3, %4)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_UPDATE_METADATA, "save_inbox_sms_update_metadata",
		"UPDATE ", Config->table_inbox, " "
			"SET ", ESCAPE_FIELD("MessageID"), " = %1"
			", ", ESCAPE_FIELD("SequencePosition"), " = %2"
			", ", ESCAPE_FIELD("PartCount"), " = %3"
			", ", ESCAPE_FIELD("Processed"), " = %5"
			" WHERE ", ESCAPE_FIELD("ID"), " = %4", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_RESTORE_INBOX_GROUPS, "restore_inbox_groups",
		"SELECT ",
			ESCAPE_FIELD("MessageID"),
			", ", ESCAPE_FIELD("SenderNumber"),
			", ", ESCAPE_FIELD("SMSCNumber"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("PartCount"),
			", ", SMSDSQL_TimeDiff(Config, "InsertIntoDB"),
			" FROM ", Config->table_inbox,
			" WHERE ", ESCAPE_FIELD("PartCount"), " > 1"
			" AND ", ESCAPE_FIELD("RecipientID"), " = %P"
			" AND ", ESCAPE_FIELD("MessageID"), " IN (SELECT ",
				ESCAPE_FIELD("MessageID"),
				" FROM ", Config->table_inbox,
				" WHERE ", ESCAPE_FIELD("PartCount"), " > 1"
				" AND ", ESCAPE_FIELD("RecipientID"), " = %P"
				" AND ", ESCAPE_FIELD("InsertIntoDB"), " >= ",
					SMSDSQL_NowMinus(Config, Config->multiparttimeout),
			") ORDER BY ", ESCAPE_FIELD("InsertIntoDB"), " ASC, ",
				ESCAPE_FIELD("ID"), " ASC", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_RECEIVED, "update_received",
		"UPDATE ", Config->table_phones, " SET ",
			ESCAPE_FIELD("Received"), " = ", ESCAPE_FIELD("Received"), " + 1"
			" WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_REFRESH_SEND_STATUS, "refresh_send_status",
		"UPDATE ", Config->table_outbox, " SET ",
			ESCAPE_FIELD("SendingTimeOut"), " = ", SMSDSQL_NowPlus(Config, locktime),
			" WHERE ", ESCAPE_FIELD("ID"), " = %1"
			" AND (", ESCAPE_FIELD("SendingTimeOut"), " < ", SMSDSQL_Now(Config),
			" OR ", ESCAPE_FIELD("SendingTimeOut"), " IS NULL)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_RETRIES, "update_retries",
		"UPDATE ", Config->table_outbox, " SET ",
			ESCAPE_FIELD("SendingTimeOut"), " = ", SMSDSQL_NowPlus(Config, Config->retrytimeout),
			", ",ESCAPE_FIELD("Retries"), " = %2"
			" WHERE ", ESCAPE_FIELD("ID"), " = %1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_OUTBOX_SMS_ID, "find_outbox_sms_id",
		"SELECT ", SMSDSQL_TopClause(Config, "%1"),
			ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("SendingDateTime"),
			", ", ESCAPE_FIELD("SenderID"),
			" FROM ", Config->table_outbox, " WHERE ",
			ESCAPE_FIELD("SendingDateTime"), " < ", SMSDSQL_Now(Config),
			" AND ", ESCAPE_FIELD("SendingTimeOut"), " < ", SMSDSQL_Now(Config),
			" AND ", ESCAPE_FIELD("SendBefore"), " >= ", SMSDSQL_CurrentTime(Config),
			" AND ", ESCAPE_FIELD("SendAfter"), " <= ", SMSDSQL_CurrentTime(Config),
			" AND ", SMSDSQL_DayMaskPredicate(Config),
			" AND ( ", ESCAPE_FIELD("SenderID"), " is NULL OR ", ESCAPE_FIELD("SenderID"), " = '' OR ", ESCAPE_FIELD("SenderID"), " = %P )",
			SMSDSQL_RownumClause(Config, "%1", TRUE),
			" ORDER BY ", ESCAPE_FIELD("Priority"), " DESC, ", ESCAPE_FIELD("InsertIntoDB"), " ASC ", SMSDSQL_LimitClause(Config, "%1"), NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_OUTBOX_BODY, "find_outbox_body",
		"SELECT ",
			ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("MultiPart"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			", ", ESCAPE_FIELD("DeliveryReport"),
			", ", ESCAPE_FIELD("CreatorID"),
			", ", ESCAPE_FIELD("Retries"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("StatusCode"),
			" FROM ", Config->table_outbox, " WHERE ",
			ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_OUTBOX_MULTIPART, "find_outbox_multipart",
		"SELECT ",
			ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("StatusCode"),
			" FROM ", Config->table_outbox_multipart, " WHERE ",
			ESCAPE_FIELD("ID"), "=%1 AND ",
			ESCAPE_FIELD("SequencePosition"), "=%2", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_SENT_ITEM, "find_sent_item",
		"SELECT ",
			ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			", ", ESCAPE_FIELD("CreatorID"),
			", ", ESCAPE_FIELD("Status"),
			" FROM ", Config->table_sentitems, " WHERE ",
			ESCAPE_FIELD("ID"), "=%1 AND ",
			ESCAPE_FIELD("SequencePosition"), "=%2", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_DELETE_OUTBOX, "delete_outbox",
		"DELETE FROM ", Config->table_outbox, " WHERE ", ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_DELETE_OUTBOX_MULTIPART, "delete_outbox_multipart",
		"DELETE FROM ", Config->table_outbox_multipart, " WHERE ", ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_CREATE_OUTBOX, "create_outbox",
		"INSERT INTO ", Config->table_outbox, " "
			"(", ESCAPE_FIELD("CreatorID"),
			", ", ESCAPE_FIELD("SenderID"),
			", ", ESCAPE_FIELD("DeliveryReport"),
			", ", ESCAPE_FIELD("MultiPart"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"), ") VALUES "
			"(%1, %P, %2, %3, ", SMSDSQL_Now(Config),
			", %E, %R, %V, %c, %u, %x, %T)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_CREATE_OUTBOX_MULTIPART, "create_outbox_multipart",
		"INSERT INTO ", Config->table_outbox_multipart, " "
			"(", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("ID"), ") VALUES (%4, %E, %c, %u, %x, %T, %5)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_OUTBOX, "update_outbox",
		"UPDATE ", Config->table_outbox, " SET ",
			ESCAPE_FIELD("Status"), "=%3 WHERE ",
			ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_OUTBOX_MULTIPART, "update_outbox_multipart",
		"UPDATE ", Config->table_outbox_multipart, " SET ",
			ESCAPE_FIELD("Status"), "=%3 WHERE ",
			ESCAPE_FIELD("ID"), "=%1 AND ",
			ESCAPE_FIELD("SequencePosition"), "=%2", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_OUTBOX_STATUSCODE, "update_outbox_statuscode",
		"UPDATE ", Config->table_outbox, " SET ",
			ESCAPE_FIELD("StatusCode"), "=%3 WHERE ",
			ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_OUTBOX_MULTIPART_STATUSCODE, "update_outbox_multipart_statuscode",
		"UPDATE ", Config->table_outbox_multipart, " SET ",
			ESCAPE_FIELD("StatusCode"), "=%3 WHERE ",
			ESCAPE_FIELD("ID"), "=%1 AND ",
			ESCAPE_FIELD("SequencePosition"), "=%4", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_ADD_SENT_INFO, "add_sent_info",
		"INSERT INTO ", Config->table_sentitems, " "
			"(", ESCAPE_FIELD("CreatorID"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("SendingDateTime"),
			", ", ESCAPE_FIELD("SMSCNumber"),
			", ", ESCAPE_FIELD("TPMR"),
			", ", ESCAPE_FIELD("SenderID"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			", ", ESCAPE_FIELD("StatusCode"),
			") "
			" VALUES (%A, %1, %2, %3, ",
			SMSDSQL_Now(Config),
			", %F, %4, %P, %E, %R, %c, %u, %x, %T, %5, %V, %6)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_SENT, "update_sent",
		"UPDATE ", Config->table_phones, " SET ",
			ESCAPE_FIELD("Sent"), "= ", ESCAPE_FIELD("Sent"), " + 1"
			" WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_REFRESH_PHONE_STATUS, "refresh_phone_status",
		"UPDATE ", Config->table_phones, " SET ",
			ESCAPE_FIELD("TimeOut"), "= ", SMSDSQL_NowPlus(Config, phone_timeout),
			", ", ESCAPE_FIELD("Battery"), " = %1"
			", ", ESCAPE_FIELD("Signal"), " = %2"
			", ", ESCAPE_FIELD("NetCode"), " = %O"
			", ", ESCAPE_FIELD("NetName"), " = %M"
			" WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}
#undef ESCAPE_FIELD

	return ERR_NONE;
}

/* Converts the given local date and time into POSIX calendar time
 *
 * The date string argument must be a system local point in time
 * formatted as "YYYY-MM-DD HH:MM:SS"
 *
 * returns the POSIX (UTC) calendar time for the given date/time, or
 * a negative time_t on error.
 */
time_t SMSDSQL_ParseDate(GSM_SMSDConfig * Config, const char *date)
{
	char *parse_res;
	struct tm tm;
	time_t time = -1;

	if (strcmp(date, "0000-00-00 00:00:00") == 0) {
		return -2;
	}

	parse_res = strptime(date, "%Y-%m-%d %H:%M:%S", &tm);
	if (parse_res != NULL && *parse_res == 0) {
	  tm.tm_isdst = -1;
	  time = mktime(&tm);
	}
	else if (Config != NULL) {
    /* Used during testing */
		SMSD_Log(DEBUG_ERROR, Config, "Failed to parse date: %s", date);
	}

	return time;
}

GSM_SMSDService SMSDSQL = {
	SMSDSQL_Init,
	SMSDSQL_Free,
	SMSDSQL_InitAfterConnect,
	SMSDSQL_SaveInboxSMS,
	SMSDSQL_FindOutboxSMS,
	SMSDSQL_MoveSMS,
	SMSDSQL_CreateOutboxSMS,
	SMSDSQL_AddSentSMSInfo,
	SMSDSQL_RefreshSendStatus,
	SMSDSQL_UpdateRetries,
	SMSDSQL_RefreshPhoneStatus,
	SMSDSQL_ReadConfiguration
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
