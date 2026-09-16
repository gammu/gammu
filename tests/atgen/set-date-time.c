/* Regression tests for AT+CCLK timezone encoding. */
#include "test_helper.h"
#include "../../libgammu/gsmstate.h"

static void check_date_time(int timezone, gboolean four_digit_year,
		const char **responses, size_t response_count, const char *expected)
{
	GSM_StateMachine *s = setup_state_machine();
	GSM_PhoneModel model;
	GSM_DateTime dt = {0};

	setup_at_engine(s);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	if (four_digit_year) {
		test_result(GSM_AddPhoneFeature(&model, F_FOUR_DIGIT_YEAR));
	}
	bind_response_handling(s);
	set_responses(responses, response_count);
	dt.Year = 2013;
	dt.Month = 9;
	dt.Day = 25;
	dt.Hour = 19;
	dt.Minute = 8;
	dt.Second = 15;
	dt.Timezone = timezone;
	test_result(GSM_SetDateTime(s, &dt) == ERR_NONE);
	test_result(strcmp((const char *)last_command(), expected) == 0);
	cleanup_state_machine(s);
}

int main(void)
{
	static const struct {
		int timezone;
		const char *suffix;
	} cases[] = {
		{0, "+00"},
		{3600, "+04"},
		{-3600, "-04"},
		{19800, "+22"},
		{20700, "+23"},
		{-12600, "-14"}
	};
	const char *ok[] = {"\r\nOK\r\n"};
	const char *retry21[] = {"\r\n+CME ERROR: 21\r\n", "\r\nOK\r\n"};
	const char *retry24[] = {"\r\n+CME ERROR: 24\r\n", "\r\nOK\r\n"};
	char expected[128];
	size_t i;
	int four_digit_year;

	for (four_digit_year = 0; four_digit_year <= 1; four_digit_year++) {
		for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
			snprintf(expected, sizeof(expected),
				"AT+CCLK=\"%s/09/25,19:08:15%s\"\r",
				four_digit_year ? "2013" : "13", cases[i].suffix);
			check_date_time(cases[i].timezone, four_digit_year, ok, 1, expected);
		}
		snprintf(expected, sizeof(expected),
			"AT+CCLK=\"%s/09/25,19:08:15\"\r",
			four_digit_year ? "2013" : "13");
		check_date_time(3600, four_digit_year, retry21, 2, expected);
		check_date_time(3600, four_digit_year, retry24, 2, expected);
	}
	return 0;
}
