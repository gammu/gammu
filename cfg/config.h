/* config.h.  Generated from config.h.in by configure.  */

#ifndef _config_h_
#define _config_h_

/* -------------------------- Gammu specific ---------------------------- */

/* Version of package */
#define VERSION "1.09.16"
#define VERSION_WIN "1,09,16,0"

#ifndef _MSC_VER
/* Define if want DEBUG info */
#define DEBUG 1
#endif

/* #undef ENABLE_LGPL */

/* ---------------------- Connections ---------------------------------- */

/* MBUS2 over cable */
#define GSM_ENABLE_MBUS2 1

/* FBUS2 over cable */
#define GSM_ENABLE_FBUS2 1
/* FBUS2 to use with DLR3 cable (init done using AT commands) */
#define GSM_ENABLE_FBUS2DLR3 1
/* FBUS2 to use with cable with PL2303 chip (init done using AT commands) */
#define GSM_ENABLE_FBUS2PL2303 1
/* FBUS2 to use with bluetooth legacy device (init done using AT commands) */
#define GSM_ENABLE_FBUS2BLUE 1
/* FBUS2 to use with direct infrared device */
#define GSM_ENABLE_FBUS2IRDA 1

/* PHONET to use with DKU2 cable */
#define GSM_ENABLE_DKU2PHONET 1
/* AT commands to use with DKU2 cable */
#define GSM_ENABLE_DKU2AT 1
/* FBUS2 to use with DKU5 cable (init done using AT commands) */
#define GSM_ENABLE_DKU5FBUS2 1

/* PHONET FBUS to use with bluetooth legacy device */
#define GSM_ENABLE_PHONETBLUE 1

/* AT protocol */
#define GSM_ENABLE_AT 1

/* Alcatel protocol */
#define GSM_ENABLE_ALCABUS 1

/* Infrared. PHONET FBUS */
#define GSM_ENABLE_IRDAPHONET 1
/* Infrared. AT commands */
#define GSM_ENABLE_IRDAAT 1
/* Infrared. OBEX */
#define GSM_ENABLE_IRDAOBEX 1
/* Infrared */
#define GSM_ENABLE_IRDAGNAPBUS 1

#ifdef WIN32
#  define GSM_ENABLE_BLUEFBUS2 1
#  define GSM_ENABLE_BLUEPHONET 1
#  define GSM_ENABLE_BLUEAT 1
#  define GSM_ENABLE_BLUEOBEX 1
#  define GSM_ENABLE_BLUEGNAPBUS 1
#else
#  define GSM_ENABLE_BLUEGNAPBUS 1
/* Bluetooth stack (like Bluez). FBUS2 (init done using AT commands) */
#  define GSM_ENABLE_BLUEFBUS2 1
/* Bluetooth stack (like Bluez). PHONET FBUS */
#  define GSM_ENABLE_BLUEPHONET 1
/* Blueooth stack (like Bluez). AT commands */
#  define GSM_ENABLE_BLUEAT 1
/* Blueooth stack (like Bluez). OBEX */
#  define GSM_ENABLE_BLUEOBEX 1
#endif

/* --------------------------- Phone modules (specific) ----------------- */

/* n0650.c models */
#define GSM_ENABLE_NOKIA650 1
/* n3320.c models */
#define GSM_ENABLE_NOKIA3320 1
/* n6110.c models */
#define GSM_ENABLE_NOKIA6110 1

/* n7110.c models */
#define GSM_ENABLE_NOKIA7110 1
/* n9210.c models */
#define GSM_ENABLE_NOKIA9210 1
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
/* Force using 6210 frames for calendar. Required by some older DCT4 */
/* #undef GSM_FORCE_DCT4_CALENDAR_6210 */

/* n3650.c (DCT4 with Symbian) models */
#define GSM_ENABLE_NOKIA3650 1

/* AT generic models */
#define GSM_ENABLE_ATGEN 1

/* Alcatel BE5 */
#define GSM_ENABLE_ALCATEL 1

/* Sony-Ericsson support */
#define GSM_ENABLE_SONYERICSSON 1

/* OBEX generic models */
#define GSM_ENABLE_OBEXGEN 1

/* Symbian with gnapplet models */
#define GSM_ENABLE_GNAPGEN 1

/* ---------------------- General settings -------------------------------- */

/* By default commented: some people reported me (for example, with 6210
 * and 3210), that their phones doesn't answer at all sometimes or answer
 * with (too) long delay. Possible reason: firmware in phone is too slow
 * I didn't have such problems personally with phone and PC with Celeron 800
 * Please also note, that CB function was always TO CHECK in MyGnokii/Gnokii
 * too.
 */
/* Enables cell broadcast */
#define GSM_ENABLE_CELLBROADCAST 1

/* If you want backup/restore stuff. Some people wanted it - they required
 * smalled library made from Gammu and didn't require it. Please note,
 * that backup/restore stuff can be easy separated from other things.
 */
#define GSM_ENABLE_BACKUP 1

/* Bu default commented: some parts of source are not ready or not tested
 * at all (or aren't good tested). Uncomment, if you want them
 */
//#define DEVELOP

/* By default disabled. It allows to make short beep after such things
 * like changing logo in phone (like in Logo Manager)
 */
//#define GSM_ENABLE_BEEP

/* ---------------------- C language specific --------------------------- */

#if !defined(WIN32) && !defined(DJGPP)
/* are the scandir functions available */
#  define HAVE_DIRENT_H 1
#  define HAVE_SCANDIR 1
#  define HAVE_ALPHASORT 1

#  define HAVE_PTHREAD 1
#  define HAVE_SYS_IOCTL_H 1
#  define HAVE_MYSQL_MYSQL_H 1
#else
//#  define HAVE_MYSQL_MYSQL_H 1
#endif

#ifndef DJGPP
#  define HAVE_ISWSPACE 1
#  define HAVE_TOWLOWER 1
#endif

#if !defined(WIN32) && !defined(DJGPP)
/* Will be used Bluez BT stack ? */
#  define GSM_ENABLE_BLUEZ 1
/* Will be used Affix BT stack ? */
/* #  undef GSM_ENABLE_AFFIX */
#endif

/* Uncomment, if you want searching RF channel during each BT connection */
/* It can make connecting longer or connection less stable */
/* With MS VC and MS BT stack required irprops.lib from MS Platform SDK to compile */
#ifdef WIN32
//#  define BLUETOOTH_RF_SEARCHING 1
#else
#  define BLUETOOTH_RF_SEARCHING 1
#endif

//MS Visual C++ Express 2005 warnings
#if _MSC_VER == 1400
#  pragma warning( disable : 4996 4244 4333)
#  ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#    define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES  1
#  endif
#endif

#endif
