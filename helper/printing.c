/**
 * Generic message printing helpers.
 */

#include <gammu-misc.h>
#include "locales.h"

#include <stdarg.h>
#include <stdio.h>

PRINTF_STYLE(1, 2)
int printf_err(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for error messages */
	printf("%s: ", _("Error"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

PRINTF_STYLE(1, 2)
int printf_warn(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for warning messages */
	printf("%s: ", _("Warning"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

PRINTF_STYLE(1, 2)
int printf_info(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for informational messages */
	printf("%s: ", _("Information"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

