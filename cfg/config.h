#ifndef config_h
#define config_h

/* This is config file. Change ONLY lines with comment */

#define VERSION "0.62"			/* Version of package 		*/

#ifndef _MSC_VER
#  define DEBUG				/* Want DEBUG info or not ?	*/
#endif

#define GSM_ENABLE_MBUS2		/* MBUS2 over cable 		*/
#define GSM_ENABLE_FBUS2		/* FBUS2 over cable. 		*/
#define GSM_ENABLE_DLR3AT		/* FBUS2 in DLR3 cable. AT init	*/
#define GSM_ENABLE_IRDA			/* Direct infrared		*/
#define GSM_ENABLE_INFRARED		/* Infrared over irda sockets	*/
#define GSM_ENABLE_AT			/* AT protocol			*/
//#define GSM_ENABLE_ALCABUS

/* Works only in Linux.
 * you need bluez-kernel-2.3-pre2 or greater, bluez-libs-2.1 
 * and bluez-utils-2.1 (not yet released) to have Bluetooth in OS
 * The kernel 2.4.20 will may include the RFCOMM implementation. 
 */
//#define GSM_ENABLE_ATBLUETOOTH	/* AT over bluetooth		*/
//#define GSM_ENABLE_DLR3BLUETOOTH	/* FBUS2 over bluetooth		*/

/* By default commented: some people reported me (for example, with 6210
 * and 3210), that their phones doesn't answer at all sometimes or answer
 * with (too) long delay. Possible reason: firmware in phone is too slow
 * I didn't have such problems personally with phone and PC with Celeron 800
 * Please also note, that CB function was always TO CHECK in MyGnokii/Gnokii
 * too.
 */
//#define GSM_ENABLE_CELLBROADCAST	/* Enables cell broadcast	*/

/* If you want backup/restore stuff. Some people wanted it - they required
 * smalled library made from Gammu and didn't require it. Please note,
 * that backup/restore stuff can be easy separated from other things.
 */
#define GSM_ENABLE_BACKUP

/* Nokia 51xx/61xx phones can make authentication. It allows only to
 * display "Nokia accessory" during start, nothing more.
 */
#define GSM_ENABLE_6110_AUTHENTICATION

/* It allows to show incoming call and SMS info for 7110 family phones.
 * By default commented: you can have some problems, when using it.
 * Example: with my 6210 after using it info about incoming sms is shown much
 * earlier than sms in phone and phone restart (once and random) later after
 * connection. Seems phone firmware isn't too good here
 */
//#define GSM_ENABLE_N71_92INCOMINGINFO	/* Show incoming Data for 7110/9210 */

#define GSM_ENABLE_NOKIA6110		/* n6110.c models		*/
#define GSM_ENABLE_NOKIA6510		/* n6510.c (DCT4) models	*/
#define GSM_ENABLE_NOKIA7110		/* n7110.c models		*/
#define GSM_ENABLE_NOKIA9210		/* n9210.c models		*/
#define GSM_ENABLE_ATGEN		/* AT generic models		*/
//#define GSM_ENABLE_ALCATEL

/* Bu default commented: some parts of source are not ready or not tested
 * at all (or aren't good tested). Uncomment, if want them
 */
//#define DEVELOP

/* By default disabled. It allows to making short beep after such things
 * like changing logo in phone (like in Logo Manager)
 */
//#define GSM_ENABLE_BEEP

#endif
