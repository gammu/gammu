/* Reconnecting must preserve the configured host, port and Unix socket. */
#include <stdio.h>
#include <string.h>
#include <gammu.h>
#include <mysql.h>
#include "common.h"

static const char *expected_host;
static const char *expected_socket;
static unsigned int expected_port;
static gboolean connect_succeeds;

static MYSQL *test_mysql_real_connect(MYSQL *mysql, const char *host,
	const char *user UNUSED, const char *password UNUSED,
	const char *database UNUSED, unsigned int port, const char *socket,
	unsigned long flags UNUSED)
{
	test_result(strcmp(host, expected_host) == 0);
	test_result(port == expected_port);
	if (expected_socket == NULL) {
		test_result(socket == NULL);
	} else {
		test_result(socket != NULL);
		test_result(strcmp(socket, expected_socket) == 0);
	}
	return connect_succeeds ? mysql : NULL;
}

static int test_mysql_query(MYSQL *mysql UNUSED, const char *query UNUSED)
{
	return 0;
}

#define mysql_real_connect test_mysql_real_connect
#define mysql_query test_mysql_query
#include "../smsd/services/mysql.c"

static void check_endpoint(const char *endpoint, const char *host,
	unsigned int port, const char *socket)
{
	GSM_SMSDConfig config;
	int attempt;

	memset(&config, 0, sizeof(config));
	config.host = endpoint;
	config.database = "smsd";
	expected_host = host;
	expected_port = port;
	expected_socket = socket;

	/* Exercise reconnects after both successful and failed connections. */
	for (attempt = 0; attempt < 4; attempt++) {
		connect_succeeds = attempt % 2 == 0;
		gammu_test_result_code(SMSDMySQL.Connect(&config), "Connect",
			(connect_succeeds ? ERR_NONE : ERR_DB_CONNECT));
		test_result(strcmp(config.host, endpoint) == 0);
		SMSDMySQL.Free(&config);
	}
}

int main(void)
{
	check_endpoint("192.0.2.1:3307", "192.0.2.1", 3307, NULL);
	check_endpoint("localhost:/tmp/gammu-test.sock", "localhost", 0,
		"/tmp/gammu-test.sock");
	check_endpoint("localhost", "localhost", 0, NULL);
	return 0;
}
