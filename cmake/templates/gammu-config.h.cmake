#ifndef _config_h_
#define _config_h_

/* -------------------------- Gammu specific ---------------------------- */

/* Version of package */
#define VERSION "@VERSION@"
#define VERSION_NUM @VERSION_NUM@

/* Define if want DEBUG info */
#cmakedefine DEBUG

/* ---------------------- Connections ---------------------------------- */

/* MBUS2 over cable */
#cmakedefine GSM_ENABLE_MBUS2

/* FBUS2 over cable */
#cmakedefine GSM_ENABLE_FBUS2
/* FBUS2 to use with DLR3 cable (init done using AT commands) */
#cmakedefine GSM_ENABLE_FBUS2DLR3
/* FBUS2 to use with cable with PL2303 chip (init done using AT commands) */
#cmakedefine GSM_ENABLE_FBUS2PL2303
/* FBUS2 to use with bluetooth legacy device (init done using AT commands) */
#cmakedefine GSM_ENABLE_FBUS2BLUE
/* FBUS2 to use with direct infrared device */
#cmakedefine GSM_ENABLE_FBUS2IRDA

/* PHONET to use with DKU2 cable */
#cmakedefine GSM_ENABLE_DKU2PHONET
/* AT commands to use with DKU2 cable */
#cmakedefine GSM_ENABLE_DKU2AT
/* FBUS2 to use with DKU5 cable (init done using AT commands) */
#cmakedefine GSM_ENABLE_DKU5FBUS2

/* PHONET FBUS to use with bluetooth legacy device */
#cmakedefine GSM_ENABLE_PHONETBLUE

/* AT protocol */
#cmakedefine GSM_ENABLE_AT

/* Alcatel protocol */
#cmakedefine GSM_ENABLE_ALCABUS

/* Infrared. PHONET FBUS */
#cmakedefine GSM_ENABLE_IRDAPHONET
/* Infrared. AT commands */
#cmakedefine GSM_ENABLE_IRDAAT
/* Infrared. OBEX */
#cmakedefine GSM_ENABLE_IRDAOBEX
/* Infrared */
#cmakedefine GSM_ENABLE_IRDAGNAPBUS

#cmakedefine GSM_ENABLE_BLUEGNAPBUS
/* Bluetooth stack (like Bluez). FBUS2 (init done using AT commands) */
#cmakedefine GSM_ENABLE_BLUEFBUS2
/* Bluetooth stack (like Bluez). PHONET FBUS */
#cmakedefine GSM_ENABLE_BLUEPHONET
/* Blueooth stack (like Bluez). AT commands */
#cmakedefine GSM_ENABLE_BLUEAT
/* Blueooth stack (like Bluez). OBEX */
#cmakedefine GSM_ENABLE_BLUEOBEX

/* --------------------------- Phone modules (specific) ----------------- */

/* n0650.c models */
#cmakedefine GSM_ENABLE_NOKIA650
/* n3320.c models */
#cmakedefine GSM_ENABLE_NOKIA3320
/* n6110.c models */
#cmakedefine GSM_ENABLE_NOKIA6110

/* n7110.c models */
#cmakedefine GSM_ENABLE_NOKIA7110
/* n9210.c models */
#cmakedefine GSM_ENABLE_NOKIA9210
/* It allows to show incoming call and SMS info for 7110 family phones.
 * By default commented: you can have some problems, when using it.
 * Example: with my 6210 after using it info about incoming sms is shown much
 * earlier than sms in phone and phone restart (once and random) later after
 * connection. Seems phone firmware isn't too good here
 */
/* Show incoming Data for 7110/9210 */
#cmakedefine GSM_ENABLE_N71_92INCOMINGINFO

/* n6510.c (DCT4) models */
#cmakedefine GSM_ENABLE_NOKIA6510
/* Force using 6210 frames for calendar. Required by some older DCT4 */
#cmakedefine GSM_FORCE_DCT4_CALENDAR_6210

/* n3650.c (DCT4 with Symbian) models */
#cmakedefine GSM_ENABLE_NOKIA3650

/* Global defines for enabling phone families */
#if defined(GSM_ENABLE_NOKIA3320) || defined(GSM_ENABLE_NOKIA650) || defined(GSM_ENABLE_NOKIA6110) || defined(GSM_ENABLE_NOKIA7110) || defined(GSM_ENABLE_NOKIA9210)
#  define GSM_ENABLE_NOKIA_DCT3
#endif
#if defined(GSM_ENABLE_NOKIA3650) || defined(GSM_ENABLE_NOKIA6510)
#  define GSM_ENABLE_NOKIA_DCT4
#endif

