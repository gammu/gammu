/* config.h.  Generated automatically by configure.  */

#ifndef _config_h_
#define _config_h_

/* -------------------------- Gammu specific ---------------------------- */

/* Version of package */
#define VERSION "0.73"

#ifndef _MSC_VER
/* Define if want DEBUG info */
/* #undef DEBUG */
#endif

/* ---------------------- Connections and general settings -------------- */

/* MBUS2 over cable */
#define GSM_ENABLE_MBUS2 1
/* FBUS2 over cable */
#define GSM_ENABLE_FBUS2 1
/* FBUS2 in DLR3 cable. AT init */
#define GSM_ENABLE_DLR3AT 1
/* Direct infrared */
#define GSM_ENABLE_IRDA 1
/* Infrared over irda sockets */
#define GSM_ENABLE_INFRARED 1
/* AT protocol */
#define GSM_ENABLE_AT 1
/* Alcatel protocol */
#define GSM_ENABLE_ALCABUS 1
/* Works only in Linux.
 * you need bluez-kernel-2.3-pre2 or greater, bluez-libs-2.1 
 * and bluez-utils-2.1 (not yet released) to have Bluetooth in OS
 * The kernel 2.4.20 will may include the RFCOMM implementation. 
 */
/* AT over bluetooth */
/* #undef GSM_ENABLE_ATBLUETOOTH */
/* FBUS2 over bluetooth	*/
/* #undef GSM_ENABLE_DLR3BLUETOOTH */

/* By default commented: some people reported me (for example, with 6210
 * and 3210), that their phones doesn't answer at all sometimes or answer
 * with (too) long delay. Possible reason: firmware in phone is too slow
 * I didn't have such problems personally with phone and PC with Celeron 800
 * Please also note, that CB function was always TO CHECK in MyGnokii/Gnokii
 * too.
 */
/* Enables cell broadcast */
/* #undef GSM_ENABLE_CELLBROADCAST */

/* If you want backup/restore stuff. Some people wanted it - they required
 * smalled library made from Gammu and didn't require it. Please note,
 * that backup/restore stuff can be easy separated from other things.
 */
#define GSM_ENABLE_BACKUP 1

/* Bu default commented: some parts of source are not ready or not tested
 * at all (or aren't good tested). Uncomment, if want them
 */
//#define DEVELOP

/* By default disabled. It allows to making short beep after such things
 * like changing logo in phone (like in Logo Manager)
 */
//#define GSM_ENABLE_BEEP

/* --------------------------- Phone modules (specific) ----------------- */

/* n6110.c models */
#define GSM_ENABLE_NOKIA6110 1

/* n7110.c models */
#define GSM_ENABLE_NOKIA7110 1

/* It allows to show incoming call and SMS info for 7110 family phones.
 * By default commented: you can have some problems, when using it.
 * Example: with my 6210 after using it info about incoming sms is shown much
 * earlier than sms in phone and phone restart (once and random) later after
 * connection. Seems phone firmware isn't too good here
 */
/* Show incoming Data for 7110/9210 */
/* #undef GSM_ENABLE_N71_92INCOMINGINFO */

/* n6510.c (DCT4) models */
#define GSM_ENABLE_NOKIA6510 1

/* Force using new frames for calendar. Can be not compatible with all phones,
 * but gives full calendar info
 */
/* #undef GSM_FORCE_DCT4_NEW_CALENDAR */

/* n9210.c models */
#define GSM_ENABLE_NOKIA9210 1
/* AT generic models */
#define GSM_ENABLE_ATGEN 1
/* Alcatel BE5 */
#define GSM_ENABLE_ALCATEL 1

/* ---------------------- C language specific --------------------------- */

#ifndef WIN32
/* are the scandir functions available */
#define HAVE_DIRENT_H 1
#define HAVE_SCANDIR 1
#define HAVE_ALPHASORT 1

#define HAVE_MYSQL_MYSQL_H 1
#endif

#define HAVE_ISWSPACE 1
#define HAVE_TOWLOWER 1

#endif
