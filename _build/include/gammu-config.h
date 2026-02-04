#ifndef _gammu_config_h_
#define _gammu_config_h_

/* -------------------------- Gammu specific ---------------------------- */

/* Version of package */
#define GAMMU_VERSION "1.42.0"
#define GAMMU_VERSION_NUM 14200

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
/* #undef GSM_ENABLE_FBUS2BLUE */
/* FBUS2 to use with direct infrared device */
/* #undef GSM_ENABLE_FBUS2IRDA */

/* PHONET to use with DKU2 cable */
#define GSM_ENABLE_DKU2PHONET
/* AT commands to use with DKU2 cable */
#define GSM_ENABLE_DKU2AT
/* FBUS2 to use with DKU5 cable (init done using AT commands) */
#define GSM_ENABLE_DKU5FBUS2

/* PHONET FBUS to use with bluetooth legacy device */
/* #undef GSM_ENABLE_PHONETBLUE */

/* AT protocol */
#define GSM_ENABLE_AT

/* Alcatel protocol */
#define GSM_ENABLE_ALCABUS

/* Infrared. PHONET FBUS */
/* #undef GSM_ENABLE_IRDAPHONET */
/* Infrared. AT commands */
/* #undef GSM_ENABLE_IRDAAT */
/* Infrared. OBEX */
/* #undef GSM_ENABLE_IRDAOBEX */
/* Infrared */
/* #undef GSM_ENABLE_IRDAGNAPBUS */

/* #undef GSM_ENABLE_BLUEGNAPBUS */

#define GSM_ENABLE_S60
/* Bluetooth stack (like Bluez). FBUS2 (init done using AT commands) */
/* #undef GSM_ENABLE_BLUEFBUS2 */
/* Bluetooth stack (like Bluez). PHONET FBUS */
/* #undef GSM_ENABLE_BLUEPHONET */
/* Blueooth stack (like Bluez). AT commands */
/* #undef GSM_ENABLE_BLUEAT */
/* Blueooth stack (like Bluez). OBEX */
/* #undef GSM_ENABLE_BLUEOBEX */

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
/* It allows one to show incoming call and SMS info for 7110 family phones.
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

/* AT with OBEX support */
#define GSM_ENABLE_ATOBEX

/* Automatic switching to OBEX for AT+MODE capable phones */
/* #undef GSM_ENABLE_ATOBEX_AUTO_MODE */

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

/* ---------------------- C language specific --------------------------- */

/* Do we have gettext support? */
#define LIBINTL_LIB_FOUND

/* Does gettext support pgettext? */
/* #undef LIBINTL_HAS_PGETTEXT */

/* are the scandir functions available */
#ifndef HAVE_DIRENT_H
#define HAVE_DIRENT_H
#endif
#ifndef HAVE_SCANDIR
#define HAVE_SCANDIR
#endif
#ifndef HAVE_ALPHASORT
#define HAVE_ALPHASORT
#endif

#ifndef HAVE_PTHREAD
#define HAVE_PTHREAD
#endif
#ifndef HAVE_SIGTIMEDWAIT
#define HAVE_SIGTIMEDWAIT
#endif
#ifndef HAVE_SYS_IOCTL_H
#define HAVE_SYS_IOCTL_H
#endif
#ifndef HAVE_MYSQL_MYSQL_H
#define HAVE_MYSQL_MYSQL_H
#endif
/* ODBC support */
/* #undef ODBC_FOUND */
/* Do we have libdbi ? */
/* #undef LIBDBI_FOUND */
#ifndef HAVE_POSTGRESQL_LIBPQ_FE_H
#define HAVE_POSTGRESQL_LIBPQ_FE_H
#endif
#ifndef HAVE_PQESCAPESTRINGCONN
#define HAVE_PQESCAPESTRINGCONN
#endif
#ifndef HAVE_SYS_UTSNAME_H
#define HAVE_SYS_UTSNAME_H
#endif

#ifndef HAVE_ALARM
#define HAVE_ALARM
#endif
#ifndef HAVE_GETPASS
#define HAVE_GETPASS
#endif
#ifndef HAVE_GETPID
#define HAVE_GETPID
#endif
#ifndef HAVE_SYSLOG
#define HAVE_SYSLOG
#endif
#ifndef HAVE_GETOPT
#define HAVE_GETOPT
#endif
#ifndef HAVE_GETOPT_LONG
#define HAVE_GETOPT_LONG
#endif
#ifndef HAVE_DAEMON_UNISTD
#define HAVE_DAEMON_UNISTD
#endif
#ifndef HAVE_DAEMON_STDLIB
/* #undef HAVE_DAEMON_STDLIB */
#endif
#if defined(HAVE_DAEMON_STDLIB) || defined(HAVE_DAEMON_UNISTD)
#define HAVE_DAEMON
#endif
#ifndef HAVE_KILL
#define HAVE_KILL
#endif
#ifndef HAVE_SIGHUP
#define HAVE_SIGHUP
#endif
#ifndef HAVE_SIGUSR1
#define HAVE_SIGUSR1
#endif
#ifndef HAVE_SIGUSR2
#define HAVE_SIGUSR2
#endif

