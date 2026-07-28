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

	memset(&config, 0, sizeof(config));
	config.driver = "odbc";
	config.sql = "mssql";

	quoted = SMSDODBC_QuoteString(&config, "Don't panic. C:\\temp");
	test_result(strcmp(quoted, "'Don''t panic. C:\\temp'") == 0);
	free(quoted);

	return 0;
}
