#ifndef _config_h_
#define _config_h_

/* Configuration for Windows compilers */

/* -------------------------- Gammu specific ---------------------------- */

/* Version of package */
#define VERSION "1.12.93"
#define VERSION_WIN "1,12,93,0"

/* Define if want DEBUG info */
/* #undef DEBUG */

/* ---------------------- Connections ---------------------------------- */

/* MBUS2 over cable */
#define GSM_ENABLE_MBUS2

/* FBUS2 over cable */
#define GSM_ENABLE_FBUS2
/* FBUS2 to use with DLR3 cable (init done using AT commands) */
#define GSM_ENABLE_FBUS2DLR3
/* FBUS2 to use with cable with PL2303 chip (init done using AT commands) */
#define GSM_ENABLE_FBUS2PL2303
/* FBUS2 to use with bluetooth legacy device (init done using AT commands) */
#define GSM_ENABLE_FBUS2BLUE
/* FBUS2 to use with direct infrared device */
#define GSM_ENABLE_FBUS2IRDA

/* PHONET to use with DKU2 cable */
#define GSM_ENABLE_DKU2PHONET
/* AT commands to use with DKU2 cable */
#define GSM_ENABLE_DKU2AT
/* FBUS2 to use with DKU5 cable (init done using AT commands) */
#define GSM_ENABLE_DKU5FBUS2

/* PHONET FBUS to use with bluetooth legacy device */
#define GSM_ENABLE_PHONETBLUE

/* AT protocol */
#define GSM_ENABLE_AT

/* Alcatel protocol */
#define GSM_ENABLE_ALCABUS

/* Infrared. PHONET FBUS */
#define GSM_ENABLE_IRDAPHONET
/* Infrared. AT commands */
#define GSM_ENABLE_IRDAAT
/* Infrared. OBEX */
#define GSM_ENABLE_IRDAOBEX
/* Infrared */
#define GSM_ENABLE_IRDAGNAPBUS

#define GSM_ENABLE_BLUEGNAPBUS
/* Bluetooth stack (like Bluez). FBUS2 (init done using AT commands) */
#define GSM_ENABLE_BLUEFBUS2
/* Bluetooth stack (like Bluez). PHONET FBUS */
#define GSM_ENABLE_BLUEPHONET
/* Blueooth stack (like Bluez). AT commands */
#define GSM_ENABLE_BLUEAT
/* Blueooth stack (like Bluez). OBEX */
#define GSM_ENABLE_BLUEOBEX

/* --------------------------- Phone modules (specific) ----------------- */

/* n0650.c models */
#define GSM_ENABLE_NOKIA650
/* n3320.c models */
#define GSM_ENABLE_NOKIA3320
/* n6110.c models */
#define GSM_ENABLE_NOKIA6110

/* n7110.c models */
#define GSM_ENABLE_NOKIA7110
/* n9210.c models */
#define GSM_ENABLE_NOKIA9210
/* It allows to show incoming call and SMS info for 7110 family phones.
 * By default commented: you can have some problems, when using it.
 * Example: with my 6210 after using it info about incoming sms is shown much
 * earlier than sms in phone and phone restart (once and random) later after
 * connection. Seems phone firmware isn't too good here
 */
/* Show incoming Data for 7110/9210 */
/* #undef GSM_ENABLE_N71_92INCOMINGINFO */

/* n6510.c (DCT4) models */
#define GSM_ENABLE_NOKIA6510
/* Force using 6210 frames for calendar. Required by some older DCT4 */
/* #undef GSM_FORCE_DCT4_CALENDAR_6210 */

/* n3650.c (DCT4 with Symbian) models */
#define GSM_ENABLE_NOKIA3650

/* Global defines for enabling phone families */
#if defined(GSM_ENABLE_NOKIA3320) || defined(GSM_ENABLE_NOKIA650) || defined(GSM_ENABLE_NOKIA6110) || defined(GSM_ENABLE_NOKIA7110) || defined(GSM_ENABLE_NOKIA9210)
#  define GSM_ENABLE_NOKIA_DCT3
#endif
#if defined(GSM_ENABLE_NOKIA3650) || defined(GSM_ENABLE_NOKIA6510)
#  define GSM_ENABLE_NOKIA_DCT4
#endif