#ifndef HAVE_GETPWNAM
#define HAVE_GETPWNAM
#endif
#ifndef HAVE_GETGRNAM
#define HAVE_GETGRNAM
#endif
#ifndef HAVE_INITGROUPS
#define HAVE_INITGROUPS
#endif
#ifndef HAVE_SETUID
#define HAVE_SETUID
#endif
#ifndef HAVE_SETGID
#define HAVE_SETGID
#endif

#ifndef HAVE_DUP_UNISTD_H
#define HAVE_DUP_UNISTD_H
#endif
#ifndef HAVE_DUP_IO_H
/* #undef HAVE_DUP_IO_H */
#endif

#ifndef HAVE_SHM
#define HAVE_SHM
#endif

#ifndef HAVE_STRPTIME
#define HAVE_STRPTIME
#endif

#ifndef HAVE_ISWSPACE
#define HAVE_ISWSPACE
#endif
#ifndef HAVE_TOWLOWER
#define HAVE_TOWLOWER
#endif
#ifndef HAVE_STRNCASECMP
#define HAVE_STRNCASECMP
#endif
#ifndef HAVE_STRCASECMP
#define HAVE_STRCASECMP
#endif
#ifndef HAVE_STRNICMP
/* #undef HAVE_STRNICMP */
#endif
#ifndef HAVE_STRICMP
/* #undef HAVE_STRICMP */
#endif
#ifndef HAVE_STRCASESTR
#define HAVE_STRCASESTR
#endif
#ifndef HAVE_STRCHRNUL
#define HAVE_STRCHRNUL
#endif
#ifndef HAVE_STRTOULL
#define HAVE_STRTOULL
#endif
/* Size of wchar_t needs to be 4 bytes to store supplementary plan unicode */
#if 4 >= 4
#define USE_WCHAR_T
#endif
#ifndef HAVE_WCHAR_T
#define HAVE_WCHAR_T
#endif
#ifndef HAVE_WINT_T
#define HAVE_WINT_T
#endif
#ifndef HAVE_WCHAR_H
#define HAVE_WCHAR_H
#endif
#ifndef HAVE_WCTYPE_H
#define HAVE_WCTYPE_H
#endif

#ifndef HAVE_UNISTD_H
#define HAVE_UNISTD_H
#endif
#ifndef HAVE_STRINGS_H
#define HAVE_STRINGS_H
#endif
#ifndef HAVE_STDINT_H
#define HAVE_STDINT_H
#endif

#ifndef HAVE_SSIZE_T
#define HAVE_SSIZE_T
#endif
#ifndef HAVE_INTPTR_T
#define HAVE_INTPTR_T
#endif
#ifndef HAVE_GETPWUID
#define HAVE_GETPWUID
#endif
#ifndef HAVE_GETUID
#define HAVE_GETUID
#endif
#ifndef HAVE_I_SETSIG
/* #undef HAVE_I_SETSIG */
#endif

/**
 * __FUNCTION__ support
 */
#define HAVE_MACRO_FUNCTION

/**
 * struct tm.tm_zone support.
 */
#ifndef HAVE_STRUCT_TM_TM_ZONE
#define HAVE_STRUCT_TM_TM_ZONE
#endif

/**
 * daylight variable
 */
#ifndef HAVE_DAYLIGHT
#define HAVE_DAYLIGHT
#endif

/**
 * __FUNC__ support
 */
/* #undef HAVE_MACRO_FUNC */

/* Iconv support */
#define ICONV_FOUND
/* #undef ICONV_SECOND_ARGUMENT_IS_CONST */

/* CURL support */
/* #undef CURL_FOUND */

/* Will be used Bluez stack ? */
/* #undef BLUEZ_FOUND */

/* Do we have libusb-1.0 ? */
/* #undef LIBUSB_FOUND */

/* Will be used BSD Bluetooth stack ? */
/* #undef BSD_BLUE_FOUND */

/* Will be used OSX Bluetooth stack ? */
/* #undef OSX_BLUE_FOUND */
/* 2.0 OSX Bluetooth API? */
/* #undef OSX_BLUE_2_0 */

/* Enable Bluetooth channnel searching? */
/* #undef BLUETOOTH_RF_SEARCHING */

/* Enable Glib */
/* #undef Glib_FOUND */

/* spawnv argument type */
/* #undef SPAWNV_ARGUMENT_IS_CONST */

/* Path where locales will be installed */
#define LOCALE_PATH "/usr/local/share/locale"

/* Path where locales will be installed */
#define GAMMU_DATA_PATH "/usr/local/share/gammu"

/* Most winapi crap can be used as well from Cygwin */
#if defined(WIN32) || defined(__CYGWIN__)
#define HAVE_WINDOWS_SERVICE
#define HAVE_WINDOWS_EVENT_LOG
#endif

#if !defined(WIN32) && defined(HAVE_PTHREAD) && defined(HAVE_SIGTIMEDWAIT)
#define GSM_ENABLE_PROXY
#endif

#endif
