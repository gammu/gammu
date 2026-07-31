/**
 * Tests ODBC string literal escaping.
 */

#include <stdlib.h>
#include <string.h>

#include <gammu-smsd.h>

#include "../smsd/core.h"
#include "common.h"

extern char *SMSDODBC_QuoteString(GSM_SMSDConfig *Config, const char *string);

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_SMSDConfig config;
	char *quoted;
	char *long_text;
	size_t quoted_length;
	long long signed_value;
	unsigned long long unsigned_value;

	memset(&config, 0, sizeof(config));
	config.driver = "odbc";
	config.sql = "mssql";

	quoted = SMSDODBC_QuoteString(&config, "Don't panic. C:\\temp");
	test_result(strcmp(quoted, "'Don''t panic. C:\\temp'") == 0);
	free(quoted);

	config.sql = "oracle";
	quoted = SMSDODBC_QuoteString(&config, "Don't panic. C:\\temp");
	test_result(strcmp(quoted, "'Don''t panic. C:\\temp'") == 0);
	free(quoted);

	test_result(strcmp(SMSDODBC_SeqIDQuery(&config, "outbox_ID_seq"),
			"SELECT outbox_ID_seq.CURRVAL FROM dual") == 0);
	test_result(SMSDODBC_SeqIDQuery(&config, "outbox.ID") == NULL);
	test_result(SMSDODBC_ParseSignedInteger("-1", &signed_value));
	test_result(signed_value == -1);
	test_result(SMSDODBC_ParseSignedInteger("9223372036854775807", &signed_value));
	test_result(signed_value == 9223372036854775807LL);
	test_result(!SMSDODBC_ParseSignedInteger("9223372036854775808", &signed_value));
	test_result(SMSDODBC_ParseUnsignedInteger("9999999999999999999", &unsigned_value));
	test_result(unsigned_value == 9999999999999999999ULL);
	test_result(!SMSDODBC_ParseUnsignedInteger("-1", &unsigned_value));
	test_result(!SMSDODBC_ParseUnsignedInteger("1.5", &unsigned_value));
	test_result(!SMSDODBC_ParseUnsignedInteger(" 1", &unsigned_value));

	long_text = malloc(4501);
	test_result(long_text != NULL);
	memset(long_text, 'a', 4500);
	long_text[100] = '\'';
	long_text[4500] = '\0';
	quoted = SMSDODBC_QuoteString(&config, long_text);
	test_result(quoted != NULL);
	quoted_length = strlen(quoted);
	test_result(strncmp(quoted, "(TO_CLOB('", 10) == 0);
	test_result(strstr(quoted, "aa''aa") != NULL);
	test_result(strstr(quoted, "') || TO_CLOB('") != NULL);
	test_result(strcmp(quoted + quoted_length - 2, "))") == 0);
	free(quoted);
	free(long_text);

	config.sql = "mssql";
	test_result(strcmp(SMSDODBC_SeqIDQuery(&config, "ignored"),
			"SELECT @@IDENTITY") == 0);

	return 0;
}