/* AT generic models */
#define GSM_ENABLE_ATGEN

/* Alcatel BE5 */
#define GSM_ENABLE_ALCATEL

/* Sony-Ericsson support */
#define GSM_ENABLE_SONYERICSSON

/* OBEX generic models */
#define GSM_ENABLE_OBEXGEN

/* Symbian with gnapplet models */
#define GSM_ENABLE_GNAPGEN

/* ---------------------- General settings -------------------------------- */

/* By default commented: some people reported me (for example, with 6210
 * and 3210), that their phones doesn't answer at all sometimes or answer
 * with (too) long delay. Possible reason: firmware in phone is too slow
 * I didn't have such problems personally with phone and PC with Celeron 800
 * Please also note, that CB function was always TO CHECK in MyGnokii/Gnokii
 * too.
 */
/* Enables cell broadcast */
#define GSM_ENABLE_CELLBROADCAST

/* If you want backup/restore stuff. Some people wanted it - they required
 * smalled library made from Gammu and didn't require it. Please note,
 * that backup/restore stuff can be easy separated from other things.
 */
#define GSM_ENABLE_BACKUP

/* Bu default commented: some parts of source are not ready or not tested
 * at all (or aren't good tested). Uncomment, if you want them
 */
//#define DEVELOP

/* By default disabled. It allows to make short beep after such things
 * like changing logo in phone (like in Logo Manager)
 */
//#define GSM_ENABLE_BEEP

/* ---------------------- C language specific --------------------------- */

/* Do we have gettext support? */
#define GETTEXTLIBS_FOUND

/* are the scandir functions available */
#ifndef HAVE_DIRENT_H
/* #undef HAVE_DIRENT_H */
#endif
#ifndef HAVE_SCANDIR
/* #undef HAVE_SCANDIR */
#endif
#ifndef HAVE_ALPHASORT
/* #undef HAVE_ALPHASORT */
#endif

#ifndef HAVE_PTHREAD
/* #undef HAVE_PTHREAD */
#endif
#ifndef HAVE_SYS_IOCTL_H
/* #undef HAVE_SYS_IOCTL_H */
#endif
#ifndef HAVE_MYSQL_MYSQL_H
/* #undef HAVE_MYSQL_MYSQL_H */
#endif
#ifndef HAVE_POSTGRESQL_LIBPQ_FE_H
/* #undef HAVE_POSTGRESQL_LIBPQ_FE_H */
#endif
#ifndef HAVE_SYS_UTSNAME_H
/* #undef HAVE_SYS_UTSNAME_H */
#endif

#ifndef HAVE_ISWSPACE
#define HAVE_ISWSPACE
#endif
#ifndef HAVE_TOWLOWER
#define HAVE_TOWLOWER
#endif
#ifndef HAVE_STRNCASECMP
/* #undef HAVE_STRNCASECMP  */
#endif
#ifndef HAVE_STRCASECMP
/* #undef HAVE_STRCASECMP  */
#endif
#ifndef HAVE_STRCASESTR
/* #undef HAVE_STRCASESTR  */
#endif
#ifndef HAVE_STRCHRNUL
/* #undef HAVE_STRCHRNUL  */
#endif

/* Will be used Bluez BT stack ? */
/* #undef GSM_ENABLE_BLUEZ */
/* Will be used Affix BT stack ? */
/* #undef GSM_ENABLE_AFFIX */

#define BLUETOOTH_RF_SEARCHING

//MS Visual C++ Express 2005 warnings
#if _MSC_VER == 1400
#  pragma warning( disable : 4996 4244 4333)
#  ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#    define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES  1
#  endif
#endif

/* Path where locales will be installed */
#define LOCALE_PATH "/usr/local/share/gammu"

#endif
