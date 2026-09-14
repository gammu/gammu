/* Failed connections must not discard a configured PostgreSQL port. */
#include <stdio.h>
#include <string.h>
#include <gammu.h>
#include <libpq-fe.h>
#include "common.h"

static const char *expected_connection;
static int connection_token;

static PGconn *test_PQconnectdb(const char *connection)
{
	test_result(strcmp(connection, expected_connection) == 0);
	return (PGconn *)&connection_token;
}

static ConnStatusType test_PQstatus(const PGconn *connection UNUSED)
{
	return CONNECTION_BAD;
}

static char *test_PQerrorMessage(const PGconn *connection UNUSED)
{
	static char message[] = "Simulated connection failure";
	return message;
}

static void test_PQfinish(PGconn *connection UNUSED)
{
}

#define PQconnectdb test_PQconnectdb
#define PQstatus test_PQstatus
#define PQerrorMessage test_PQerrorMessage
#define PQfinish test_PQfinish
/* The driver defines this feature macro itself. */
#undef _DEFAULT_SOURCE
#include "../smsd/services/pgsql.c"

static void check_endpoint(const char *endpoint, const char *connection)
{
	GSM_SMSDConfig config;
	int attempt;

	memset(&config, 0, sizeof(config));
	config.host = endpoint;
	config.user = "smsd";
	config.password = "test";
	config.database = "smsd";
	expected_connection = connection;

	for (attempt = 0; attempt < 3; attempt++) {
		gammu_test_result_code(SMSDPgSQL.Connect(&config), "Connect", ERR_DB_CONNECT);
		test_result(strcmp(config.host, endpoint) == 0);
	}
}

int main(void)
{
	check_endpoint("192.0.2.1:5433",
		"host = '192.0.2.1' user = 'smsd' password = 'test' dbname = 'smsd' port = 5433");
	check_endpoint("localhost",
		"host = 'localhost' user = 'smsd' password = 'test' dbname = 'smsd' port = 5432");
	return 0;
}
