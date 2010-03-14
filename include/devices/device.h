/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file device access code.

*/

#ifndef __device_h
#define __device_h

#include "misc.h"

/* Maximum length of device name for serial port */
#define GSM_MAX_DEVICE_NAME_LENGTH (100)

/* Type of connection. Now we support serial connection with FBUS cable and
   IR (only with 61x0 models) and MBUS (61x0, 640, 6160) */
typedef enum {
  GCT_FBUS,     /* FBUS connection. */
  GCT_Infrared, /* FBUS over direct infrared connection - 61xx. */
  GCT_Irda,     /* FBUS over Irda sockets - 7110, 6210, etc. */
  GCT_MBUS,     /* MBUS */
  GCT_DLR3,     /* FBUS over DLR3 cable */
  GCT_Tekram,   /* FBUS over direct infrared connection (Tekram device) - 61xx. */
  GCT_AT,       /* AT commands */
  GCT_FBUS3110  /* FBUS version 1 used in N3110 */
} GSM_ConnectionType;

#ifndef WIN32
  #include <unistd.h>
  #include "misc.h"

  int device_getfd(void);

  int device_open(__const char *__file, int __with_odd_parity);
  void device_reset(void);

  size_t device_read(__ptr_t __buf, size_t __nbytes);
  size_t device_write(__const __ptr_t __buf, size_t __n);

  #ifdef DEBUG
    void device_dumpserial(void);
  #endif
#else
  size_t device_write(const __ptr_t __buf, size_t __n);
#endif

  void device_close(void);

  void device_setdtrrts(int __dtr, int __rts);
  void device_changespeed(int __speed);


extern char PortDevice[GSM_MAX_DEVICE_NAME_LENGTH];

bool StartConnection (char *port_device, bool with_odd_parity, GSM_ConnectionType con);

#endif  /* __device_h */
