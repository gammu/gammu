#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_argument(const char *actual, const char *expected)
{
	if (strcmp(actual, expected) == 0) {
		return 0;
	}

	fprintf(stderr, "Argument mismatch:\nexpected: %s\nactual: %s\n", expected, actual);
	return 1;
}

int main(int argc, char **argv)
{
	const char *marker;
	char expected[4096];

	if (getenv("SMSD_TEST_EMPTY") != NULL) {
		if (argc != 1) {
			fprintf(stderr, "Expected no arguments, got %d\n", argc - 1);
			return 1;
		}
		return 0;
	}

	if (getenv("SMSD_TEST_SINGLE") != NULL) {
		if (argc != 2) {
			fprintf(stderr, "Expected one argument, got %d\n", argc - 1);
			return 1;
		}
		return check_argument(argv[1], "single argument");
	}

	marker = getenv("SMSD_TEST_MARKER");
	if (marker == NULL) {
		fprintf(stderr, "SMSD_TEST_MARKER is not set\n");
		return 1;
	}
	if (argc != 12) {
		fprintf(stderr, "Expected 11 arguments, got %d\n", argc - 1);
		return 1;
	}

	if (check_argument(argv[1], "normal") != 0) return 1;

	snprintf(expected, sizeof(expected), "$(touch %s)", marker);
	if (check_argument(argv[2], expected) != 0) return 1;

	snprintf(expected, sizeof(expected), "`touch %s`", marker);
	if (check_argument(argv[3], expected) != 0) return 1;

	snprintf(expected, sizeof(expected), "; touch %s", marker);
	if (check_argument(argv[4], expected) != 0) return 1;

	snprintf(expected, sizeof(expected), "& touch %s", marker);
	if (check_argument(argv[5], expected) != 0) return 1;

	snprintf(expected, sizeof(expected), "(touch %s)", marker);
	if (check_argument(argv[6], expected) != 0) return 1;

	if (check_argument(argv[7], "value with spaces\tand a tab") != 0) return 1;
	if (check_argument(argv[8], "line one\nline two") != 0) return 1;
	if (check_argument(argv[9], "$PATH") != 0) return 1;
	if (check_argument(argv[10], "\"'") != 0) return 1;
	if (check_argument(argv[11], "") != 0) return 1;

	return 0;
}
