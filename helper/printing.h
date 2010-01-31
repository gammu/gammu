/**
 * Generic message printing helpers.
 */

#ifndef __helper_printing_h__
#define __helper_printing_h__

#include <gammu-misc.h>

/**
 * Write error to user.
 */
PRINTF_STYLE(1, 2)
int printf_err(const char *format, ...);

/**
 * Write warning to user.
 */
PRINTF_STYLE(1, 2)
int printf_warn(const char *format, ...);

/**
 * Write information to user.
 */
PRINTF_STYLE(1, 2)
int printf_info(const char *format, ...);


#endif