/* AT generic models */
#cmakedefine GSM_ENABLE_ATGEN

/* Alcatel BE5 */
#cmakedefine GSM_ENABLE_ALCATEL

/* AT with OBEX support */
#cmakedefine GSM_ENABLE_ATOBEX

/* OBEX generic models */
#cmakedefine GSM_ENABLE_OBEXGEN

/* Symbian with gnapplet models */
#cmakedefine GSM_ENABLE_GNAPGEN

/* ---------------------- General settings -------------------------------- */

/* By default commented: some people reported me (for example, with 6210
 * and 3210), that their phones doesn't answer at all sometimes or answer
 * with (too) long delay. Possible reason: firmware in phone is too slow
 * I didn't have such problems personally with phone and PC with Celeron 800
 * Please also note, that CB function was always TO CHECK in MyGnokii/Gnokii
 * too.
 */
/* Enables cell broadcast */
#cmakedefine GSM_ENABLE_CELLBROADCAST

/* If you want backup/restore stuff. Some people wanted it - they required
 * smalled library made from Gammu and didn't require it. Please note,
 * that backup/restore stuff can be easy separated from other things.
 */
#cmakedefine GSM_ENABLE_BACKUP

/* Bu default commented: some parts of source are not ready or not tested
 * at all (or aren't good tested). Uncomment, if you want them
 */
/*#define DEVELOP*/

/* By default disabled. It allows to make short beep after such things
 * like changing logo in phone (like in Logo Manager)
 */
/*#define GSM_ENABLE_BEEP*/

/* ---------------------- C language specific --------------------------- */

/* Do we have gettext support? */
#cmakedefine GETTEXTLIBS_FOUND

/* are the scandir functions available */
#ifndef HAVE_DIRENT_H
#cmakedefine HAVE_DIRENT_H
#endif
#ifndef HAVE_SCANDIR
#cmakedefine HAVE_SCANDIR
#endif
#ifndef HAVE_ALPHASORT
#cmakedefine HAVE_ALPHASORT
#endif

#ifndef HAVE_PTHREAD
#cmakedefine HAVE_PTHREAD
#endif
#ifndef HAVE_SYS_IOCTL_H
#cmakedefine HAVE_SYS_IOCTL_H
#endif
#ifndef HAVE_MYSQL_MYSQL_H
#cmakedefine HAVE_MYSQL_MYSQL_H
#endif
/* Do we have libdbi ? */
#cmakedefine LIBDBI_FOUND
#ifndef HAVE_POSTGRESQL_LIBPQ_FE_H
#cmakedefine HAVE_POSTGRESQL_LIBPQ_FE_H
#endif
#ifndef HAVE_PQESCAPESTRINGCONN
#cmakedefine HAVE_PQESCAPESTRINGCONN
#endif
#ifndef HAVE_SYS_UTSNAME_H
#cmakedefine HAVE_SYS_UTSNAME_H
#endif

#ifndef HAVE_ALARM
#cmakedefine HAVE_ALARM
#endif
#ifndef HAVE_GETPASS
#cmakedefine HAVE_GETPASS
#endif
#ifndef HAVE_GETPID
#cmakedefine HAVE_GETPID
#endif
#ifndef HAVE_SYSLOG
#cmakedefine HAVE_SYSLOG
#endif
#ifndef HAVE_GETOPT
#cmakedefine HAVE_GETOPT
#endif
#ifndef HAVE_GETOPT_LONG
#cmakedefine HAVE_GETOPT_LONG
#endif
#ifndef HAVE_DAEMON_UNISTD
#cmakedefine HAVE_DAEMON_UNISTD
#endif
#ifndef HAVE_DAEMON_STDLIB
#cmakedefine HAVE_DAEMON_STDLIB
#endif
#if defined(HAVE_DAEMON_STDLIB) || defined(HAVE_DAEMON_UNISTD)
#define HAVE_DAEMON
#endif
#ifndef HAVE_KILL
#cmakedefine HAVE_KILL
#endif
#ifndef HAVE_SIGHUP
#cmakedefine HAVE_SIGHUP
#endif

