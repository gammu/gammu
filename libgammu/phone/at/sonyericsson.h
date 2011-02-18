/* Sony Ericsson specific functions
 * Copyright (C) 2011 Márton Németh <nm127@freemail.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (c) 2011 MÃ¡rton NÃ©meth <nm127@freemail.hu>
 */

/**
 * \file
 * @author MÃ¡rton NÃ©meth
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @addtogroup Sony Ericsson
 * @{
 */

#ifndef __sonyericsson_h
#define __sonyericsson_h

#include <gammu-config.h>
#include "../../gsmstate.h"

#define SONYERICSSON_TIMEOUT 60

GSM_Error SONYERICSSON_GetScreenshot(GSM_StateMachine *s, GSM_BinaryPicture *picture);
GSM_Error SONYERICSSON_Reply_Screenshot(GSM_Protocol_Message *msg, GSM_StateMachine *s);
GSM_Error SONYERICSSON_Reply_ScreenshotData(GSM_Protocol_Message *msg, GSM_StateMachine *s);

#endif


/*@}*/
/*@}*/
