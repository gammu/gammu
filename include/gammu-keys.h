/**
 * \file gammu-keys.h
 * \author Michal Čihař
 *
 * Keys data and functions.
 */
#ifndef __gammu_keys_h
#define __gammu_keys_h

/**
 * \defgroup Keys Keys
 * Keyboard manipulations.
 */

#include <gammu-error.h>
#include <gammu-statemachine.h>
#include <gammu-types.h>

#include <stdlib.h>		/* Needed for size_t declaration */

/**
 * Key event identifiers.
 *
 * \ingroup Keys
 */
typedef enum {
	GSM_KEY_NONE = 0x00,
	GSM_KEY_1 = 0x01,
	GSM_KEY_2,
	GSM_KEY_3,
	GSM_KEY_4,
	GSM_KEY_5,
	GSM_KEY_6,
	GSM_KEY_7,
	GSM_KEY_8,
	GSM_KEY_9,
	GSM_KEY_0,
	/**
	 * #
	 */
	GSM_KEY_HASH,
	/**
	 * *
	 */
	GSM_KEY_ASTERISK,
	/**
	 * Power key.
	 */
	GSM_KEY_POWER,
	/**
	 * in some phone ie. N5110 sometimes works identical to POWER
	 */
	GSM_KEY_GREEN,
	/**
	 * (c) key in some phone: ie. N5110
	 */
	GSM_KEY_RED,
	/**
	 * Not available in some phones as separate button: ie. N5110
	 */
	GSM_KEY_INCREASEVOLUME,
	/**
	 * Not available in some phones as separate button: ie. N5110
	 */
	GSM_KEY_DECREASEVOLUME,
	GSM_KEY_UP = 0x17,
	GSM_KEY_DOWN,
	GSM_KEY_MENU,
	/**
	 * Not available in some phone: ie. N5110
	 */
	GSM_KEY_NAMES,
	/**
	 * Left arrow
	 */
	GSM_KEY_LEFT,
	/**
	 * Right arrow
	 */
	GSM_KEY_RIGHT,
	/**
	 * Software key which has assigned mening on display.
	 */
	GSM_KEY_SOFT1,
	/**
	 * Software key which has assigned mening on display.
	 */
	GSM_KEY_SOFT2,
	/**
	 * Button on headset
	 */
	GSM_KEY_HEADSET,
	/**
	 * Joystick pressed
	 */
	GSM_KEY_JOYSTICK,
	/**
	 * Camera button pressed
	 */
	GSM_KEY_CAMERA,
	/**
	 * Media player button
	 */
	GSM_KEY_MEDIA,
	/**
	 * Multi function key, desktop
	 */
	GSM_KEY_DESKTOP,
	/**
	 * Operator button
	 */
	GSM_KEY_OPERATOR,
	/**
	 * Return button
	 */
	GSM_KEY_RETURN,
	/**
	 * Clear button
	 */
	GSM_KEY_CLEAR,
} GSM_KeyCode;

/**
 * Creates key sequence from string.
 *
 * \param text Text to convert.
 * \param KeyCode Storage for key codes.
 * \param Length Storage for resulting length.
 *
 * \return Error code.
 *
 * \ingroup Keys
 */
GSM_Error MakeKeySequence(char *text, GSM_KeyCode * KeyCode, size_t * Length);

/**
 * Emulates key press or key release.
 *
 * \ingroup Keys
 */
GSM_Error GSM_PressKey(GSM_StateMachine * s, GSM_KeyCode Key, gboolean Press);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