#ifndef HAVE_GETPWNAM
#cmakedefine HAVE_GETPWNAM
#endif
#ifndef HAVE_GETGRNAM
#cmakedefine HAVE_GETGRNAM
#endif
#ifndef HAVE_INITGROUPS
#cmakedefine HAVE_INITGROUPS
#endif
#ifndef HAVE_SETUID
#cmakedefine HAVE_SETUID
#endif
#ifndef HAVE_SETGID
#cmakedefine HAVE_SETGID
#endif

#ifndef HAVE_DUP_UNISTD_H
#cmakedefine HAVE_DUP_UNISTD_H
#endif
#ifndef HAVE_DUP_IO_H
#cmakedefine HAVE_DUP_IO_H
#endif

#ifndef HAVE_SHM
#cmakedefine HAVE_SHM
#endif

#ifndef HAVE_STRPTIME
#cmakedefine HAVE_STRPTIME
#endif

#ifndef HAVE_ISWSPACE
#cmakedefine HAVE_ISWSPACE
#endif
#ifndef HAVE_TOWLOWER
#cmakedefine HAVE_TOWLOWER
#endif
#ifndef HAVE_STRNCASECMP
#cmakedefine HAVE_STRNCASECMP 
#endif
#ifndef HAVE_STRCASECMP
#cmakedefine HAVE_STRCASECMP 
#endif
#ifndef HAVE_STRCASESTR
#cmakedefine HAVE_STRCASESTR 
#endif
#ifndef HAVE_STRCHRNUL
#cmakedefine HAVE_STRCHRNUL 
#endif
#ifndef HAVE_STRTOULL
#cmakedefine HAVE_STRTOULL
#endif
#ifndef HAVE_WCHAR_T
#cmakedefine HAVE_WCHAR_T
#endif
#ifndef HAVE_WINT_T
#cmakedefine HAVE_WINT_T
#endif
#ifndef HAVE_WCHAR_H
#cmakedefine HAVE_WCHAR_H
#endif
#ifndef HAVE_WCTYPE_H
#cmakedefine HAVE_WCTYPE_H
#endif

#ifndef HAVE_UNISTD_H
#cmakedefine HAVE_UNISTD_H
#endif
#ifndef HAVE_STRINGS_H
#cmakedefine HAVE_STRINGS_H
#endif

#ifndef HAVE_SSIZE_T
#cmakedefine HAVE_SSIZE_T
#endif
#ifndef HAVE_INTPTR_T
#cmakedefine HAVE_INTPTR_T
#endif

/**
 * __FUNCTION__ support
 */
#cmakedefine HAVE_MACRO_FUNCTION

/**
 * struct tm.tm_zone support.
 */
#ifndef HAVE_STRUCT_TM_TM_ZONE
#cmakedefine HAVE_STRUCT_TM_TM_ZONE
#endif

/**
 * __FUNC__ support
 */
#cmakedefine HAVE_MACRO_FUNC

/* Iconv support */
#cmakedefine ICONV_FOUND
#cmakedefine ICONV_SECOND_ARGUMENT_IS_CONST

/* CURL support */
#cmakedefine CURL_FOUND

/* Will be used Bluez stack ? */
#cmakedefine BLUEZ_FOUND

/* Do we have libusb-1.0 ? */
#cmakedefine LIBUSB_FOUND

/* Will be used FreeBSD Bluetooth stack ? */
#cmakedefine FBSD_BLUE_FOUND

/* Will be used OSX Bluetooth stack ? */
#cmakedefine OSX_BLUE_FOUND
/* 2.0 OSX Bluetooth API? */
#cmakedefine OSX_BLUE_2_0

/* Enable Bluetooth channnel searching? */
#cmakedefine BLUETOOTH_RF_SEARCHING

/* MS Visual C++ Express 2005 warnings */
#if _MSC_VER == 1400
#  pragma warning( disable : 4996 4244 4333)
#  ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#    define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES  1
#  endif
#endif

/* Path where locales will be installed */
#define LOCALE_PATH "@CMAKE_INSTALL_PREFIX@/@INSTALL_LOC_DIR@"

/* Most winapi crap can be used as well from Cygwin */
#if defined(WIN32) || defined(__CYGWIN__)
#define HAVE_WINDOWS_SERVICE
#define HAVE_WINDOWS_EVENT_LOG
#endif

#endif
